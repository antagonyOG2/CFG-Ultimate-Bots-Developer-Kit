#pragma once
#ifndef CFG_J35_MENU_REENTRY_FIX
#include "J24FeatureFlags.hpp"
#endif
namespace cfgmenu::maparrival {
// Native Jason does not own the counselor picker transaction. An unclaimed
// native-map observation must never drive its Arrive state machine.
inline bool ShouldArrive(bool counselorClaim)noexcept{
 return !CFG_J35_MENU_REENTRY_FIX||counselorClaim;
}
}
