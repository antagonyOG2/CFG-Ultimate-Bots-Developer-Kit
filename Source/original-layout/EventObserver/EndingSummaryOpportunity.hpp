#pragma once
#include "NativeSummaryObservation.hpp"
namespace cfgsummaryending {
// Cheap transport hint only. Every effect still needs fresh SummaryScope,
// native terminal phase, current participant and actual owned display proof.
// Only native HUD/scoreboard/current stack roots; no world/UObject census.
template<class R>bool WidgetCandidate(R& r,std::uintptr_t controller,std::uintptr_t object){
    std::uintptr_t hud=0,widget=0,score=0,display=0,stack=0,root=0;
    if(!object||!r.Read(controller+cfgsweater::huddisk::ControllerHUD,hud)||!hud)return false;
    if(object==hud)return true;
    if(r.Read(hud+cfgsweater::huddisk::HUDWidget,widget)&&widget&&object==widget)return true;
    if(r.Read(hud+cfgsummary::disk::Scoreboard,score)&&score){
        if(object==score)return true;
        if(r.Read(score+cfgsummary::disk::Display,display)&&display&&object==display)return true;
    }
    cfgmenu::active::StackArray records{};
    return r.Read(hud+cfgmenu::execdisk::HUDStack,stack)&&stack&&
        r.Read(stack+cfgmenu::execdisk::StackArray,records)&&records.Sane()&&records.count>0&&
        r.Read(records.data+static_cast<std::uintptr_t>(records.count-1)*cfgmenu::execdisk::RecordBytes+
            cfgmenu::execdisk::RecordWidget,root)&&root&&object==root;
}
}
