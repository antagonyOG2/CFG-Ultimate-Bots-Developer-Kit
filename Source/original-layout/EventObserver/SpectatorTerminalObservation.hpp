#pragma once
#include "CounselorTerminalObservation.hpp"
namespace cfgspectator {
// Only this exact native pawn/controller mutual ownership chain may issue a
// serial0 STATE identity. Actors/world/controller never receive this flag.
// Retained targets must repeat this proof before camera publication.
template<class R,class Identity> bool OwnedPlayerState(R& r,const cfgprobe::lifetime::ArrayHeader& registry,
 std::uintptr_t pawn,std::uintptr_t controller,std::uintptr_t state,std::uintptr_t stateType,Identity& out){
 out={};cfgprobe::lifetime::SlotObservation pawnSlot{},controllerSlot{},stateSlot{},last{};
 if(!cfgspawn::ObjectSlot(r,registry,pawn,pawnSlot)||!pawnSlot.HasAssignedSerial()||
  !cfgspawn::ObjectSlot(r,registry,controller,controllerSlot)||!controllerSlot.HasAssignedSerial()||
  !cfgspawn::ObjectSlot(r,registry,state,stateSlot)||!cfgspawn::ObjectIs(r,state,stateType)||
  !cfgescape::Same(r,pawn+cfgescape::disk::Controller,controller)||!cfgescape::Same(r,controller+cfgescape::disk::Pawn,pawn)||
  !cfgescape::Same(r,pawn+cfgescape::disk::PlayerState,state)||!cfgescape::Same(r,controller+cfgescape::disk::PlayerState,state)||
  !cfgspawn::ObjectSlot(r,registry,pawn,last)||!(last==pawnSlot)||
  !cfgspawn::ObjectSlot(r,registry,controller,last)||!(last==controllerSlot)||
  !cfgspawn::ObjectSlot(r,registry,state,last)||!(last==stateSlot))return false;
 out={state,stateSlot.serial,stateSlot.serial==0};return true;
}
// The escaped/dead counselor pawn can be unpossessed and destroyed natively.
// Camera admission must not require its continued allocation. The retained
// local controller owns this current native SCPlayerState terminal witness;
// the caller separately proves its local-player chain and SCSpectatorPawn.
// PlayerState serial0 is a same-call strong ownership observation only, never
// a retained identity or permission to write that state/allocate a serial.
template<class R> bool LocalTerminal(R& r,const cfgprobe::lifetime::ArrayHeader& registry,std::uintptr_t module,
 std::uintptr_t controller,cfgescape::State& result){
 result=cfgescape::State::Unknown;std::uintptr_t state=0,type=0,ctor=0,again=0;
 std::uint8_t escaped=0,dead=0,lastEscaped=0,lastDead=0;cfgprobe::lifetime::SlotObservation slot{},last{};
 if(!r.Read(controller+cfgescape::disk::PlayerState,state)||!cfgprobe::ObjectPointer(state)||
  !r.Read(module+cfgescape::disk::SCPlayerStateReflection,type)||!type||
  !r.Read(module+cfgescape::disk::SCPlayerStateConstructor,ctor)||ctor!=type||
  !cfgspawn::ObjectSlot(r,registry,state,slot)||!cfgspawn::ObjectIs(r,state,type)||
  !r.Read(state+cfgescape::disk::bEscaped,escaped)||escaped>1||!r.Read(state+cfgescape::disk::bDead,dead)||dead>1||
  !r.Read(controller+cfgescape::disk::PlayerState,again)||again!=state||
  !r.Read(state+cfgescape::disk::bEscaped,lastEscaped)||lastEscaped!=escaped||
  !r.Read(state+cfgescape::disk::bDead,lastDead)||lastDead!=dead||
  !cfgspawn::ObjectSlot(r,registry,state,last)||!(last==slot))return false;
 result=escaped?cfgescape::State::Escaped:dead?cfgescape::State::Dead:cfgescape::State::Living;
 return true;
}
// Offline native death can put the local controller into its OWN spectator
// pawn before SCPlayerState's death bit catches up. A terminal current human
// pawn plus the exact attached state corroborates this; a living Tommy or an
// unrelated dead counselor never grants camera authority. Caller also proves
// the spectator's native class, local-player chain, world and no-GC scope.
template<class R> bool TerminalSpectatorOwner(R& r,const cfgprobe::lifetime::ArrayHeader& registry,std::uintptr_t module,
 std::uintptr_t controller,const cfgprobe::lifetime::SlotObservation& human,std::uintptr_t counselorClass,
 const cfgprobe::lifetime::SlotObservation& spectator,cfgescape::State& result){
 if(LocalTerminal(r,registry,module,controller,result)&&cfgescape::Terminal(result))return true;
 result=cfgescape::State::Unknown;cfgprobe::lifetime::SlotObservation current{},again{};std::uintptr_t state=0,possessed=0;
 if(!human.HasAssignedSerial()||!spectator.HasAssignedSerial()||human.object==spectator.object||
    !cfgspawn::ObjectSlot(r,registry,human.object,current)||!(current==human)||
    !cfgspawn::ObjectSlot(r,registry,spectator.object,current)||!(current==spectator)||
    !cfgescape::Same(r,spectator.object+cfgescape::disk::Controller,controller)||
    !r.Read(controller+cfgescape::disk::Pawn,possessed)||(possessed&&possessed!=spectator.object&&possessed!=human.object)||
    !r.Read(controller+cfgescape::disk::PlayerState,state)||!state||
    !cfgescape::Same(r,human.object+cfgescape::disk::PlayerState,state)||
    !cfgescape::Counselor(r,registry,module,human.object,counselorClass,result)||!cfgescape::Terminal(result)||
    !cfgescape::Same(r,controller+cfgescape::disk::PlayerState,state)||
    !cfgescape::Same(r,controller+cfgescape::disk::Pawn,possessed)||
    !cfgspawn::ObjectSlot(r,registry,human.object,again)||!(again==human)||
    !cfgspawn::ObjectSlot(r,registry,spectator.object,again)||!(again==spectator)){
     result=cfgescape::State::Unknown;return false;}
 return true;
}
// Jason's mask/setup damage may reach zero without native death, unlike a
// counselor. Keep finite health evidence, but use native dying/dead flags.
template<class R> bool KillerLiving(R& r,std::uintptr_t pawn,std::uintptr_t state,bool& living){
 living=false;float health=0,lastHealth=0;std::uint8_t dying=0,dead=0,lastDying=0,lastDead=0;
 if(!r.Read(pawn+cfghunter::disk::Health,health)||!std::isfinite(health)||
  !r.Read(pawn+cfgescape::disk::DyingFlags,dying)||!r.Read(state+cfgescape::disk::bDead,dead)||dead>1||
  !r.Read(pawn+cfghunter::disk::Health,lastHealth)||lastHealth!=health||
  !r.Read(pawn+cfgescape::disk::DyingFlags,lastDying)||lastDying!=dying||
  !r.Read(state+cfgescape::disk::bDead,lastDead)||lastDead!=dead)return false;
 living=!(dying&cfgescape::disk::DyingMask)&&!dead;return true;
}
}
