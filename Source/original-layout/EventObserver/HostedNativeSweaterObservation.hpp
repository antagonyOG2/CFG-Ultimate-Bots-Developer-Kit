#pragma once
#include "CounselorTerminalObservation.hpp"
namespace cfghostedsweater {
// Observation only, called for the current bound hosted World. An earned
// native pickup may start convergence before the separate power binder runs.
template<class R>bool Present(R& r,const cfgprobe::lifetime::ArrayHeader& array,
 const cfgprobe::lifetime::SlotObservation& pawn,std::uintptr_t controller,std::uintptr_t counselorClass){
    std::uint8_t has=0;float health=0;
    return pawn.HasAssignedSerial()&&cfgjason::SameSlot(r,array,pawn)&&cfgspawn::ObjectIs(r,pawn.object,counselorClass)&&
        r.Read(pawn.object+cfghunter::disk::Health,health)&&std::isfinite(health)&&health>0&&
        r.Read(pawn.object+cfgfeature::disk::HasSweater,has)&&has==1&&
        cfgobserver::Same(r,controller+cfgprobe::ControllerPawn,pawn.object)&&
        cfgobserver::Same(r,pawn.object+cfgescape::disk::Controller,controller)&&cfgjason::SameSlot(r,array,pawn);
}
}
