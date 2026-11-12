#pragma once
#include "NativeCounselorRoutePolicy.hpp"
namespace cfgmenu::acceptedcommit {
inline bool Commit(bool validatedRow,counselorroute::State before,counselorroute::State after) noexcept {
    using S=counselorroute::State;
    // The coordinator can admit first counselor arrival inside actual Accept,
    // before its next paced HUD callback. Native synchronous map arrival may
    // likewise advance the route before the outer Accept returns.
    return validatedRow&&(before==S::AwaitCounselor||before==S::CounselorReady)&&
        (after==S::AwaitMap||after==S::MapReady||after==S::SettingsReady);
}
}
