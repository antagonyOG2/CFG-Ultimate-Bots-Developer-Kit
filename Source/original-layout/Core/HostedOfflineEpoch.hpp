#pragma once
#include <cstdint>

// H2.75: an offline match left its bundle epoch live (bundleStarted/Schedule.active, world = plan.world) and a
// hosted World has now arrived. Pure decision only; 0 = leave alone, 1 = retire the leftover epoch, 2 = leftover but blocked.
namespace cfghostedoffline {
constexpr unsigned Leave=0,Retire=1,Blocked=2;
constexpr unsigned LeftoverEpochDecision(bool enabled,bool hostedWorld,bool hostedActive,bool epochStarted,bool scheduleActive,
    std::uintptr_t bundleWorld,std::uintptr_t currentWorld,bool rootLease,bool retirable)noexcept{
    return !enabled||!hostedWorld||hostedActive||rootLease||!epochStarted||!scheduleActive||!bundleWorld||!currentWorld||bundleWorld==currentWorld?Leave:
        retirable?Retire:Blocked;
}
}
