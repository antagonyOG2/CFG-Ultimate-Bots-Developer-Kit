#pragma once
#include "DonorJasonTargeting.hpp"
#include <cmath>
#include <cstdint>
namespace cfgdonorhunt {
struct CombatState {
    bool AttackPressed=false;
    std::uintptr_t Jason=0,Target=0;
    std::uint64_t AttackReleaseAt=0,GrabKillReadyAt=0,NextCombatAttemptAt=0;
    unsigned NextGrabKillDirection=0,CombatAttemptCounter=0;
};
// Active rc1 FEAT RunJasonAICombatOnGameThread9774-10317. Native inputs,
// reflection cache validation and selected SCDoor observation stay host seams.
// Owed release precedes knife, grab-kill settling, distance and door gates.
template<class Host>bool DonorRunJasonAICombatOnGameThread(Host& host){
    auto& s=host.Combat();const auto jason=s.Jason;
    if(!jason||!host.ReadableActor(jason))return false;
    const auto now=host.Now();
    if(!host.ModuleReady())return false;
    if(s.AttackPressed&&now>=s.AttackReleaseAt){
        if(host.AttackReleaseReadable())host.ReleaseAttack();
        s.AttackPressed=false;s.AttackReleaseAt=0;
    }
    if(host.TickJasonAIKnifeSequenceOnGameThread(now))return true;
    if(s.GrabKillReadyAt!=0&&now>=s.GrabKillReadyAt){
        const auto direction=s.NextGrabKillDirection&3;
        host.UseReflectedGrabKill(direction);
        s.NextGrabKillDirection=(direction+1)&3;s.GrabKillReadyAt=0;
        s.NextCombatAttemptAt=now+700;return true;
    }
    if(now<s.NextCombatAttemptAt)return true;
    const auto target=host.FindNearestJasonAICounselorTarget(jason);
    if(!target)return true;
    s.Target=target;
    Position jasonLocation{},targetLocation{};
    if(!host.GetJasonAIActorLocation(jason,jasonLocation)||
       !host.GetJasonAIActorLocation(target,targetLocation))return true;
    const float dx=targetLocation.X-jasonLocation.X,dy=targetLocation.Y-jasonLocation.Y,
        dz=targetLocation.Z-jasonLocation.Z;
    const float distance=std::sqrt(dx*dx+dy*dy+dz*dz);
    if(host.TryStartJasonAIKnifeThrowOnGameThread(target,distance,now))return true;
    if(distance>260.0f)return true;
    if(host.SelectedInteractableIsSCDoor())return true;
    const bool shouldTryGrab=distance<=220.0f&&((s.CombatAttemptCounter%3)==0);
    ++s.CombatAttemptCounter;
    if(shouldTryGrab&&host.GrabReadable()){
        s.GrabKillReadyAt=host.TryGrab()?now+300:0;
        s.NextCombatAttemptAt=now+850;return true;
    }
    if(host.AttackPressReadable()){
        if(host.PressAttack()){s.AttackPressed=true;s.AttackReleaseAt=now+120;}
    }
    s.NextCombatAttemptAt=now+1050;
    return true;
}
// H2.43: slash a counselor inside the donor's 260 uu melee window while the
// startup-trap walk is running. Donor TickJasonAIStartupMorphTransit keeps the
// same window (MeleeGrabDistance=260) and the same PressAttack/release timing;
// this variant omits knife throw and grab so a placement walk is only ever
// interrupted by the existing slash action. Returns true when a press was sent.
#ifndef CFG_H243_TRAP_SLASH
#define CFG_H243_TRAP_SLASH 1
#endif
#ifndef CFG_H245_TRAP_SLASH_FACE
#define CFG_H245_TRAP_SLASH_FACE 1
#endif
// H2.45: face the counselor (donor Features.cpp:10817) before the swing. Hosts
// exposing FaceJasonAtYaw use the shared J2.19 control+body rotation pair;
// hosts without it keep the H2.43 behaviour. A failed face never blocks the swing.
template<class H>auto TrapSlashFace(H& h,float yaw,int)->decltype(h.FaceJasonAtYaw(yaw),void()){h.FaceJasonAtYaw(yaw);}
template<class H>void TrapSlashFace(H&,float,long){}
template<class Host>bool DonorRunTrapSlash(Host& host){
    auto& s=host.Combat();const auto jason=s.Jason;
    if(!jason||!host.ReadableActor(jason)||!host.ModuleReady())return false;
    const auto now=host.Now();
    if(s.AttackPressed&&now>=s.AttackReleaseAt){
        if(host.AttackReleaseReadable())host.ReleaseAttack();
        s.AttackPressed=false;s.AttackReleaseAt=0;
    }
    if(s.AttackPressed||s.GrabKillReadyAt!=0||now<s.NextCombatAttemptAt)return false;
    const auto target=host.FindNearestJasonAICounselorTarget(jason);
    if(!target)return false;
    Position jasonLocation{},targetLocation{};
    if(!host.GetJasonAIActorLocation(jason,jasonLocation)||
       !host.GetJasonAIActorLocation(target,targetLocation))return false;
    const float dx=targetLocation.X-jasonLocation.X,dy=targetLocation.Y-jasonLocation.Y,
        dz=targetLocation.Z-jasonLocation.Z;
    if(!(std::sqrt(dx*dx+dy*dy+dz*dz)<=260.0f))return false;
    if(host.SelectedInteractableIsSCDoor())return false;
    s.Target=target;
    if(!host.AttackPressReadable())return false;
#if CFG_H245_TRAP_SLASH_FACE
    if(dx*dx+dy*dy>=1.0f)TrapSlashFace(host,std::atan2(dy,dx)*57.2957795f,0);
#endif
    const bool pressed=host.PressAttack();
    if(pressed){s.AttackPressed=true;s.AttackReleaseAt=now+120;}
    s.NextCombatAttemptAt=now+1050;
    return pressed;
}
}
