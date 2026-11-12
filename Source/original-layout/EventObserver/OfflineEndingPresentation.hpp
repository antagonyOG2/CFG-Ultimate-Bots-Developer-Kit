#pragma once
#include "NativeEndingSkin.hpp"
#include "../RuntimeProbe/SnapshotBindings.hpp"
namespace cfgending::skin {
// Presentation-only bridge for an owned OFFLINE AI state. CFG's profile skin
// setter requires an ILLPlayerController; the donor setter does not. Do not
// change bot/ownership/inventory flags to impersonate a human. Transfer the
// already selected, concretely typed intro skin into the null outro metadata
// field; Transfer owns one-shot entry and next-callback readback. Weapon stays
// on its native deep-copy/SettingsLoaded route.
// One implementation for both modes (H2.44). The offline entry keeps the
// offline gate (WorldNetDriver==0) exactly as before; the hosted entry is the
// same code with that single gate replaced by the caller's hosted scope proof.
template<class R,class Scope,class Exchange> bool PublishPresentation(R& r,std::uintptr_t module,
    const cfgprobe::lifetime::ArrayHeader& registry,std::uintptr_t world,std::uintptr_t state,
    std::uintptr_t controller,std::uintptr_t pawn,std::uintptr_t human,std::uintptr_t value,Scope&& scope,Exchange&& exchange,
    bool requireOfflineWorld){
    std::uint32_t flags=0;std::uint8_t role=0;std::uintptr_t previous=1;
    cfgprobe::lifetime::SlotObservation ps{},pc{},actor{},again{};
    if(!value||!state||state==human||!scope()||!Safe(r,module)||
       (requireOfflineWorld&&!Same(r,world+cfgprobe::WorldNetDriver,std::uintptr_t{}))||
       !LiveSlot(r,registry,state,ps)||!LiveSlot(r,registry,controller,pc)||!LiveSlot(r,registry,pawn,actor)||
       !r.Read(state+0x394,flags)||!(flags&4)||!r.Read(state+0x110,role)||role!=3||
       !Same(r,state+0x88,controller)||!Same(r,controller+0x388,state)||
       !Same(r,pawn+0x388,state)||!Same(r,pawn+0x3A0,controller)||!Same(r,controller+0x370,pawn)||
       !Concrete(r,module,registry,value)||!scope()||!r.Read(state+disk::PickedSkin,previous)||previous||
       !LiveSlot(r,registry,state,again)||!(again==ps)||!LiveSlot(r,registry,controller,again)||!(again==pc)||
       !LiveSlot(r,registry,pawn,again)||!(again==actor)||!Safe(r,module))return false;
    return exchange(state+disk::PickedSkin,value); // null-only CAS; no retry after ambiguous entry
}
template<class R,class Scope,class Exchange> bool PublishOfflinePresentation(R& r,std::uintptr_t module,
    const cfgprobe::lifetime::ArrayHeader& registry,std::uintptr_t world,std::uintptr_t state,
    std::uintptr_t controller,std::uintptr_t pawn,std::uintptr_t human,std::uintptr_t value,Scope&& scope,Exchange&& exchange){
    return PublishPresentation(r,module,registry,world,state,controller,pawn,human,value,
        static_cast<Scope&&>(scope),static_cast<Exchange&&>(exchange),true);
}
}
