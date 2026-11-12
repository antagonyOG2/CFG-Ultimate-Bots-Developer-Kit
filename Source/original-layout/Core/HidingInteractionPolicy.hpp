#pragma once
#include <cstdint>
namespace cfghiding {
enum class Phase:unsigned {Idle,Approach,Stop,FaceActor,Face,Attempt,Confirm,Locked,RepositionStop,Reposition,Cancel,Unlock,Recover,Retreat};
enum class LockDecision:unsigned {Continue,Owned,ForeignGrace,YieldForeign,Expire};
struct State {
    std::uintptr_t counselor=0,spot=0,component=0,ignoredCounselor=0,ignoredSpot=0;
    std::uint64_t started=0,next=0,lockedAt=0,ignoreUntil=0,attemptAt=0;
    Phase phase=Phase::Idle;unsigned attempts=0,totalAttempts=0,timeouts=0,recoveries=0;
    bool repositioned=false,repositionAttempted=false;unsigned repositionIndex=0;
    bool Active()const{return phase!=Phase::Idle;}
    bool Ignored(std::uintptr_t pawn,std::uintptr_t hiding,std::uint64_t now)const{
        return pawn==ignoredCounselor&&hiding==ignoredSpot&&now<ignoreUntil;
    }
    void Clear(){counselor=spot=component=0;started=next=lockedAt=attemptAt=0;phase=Phase::Idle;attempts=0;repositionIndex=0;repositioned=repositionAttempted=false;}
    void Begin(std::uintptr_t pawn,std::uintptr_t hiding,std::uintptr_t interact,std::uint64_t now){
        Clear();counselor=pawn;spot=hiding;component=interact;started=now;phase=Phase::Approach;
    }
    bool Expired(std::uint64_t now)const{return Active()&&now>=started&&(now-started>=14000||attempts>=6);}
    void Ignore(std::uint64_t now,bool alternative){ignoredCounselor=counselor;ignoredSpot=spot;ignoreUntil=now+(alternative?25000:7000);}
    void Abandon(std::uint64_t now,bool alternative=false){Ignore(now,alternative);
        ++timeouts;phase=Phase::Cancel;next=now;}
    bool Locked(std::uint64_t now){if(!lockedAt)lockedAt=now;return now>=started&&now-started<12000;}
    bool Recovery()const{return phase>=Phase::Cancel&&phase<=Phase::Recover;}
    LockDecision ObserveLock(std::uintptr_t locked,std::uint64_t now){
        if(locked==component&&locked)return Locked(now)?LockDecision::Owned:LockDecision::Expire;
        if(locked)return now>=started&&now-started<2000?LockDecision::ForeignGrace:LockDecision::YieldForeign;
        return Expired(now)?LockDecision::Expire:LockDecision::Continue;
    }
    bool AttemptDue(std::uint64_t now)const{return !attemptAt||(now>=attemptAt&&now-attemptAt>=850);}
    void MarkAttempt(std::uint64_t now){attemptAt=now;++attempts;++totalAttempts;phase=Phase::Confirm;next=now+650;}
};
}
