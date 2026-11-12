#pragma once
#include "JasonInstanceObservation.hpp"
#include "../RuntimeProbe/EscapeBindings.generated.hpp"
namespace cfgknife {
// Callback-local credit requires the caller's full Context/LinkedKiller proof.
// This does not replace thread, offline, no-GC or fresh world checks.
template<class R> bool FreshOwnerLinks(R& r,const cfgprobe::lifetime::ArrayHeader& registry,
    const cfgjason::NewActorEvidence& j,const std::array<std::uintptr_t,3>& classes,
    const cfgprobe::lifetime::SlotObservation& state,std::uintptr_t stateClass){
    return cfgjason::SameSlot(r,registry,j.pawn)&&cfgjason::SameSlot(r,registry,j.killerController)&&
        cfgjason::OwnedComponentSlot(r,registry,j.path)&&cfgjason::OwnedComponentSlot(r,registry,state)&&
        cfgobserver::Same(r,j.pawn.object+0x10,classes[0])&&
        cfgobserver::Same(r,j.killerController.object+0x10,classes[1])&&
        cfgobserver::Same(r,j.path.object+0x10,classes[2])&&cfgobserver::Same(r,state.object+0x10,stateClass)&&
        cfgobserver::Same(r,j.pawn.object+0x3A0,j.killerController.object)&&
        cfgobserver::Same(r,j.killerController.object+0x370,j.pawn.object)&&
        cfgobserver::Same(r,j.killerController.object+0x410,j.path.object)&&
        cfgobserver::Same(r,j.path.object+0x20,j.killerController.object)&&
        cfgobserver::Same(r,j.path.object+cfgjason::disk::ComponentOwner,j.killerController.object)&&
        cfgobserver::Same(r,j.pawn.object+cfgescape::disk::PlayerState,state.object)&&
        cfgobserver::Same(r,j.killerController.object+cfgescape::disk::PlayerState,state.object)&&
        cfgobserver::Same(r,state.object+0x88,j.killerController.object);
}
}
