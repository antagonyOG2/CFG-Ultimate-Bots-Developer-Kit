#pragma once
#include "J22FeatureFlags.hpp"
#include <cmath>
#include <cstdint>
// H2.81: "Jason is never frozen". Pure timing decisions for the water escape and the frozen watchdog.
// No reads, no native calls: the caller supplies observations and performs the (existing) nav-ring teleport.
namespace cfgunstick {
constexpr std::uint64_t WaterEscapeMs=8000,WaterWetTargetMs=20000,RetryMs=3000,FrozenMs=12000,RechargeMs=20000,SampleMs=500,GapResetMs=2000;
constexpr float FrozenRadiusCm=150.0f;
enum class Action:unsigned {None,NeedTarget,WaterEscape,FrozenEscape};
enum class Reason:unsigned {None,Water,WaterWetTarget,Frozen,Busy,NoTarget,Refused};
struct Rows {std::uint32_t waterEscapes=0,waterRefused=0,frozenEscapes=0,frozenRefused=0,lastReason=0;};
struct State {
    std::uint64_t waterSince=0,waterSeenAt=0,nextWaterAt=0,freeSeenAt=0,anchorAt=0,nextSampleAt=0,nextFrozenAt=0;
    float ax=0,ay=0,az=0;bool haveAnchor=false;Rows rows{};
    // H2.88: a counselor whose every ring cell failed (no walkable landing near them) is skipped for 30 s by the next escape.
    std::uintptr_t failedTarget=0;std::uint64_t failedUntil=0;std::uint32_t failedRings=0,otherTargets=0;
    std::uintptr_t pendingTarget=0;unsigned pendingPurpose=0;std::uint64_t pendingBase=0; // H2.90: unstick ring in flight (target, purpose, ringFailed baseline)
    void RingFailed(std::uintptr_t target,std::uint64_t now){if(!target)return;failedTarget=target;failedUntil=now+30000;++failedRings;}
    std::uintptr_t Avoid(std::uint64_t now)const{return CFG_H288_UNSTICK_OTHER_TARGET&&now<failedUntil?failedTarget:0;}
    // In water, no victim, match in progress. targetWet: -1 unknown, 0 dry, 1 in water.
    // A gap in observations (busy, held, a ring in flight) restarts the clock, never extends it.
    Action Water(std::uint64_t now,bool inWater,int targetWet){
        if(!inWater){waterSince=0;return Action::None;}
        if(!waterSince||now>waterSeenAt+GapResetMs)waterSince=now;
        waterSeenAt=now;
        if(now-waterSince<WaterEscapeMs||now<nextWaterAt)return Action::None;
        if(targetWet<0)return Action::NeedTarget;
        if(targetWet&&now-waterSince<WaterWetTargetMs){rows.lastReason=static_cast<std::uint32_t>(Reason::WaterWetTarget);nextWaterAt=now+SampleMs;return Action::None;}
        return Action::WaterEscape;
    }
    // Out of water, free of every owned interaction (the caller reaches this only at the donor drive point).
    // Returns true when a position sample is wanted now; keeps the observation-gap bookkeeping.
    bool SampleDue(std::uint64_t now){
        const bool gap=!freeSeenAt||now>freeSeenAt+GapResetMs;freeSeenAt=now;waterSince=0;
        if(gap)haveAnchor=false;
        return !haveAnchor||now>=nextSampleAt;
    }
    Action Frozen(std::uint64_t now,float x,float y,float z){
        if(!std::isfinite(x)||!std::isfinite(y)||!std::isfinite(z))return Action::None;
        nextSampleAt=now+SampleMs;
        const float dx=x-ax,dy=y-ay;
        if(!haveAnchor||dx*dx+dy*dy>=FrozenRadiusCm*FrozenRadiusCm){ax=x;ay=y;az=z;anchorAt=now;haveAnchor=true;return Action::None;}
        if(now-anchorAt<FrozenMs||now<nextFrozenAt)return Action::None;
        return Action::FrozenEscape;
    }
    // The ring was begun (or finished at once). Water retries every 3 s until he is out; frozen recharges 20 s and re-measures.
    void Accepted(Action what,std::uint64_t now){
        if(what==Action::WaterEscape){++rows.waterEscapes;nextWaterAt=now+RetryMs;rows.lastReason=static_cast<std::uint32_t>(Reason::Water);}
        else if(what==Action::FrozenEscape){++rows.frozenEscapes;nextFrozenAt=now+RechargeMs;haveAnchor=false;rows.lastReason=static_cast<std::uint32_t>(Reason::Frozen);}
    }
    void Refused(Action what,std::uint64_t now,Reason why){
        if(what==Action::WaterEscape){++rows.waterRefused;nextWaterAt=now+RetryMs;}
        else if(what==Action::FrozenEscape){++rows.frozenRefused;nextFrozenAt=now+RetryMs;}
        rows.lastReason=static_cast<std::uint32_t>(why);
    }
};
}
