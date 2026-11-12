#pragma once
#include "NativeOfflineOwnerObservation.hpp"
#include "CounselorTerminalObservation.hpp"
#include "JasonInstanceObservation.hpp"
#include "../RuntimeProbe/TommyEndRaceBindings.generated.hpp"
namespace cfgtommyend {
// Set only after our own pinned hook preparation. Other callers still require
// an exact current full body, including the one owned detour instruction.
inline std::uintptr_t guardedModule=0;
inline std::array<unsigned char,682> guardedCode{};
template<class R> bool EndBodyCurrent(R& r,std::uintptr_t module){
    std::array<unsigned char,682> code{};
    return r.Read(module+cfgescape::disk::EndMatchNative,code)&&
        code==(guardedModule==module?guardedCode:cfgescape::disk::EndMatchNativeCode);
}
// Veto ONLY the OfflineBots timer's no-living-counselor EndMatch call, not its
// base countdown or other ending sources. Re-observe each time; no grace timer,
// saved pawn, death bit write, possession operation, native call or world scan.
template<class R> bool Protect(R& r,cfgmenu::promotion::Context owner,std::uintptr_t state,
    std::uintptr_t receiver,std::uintptr_t caller,unsigned thread,unsigned& reason){
    reason=0;
    if(receiver!=owner.mode||caller!=owner.module+disk::TimerReturn||
       !cfgmenu::offlineowner::Current(r,owner,thread)||
       !cfgprobe::lifetime::ReadArrayHeader(r,owner.module+cfgprobe::lifetime::ObjectArrayRva,owner.registry))return false;
    std::array<unsigned char,298> timerCode{};
    if(!r.Read(owner.module+disk::Timer,timerCode)||timerCode!=disk::TimerCode)return false;
    cfgnames::Words phase{};cfgnames::Text text;cfgnames::Observation<R> names(r,owner.module);
    std::uintptr_t killer=0,killerType=0,stateType=0,counselor=0,ctor=0,pending=0,pendingType=0,current=0;int timer=0;float health=0;std::uint8_t dying=0;
    cfgprobe::lifetime::SlotObservation gameState{},killerSlot{},currentSlot{},pendingSlot{};
    if(!cfgobserver::Same(r,owner.world+cfgprobe::WorldGameState,state)||
       !cfgspawn::ObjectSlot(r,owner.registry,state,gameState)||!gameState.HasAssignedSerial()||
       !r.Read(owner.module+cfgescape::disk::SCGameStateReflection,stateType)||!stateType||
       !cfgobserver::Same(r,owner.module+cfgescape::disk::SCGameStateConstructor,stateType)||!cfgspawn::ObjectIs(r,state,stateType)||
       !r.Read(owner.mode+cfgescape::disk::MatchState,phase)||!names.Decode(phase,text)||text.View()!=u"InProgress"||
       !r.Read(state+0x420,killer)||!cfgspawn::ObjectSlot(r,owner.registry,killer,killerSlot)||!killerSlot.HasAssignedSerial()||
       !r.Read(owner.module+cfgmenu::birthdisk::SCKillerCharacterClass,killerType)||!killerType||
       !cfgobserver::Same(r,owner.module+cfgmenu::birthdisk::SCKillerCharacterConstructor,killerType)||!cfgspawn::ObjectIs(r,killer,killerType)||
       !r.Read(killer+cfghunter::disk::Health,health)||!std::isfinite(health)||health<=cfghunter::disk::AliveThreshold||
       !r.Read(killer+cfgescape::disk::DyingFlags,dying)||(dying&cfgescape::disk::DyingMask)||
       !r.Read(owner.mode+cfgescape::disk::HunterToSpawn,pending)||
       !r.Read(state+cfgescape::disk::HunterTimeRemaining,timer)||timer< -1||timer>600||
       !r.Read(owner.controller+cfgprobe::ControllerPawn,current))return false;
    unsigned observed=0;
    if(pending){
        if(!r.Read(owner.module+cfgescape::disk::ControllerReflection,pendingType)||!pendingType||
           !cfgobserver::Same(r,owner.module+cfgescape::disk::ControllerConstructor,pendingType)||
           !cfgspawn::ObjectSlot(r,owner.registry,pending,pendingSlot)||!pendingSlot.HasAssignedSerial()||
           !cfgspawn::ObjectIs(r,pending,pendingType)||!cfgescape::Same(r,pending+0x110,std::uint8_t{3}))return false;
        observed=1;
    }else if(timer>0)observed=2;
    else if(current){
        cfgescape::State status{};
        if(!r.Read(owner.module+cfgescape::disk::SCCounselorCharacterReflection,counselor)||
           !r.Read(owner.module+cfgescape::disk::SCCounselorCharacterConstructor,ctor)||counselor!=ctor||!counselor||
           !cfgspawn::ObjectSlot(r,owner.registry,current,currentSlot)||!currentSlot.HasAssignedSerial()||
           !cfgobserver::Same(r,current+cfgescape::disk::Controller,owner.controller)||
           !cfgobserver::Same(r,current+cfgescape::disk::PlayerState,owner.playerState)||
           !cfgescape::Counselor(r,owner.registry,owner.module,current,counselor,status)||status!=cfgescape::State::Living)return false;
        observed=3;
    }
    if(!observed||!cfgmenu::offlineowner::Current(r,owner,thread)||
       !cfgobserver::Same(r,owner.world+cfgprobe::WorldGameState,state)||
       !cfgjason::SameSlot(r,owner.registry,gameState)||!cfgjason::SameSlot(r,owner.registry,killerSlot)||
       !cfgobserver::Same(r,state+0x420,killer)||!cfgescape::Same(r,killer+cfghunter::disk::Health,health)||
       !cfgobserver::Same(r,owner.module+cfgescape::disk::SCGameStateReflection,stateType)||
       !cfgobserver::Same(r,owner.module+cfgescape::disk::SCGameStateConstructor,stateType)||!cfgspawn::ObjectIs(r,state,stateType)||
       !cfgobserver::Same(r,owner.module+cfgmenu::birthdisk::SCKillerCharacterClass,killerType)||
       !cfgobserver::Same(r,owner.module+cfgmenu::birthdisk::SCKillerCharacterConstructor,killerType)||!cfgspawn::ObjectIs(r,killer,killerType)||
       !cfgescape::Same(r,killer+cfgescape::disk::DyingFlags,dying)||
       !cfgescape::Same(r,owner.mode+cfgescape::disk::MatchState,phase)||
       !cfgobserver::Same(r,owner.mode+cfgescape::disk::HunterToSpawn,pending)||
       !cfgescape::Same(r,state+cfgescape::disk::HunterTimeRemaining,timer)||
       !cfgobserver::Same(r,owner.controller+cfgprobe::ControllerPawn,current))return false;
    if(observed==1&&(!cfgjason::SameSlot(r,owner.registry,pendingSlot)||
       !cfgobserver::Same(r,owner.module+cfgescape::disk::ControllerReflection,pendingType)||
       !cfgobserver::Same(r,owner.module+cfgescape::disk::ControllerConstructor,pendingType)||
       !cfgspawn::ObjectIs(r,pending,pendingType)||!cfgescape::Same(r,pending+0x110,std::uint8_t{3})))return false;
    if(observed==3){cfgescape::State fresh{};
        if(!cfgjason::SameSlot(r,owner.registry,currentSlot)||
           !cfgobserver::Same(r,current+cfgescape::disk::Controller,owner.controller)||
           !cfgobserver::Same(r,current+cfgescape::disk::PlayerState,owner.playerState)||
           !cfgescape::Counselor(r,owner.registry,owner.module,current,counselor,fresh)||fresh!=cfgescape::State::Living)return false;}
    reason=observed;return true;
}
}
