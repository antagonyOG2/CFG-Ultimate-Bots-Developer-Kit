#pragma once
#include "HostedLobbyObservation.hpp"
namespace cfghostedlobby {
// Donor host premise for preference/input feedback ONLY. No spawn, possession,
// native dispatch or retention is admitted here, including while traveling.
template<class R>bool InputHost(R& r,std::uintptr_t module,std::uint32_t thread){
 Context c;c.module=module;c.thread=thread;std::uintptr_t driver=0,state=0;
 std::uint32_t current=0;std::uint8_t flag=0,role=0;Slot slot;cfghosted::listen35::LocalPlayers players{};
 if(!r.Read(module+cfgprobe::dispatch::ThreadIdRva,current)||current!=thread||
    !r.Read(module+cfgprobe::dispatch::ThreadFlagRva,flag)||flag!=1||
    !r.Read(module+cfgprobe::EngineGlobalCandidate,c.engine)||!r.Read(c.engine+cfgprobe::EngineGameViewport,c.viewport)||
    !r.Read(c.viewport+cfgprobe::ViewportWorld,c.world)||!cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,c.registry)||
    !r.Read(c.world+cfgprobe::WorldGameMode,c.mode)||!cfgpracticeobs::Clear(r,c.registry,c.mode,slot)||
    !r.Read(c.world+cfgprobe::WorldGameState,state)||!cfgpracticeobs::Clear(r,c.registry,state,slot)||
    !r.Read(c.world+cfgprobe::WorldNetDriver,driver)||!Typed(r,c,driver,cfghosted::disk35::NetDriverReflection,slot)||
    !cfgobserver::Same(r,driver+cfghosted::disk35::ServerConnection,std::uintptr_t{})||
    !cfgobserver::Same(r,driver+cfghosted::disk35::DriverWorld,c.world)||
    !r.Read(c.mode+cfghosted::disk35::ActorRole,role)||role!=3||
    !r.Read(c.world+cfgprobe::WorldGameInstance,c.instance)||!r.Read(c.instance+cfgprobe::GameInstanceLocalPlayers,players)||
    players.count<1||players.count>16||players.capacity<players.count||players.capacity>16||!cfgprobe::ObjectPointer(players.data)||
    !r.Read(players.data,c.player)||!r.Read(c.player+cfghosted::disk35::PlayerController,c.controller)||
    !Typed(r,c,c.controller,disk::ControllerReflection,slot)||
    !cfgobserver::Same(r,c.controller+cfghosted::disk35::ControllerPlayer,c.player)||
    !r.Read(c.controller+cfghosted::disk35::ActorRole,role)||role!=3)return false;
 return cfgobserver::Same(r,c.viewport+cfgprobe::ViewportWorld,c.world)&&
    cfgobserver::Same(r,c.world+cfgprobe::WorldGameMode,c.mode)&&cfgobserver::Same(r,c.world+cfgprobe::WorldGameState,state)&&
    cfgobserver::Same(r,c.world+cfgprobe::WorldNetDriver,driver)&&
    cfgobserver::Same(r,c.player+cfghosted::disk35::PlayerController,c.controller);
}
}
