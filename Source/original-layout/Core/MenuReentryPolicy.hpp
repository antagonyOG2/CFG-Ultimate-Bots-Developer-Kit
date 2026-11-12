#pragma once
#include "../RuntimeProbe/ActiveMenuObservation.hpp"
namespace cfgmenu::reentry {
// Revisit only within the same positively identified native frontend. A new
// world/controller or recycled widget is never a continuation of this setup.
inline bool Infrastructure(const active::Context& a,const active::Context& b) noexcept {
    if(a.module!=b.module||a.engine!=b.engine||a.viewport!=b.viewport||a.world!=b.world||a.mode!=b.mode||
       a.gameInstance!=b.gameInstance||a.players!=b.players||a.player!=b.player||a.controller!=b.controller||
       a.hud!=b.hud||a.stack!=b.stack||!a.thread||a.thread!=b.thread)return false;
    for(const auto i:{0U,1U,3U})if(!a.identities[i].HasAssignedSerial()||!(a.identities[i]==b.identities[i]))return false;
    return b.identities[2].HasAssignedSerial()&&b.identities[2].object==b.widget;
}
inline bool SameWidget(const active::Context& a,const active::Context& b) noexcept {
    return Infrastructure(a,b)&&a.widget==b.widget&&a.type==b.type&&a.identities[2]==b.identities[2];
}
}
