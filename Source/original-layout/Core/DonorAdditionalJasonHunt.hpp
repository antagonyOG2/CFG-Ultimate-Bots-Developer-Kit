#pragma once
#include "DonorJasonTargeting.hpp"
#include "DonorNavigationYield.hpp"
#include "ChaseLead.hpp"
#include <algorithm>
namespace cfgdonorhunt {
enum class MatchPhase {Unknown,InProgress,NotInProgress};
struct AdditionalHuntState {
    std::uintptr_t World=0,Jason=0,Controller=0,Target=0,KnifeTarget=0;
    bool FinalKillStopped=false,KnifeActive=false,AttackPressed=false,HasProgressSample=false;
    std::uint64_t HidingInteractionUntil=0,NextHidingAt=0,AttackReleaseAt=0,
        DoorInteractionUntil=0,DoorNextEarlyProbeAt=0,NextMoveAt=0,NextProgressAt=0,
        NextMorphAt=0,NextGrabAt=0,NextCombatAt=0;
    unsigned StalledSamples=0;
    Position LastProgressLocation{};
    std::uint64_t DoorNextAttemptAt=0,DoorSameRetryAt=0;std::uintptr_t DoorSampleTarget=0,DoorLast=0;
    unsigned DoorBlockedSamples=0;bool DoorHasSample=false;Position DoorLastLocation{};
    std::uint64_t NextKnifeAt=0,KnifeReleaseAt=0,KnifeReleasedAt=0,KnifeEndAt=0;
    std::int32_t KnifeCountAtRelease=-1,KnifeTargetSerial=0;std::uintptr_t KnifeDriverAtRelease=0;bool KnifeReleased=false,KnifeLaunched=false;
    std::uintptr_t OccupiedCar=0,CarSeat=0,CarOccupant=0;
    std::uint64_t NextCarActionAt=0,NextCarMorphAt=0,NextCarSpeedAt=0,HoodInputAt=0,DriverReadySince=0,LastExtractionAt=0;
    float CarForwardSpeed=0;bool HaveCachedDriverDoorPoint=false;
    Position CachedDriverDoorPoint{};
    std::uint64_t NextSwimAt=0,HiderBlockedUntil=0;float HiderLastDistance=-1.0f;unsigned HiderStalls=0;
};
// FJB21398-21743 FrozenBridgeDriveAdditionalJason, ACTIVE pointer-only root.
// Owner lookup precedes this body in the native adapter. The primary Jason
// globals are NEVER swapped to the reinforcement. Diagnostics-only trace
// formatting is omitted; every gameplay gate/timer/order is retained.
template<class Host>bool DonorFrozenBridgeDriveAdditionalJason(Host& host){
    auto& s=host.Additional();const auto controller=host.Controller();
    if(!controller||controller!=s.Controller)return false;
    const auto world=host.World();
    if(!world||s.World!=world||!s.Jason||!host.ReadableActor(s.Jason)||!host.ReadableController(controller)){
        s={};return false;
    }
    const auto now=host.Now();const bool hosted=host.Hosted();
    if(hosted&&host.EngineWorld()!=world)return true;
    const auto phase=host.ReadCounselorRouteMatchPhase();
    if(phase==MatchPhase::NotInProgress||(hosted&&phase==MatchPhase::Unknown)){
        if(!s.FinalKillStopped){host.StopMovement();s.FinalKillStopped=true;}
        if(!hosted&&!host.NativeJasonDeathCinematicObserved())host.BindNativeCurrentPrimaryKiller();
        return true;
    }
    host.RunBaseAIControllerTick();
    if((!hosted||host.IsHostedAuthorityWorld())&&
       (host.IsPamelaExclusiveKillLaneActive(now)||host.PermanentHumanSweaterActivationActive(now))){
        if(!s.FinalKillStopped){host.StopMovement();s.FinalKillStopped=true;}
        return true;
    }
    s.FinalKillStopped=false;
    if(host.DriveAdditionalJasonHeldGrab(now))return true;
    if(s.KnifeActive&&host.DriveAdditionalJasonKnife(s.KnifeTarget,0.0f,now))return true;
    if(host.DriveAdditionalJasonOccupiedCar(now))return true;
    if(now<s.HidingInteractionUntil)return true;
    if(now>=s.NextHidingAt){
        s.NextHidingAt=now+450;
        std::uintptr_t spot=0,hidden=0,interactable=0;
        if(host.FindNearbyOccupiedHidingSpotForJason(s.Jason,spot,hidden,interactable)){
            Position jasonAt{},spotAt{};
            if(host.GetJasonAIActorLocation(s.Jason,jasonAt)&&host.GetJasonAIActorLocation(spot,spotAt)){
                const float dx=spotAt.X-jasonAt.X,dy=spotAt.Y-jasonAt.Y;
                if(dx*dx+dy*dy>170.0f*170.0f){if(!hosted)host.EnableAdditionalJasonAuthoredRunSpeed(now);host.IssueMove(spotAt,110.0f);s.Target=hidden;s.NextMoveAt=now+700;return true;} // the ordinary chase must not replace this with an unreachable path to the person inside
                host.StopMovement();
                if(host.AttemptJasonHidingSpotComponentForJason(s.Jason,interactable))s.HidingInteractionUntil=now+2000;
                return true;
            }
        }
    }
    if(s.AttackPressed&&now>=s.AttackReleaseAt){
        if(host.ModuleReady()&&host.AttackReleaseReadable())host.ReleaseAttack();
        s.AttackPressed=false;s.AttackReleaseAt=0;
    }
    // Actual donor: prefer an alternative within80m horizontally, otherwise
    // fall back to *all* counselors, including the local human/Tommy.
    const auto local=hosted?0:host.LocalCounselor();
    auto target=host.FindNearestJasonAICounselorTarget(s.Jason,local);
    Position here{},there{};
    if(target&&host.GetJasonAIActorLocation(s.Jason,here)&&host.GetJasonAIActorLocation(target,there)){
        const float dx=there.X-here.X,dy=there.Y-here.Y;
        if(dx*dx+dy*dy>8000.0f*8000.0f)target=0;
    }else target=0;
    if(!target)target=host.FindNearestJasonAICounselorTarget(s.Jason,0);
    if(!target){if(s.Target)host.StopMovement();s.Target=0;return true;}
    if(!host.GetJasonAIActorLocation(s.Jason,here)||!host.GetJasonAIActorLocation(target,there))return true;
    const float dx=there.X-here.X,dy=there.Y-here.Y,dz=there.Z-here.Z;
    const float squared=dx*dx+dy*dy+dz*dz;
    if(!std::isfinite(squared))return true;
    if(!hosted)host.EnableAdditionalJasonAuthoredRunSpeed(now);
    if(now<s.DoorInteractionUntil)return true;
    if(host.DriveAdditionalJasonSwim(target,now))return true;
    if(!hosted&&phase==MatchPhase::InProgress&&!s.AttackPressed&&!s.KnifeActive&&now>=s.DoorNextEarlyProbeAt){
        s.DoorNextEarlyProbeAt=now+250;
        if(host.TryAdditionalJasonNativeDoor(target,here,there,now,true)){s.NextMoveAt=s.DoorInteractionUntil;return true;}
    }
    if(host.DriveAdditionalJasonKnife(target,std::sqrt(squared),now))return true;
    if(now>=s.NextMoveAt&&host.TryAdditionalJasonNativeDoor(target,here,there,now,false)){
        s.NextMoveAt=s.DoorInteractionUntil;return true;
    }
    if(now>=s.NextProgressAt){
        if(s.HasProgressSample&&squared>1800.0f*1800.0f){
            const float mx=here.X-s.LastProgressLocation.X,my=here.Y-s.LastProgressLocation.Y;
            s.StalledSamples=mx*mx+my*my<100.0f*100.0f?(std::min)(s.StalledSamples+1,3u):0;
        }else s.StalledSamples=0;
        s.LastProgressLocation=here;s.HasProgressSample=true;s.NextProgressAt=now+4000;
    }
    if(squared>5000.0f*5000.0f&&now>=s.NextMorphAt&&!s.AttackPressed){
        s.NextMorphAt=now+6000;
        if(host.RunJasonAINavRingTeleportOnGameThread(s.Jason,target)){
            s.NextMorphAt=now+20000;s.StalledSamples=0;s.HasProgressSample=false;s.NextMoveAt=now+1000;return true;
        }
        if(DonorNavigationPending(host,0))return true;
    }
    if(s.Target!=target||now>=s.NextMoveAt){
        auto aim=there;DonorChaseLead(host,target,aim,0); // H2.86: 2 s ahead of a running counselor while bursting
        host.IssueMove(aim,150.0f);s.Target=target;
        s.NextMoveAt=DonorChaseBursting(host,0)?now+cfgchaselead::BurstRefreshMs:now+1200+static_cast<std::uint64_t>(host.StateIndex()%5)*171;
    }
    if(squared<=175.0f*175.0f&&now>=s.NextGrabAt&&!s.AttackPressed){
        s.NextGrabAt=now+4000;
        if(host.ModuleReady()&&host.GrabReadable()&&host.TryGrab()){s.NextCombatAt=now+700;return true;}
    }
    if(squared<=275.0f*275.0f&&now>=s.NextCombatAt&&!s.AttackPressed){
        if(host.ModuleReady()&&host.AttackPressReadable()&&host.PressAttack()){
            s.AttackPressed=true;s.AttackReleaseAt=now+120;
        }
        s.NextCombatAt=now+911+static_cast<std::uint64_t>(host.StateIndex()%4)*113;
    }
    return true;
}
}
