#pragma once
#include "DonorJasonTargeting.hpp"
#include "DonorNavigationYield.hpp"
#include "ChaseLead.hpp"
#include <cmath>
#include <cstdint>
#include <type_traits>
#include <utility>
#ifndef CFG_H245_STUCK
#define CFG_H245_STUCK 1
#endif
#ifndef CFG_H248_STUCK_BUSY
#define CFG_H248_STUCK_BUSY 1
#endif
#ifndef CFG_H250_DOOR_ESCAPE
#define CFG_H250_DOOR_ESCAPE 1
#endif

namespace cfgdonorhunt {
// H2.48c: a Jason standing still in a grab/kill montage is not stuck (live: the
// stuck failsafe teleported Jason out of a kneeling kill). Hosts that expose the
// donor busy predicate (IsJasonBridgeCombatBusy) veto stuck counting while busy.
template<class H,class=void> struct HasCombatBusy:std::false_type{};
template<class H> struct HasCombatBusy<H,std::void_t<decltype(std::declval<H&>().IsJasonBridgeCombatBusy(std::uint64_t{},false,nullptr))>>:std::true_type{};
template<class H> bool StuckCheckBusy(H& host,std::uint64_t now){
    if constexpr(CFG_H248_STUCK_BUSY&&HasCombatBusy<H>::value)return host.IsJasonBridgeCombatBusy(now,false,nullptr);
    else{(void)host;(void)now;return false;}
}
// rc1 FEAT RunJasonAIChaseUpdateOnGameThread6930-7518, ACTIVE
// F13_BASE_GAME_PORT branch. Raw UE reads/calls are host proof seams; the
// Resurrected private FAIMoveRequest branch is not part of the base donor.
struct ChaseState {
    bool Active=false,InitialTeleportAttempted=false,PathLocked=false,HaveLastLocation=false;
    std::uintptr_t World=0,Jason=0,Controller=0,Target=0;
    std::uint64_t InitialTeleportAt=0,NextStuckCheckAt=0,LastAcceptedMoveAt=0,
        PathLockUntil=0,DoorTraversalGraceUntil=0,nextRunSpeedCheckAt=0,
        LastMorphTeleportAt=0,NextDistanceTeleportAt=0,nextFiftyMeterProbeAt=0,MorphCooldownMs=20000;
    float StuckThreshold=30.0f,PathLockDurationSeconds=3.0f;
    unsigned ConsecutiveStuckChecks=0,ConsecutiveNearTargetStuckDeferrals=0;
    // H2.50d: door interactions that "succeeded" at the same spot without Jason getting
    // anywhere (live: StuckDoorCalls=16, StuckFailsafeCalls=0, stuck in a doorway).
    unsigned DoorAttemptsWithoutProgress=0;Position DoorAttemptAt{};
    Position LastLocation{};
};
// FrozenJasonBridge.cpp:24542 InitializeHostedPrimaryJasonState. The hosted
// adoption input must seed the same opening/cooldown clocks before J2.19 runs;
// offline BindCurrent keeps its existing FEAT4289 two-second opening.
inline void DonorInitializeHostedPrimaryChase(ChaseState& s,std::uint64_t now){
    s.LastMorphTeleportAt=now;
    s.NextDistanceTeleportAt=now+s.MorphCooldownMs;
    s.InitialTeleportAt=now+s.MorphCooldownMs;
    s.NextStuckCheckAt=now+1375;
}
template<class Host>bool DonorRunJasonAIChaseUpdateOnGameThread(Host& host){
    auto& s=host.Chase();
    if(!s.Active)return false;
    const auto world=host.World();
    if(!world||world!=s.World||!s.Jason||!s.Controller||
       !host.ReadableActor(s.Jason)||!host.ReadableController(s.Controller)){
        s.Active=false;return false;
    }
    const auto now=host.Now();
    if(now>=s.nextRunSpeedCheckAt){
        s.nextRunSpeedCheckAt=now+30000;
        float walk=0,run=0;
        if(host.ReadAuthoredMovementSpeeds(walk,run)&&std::isfinite(walk)&&std::isfinite(run)&&
           walk>25.0f&&run>walk+25.0f&&run<1500.0f)host.UseAuthoredRunSpeed(run);
    }
    if(!s.InitialTeleportAttempted){
        if(now<s.InitialTeleportAt)return true;
        s.InitialTeleportAttempted=true;
        if(host.RunOfflineBotsInitialTeleportOnGameThread())host.MarkJasonAIMorphTeleportUsed();
        s.PathLocked=false;s.PathLockUntil=0;
        return true; // never MoveTo in the teleport frame
    }
    if(host.RunJasonAIDistanceTeleportOnGameThread())return true;
    if(DonorNavigationPending(host,0))return true;
    if(now>=s.NextStuckCheckAt){
        s.NextStuckCheckAt=now+1375;
        Position currentLocation{};
        if(host.GetJasonAIActorLocation(s.Jason,currentLocation)){
            if(s.HaveLastLocation){
                const float dx=currentLocation.X-s.LastLocation.X;
                const float dy=currentLocation.Y-s.LastLocation.Y;
                const float dz=currentLocation.Z-s.LastLocation.Z;
                const float distanceMoved=std::sqrt(dx*dx+dy*dy+dz*dz);
                // Donor reads PFC for logging only; no status==3 gate. Its
                // actual recovery gate is a successfully accepted recent move.
                const bool recentMoveAccepted=s.LastAcceptedMoveAt!=0&&now>=s.LastAcceptedMoveAt&&
                    now-s.LastAcceptedMoveAt<=5000;
#if CFG_H245_STUCK
                // H2.45: with no counselor target no MoveTo is issued, so the
                // 5 s accepted-move window lapses and an unfinished recovery
                // is never retried. Keep an already-confirmed stuck alive.
                const bool continueStuck=s.ConsecutiveStuckChecks>0&&!host.FindNearestJasonAICounselorTarget(s.Jason);
#else
                const bool continueStuck=false;
#endif
                if(distanceMoved<s.StuckThreshold&&(recentMoveAccepted||continueStuck))++s.ConsecutiveStuckChecks;
                else{s.ConsecutiveStuckChecks=0;s.ConsecutiveNearTargetStuckDeferrals=0;s.DoorAttemptsWithoutProgress=0;}
                if(s.ConsecutiveStuckChecks>=1){
                    const float ax=currentLocation.X-s.DoorAttemptAt.X,ay=currentLocation.Y-s.DoorAttemptAt.Y;
                    const bool sameDoorSpot=s.DoorAttemptsWithoutProgress>0&&ax*ax+ay*ay<150.0f*150.0f;
                    // H2.50d: after two door "successes" at the same spot with no progress, stop
                    // retrying the door and escalate straight to the stuck recovery (step-out/teleport).
                    const bool doorExhausted=CFG_H250_DOOR_ESCAPE&&sameDoorSpot&&s.DoorAttemptsWithoutProgress>=2;
                    if(doorExhausted){
                        s.DoorTraversalGraceUntil=0;s.ConsecutiveStuckChecks=2;s.ConsecutiveNearTargetStuckDeferrals=2;
                        s.DoorAttemptsWithoutProgress=0;
                    }
                    if(!doorExhausted&&host.RunJasonAIDoorInteractionOnGameThread()){
                        if(sameDoorSpot)++s.DoorAttemptsWithoutProgress;else{s.DoorAttemptsWithoutProgress=1;s.DoorAttemptAt=currentLocation;}
                        s.ConsecutiveStuckChecks=0;s.ConsecutiveNearTargetStuckDeferrals=0;
                        s.LastAcceptedMoveAt=0;s.HaveLastLocation=false;s.PathLocked=false;s.PathLockUntil=0;
                        s.NextStuckCheckAt=now+750;
                        return true;
                    }
                    if(now<s.DoorTraversalGraceUntil){
                        s.ConsecutiveStuckChecks=0;s.ConsecutiveNearTargetStuckDeferrals=0;
                    }else if(s.ConsecutiveStuckChecks<2){
                        // First confirmed sample preserves the accepted path.
                    }else{
                        bool recoveryEligible=false;
                        float targetDistance=FLT_MAX;
                        const auto morphReadyAt=host.JasonAIMorphReadyAt();
                        const auto recoveryTarget=host.FindNearestJasonAICounselorTarget(s.Jason);
                        Position recoveryTargetLocation{};
#if CFG_H245_STUCK
                        if(!recoveryTarget)recoveryEligible=morphReadyAt==0||now>=morphReadyAt; // no target: still step out
#endif
                        if(recoveryTarget&&host.GetJasonAIActorLocation(recoveryTarget,recoveryTargetLocation)){
                            const float tx=recoveryTargetLocation.X-currentLocation.X;
                            const float ty=recoveryTargetLocation.Y-currentLocation.Y;
                            const float tz=recoveryTargetLocation.Z-currentLocation.Z;
                            targetDistance=std::sqrt(tx*tx+ty*ty+tz*tz);
                            recoveryEligible=std::isfinite(targetDistance)&&targetDistance>=3000.0f&&
                                (morphReadyAt==0||now>=morphReadyAt);
                            if(std::isfinite(targetDistance)&&targetDistance<3000.0f&&
                               (morphReadyAt==0||now>=morphReadyAt)){
                                ++s.ConsecutiveNearTargetStuckDeferrals;
                                recoveryEligible=s.ConsecutiveNearTargetStuckDeferrals>=2;
                            }else if(targetDistance>=3000.0f)s.ConsecutiveNearTargetStuckDeferrals=0;
                        }
                        bool recovered=false;
                        // H2.48d: veto only the recovery TELEPORT while combat-busy (grab/kill montage).
                        // The H2.48c early return also suppressed the door-interaction step above,
                        // because attacking a door counts as busy (live: Jason stuck at a door).
                        if(StuckCheckBusy(host,now))recoveryEligible=false;
                        if(recoveryEligible){
                            recovered=host.TryOfflineBotsFailsafeTeleportOnGameThread(currentLocation);
                            if(!recovered)recovered=host.RunOfflineBotsInitialTeleportOnGameThread();
                        }
                        if(DonorNavigationPending(host,0))return true;
                        if(recovered){
                            s.ConsecutiveStuckChecks=0;s.ConsecutiveNearTargetStuckDeferrals=0;
                            s.LastAcceptedMoveAt=0;s.HaveLastLocation=false;s.PathLocked=false;s.PathLockUntil=0;
                            return true;
                        }
                        s.ConsecutiveStuckChecks=1;s.NextStuckCheckAt=now+4750;
                    }
                }
            }
            s.LastLocation=currentLocation;s.HaveLastLocation=true;
        }
    }
    // H2.86: while bursting, a 6 s path lock leaves Jason at a stale point; re-aim every 500 ms.
    const bool bursting=DonorChaseBursting(host,0);
    if(s.PathLocked){if(now<s.PathLockUntil&&!(bursting&&s.PathLockUntil>now+cfgchaselead::BurstRefreshMs))return true;s.PathLocked=false;}
    const auto previousTarget=s.Target;
    const auto target=host.FindNearestJasonAICounselorTarget(s.Jason);
    if(!target)return true;
    s.Target=target;
    if(!bursting&&target==previousTarget&&s.LastAcceptedMoveAt!=0&&now>=s.LastAcceptedMoveAt&&
       now<s.LastAcceptedMoveAt+6107){
        s.PathLocked=true;s.PathLockUntil=s.LastAcceptedMoveAt+6107;return true;
    }
    s.PathLocked=true;
    s.PathLockUntil=now+(bursting?cfgchaselead::BurstRefreshMs:static_cast<std::uint64_t>(s.PathLockDurationSeconds*1000.0f));
    host.IssueStockReflectedJasonMoveOnGameThread(target);
    return true;
}
}
