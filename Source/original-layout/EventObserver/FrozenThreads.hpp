#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <array>
#include <algorithm>
#include <cstdint>
namespace cfgobserver {
class FrozenThreads {
public:
    static constexpr std::size_t Capacity=256;
    FrozenThreads(HANDLE process,DWORD pid,std::uintptr_t target) noexcept : process_(process),pid_(pid),target_(target) {}
    FrozenThreads(const FrozenThreads&)=delete;
    FrozenThreads& operator=(const FrozenThreads&)=delete;
    ~FrozenThreads() { if (!finished_) Resume(); }
    bool Freeze() noexcept {
        start_=GetTickCount64();
        if (begun_ || pid_==GetCurrentProcessId() || GetProcessId(process_)!=pid_) return false;
        begun_=true;
        if (!Ids(before_,count_)) return false;
        for (std::size_t i=0;i<count_;++i) {
            auto& t=records_[opened_++];
            t.handle=OpenThread(THREAD_SUSPEND_RESUME|THREAD_GET_CONTEXT|THREAD_QUERY_LIMITED_INFORMATION,FALSE,before_[i]);
            if (!t.handle || GetProcessIdOfThread(t.handle)!=pid_) return false;
            t.previous=SuspendThread(t.handle);
            if (t.previous==static_cast<DWORD>(-1)) return false;
            t.held=true;
            if (t.previous!=0 || !WithinDeadline()) return false;
        }
        return true;
    }
    bool SpanClear() noexcept {
        if (!begun_ || finished_ || opened_!=count_) return false;
        for (std::size_t i=0;i<opened_;++i) {
            CONTEXT c{}; c.ContextFlags=CONTEXT_CONTROL;
            if (!records_[i].held || !GetThreadContext(records_[i].handle,&c) ||
                (c.Rip>=target_ && c.Rip-target_<5) || !WithinDeadline()) return false;
        }
        return true;
    }
    bool ThreadSetStable() noexcept {
        std::array<DWORD,Capacity> after{}; std::size_t count=0;
        return begun_ && !finished_ && Ids(after,count) && count==count_ &&
            std::equal(before_.begin(),before_.begin()+count_,after.begin());
    }
    bool WithinDeadline() const noexcept { return begun_ && GetTickCount64()-start_<=1000; }
    bool Resume() noexcept {
        if (finished_) return resumed_; // Never attempt a second resume.
        finished_=true; resumed_=true;
        for (std::size_t i=0;i<opened_;++i) {
            auto& t=records_[i];
            if (t.held) {
                const auto previous=ResumeThread(t.handle); t.held=false;
                if (previous!=t.previous+1) resumed_=false;
            }
            if (t.handle) { CloseHandle(t.handle); t.handle=nullptr; }
        }
        return resumed_;
    }
    std::size_t Count() const noexcept { return count_; }
    ULONGLONG Elapsed() const noexcept { return begun_?GetTickCount64()-start_:0; }
private:
    bool Ids(std::array<DWORD,Capacity>& ids,std::size_t& count) noexcept {
        count=0; HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
        if (snapshot==INVALID_HANDLE_VALUE) return false;
        THREADENTRY32 entry{}; entry.dwSize=sizeof(entry);
        bool okay=Thread32First(snapshot,&entry)!=FALSE;
        if (okay) {
            do {
                if (entry.th32OwnerProcessID==pid_) {
                    if (count==ids.size()) { okay=false; break; }
                    ids[count++]=entry.th32ThreadID;
                }
                entry.dwSize=sizeof(entry);
            } while (Thread32Next(snapshot,&entry));
            if (okay && GetLastError()!=ERROR_NO_MORE_FILES) okay=false;
        }
        CloseHandle(snapshot); std::sort(ids.begin(),ids.begin()+count); return okay && count>0;
    }
    struct Record { HANDLE handle=nullptr; DWORD previous=0; bool held=false; };
    HANDLE process_; DWORD pid_; std::uintptr_t target_; ULONGLONG start_=0;
    std::array<DWORD,Capacity> before_{};
    std::array<Record,Capacity> records_{};
    std::size_t count_=0,opened_=0;
    bool begun_=false,finished_=false,resumed_=true;
};
} // namespace cfgobserver
