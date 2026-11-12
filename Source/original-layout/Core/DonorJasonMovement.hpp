#pragma once
#include "DonorJasonChase.hpp"
#include "../EventObserver/TrapNativeParameters.hpp"
#ifndef CFG_H245_STUCK
#define CFG_H245_STUCK 1
#endif
namespace cfgdonorhunt {
template<class Host>static auto DonorStuckTrace(Host& host,unsigned stage,int)->decltype(host.StuckRecoveryStage(stage),void()){
    host.StuckRecoveryStage(stage);
}
template<class Host>static void DonorStuckTrace(Host&,unsigned,long){}
// FJB15002 ACTIVE public reflected helper. The general on-foot F1 move
// permits strafing; exact car centerline disables all four optional flags.
// This differs deliberately from FEAT6851's primary stock strafe=false.
template<class Host>bool DonorIssueAIMoveToLocationOnGameThread(Host& host,std::uintptr_t controller,
    Position destination,float acceptance,bool exactCenterline=false){
    if(!controller||!host.ReadableControllerAndClass(controller)||!host.MoveToLocationFunction())return false;
    cfgtrap::MoveLocationParameters packet{};packet.destination={destination.X,destination.Y,destination.Z};
    packet.acceptance=acceptance;packet.pathfinding=packet.projectNavigation=packet.strafe=packet.allowPartial=exactCenterline?0:1;
    return host.SafeMoveToLocation(packet)&&packet.result!=0;
}
// FEAT6851-6929 ACTIVE stock branch: MoveToLOCATION,85cm acceptance;
// stop-on-overlap/strafe false,pathfinding/project/partial true,filter null.
template<class Host>bool DonorIssueStockReflectedJasonMoveToLocationOnGameThread(Host& host,
    Position destination,float acceptanceRadius=85.0f){
    if(!host.ReadableControllerAndClass(host.Chase().Controller)||!host.MoveToLocationFunction())return false;
    cfgtrap::MoveLocationParameters packet{};
    packet.destination={destination.X,destination.Y,destination.Z};packet.acceptance=acceptanceRadius;
    packet.pathfinding=1;packet.projectNavigation=1;packet.allowPartial=1;
    const bool callOK=host.SafeMoveToLocation(packet);
    if(callOK&&packet.result==2)host.Chase().LastAcceptedMoveAt=host.Now();
    return callOK&&packet.result!=0;
}
template<class Host>bool DonorIssueStockReflectedJasonMoveOnGameThread(Host& host,std::uintptr_t goal){
    if(!goal||!host.ReadableActor(goal))return false;
    Position destination{};
    if(!host.GetJasonAIActorLocation(goal,destination))return false;
    DonorChaseLead(host,goal,destination,0); // H2.86: 2 s ahead of a running counselor while bursting
    return DonorIssueStockReflectedJasonMoveToLocationOnGameThread(host,destination);
}
// FEAT6017-6385. The native host proves reflection frame offsets for the
// forward getter and K2_SetActorLocation; no guessed FHitResult layout here.
template<class Host>bool DonorTryOfflineBotsFailsafeTeleportOnGameThread(Host& host,Position current){
    DonorStuckTrace(host,1,0); // entered
    const auto jason=host.Chase().Jason;
    if(!jason||!host.World()||!host.ReadableActorAndClass(jason))return false;
    DonorStuckTrace(host,2,0); // actor/world proof passed
    Position forward{};
    if(!host.GetActorForwardVector(jason,forward))return false;
    DonorStuckTrace(host,3,0); // native forward-vector read passed
    const float length=std::sqrt(forward.X*forward.X+forward.Y*forward.Y+forward.Z*forward.Z);
    if(!std::isfinite(length)||length<0.01f)return false;
    DonorStuckTrace(host,4,0); // normalized vector is usable
    forward.X/=length;forward.Y/=length;forward.Z/=length;
    const Position candidate{current.X+forward.X*500.0f,current.Y+forward.Y*500.0f,current.Z+forward.Z*500.0f};
    Position projected{};
    bool haveProjection=host.ProjectJasonAINavPointOnGameThread(candidate,Position{100,100,10},projected);
#if CFG_H245_STUCK
    // H2.45: rc1 gives up when the point 500uu ahead has no navmesh (Jason
    // nose-first against a parked car). Same projection seam, same native
    // set-location: also try both side-steps and then straight back.
    if(!haveProjection){
        const Position options[3]={{-forward.Y,forward.X,0.0f},{forward.Y,-forward.X,0.0f},{-forward.X,-forward.Y,0.0f}};
        for(const auto& d:options){
            const Position c{current.X+d.X*500.0f,current.Y+d.Y*500.0f,current.Z};
            if(host.ProjectJasonAINavPointOnGameThread(c,Position{100,100,10},projected)){haveProjection=true;break;}
        }
    }
#endif
    if(!haveProjection)return false;
    DonorStuckTrace(host,5,0); // nav projection passed
    // NewLocation=projected,bSweep=false,bTeleport=true. Actual native return
    // decides success; there is no synthetic move-success or cooldown here.
    bool moved=false;
    const bool accepted=host.SetActorLocation(jason,projected,false,true,moved)&&moved;
    DonorStuckTrace(host,accepted?7u:6u,0); // native location accepted/refused
    return accepted;
}
}
