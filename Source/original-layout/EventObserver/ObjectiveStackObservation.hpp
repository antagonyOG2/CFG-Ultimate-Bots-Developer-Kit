#pragma once
#include "../RuntimeProbe/ActiveMenuObservation.hpp"
#include "../RuntimeProbe/MenuStackParent.generated.hpp"
#include "SpawnAssetObservation.hpp"
namespace cfgobjective::stack {
struct Observation {
    cfgprobe::lifetime::SlotObservation hud{},stack{},widget{};
    cfgmenu::active::StackArray records{};unsigned index=0;
};
// CFG's GetCurrentMenu proves SCHUD.Stack450 / recordsF0 / stride32 / widget8.
// This visits only the supplied, typed local HUD stack; never the UObject array.
// Host proves native SCHUD / SCMenuStackComponent ancestry and offline scope.
template<class R>bool Observe(R& r,const cfgprobe::lifetime::ArrayHeader& registry,
    std::uintptr_t controller,std::uintptr_t hud,std::uintptr_t stack,std::uintptr_t rootClass,Observation& out){
    out={};Observation candidate;using namespace cfgmenu::execdisk;
    if(!rootClass||!cfgspawn::ObjectSlot(r,registry,hud,candidate.hud)||!candidate.hud.HasAssignedSerial()||
       !cfgspawn::ObjectSlot(r,registry,stack,candidate.stack)||!candidate.stack.HasAssignedSerial()||
       !r.Read(stack+StackArray,candidate.records)||!candidate.records.Sane())return false;
    bool found=false;
    for(int i=candidate.records.count-1;i>=0;--i){
        std::uintptr_t widget=0,type=0;
        if(!r.Read(candidate.records.data+static_cast<std::uintptr_t>(i)*RecordBytes+RecordWidget,widget))return false;
        if(!widget)continue;
        if(!cfgprobe::ObjectPointer(widget)||!r.Read(widget+cfgprobe::ObjectClass,type))return false;
        if(type!=rootClass)continue;
        if(found)return false; // ambiguous stack never approves a finder replay
        if(!cfgspawn::ObjectSlot(r,registry,widget,candidate.widget)||!candidate.widget.HasAssignedSerial())return false;
        candidate.index=static_cast<unsigned>(i);found=true;
    }
    if(!found)return false;
    const auto same=[&](std::uintptr_t at,std::uintptr_t expected){std::uintptr_t value=0;return r.Read(at,value)&&value==expected;};
    cfgmenu::active::StackArray fresh;cfgprobe::lifetime::SlotObservation hs,ss,ws;
    if(!same(controller+ControllerHUD,hud)||!same(hud+HUDStack,stack)||!same(stack+0x20,hud)||
       !same(candidate.widget.object+0x20,stack)||!same(candidate.widget.object+cfgprobe::ObjectClass,rootClass)||
       !same(candidate.records.data+candidate.index*RecordBytes+RecordWidget,candidate.widget.object)||
       !r.Read(stack+StackArray,fresh)||!(fresh==candidate.records)||
       !cfgspawn::ObjectSlot(r,registry,hud,hs)||!(hs==candidate.hud)||
       !cfgspawn::ObjectSlot(r,registry,stack,ss)||!(ss==candidate.stack)||
       !cfgspawn::ObjectSlot(r,registry,candidate.widget.object,ws)||!(ws==candidate.widget))return false;
    out=candidate;return true;
}
}
