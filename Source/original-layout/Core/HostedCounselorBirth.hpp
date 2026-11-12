#pragma once
#include <cstdint>
namespace cfghostedfill {
enum class Result:unsigned {None,Refused,Completed,Quarantined};
enum class BrainRetry:unsigned {Cancel,Wait,Dispatch};
struct BrainRetryLocalIdentity {
    std::uintptr_t world=0,controller=0,pawn=0;
    std::int32_t controllerIndex=-1,controllerSerial=0,pawnIndex=-1,pawnSerial=0;
};
inline bool MatchBrainRetryBirth(std::uintptr_t expectedWorld,std::uintptr_t expectedController,std::uintptr_t expectedState,
    std::uintptr_t expectedPawn,std::uintptr_t currentWorld,std::uintptr_t currentController,std::uintptr_t currentState,
    std::uintptr_t currentPawn){
    return expectedWorld&&expectedController&&expectedState&&expectedPawn&&expectedWorld==currentWorld&&
        expectedController==currentController&&expectedState==currentState&&expectedPawn==currentPawn;
}
inline bool RetireBrainTicket(bool changedWorldProven,bool localPlanProven,bool localPlanMatches){
    return changedWorldProven||(localPlanProven&&!localPlanMatches);
}
inline bool MatchBrainRetryLocal(const BrainRetryLocalIdentity& expected,std::uintptr_t currentWorld,std::uintptr_t actor,
    std::uintptr_t controller,std::int32_t controllerIndex,std::int32_t controllerSerial,
    std::uintptr_t pawn,std::int32_t pawnIndex,std::int32_t pawnSerial){
    return expected.world&&expected.controller&&expected.pawn&&expected.controllerSerial>0&&expected.pawnSerial>0&&
        expected.world==currentWorld&&expected.pawn==actor&&expected.controller==controller&&expected.pawn==pawn&&
        expected.controllerIndex>=0&&expected.controllerIndex==controllerIndex&&expected.controllerSerial==controllerSerial&&
        expected.pawnIndex>=0&&expected.pawnIndex==pawnIndex&&expected.pawnSerial==pawnSerial;
}
inline BrainRetry DecideBrainRetry(bool tupleCurrent,bool slotsReady,bool dispatchAttempted,unsigned retries){
    if(!tupleCurrent||dispatchAttempted||retries>=3)return BrainRetry::Cancel;
    return slotsReady?BrainRetry::Dispatch:BrainRetry::Wait;
}
// A serial-zero birth slot is promotable only after the runtime has completed
// its independent current-world/type/role/roster/backlink proof. Preserve the
// exact object and index observed at birth; never accept a different slot.
inline bool PromoteUnassignedSlot(std::uintptr_t bornObject,std::int32_t bornIndex,std::int32_t bornSerial,
    std::uintptr_t currentObject,std::int32_t currentIndex,std::int32_t currentSerial,bool corroborated){
    return corroborated&&bornObject&&bornObject==currentObject&&bornIndex>=0&&bornIndex==currentIndex&&
        bornSerial==0&&currentSerial>0;
}
// One explicit press, one birth. The adapter owns current-host/retention/roster
// proof around every allocating operation; no retries after native entry.
template<class Host>Result AddOne(Host& h,unsigned& reason){
    reason=1;if(!h.Preflight())return Result::Refused;
    reason=2;if(!h.Spawn())return h.Clean()?Result::Refused:Result::Quarantined;
    const auto fail=[&](){return h.Rollback()?Result::Refused:Result::Quarantined;};
    reason=3;if(!h.NewController())return fail();
    reason=4;if(!h.SetClass())return fail();
    reason=5;if(!h.Restart())return fail();
    reason=6;if(!h.Traveled())return fail();
    reason=7;if(!h.Account())return fail();
    reason=8;if(!h.Postconditions())return fail();
    reason=0;return Result::Completed;
}
}
