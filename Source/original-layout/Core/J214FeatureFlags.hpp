#pragma once
// J2.14 integration switches. Candidate builds opt in after focused gates;
// historical build targets retain their previous behavior by default.
#ifndef CFG_J214_TRAP_PRIORITY
#define CFG_J214_TRAP_PRIORITY 0
#endif
#ifndef CFG_J214_INTRO_END_TO_END
#define CFG_J214_INTRO_END_TO_END 0
#endif
#ifndef CFG_J214_HUMAN_JASON_OPTIONS
#define CFG_J214_HUMAN_JASON_OPTIONS 0
#endif
#ifndef CFG_J36_CHALLENGE_WALLS
#define CFG_J36_CHALLENGE_WALLS 0
#endif

#include "J215FeatureFlags.hpp"
