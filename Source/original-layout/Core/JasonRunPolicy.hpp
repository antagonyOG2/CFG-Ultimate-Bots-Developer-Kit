#pragma once
#include <cmath>
#include <cstdint>
#include "PursuitSpeedPolicy.hpp"
namespace cfgrun {
// User-requested total pursuit speed, for counselor and vehicle chases alike.
// Not an extra difficulty multiplier and not a change to native Shift.
constexpr float PursuitMultiplier=cfgpursuitspeed::Multiplier;
// Explicit all-variant pursuit override requested by the user. The donor's
// authored-only policy deliberately leaves non-running variants walking.
constexpr float MinimumPursuitRun=450.f;
// Authored walk/run validation retained from EnableStockJasonRunSpeed. The
// explicit pursuit multiplier is applied once to that authored run baseline.
inline bool Desired(float walk,float run,float& desired){
    return cfgpursuitspeed::AuthoredRun(walk,run,desired);
}
inline bool DesiredScaled(float walk,float run,float multiplier,float& desired){
    return cfgpursuitspeed::ScaledRun(walk,run,multiplier,desired);
}
struct Policy {
    std::uint64_t next=0,stateAt=0;
    unsigned writes=0,refused=0;
    template<class H>static auto State(H& h,int)->decltype(h.RunState(),bool()){return h.RunState();}
    template<class H>static bool State(H&,long){return false;}
    template<class H>static auto Auxiliary(H& h,float speed,int)->decltype(h.RunAuxiliary(speed),bool()){return h.RunAuxiliary(speed);}
    template<class H>static bool Auxiliary(H&,float,long){return false;}
    template<class Host>bool Step(Host& host,std::uint64_t now,bool carOwned,float multiplier=1.f){
        if(carOwned)return false;
        // Native interactions can release Running independently of speed limits.
        // Reassert promptly when safe, without changing burst/car speed leases.
        if(now>=stateAt){stateAt=now+250;if(State(host,0))return true;}
        if(now<next)return false;
        // Native actions may restore Walk while leaving Running set. Validate
        // promptly, but never inside a temporary foot/car speed lease.
        next=now+1000;
        float walk=0,run=0,desired=0;
        if(!host.RunObserve(walk,run)||!DesiredScaled(walk,run,multiplier,desired)){++refused;return true;}
        // Repair one native run/sprint/slow-run field per independent turn.
        // Updating Walk alone does not fix a native Running path that chooses
        // one of those other movement limits. Never overlap a speed lease.
        if(Auxiliary(host,desired,0)){next=now+100;return true;}
        if(desired!=walk){if(host.RunWrite(walk,desired))++writes;else ++refused;return true;}
        return false; // a read-only no-op must not replace the pursuit turn
    }
};
}
