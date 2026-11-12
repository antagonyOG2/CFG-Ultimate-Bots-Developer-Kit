#pragma once
#include "../RuntimeProbe/ActiveMenuObservation.hpp"
#include "../RuntimeProbe/FrontendControllerObservation.hpp"
#include "../RuntimeProbe/NameObservation.hpp"
#include "SpawnBindings.hpp"
namespace cfgmenu::challengereturn {
// Read-only admission for a NEW current standalone Challenges lobby. The old
// source's World/HUD/stack/widget are deliberately never dereferenced: travel
// can retire them. This is not permission to restore/call an old menu instance.
template<class R>bool Observe(R& r,const active::Context& origin,std::uint32_t thread,active::Context& out){
 out={};if(!thread||thread!=origin.thread)return false;
 std::uintptr_t world=0,mode=0,native=0,ctor=0;
 if(!r.Read(origin.viewport+cfgprobe::ViewportWorld,world)||!cfgprobe::ObjectPointer(world)||
    !r.Read(world+cfgprobe::WorldGameMode,mode)||!cfgprobe::ObjectPointer(mode)||
    !r.Read(origin.module+execdisk::FrontendClass,native)||
    !r.Read(origin.module+execdisk::FrontendConstructor,ctor)||!cfgprobe::MatchingClassCache(native,ctor)||
    !cfgspawn::ObjectIs(r,mode,native))return false;
 active::Context actual;cfgnames::Observation<R> names(r,origin.module);cfgnames::Text label;
 if(!active::Observe(r,origin.module,actual)||actual.thread!=thread||
    actual.engine!=origin.engine||actual.viewport!=origin.viewport||actual.player!=origin.player||
    actual.gameInstance!=origin.gameInstance||actual.world!=world||actual.mode!=mode||
    !controllergate::Menu(r,actual.module,actual.controller)||
    !names.Object(actual.type,label)||label.View()!=u"SP_LobbyWidget_C")return false;
 std::uintptr_t fresh=0;
 if(!active::Current(r,actual,thread)||!r.Read(actual.module+execdisk::FrontendClass,fresh)||fresh!=native||
    !r.Read(actual.module+execdisk::FrontendConstructor,fresh)||fresh!=ctor||!cfgspawn::ObjectIs(r,mode,native))return false;
 out=actual;return true;
}
}
