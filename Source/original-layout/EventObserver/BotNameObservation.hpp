#pragma once
#include "SpectatorTerminalObservation.hpp"
namespace cfgbotname {
struct StateWitness {std::uintptr_t object=0;int serial=0;bool strongOwnedState=false;};
// Same-call native ownership proof, never a serial0 weak identity or a root.
// The caller ALSO proves owned bot roster/AI type/offline world/native setter.
template<class R>bool OwnedState(R& r,const cfgprobe::lifetime::ArrayHeader& objects,
 std::uintptr_t pawn,std::uintptr_t controller,std::uintptr_t state,std::uintptr_t type,
 std::uintptr_t humanPawn,std::uintptr_t humanController,std::uintptr_t humanState,
 cfgprobe::lifetime::SlotObservation& out){
 out={};StateWitness witness;
 return pawn&&controller&&state&&pawn!=humanPawn&&controller!=humanController&&state!=humanState&&
  cfgspectator::OwnedPlayerState(r,objects,pawn,controller,state,type,witness)&&
  cfgobserver::Same(r,state+cfgspawn::ActorOwner,controller)&&
  cfgspawn::ObjectSlot(r,objects,state,out)&&out.object==witness.object&&out.serial==witness.serial;
}
}
