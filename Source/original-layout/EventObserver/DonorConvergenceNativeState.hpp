#pragma once
#include "../Core/DonorCounselorConvergence.hpp"
// Value-only identities. No callback roots or native-call permits are saved.
struct DonorConvergenceBot {
    cfgprobe::lifetime::SlotObservation pawn{},controller{},brain{},blackboard{};
    cfgconvergence::BlackboardSchema schema{};
    std::array<std::uintptr_t,cfgconvergence::KeyCount> objects{};
    std::array<bool,cfgconvergence::KeyCount> booleans{};
    unsigned pending=0,uncertainKeys=0;bool brainOwned=false,grantAmbiguous=false;
};
struct DonorConvergenceNativeState {
    cfgdonorconvergence::State donor{};
    cfgprobe::lifetime::SlotObservation world{};
    std::array<DonorConvergenceBot,64> bots{};
    unsigned publishCursor=0;bool grantThisTurn=false;unsigned grantStage=0;
    std::uint64_t warmAt=0,publishAt=0;
    std::uint64_t turns=0,grants=0,ambiguousGrants=0,deferrals=0,publications=0,brainResumes=0,brainLost=0;
    std::uint64_t uncertainPublications=0;
};
