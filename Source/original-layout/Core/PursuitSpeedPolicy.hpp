#pragma once
#include <cmath>
namespace cfgpursuitspeed {
constexpr float Multiplier=5.f;
inline bool AuthoredRun(float walk,float run,float& desired){
    desired=walk;
    // Offline CFG can retain a previously scaled MaxWalkSpeed while native
    // MaxRunSpeed has returned to its authored value (for example 2250/450).
    // Treat both bounded native values as candidates for pursuit speed; the
    // old run>=walk requirement silently refused this state and left Jason
    // with a running flag but walk-speed movement. Exact prior scaling is
    // still recognized by ScaledRun below, so this does not compound boosts.
    if(!std::isfinite(walk)||!std::isfinite(run)||walk<25.f||walk>2500.f||run<25.f||run>2500.f)return false;
    if(run>walk)desired=run;
    return true;
}
inline bool ScaledRun(float walk,float run,float multiplier,float& desired){
    if(multiplier!=1.f&&multiplier!=2.f&&multiplier!=Multiplier)return false;
    // Nav-walk can carry this policy's previous result. Native Run is the
    // authored baseline outside the separately guarded car-speed lease.
    const float observed=walk==run*multiplier?run:walk;
    float authored=0;if(!AuthoredRun(observed,run,authored))return false;
    desired=authored*multiplier;return std::isfinite(desired)&&desired<=2500.f*multiplier;
}
}
