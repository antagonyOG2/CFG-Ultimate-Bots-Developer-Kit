#pragma once
#include "DonorJasonMorph.hpp"
namespace cfgdonorhunt {
struct NonAggroStartupState {
    bool Active=false,ObjectivesReady=false,Teleported=false,AttemptSent=false,CountConsumed=false,
        PhoneApproachActive=false,TransitActive=false;
    std::uintptr_t TransitGoal=0;
    unsigned ObjectiveIndex=0,ObjectiveCount=0;
    std::uint64_t NextActionAt=0,TransitNextMoveAt=0,LastNonAggroReadyAt=0;
};
// FJB20034 PrepareNonAggroStartupTick, ACTIVE. No diagnostic whole-world
// census. Resolve only when due; retain the accepted objective path between
// placements. Final recharge waiting gets one StopMovement on entry.
template<class Host>void DonorPrepareNonAggroStartupTick(Host& host){
    auto& s=host.Startup();
    if(!s.Active||!host.Chase().Jason)return;
    const auto now=host.Now();
    if(!s.ObjectivesReady&&now>=s.NextActionAt)host.ResolveCounselorRouteStartupObjectives(host.World());
    const auto readyAt=host.JasonAIMorphReadyAt();
    const bool between=!s.Teleported&&!s.AttemptSent&&!s.CountConsumed&&!s.PhoneApproachActive;
    const bool visited=s.ObjectivesReady&&s.ObjectiveIndex>=s.ObjectiveCount;
    const bool empty=s.ObjectivesReady&&host.ReadJasonAITrapCount(host.Chase().Jason)==0;
    const bool finalWait=between&&(visited||empty);
    if(finalWait&&readyAt!=0&&now<readyAt){
        if(s.LastNonAggroReadyAt!=readyAt)host.StopJasonAIMovementForKnifeOnGameThread();
        s.TransitActive=false;s.TransitGoal=0;s.TransitNextMoveAt=0;
        if(s.NextActionAt<readyAt)s.NextActionAt=readyAt;
        if(s.LastNonAggroReadyAt!=readyAt)s.LastNonAggroReadyAt=readyAt;
    }
    if(!finalWait||readyAt==0||now<readyAt)return;
    if(visited||empty){
        host.StopJasonAIMovementForKnifeOnGameThread();
        host.FinishJasonAIStartupTrapSetupOnGameThread(visited);
    }
}
}
