#pragma once
#include "GameplayLimits.hpp"
namespace cfgpopulation {
// Donor parity: keep the normal seven-total cohort on RestartPlayer, then
// place up to three extras through the existing authored-start/nav pipeline.
// This is our construction budget, not a claimed engine-wide player limit.
constexpr unsigned NativeHelpers=6;
struct Plan {unsigned native=0,deferred=0;bool valid=false;};
constexpr Plan Make(unsigned helpers){
    return helpers>cfglimits::MaximumHelpers?Plan{}:
        Plan{helpers<NativeHelpers?helpers:NativeHelpers,
            helpers>NativeHelpers?helpers-NativeHelpers:0,true};
}
// Must be consumed once: a failed human birth never starts bots, and a partial
// native population never repeats previously allocated actors on reentry.
template<class Host> bool Birth(Host& host,unsigned helpers){
    const auto cohort=Make(helpers);
    return cohort.valid&&host.Human()&&host.Population(cohort.native);
}
}
