#pragma once
#include "../Core/KillPolicy.hpp"
#include "../RuntimeProbe/KillBindings.generated.hpp"
#if defined(_WIN32)
#include <windows.h>
#include <intrin.h>
#endif
namespace cfgkill {
using EligibilityTable=std::array<std::uintptr_t,disk::EligibilityVtableEntries+1>;
// Pure construction, no object/vtable mutation. Caller owns process-pinned
// immutable storage and installs &out[1] on one freshly revalidated component.
// Every original native slot, including the preceding word, must match disk.
template<class Reader> bool BuildEligibilityTable(Reader& r,std::uintptr_t module,std::uintptr_t source,
                                                 std::uintptr_t replacement,EligibilityTable& out){
    if(!module||module>UINTPTR_MAX-disk::EligibilitySecondaryVtable||!replacement||source!=module+disk::EligibilityVtable)return false;
    EligibilityTable candidate{};
    if(!r.Read(source-sizeof(std::uintptr_t),candidate)||candidate[0]!=module+disk::EligibilityVtablePrefix)return false;
    for(unsigned i=0;i<disk::EligibilityVtableEntries;++i)
        if(candidate[i+1]!=module+disk::EligibilityVtableRvas[i])return false;
    candidate[1+disk::EligibilityVtableSlot/sizeof(std::uintptr_t)]=replacement;out=candidate;return true;
}
// Created afresh by a host after validating retained OFFLINE world, exact
// primary Jason + native JasonKillClass context + owned component serials,
// exact tracked living finisher/held axe-or-machete and complete predicate code.
// Never create a ticket from names/proximity alone. No install is performed here.
struct EligibilityTicket {
    Identity identity{};std::uintptr_t context=0,component=0,counselor=0;
    bool offline=false,identities=false,living=false,heldWeapon=false,nativeCode=false;
};
inline bool MayRelax(const Policy& p,const EligibilityTicket& t,std::uintptr_t component,
                     std::uintptr_t counselor,std::uint64_t now){
    return p.Exclusive(now)&&p.context&&p.finisher&&p.identity==t.identity&&
        (p.stage==Stage::NativeContext||p.stage==Stage::NativeActive)&&
        t.offline&&t.identities&&t.living&&t.heldWeapon&&t.nativeCode&&
        t.context==p.context&&t.component&&component==t.component&&
        t.counselor==p.finisher&&counselor==t.counselor;
}
#if defined(_WIN32)
struct EligibilityVector {float x,y,z;};
struct EligibilityPacket {
    std::uintptr_t counselor=0;EligibilityVector location{},rotation{};
    std::int32_t result=0;std::uint32_t padding=0;
};
static_assert(sizeof(EligibilityPacket)==40&&offsetof(EligibilityPacket,result)==32);
using EligibilityOriginal=int(__fastcall*)(void*,void*,const EligibilityVector*,const EligibilityVector*);
// POD-only SEH boundary, same narrow duration as FrozenJasonBridge's
// CallContextKillEligibilityWithTemporaryHunter. Caller must pass a validated
// writable typed bIsHunter byte only after MayRelax, never a whole interaction.
// No weapon-list, return-mask, disabled/context/range gate is overwritten.
__declspec(noinline) inline int CallEligibilityWithHunter(EligibilityOriginal original,void* component,void* counselor,
    const EligibilityVector* location,const EligibilityVector* direction,volatile char* hunter) {
    if(!original)return 0;
    if(!hunter)return original(component,counselor,location,direction);
    const char prior=_InterlockedCompareExchange8(hunter,1,0);
    if(prior!=0)return original(component,counselor,location,direction);
    int result=0;
    __try {result=original(component,counselor,location,direction);}
    __finally {_InterlockedCompareExchange8(hunter,0,1);}
    return result;
}
#endif
}
