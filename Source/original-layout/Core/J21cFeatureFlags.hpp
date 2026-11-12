#pragma once
#include <array>
#include <cstdint>
// Each mailbox298 item can be isolated without deleting accepted code.
#ifndef CFG_J21C_HELD_RECOVERY
#define CFG_J21C_HELD_RECOVERY 1
#endif
#ifndef CFG_J21C_SHARED_CONVERTER
#define CFG_J21C_SHARED_CONVERTER 1
#endif
#ifndef CFG_J21C_READER_COUNTERS
#define CFG_J21C_READER_COUNTERS 1
#endif
#ifndef CFG_J21C_CALLBACK_CACHE
#define CFG_J21C_CALLBACK_CACHE 0
#endif
#ifndef CFG_J21C_RESIDENT_WEAPONS
#define CFG_J21C_RESIDENT_WEAPONS 0
#endif
#ifndef CFG_J21C_LEAN_SLICES
#define CFG_J21C_LEAN_SLICES 0
#endif
#ifndef CFG_J21C_LAZY_INPUT_ROOTS
#define CFG_J21C_LAZY_INPUT_ROOTS 0 // awaits separate call-site retention review
#endif
#ifndef CFG_J3_SOUND_OPTIONS
#define CFG_J3_SOUND_OPTIONS 0
#endif
#ifndef CFG_J3_MINIMAL_HUD
#define CFG_J3_MINIMAL_HUD 0
#endif
#ifndef CFG_J3_HARD_MOVEMENT
#define CFG_J3_HARD_MOVEMENT 0
#endif
#ifndef CFG_J3_END_SUMMARY
#define CFG_J3_END_SUMMARY 0
#endif
#ifndef CFG_J3_LATE_HUNT
#define CFG_J3_LATE_HUNT 0
#endif
#ifndef CFG_J3_BOT_SWEATER
#define CFG_J3_BOT_SWEATER 0
#endif
namespace cfgj21c {
struct HeldDiagnostics {
    std::array<std::uint64_t,16> observeFailures{},notEntered{};
    std::uint64_t allowedWaits=0,killingWaits=0,finalWaits=0,seatWaits=0,deadWaits=0;
    std::uint64_t staleReleases=0,fallbacks=0,queryReads=0,readBudget=0,callBudget=0;
    unsigned observeStage=0,queryStage=0,inputStage=0;
};
}
