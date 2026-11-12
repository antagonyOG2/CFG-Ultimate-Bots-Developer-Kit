#pragma once
#include "KillEligibility.hpp"
namespace cfgkill {
inline bool PamelaOverrideAllowed(const Policy& policy,const EligibilityTicket& ticket,
    std::uintptr_t component,std::uintptr_t pawn,std::uint64_t now,std::uintptr_t human){
    return policy.PamelaAccepted()&&pawn==human&&MayRelax(policy,ticket,component,pawn,now);
}
// Donor ContextKillCanInteractHook accepts ANY proved living counselor on
// the accepted death component. Native range/view/lock gates are preserved.
inline bool PamelaNativeOverrideAllowed(const Policy& policy,const EligibilityTicket& ticket,
    std::uintptr_t component,std::uintptr_t pawn,std::uint64_t now){
    return policy.PamelaAccepted()&&policy.Exclusive(now)&&policy.identity==ticket.identity&&
        policy.context&&ticket.context==policy.context&&ticket.component==component&&component&&
        ticket.counselor==pawn&&pawn&&ticket.offline&&ticket.identities&&ticket.living&&ticket.heldWeapon&&ticket.nativeCode;
}
// Only called after the existing exact accepted-death-context/finisher proof.
// POD-only SEH: no allocation, getters, ownership acquisition or native packet
// modification. Stock still owns all range/view/lock/live prerequisites.
__declspec(noinline) inline int CallEligibilityWithHeldWeapon(EligibilityOriginal original,void* component,void* counselor,
    const EligibilityVector* location,const EligibilityVector* direction,volatile char* hunter,
    std::uintptr_t* allowed,std::uintptr_t expected,std::uintptr_t heldClass){
    if(!original)return 0;
    if(!hunter||!heldClass)return original(component,counselor,location,direction);
    bool changedWeapon=false;char before=0;bool changedHunter=false;int result=0;
    __try{
        if(allowed){
            const auto prior=InterlockedCompareExchangePointer(reinterpret_cast<void* volatile*>(allowed),
                reinterpret_cast<void*>(heldClass),reinterpret_cast<void*>(expected));
            if(prior!=reinterpret_cast<void*>(expected))return original(component,counselor,location,direction);
            changedWeapon=true;
        }
        before=_InterlockedCompareExchange8(hunter,1,0);changedHunter=before==0;
        result=original(component,counselor,location,direction);
    }__finally{
        if(changedHunter)_InterlockedCompareExchange8(hunter,0,1);
        if(changedWeapon)InterlockedCompareExchangePointer(reinterpret_cast<void* volatile*>(allowed),
            reinterpret_cast<void*>(expected),reinterpret_cast<void*>(heldClass));
    }
    return result;
}
}
