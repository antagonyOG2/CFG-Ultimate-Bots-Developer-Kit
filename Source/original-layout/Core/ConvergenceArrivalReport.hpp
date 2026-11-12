#pragma once
#include "CounselorConvergencePolicy.hpp"
namespace cfglean {
struct ArrivalReport {
    std::uint64_t at=0,brainAt=0,weaponAt=0,nearMoves=0;
    std::uintptr_t pawn=0,controller=0,weapon=0,weaponClass=0;
    float distance=0;unsigned brainKnown=0,brainEnabled=0,weaponKnown=0,held=0,knownKeys=0;
    std::array<std::uintptr_t,cfgconvergence::KeyCount> values{};
    std::array<std::uint64_t,cfgconvergence::KeyCount> readAt{};
    // Native BT melee task entry remains UNVERIFIED until its named binding
    // is proved. Never infer task entry from proximity or blackboard writes.
};
inline bool ValidArrival(const ArrivalReport& r){
    return r.brainKnown<=1&&r.brainEnabled<=1&&r.weaponKnown<=1&&r.held<=1&&r.knownKeys<(1u<<cfgconvergence::KeyCount)&&
        (!r.held||(r.weaponKnown&&r.weapon&&r.weaponClass));
}
}
