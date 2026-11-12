#pragma once
#include <cstdint>
namespace cfgsweater {
struct Publication {
    std::uintptr_t pawn=0,ability=0,expectedAbility=0,expectedOwner=0;
    std::uint8_t has=0,wearing=0,unlimited=0,used=0;
    bool rearm=false;
    bool Valid()const{return pawn>=0x0/*CFG3.5 address withheld*/&&!(pawn&7)&&ability>=0x0/*CFG3.5 address withheld*/&&!(ability&7)&&
        (!expectedOwner||expectedOwner==pawn)&&(!expectedAbility||expectedAbility==ability)&&
        has<=1&&wearing<=1&&unlimited<=1&&used==(rearm?1:0);}
};
// Native owner-thread caller proves exact fields/identities/API BEFORE applying.
// Expected-value writes refuse mismatch and read back. A partial publication
// must terminally disable the feature; no automatic retry/rollback over native state.
template<class Host>bool Publish(Host& h,const Publication& p){
    if(!p.Valid()||!h.Fresh(p))return false;
    return h.Owner(p.expectedOwner,p.pawn)&&h.Ability(p.expectedAbility,p.ability)&&
        h.Has(p.has,1)&&h.Wearing(p.wearing,1)&&h.Unlimited(p.unlimited,1)&&
        (!p.rearm||h.Used(p.used,0))&&h.Visual();
}
}
