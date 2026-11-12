#pragma once
#include <array>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <windows.h>

namespace cfgownednative {
// Resident, bounded records for independently audited code-gate overlaps only.
// Publication happens on the serialized hook-install thread, before enabling.
// Readers never mutate records or adopt an unregistered/foreign detour.
constexpr unsigned Capacity=8,MaximumCode=1024;
struct Entry {
    std::uintptr_t image=0,target=0,trampoline=0,detour=0;
    unsigned bytes=0,copied=0;
    std::array<unsigned char,MaximumCode> expected{},installed{};
    std::atomic<bool> enabled{false};
};
inline std::array<unsigned char,14> Jump(std::uintptr_t destination)noexcept{
    std::array<unsigned char,14> out{{0xFF,0x25,0,0,0,0}};
    std::memcpy(out.data()+6,&destination,8);return out;
}
class Registry {
    std::array<Entry,Capacity> entries_{};std::atomic<unsigned> count_{0};
    template<class R>static bool Continuation(R& r,const Entry& e)noexcept{
        // Only simple, already audited unrelocated prologues are supported.
        // MinHook appends FF25->original+copied, then FF25->our detour.
        std::array<unsigned char,60> actual{};
        const auto size=e.copied+28;
        return r.Executable(e.trampoline,size)&&r.Executable(e.detour,1)&&
            r.ReadBytes(e.trampoline,actual.data(),size)&&
            !std::memcmp(actual.data(),e.expected.data(),e.copied)&&
            !std::memcmp(actual.data()+e.copied,Jump(e.target+e.copied).data(),14)&&
            !std::memcmp(actual.data()+e.copied+14,Jump(e.detour).data(),14);
    }
public:
    template<class R,std::size_t N>bool Prepare(R& r,std::uintptr_t image,std::uintptr_t target,
        const std::array<unsigned char,N>& expected,std::uintptr_t trampoline,std::uintptr_t detour,unsigned copied)noexcept{
        static_assert(N<=MaximumCode);
        const auto count=count_.load(std::memory_order_acquire);
        if(count>=Capacity||N<5||copied<5||copied>32||copied>N||!image||target<image||
           target>UINTPTR_MAX-N||!trampoline||trampoline>UINTPTR_MAX-60||!detour)return false;
        for(unsigned i=0;i<count;++i)if(entries_[i].target==target)return false;
        std::array<unsigned char,N> current{};
        if(!r.Image(target,image,N)||!r.ReadBytes(target,current.data(),N)||current!=expected)return false;
        auto& e=entries_[count];e.image=image;e.target=target;e.trampoline=trampoline;e.detour=detour;e.bytes=static_cast<unsigned>(N);e.copied=copied;
        std::memcpy(e.expected.data(),expected.data(),N);std::memcpy(e.installed.data(),expected.data(),N);
        if(!Continuation(r,e))return false;
        const auto relay=trampoline+copied+14;
        const auto displacement=static_cast<std::int64_t>(relay)-static_cast<std::int64_t>(target+5);
        if(displacement<INT32_MIN||displacement>INT32_MAX)return false;
        const auto relative=static_cast<std::int32_t>(displacement);e.installed[0]=0xE9;
        std::memcpy(e.installed.data()+1,&relative,4);
        count_.store(count+1,std::memory_order_release);return true;
    }
    template<class R>bool Enable(R& r,std::uintptr_t target)noexcept{
        const auto count=count_.load(std::memory_order_acquire);
        for(unsigned i=0;i<count;++i){auto& e=entries_[i];if(e.target!=target)continue;
            std::array<unsigned char,MaximumCode> code{};
            if(!r.Image(target,e.image,e.bytes)||!r.ReadBytes(target,code.data(),e.bytes)||
               std::memcmp(code.data(),e.installed.data(),e.bytes)||!Continuation(r,e))return false;
            e.enabled.store(true,std::memory_order_release);return true;
        }return false;
    }
    void Disable(std::uintptr_t target)noexcept{
        const auto count=count_.load(std::memory_order_acquire);
        for(unsigned i=0;i<count;++i)if(entries_[i].target==target)entries_[i].enabled.store(false,std::memory_order_release);
    }
    template<class R,std::size_t N>std::uintptr_t Resolve(R& r,std::uintptr_t image,std::uintptr_t target,
        const std::array<unsigned char,N>& expected)const noexcept{
        static_assert(N<=MaximumCode);
        std::array<unsigned char,N> current{};
        if(!r.Image(target,image,N)||!r.ReadBytes(target,current.data(),N))return 0;
        if(current==expected)return target;
        const auto count=count_.load(std::memory_order_acquire);
        for(unsigned i=0;i<count;++i){const auto& e=entries_[i];
            if(e.target!=target||e.image!=image||e.bytes!=N||!e.enabled.load(std::memory_order_acquire)||
               std::memcmp(e.expected.data(),expected.data(),N))continue;
            return !std::memcmp(current.data(),e.installed.data(),N)&&Continuation(r,e)?e.trampoline:0;
        }return 0;
    }
};
struct CurrentProcess {
    bool ReadBytes(std::uintptr_t at,void* out,std::size_t bytes)noexcept{
        SIZE_T done=0;return at&&bytes&&at<=UINTPTR_MAX-bytes&&
            ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<const void*>(at),out,bytes,&done)&&done==bytes;
    }
    bool Image(std::uintptr_t at,std::uintptr_t image,std::size_t bytes)noexcept{
        MEMORY_BASIC_INFORMATION m{};
        if(!at||!image||!bytes||at>UINTPTR_MAX-bytes||!VirtualQuery(reinterpret_cast<const void*>(at),&m,sizeof(m))||
           m.State!=MEM_COMMIT||m.Type!=MEM_IMAGE||reinterpret_cast<std::uintptr_t>(m.AllocationBase)!=image||
           (m.Protect!=PAGE_EXECUTE_READ&&m.Protect!=PAGE_EXECUTE_WRITECOPY))return false;
        const auto base=reinterpret_cast<std::uintptr_t>(m.BaseAddress);
        return at>=base&&at-base<=m.RegionSize&&bytes<=m.RegionSize-(at-base);
    }
    bool Executable(std::uintptr_t at,std::size_t bytes)noexcept{
        MEMORY_BASIC_INFORMATION m{};
        if(!at||!bytes||at>UINTPTR_MAX-bytes||!VirtualQuery(reinterpret_cast<const void*>(at),&m,sizeof(m))||m.State!=MEM_COMMIT||
           (m.Protect&(PAGE_GUARD|PAGE_NOACCESS))||!(m.Protect&(PAGE_EXECUTE|PAGE_EXECUTE_READ|PAGE_EXECUTE_READWRITE|PAGE_EXECUTE_WRITECOPY)))return false;
        const auto base=reinterpret_cast<std::uintptr_t>(m.BaseAddress);
        return at>=base&&at-base<=m.RegionSize&&bytes<=m.RegionSize-(at-base);
    }
};
inline Registry resident;
template<std::size_t N>std::uintptr_t Resolve(std::uintptr_t image,std::uintptr_t target,const std::array<unsigned char,N>& expected)noexcept{
    CurrentProcess r;return resident.Resolve(r,image,target,expected);
}
}
