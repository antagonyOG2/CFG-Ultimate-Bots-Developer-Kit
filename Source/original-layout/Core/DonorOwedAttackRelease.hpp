#pragma once
#include "DonorJasonNavContinuation.hpp"
#include "DonorJasonCombat.hpp"
namespace cfgdonorhunt {
enum class ReleaseDrain {NothingOwed,Waiting,Deferred,Returned,Uncertain};
// Mandatory3.5 admission adaptation for BOTH donor attack-state layouts.
// Call before roster/nav/chase can consume the callback budget. A pre-entry
// refusal keeps the owed release; an uncertain entered call is consumed and
// quarantines new work in the caller, never blindly replayed.
template<class Host,class AttackState>ReleaseDrain DonorDrainOwedAttackRelease(
    Host& host,AttackState& state,std::uint64_t now){
    if(!state.AttackPressed)return ReleaseDrain::NothingOwed;
    if(now<state.AttackReleaseAt)return ReleaseDrain::Waiting;
    const auto outcome=host.ReleaseAttackOutcome();
    if(outcome==NativeOutcome::NotEntered)return ReleaseDrain::Deferred;
    state.AttackPressed=false;state.AttackReleaseAt=0;
    return outcome==NativeOutcome::ReturnedTrue?ReleaseDrain::Returned:ReleaseDrain::Uncertain;
}
}
