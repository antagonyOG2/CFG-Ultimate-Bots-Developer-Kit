#pragma once
#include <atomic>
#include <cstdint>
namespace cfginput {
constexpr std::uint32_t CounselorInputMagic=0x43464933,CounselorInputVersion=1;
struct CounselorInputSnapshot {
    std::uint32_t magic=CounselorInputMagic,version=CounselorInputVersion,bytes=sizeof(CounselorInputSnapshot),reserved=0;
    std::uint64_t sequence=0,at=0,keyMessages=0,buttonRequests=0;
    std::uintptr_t window=0;
};
static_assert(sizeof(CounselorInputSnapshot)==56,"x64 input observation ABI");
// Plain input observations only: no UObject pointers, native calls, spawning,
// retained scopes or game state may be accessed from a window/render thread.
class CounselorInputMailbox {
    std::atomic<std::uint64_t> sequence_{0},at_{0},messages_{0},buttons_{0};
    std::atomic<bool> down_{false};
    void Publish(std::uint64_t now)noexcept{at_.store(now,std::memory_order_relaxed);sequence_.fetch_add(1,std::memory_order_release);}
public:
    void Key(bool down,bool repeat,bool foreground,bool menu,std::uint64_t now)noexcept{
        messages_.fetch_add(1,std::memory_order_relaxed);
        if(!down){down_.store(false,std::memory_order_release);return;}
        const bool prior=down_.exchange(true,std::memory_order_acq_rel);
        if(!prior&&!repeat&&foreground&&!menu)Publish(now);
    }
    void FocusLost()noexcept{down_.store(false,std::memory_order_release);}
    void Button(bool foreground,std::uint64_t now)noexcept{
        buttons_.fetch_add(1,std::memory_order_relaxed);if(foreground)Publish(now);
    }
    bool Snapshot(CounselorInputSnapshot& out,std::uintptr_t window)const noexcept{
        out={};if(!window)return false;
        const auto before=sequence_.load(std::memory_order_acquire);CounselorInputSnapshot value;
        value.sequence=before;value.at=at_.load(std::memory_order_relaxed);value.window=window;
        value.keyMessages=messages_.load(std::memory_order_relaxed);value.buttonRequests=buttons_.load(std::memory_order_relaxed);
        if(sequence_.load(std::memory_order_acquire)!=before)return false;out=value;return true;
    }
};
class CounselorInputConsumer {
    std::uint64_t seen_=0;bool initialized_=false;
public:
    static constexpr std::uint64_t LifetimeMs=15000;
    bool Poll(const CounselorInputSnapshot& input,std::uint64_t now,bool foreground)noexcept{
        if(input.magic!=CounselorInputMagic||input.version!=CounselorInputVersion||input.bytes!=sizeof(input)||input.reserved||!input.window)return false;
        if(!initialized_||input.sequence<seen_){seen_=input.sequence;initialized_=true;return false;}
        if(input.sequence==seen_)return false;seen_=input.sequence;
        return foreground&&input.at<=now&&now-input.at<=LifetimeMs;
    }
};
}
