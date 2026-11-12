#pragma once
#include <windows.h>
#include <array>
#include <cstdint>
#include <cstring>
#include <limits>
extern "C" {
#include "../SourceBaseline/gameplay-backend/port/vendor/minhook/src/buffer.h"
#include "../SourceBaseline/gameplay-backend/port/vendor/minhook/src/trampoline.h"
#ifdef CFG_FRONTEND_BUNDLE
LPVOID CfgObserverAllocateBuffer(LPVOID origin);
#endif
}

namespace cfgobserver {
inline constexpr std::array<unsigned char,20> ExpectedPrefix{
    0x48,0x89,0x5C,0x24,0x08,0x48,0x89,0x6C,0x24,0x10,
    0x48,0x89,0x74,0x24,0x18,0x57,0x48,0x83,0xEC,0x20
};
struct Prepared {
    std::uintptr_t target=0,trampoline=0,relay=0,detour=0;
    std::array<unsigned char,5> patch{};
};
inline bool ExecutableRange(const void* address,std::size_t bytes) noexcept {
    MEMORY_BASIC_INFORMATION m{};
    const auto a=reinterpret_cast<std::uintptr_t>(address);
    if (!a || !bytes || bytes>(std::numeric_limits<std::uintptr_t>::max)()-a ||
        VirtualQuery(address,&m,sizeof(m))!=sizeof(m) || m.State!=MEM_COMMIT ||
        (m.Protect&(PAGE_GUARD|PAGE_NOACCESS)) ||
        !(m.Protect&(PAGE_EXECUTE|PAGE_EXECUTE_READ|PAGE_EXECUTE_READWRITE|PAGE_EXECUTE_WRITECOPY))) return false;
    const auto base=reinterpret_cast<std::uintptr_t>(m.BaseAddress);
    return a>=base && a-base<=m.RegionSize && bytes<=m.RegionSize-(a-base);
}
inline bool AbsoluteJump(const unsigned char* p,std::uintptr_t destination) noexcept {
    const unsigned char prefix[]{0xFF,0x25,0,0,0,0};
    std::uintptr_t actual=0; std::memcpy(&actual,p+6,sizeof(actual));
    return std::memcmp(p,prefix,sizeof(prefix))==0 && actual==destination;
}
// One preparation per process, completed while its threads are running normally.
// No target patch, suspension or native game call is performed here. Only the
// observer's private trampoline allocation is sealed read/execute.
// Frontend builds link a separate allocator instance for the native lifecycle
// hooks; those hooks must never allocate from or release this sealed pool.
// Slot is resident even on failed preparation; no teardown can invalidate it.
// Target prefix/extent must additionally be verified by the exact-build caller.
inline bool Prepare(void* target,void* detour,Prepared& output) noexcept {
    output={};
    if (!ExecutableRange(target,ExpectedPrefix.size()) || !ExecutableRange(detour,1) ||
        std::memcmp(target,ExpectedPrefix.data(),ExpectedPrefix.size())!=0) return false;
    static volatile LONG attempted=0;
    if (InterlockedCompareExchange(&attempted,1,0)!=0) return false;
#ifdef CFG_FRONTEND_BUNDLE
    // The lifecycle MinHook backend allocates after this page is sealed RX.
    // It must never share our allocator's in-page free-list/bookkeeping.
    auto* slot=static_cast<unsigned char*>(CfgObserverAllocateBuffer(target));
#else
    auto* slot=static_cast<unsigned char*>(AllocateBuffer(target));
#endif
    if (!slot || !ExecutableRange(slot,MEMORY_SLOT_SIZE)) return false;
    TRAMPOLINE t{}; t.pTarget=target; t.pDetour=detour; t.pTrampoline=slot;
    if (!CreateTrampolineFunction(&t) || t.patchAbove || t.nIP!=2 ||
        t.oldIPs[0]!=0 || t.oldIPs[1]!=5 || t.newIPs[0]!=0 || t.newIPs[1]!=5 ||
        t.pRelay!=slot+19 || std::memcmp(slot,ExpectedPrefix.data(),5)!=0 ||
        !AbsoluteJump(slot+5,reinterpret_cast<std::uintptr_t>(target)+5) ||
        !AbsoluteJump(slot+19,reinterpret_cast<std::uintptr_t>(detour))) return false;
    const auto a=reinterpret_cast<std::uintptr_t>(target);
    const auto relay=reinterpret_cast<std::uintptr_t>(t.pRelay);
    const auto diff=static_cast<std::int64_t>(relay)-static_cast<std::int64_t>(a+5);
    if (diff<(std::numeric_limits<std::int32_t>::min)() || diff>(std::numeric_limits<std::int32_t>::max)() ||
        std::memcmp(target,ExpectedPrefix.data(),ExpectedPrefix.size())!=0 ||
        !FlushInstructionCache(GetCurrentProcess(),slot,33)) return false;
    Prepared result; result.target=a; result.trampoline=reinterpret_cast<std::uintptr_t>(slot);
    result.relay=relay; result.detour=reinterpret_cast<std::uintptr_t>(detour);
    result.patch[0]=0xE9; const auto rel=static_cast<std::int32_t>(diff);
    std::memcpy(result.patch.data()+1,&rel,sizeof(rel));
    // Allocator instance belongs only to this module/prototype. Seal its ONE
    // private allocation before publication. No subsequent AllocateBuffer,
    // FreeBuffer/UninitializeBuffer call is permitted, including on failure.
    // MinHook initially allocates RWX; resident trampoline is then RX, not RWX.
    MEMORY_BASIC_INFORMATION region{};
    if (VirtualQuery(slot,&region,sizeof(region))!=sizeof(region) || region.Type!=MEM_PRIVATE ||
        region.AllocationBase!=region.BaseAddress || region.RegionSize!=4096) return false;
    DWORD previous=0;
    if (!VirtualProtect(region.AllocationBase,4096,PAGE_EXECUTE_READ,&previous) ||
        VirtualQuery(slot,&region,sizeof(region))!=sizeof(region) || region.Protect!=PAGE_EXECUTE_READ ||
        !FlushInstructionCache(GetCurrentProcess(),slot,33)) return false;
    output=result; return true;
}
} // namespace cfgobserver
