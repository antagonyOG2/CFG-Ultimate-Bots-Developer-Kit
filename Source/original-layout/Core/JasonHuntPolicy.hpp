#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
// Jason "better killer" pursuit policy (original design; ideas only: enemy
// tracking, target prediction and loop counter-play as in common killer-bot
// architectures). Pure arithmetic over positions the hunt lane ALREADY
// observes for its distance check: no reads, native calls or extra per-frame
// work. The runtime still owns every proof and reuses the existing verified
// MoveToLocation binding; this only proposes a goal point.
namespace cfghunt {
struct V3 {float x=0,y=0,z=0;};
inline bool Finite(const V3& v){return std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.z);}
inline float Flat2(const V3& a,const V3& b){const float x=a.x-b.x,y=a.y-b.y;return x*x+y*y;}

// Per logical target slot: last observed position and a smoothed velocity.
// Teleports, vehicles and stale samples reset instead of extrapolating.
class TargetTracker {
public:
    static constexpr unsigned Slots=16;
    static constexpr float MaximumFootSpeed=1100.f; // cm/s; above this = vehicle/teleport, reset
    struct Track {std::uintptr_t actor=0;V3 position{},velocity{};std::uint64_t at=0;bool moving=false;unsigned samples=0;};
    void Observe(unsigned slot,std::uintptr_t actor,const V3& p,std::uint64_t now){
        if(slot>=Slots||!actor||!Finite(p))return;
        auto& t=tracks_[slot];
        if(t.actor!=actor||!t.at||now<=t.at||now-t.at>1000){t={};t.actor=actor;t.position=p;t.at=now;t.samples=1;return;}
        const auto dtMs=now-t.at;if(dtMs<60)return; // too close in time to measure, keep earlier anchor
        const float dt=static_cast<float>(dtMs)/1000.f;
        const V3 v{(p.x-t.position.x)/dt,(p.y-t.position.y)/dt,0.f};
        const float speed=std::sqrt(v.x*v.x+v.y*v.y);
        if(!std::isfinite(speed)||speed>MaximumFootSpeed){t={};t.actor=actor;t.position=p;t.at=now;t.samples=1;return;}
        const float a=t.samples>1?.5f:1.f;
        t.velocity={t.velocity.x+(v.x-t.velocity.x)*a,t.velocity.y+(v.y-t.velocity.y)*a,0.f};
        t.position=p;t.at=now;++t.samples;
        t.moving=Speed(t)>=180.f;
    }
    const Track* Current(unsigned slot,std::uintptr_t actor,std::uint64_t now)const{
        if(slot>=Slots)return nullptr;const auto& t=tracks_[slot];
        return t.actor==actor&&t.at&&now>=t.at&&now-t.at<=600&&t.samples>=3?&t:nullptr;
    }
    static float Speed(const Track& t){return std::sqrt(t.velocity.x*t.velocity.x+t.velocity.y*t.velocity.y);}
    void Clear(){tracks_={};}
private:
    std::array<Track,Slots> tracks_{};
};

// Loop/kite detection: Jason chasing at mid range without closing distance
// (typical cabin or car loop). While stalled, predict further ahead so the
// path cuts across the loop instead of trailing the counselor around it.
class ChaseProgress {
    std::uintptr_t target_=0;std::uint64_t since_=0;float best_=0;
public:
    static constexpr std::uint64_t Window=5000;
    void Observe(std::uintptr_t target,float squared,std::uint64_t now){
        if(!std::isfinite(squared)||squared<0)return;
        const float d=std::sqrt(squared);
        if(target!=target_||!since_||d>3000.f){target_=target;since_=now;best_=d;return;}
        if(d<best_-150.f){best_=d;since_=now;} // meaningful closing resets the window
    }
    bool Stalled(std::uintptr_t target,std::uint64_t now)const{return target==target_&&since_&&now>=since_+Window;}
    std::uint64_t StalledMs(std::uintptr_t target,std::uint64_t now)const{return target==target_&&since_&&now>since_?now-since_:0;}
    void Clear(){target_=0;since_=0;best_=0;}
};

// Chase fatigue: after kiting Jason for 12 s without meaningful closing, the
// counselor is deprioritized for 15 s so he hunts a reachable one instead of
// circling a loop forever (and does not flip straight back).
struct ChaseFatigue {
    static constexpr std::uint64_t After=12000,Hold=15000;
    unsigned slot=~0u;std::uintptr_t actor=0;std::uint64_t until=0;
    void Observe(unsigned targetSlot,std::uintptr_t target,const ChaseProgress& progress,std::uint64_t now){
        if(target&&progress.StalledMs(target,now)>=After&&!(actor==target&&now<until)){slot=targetSlot;actor=target;until=now+Hold;}
    }
    unsigned Slot(std::uint64_t now)const{return actor&&now<until?slot:~0u;}
    void Clear(){*this={};}
};

// Intercept goal: lead a moving counselor at mid range. Close range keeps the
// native MoveToActor follow (melee/grab own it); vehicles keep car pursuit.
struct Intercept {
    static constexpr float Near=700.f,Far=4000.f,MinimumOffset=150.f,MaximumOffset=1400.f;
    static bool Plan(const V3& jason,const TargetTracker::Track& t,bool stalled,V3& goal){
        if(!Finite(jason)||!Finite(t.position)||!t.moving)return false;
        const float d=std::sqrt(Flat2(jason,t.position));
        if(!std::isfinite(d)||d<Near||d>Far)return false;
        const float speed=TargetTracker::Speed(t);if(speed<180.f)return false;
        // Lead grows with distance; a stalled chase leads further to cut a loop.
        float lead=std::clamp(d/1000.f*.55f,.35f,1.4f);
        if(stalled)lead=std::min(lead+.9f,2.2f);
        V3 ahead{t.velocity.x*lead,t.velocity.y*lead,0.f};
        const float offset=std::sqrt(ahead.x*ahead.x+ahead.y*ahead.y);
        if(!std::isfinite(offset)||offset<MinimumOffset)return false;
        if(offset>MaximumOffset){const float s=MaximumOffset/offset;ahead.x*=s;ahead.y*=s;}
        // Never lead to a point farther from Jason than the target plus the
        // offset would justify (target running straight at Jason => no lead).
        const V3 candidate{t.position.x+ahead.x,t.position.y+ahead.y,t.position.z};
        if(std::sqrt(Flat2(jason,candidate))>d+MaximumOffset*.5f)return false;
        goal=candidate;return Finite(goal);
    }
};
}
