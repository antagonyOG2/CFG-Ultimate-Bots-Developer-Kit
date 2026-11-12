#pragma once
#include "NativeSpawnParameters.hpp"
namespace cfgdoor {
inline bool Approach(cfgspawn::Vec3 here,cfgspawn::Vec3 anchor,cfgspawn::Vec3& goal){
    goal={};if(!cfgspawn::Finite(here)||!cfgspawn::Finite(anchor)||std::fabs(here.z-anchor.z)>250.f)return false;
    const float x=anchor.x-here.x,y=anchor.y-here.y,d=std::sqrt(x*x+y*y);
    if(!std::isfinite(d)||d<=180.f||d>500.f)return false;
    // Stay100cm on the CURRENT side. Never target a point beyond the closed
    // door or assume a returned interaction at250cm has begun an animation.
    const float scale=(d-100.f)/d;goal={here.x+x*scale,here.y+y*scale,here.z};return true;
}
enum class Result { None, Unavailable, Acted, Fatal };
struct InteractPacket {
    std::uintptr_t component=0;
    std::uint8_t method=1,padding[7]{}; // exact native EILLInteractMethod::Press
};
static_assert(sizeof(InteractPacket)==16&&offsetof(InteractPacket,method)==8);
struct Recovery {
    cfgspawn::Vec3 last{};
    bool have=false,blocked=false,breaking=false;
    unsigned phase=0;std::uintptr_t door=0;
    std::uint64_t sampleAt=0,next=0,traversalUntil=0,breakStartedAt=0,decisionStartedAt=0;
    std::uintptr_t lastAttemptDoor=0;std::uint64_t sameDoorUntil=0;
    bool DecisionExpired(std::uint64_t now)const{return phase&&!breaking&&now>=decisionStartedAt&&now-decisionStartedAt>=3000;}
    void YieldDecision(std::uint64_t now){phase=0;door=0;decisionStartedAt=0;next=now+3000;}
    bool ContinuationDue(std::uint64_t now)const{return (phase||breaking)&&now>=next;}
    void NativeBreak(std::uintptr_t selected,std::uint64_t now){door=selected;phase=0;breaking=true;breakStartedAt=now;next=now+160;}
    void ObserveBreak(std::uint64_t now){next=now+160;} // observation, NOT repeated attack input
    void EndBreak(std::uint64_t now,bool passable){breaking=false;phase=0;door=0;breakStartedAt=0;next=now+1400;if(passable)Traversal(now);}
    bool SampleDue(std::uint64_t now)const{return now>=sampleAt;}
    bool Sample(std::uint64_t now,cfgspawn::Vec3 position,float targetDistance){
        if(!SampleDue(now)||!cfgspawn::Finite(position)||!std::isfinite(targetDistance))return false;
        sampleAt=now+2000;
        const auto dx=position.x-last.x,dy=position.y-last.y,dz=position.z-last.z;
        // Donor TryStockBlockedDoor: a target just behind a closed cabin door
        // can be only 1-2 metres away; small path shuffles are still blocked.
        blocked=have&&targetDistance>100.f*100.f&&dx*dx+dy*dy<=80.f*80.f&&std::fabs(dz)<=80.f;
        last=position;have=true;return true;
    }
    bool Due(std::uint64_t now)const{return blocked&&now>=next;}
    bool SelectedDue(std::uint64_t now,std::uintptr_t selected,float distance){
        if(!selected||!std::isfinite(distance)||distance>500.f*500.f){phase=0;door=0;return false;}
        // Donor waits8s between pulses and30s before the same door. A returned
        // opening request is not progress and must not restart StopMovement
        // every1.4s, starving pursuit of its movement opportunity.
        if(selected==lastAttemptDoor&&now<sameDoorUntil)return false;
        if(door&&door!=selected){phase=0;door=0;}
        if(phase)return now>=next;
        // A proved closed, native-selected door in reach is decisive evidence;
        // back-and-forth path shuffles need not first become stationary.
        return now>=next&&(blocked||distance<=275.f*275.f);
    }
    void Stopped(std::uintptr_t selected,std::uint64_t now){door=selected;phase=1;decisionStartedAt=now;next=now+25;}
    void Faced(std::uint64_t now){phase=2;next=now+100;}
    void Attempted(std::uint64_t now){lastAttemptDoor=door;sameDoorUntil=now+30000;phase=0;next=now+8000;}
    // Donor early-probe retry: return alone is not open/breakdown progress.
    // Caller must freshly prove no native target/owner before this short retry.
    void Unconfirmed(std::uint64_t now){if(!breaking){sameDoorUntil=now+2000;next=now+2000;}}
    void Traversal(std::uint64_t now){phase=0;breaking=false;door=0;breakStartedAt=0;next=now+3000;traversalUntil=next;blocked=false;have=false;}
};
}
