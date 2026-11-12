#pragma once
#include "DonorJasonDecisionFrame.hpp"
#include "HostedH1Report.hpp"
#include "../Core/DonorDriverLiveness.hpp"
#include "../Core/DonorTrapPriorityBridge.hpp"
#include "../Core/DonorUnstick.hpp"
// Included in the bundle's namespace after native ownership and donor types.
// All members are weak bookkeeping/value state. No saved native-call lease.
struct DonorNativeJasonRuntime {
    cfgjasontick::Owner birth{};
    cfgdonorhunt::ChaseState chase{};
    cfgdonorhunt::CombatState combat{};
    cfgdonorhunt::PursuitBoost boost{};
    cfgdonorhunt::BusyState busy{};
    cfgdonorhunt::NonAggroStartupState startup{};
    bool trapSlashTurn=false; // H2.43: trap Travel stage lets the donor tick slash a close counselor
    cfgdonorhunt::AdditionalHuntState additional{};
    cfgdonorhunt::NavRingContinuation navigation{};
    cfgdonorframe::Frame frame{};
    cfgdonorhunt::DriverLiveness health{};
    cfgj214trap::Intent trapIntent{};
    std::uint32_t moveRequests=0;
    std::uint8_t lastMoveReturn=255;
    cfgprobe::lifetime::SlotObservation movement{};
    bool driverTailLogged=false,interactionOwned=false,startupFinishPending=false;
    std::uint64_t runStateAt=0,runningLastAt=0,runningObserveAt=0;
    bool hardMovementAttempted=false;cfgprobe::lifetime::SlotObservation hardMovement{};
    std::array<float,2> hardBefore{},hardDesired{};unsigned hardOwned=0;
    cfgdonorhunt::MatchPhase preparedPhase=cfgdonorhunt::MatchPhase::Unknown;
    bool bound=false,isAdditional=false,quarantined=false,intendedYield=false,decisionTargetLive=false;
    bool heldStopPending=false;cfgjasontick::Owner heldStopOwner{}; // weak owed-stop receipt, not a call capability
    std::uint32_t externalReasonMask=0;std::uint64_t externalReasonAt=0;
    std::uint64_t decisions=0,readYields=0,nativeYields=0;
    cfgh1::StallTracker hostedStall{};std::uint64_t stallNextSampleAt=0,stallTarget=0,stallTargetAt=0;
    cfgunstick::State unstick{}; // H2.81 water/frozen backstop timers
    float stallDistance=0;std::uint32_t stallExternalMask=0,stallHeld=0;
    std::uint32_t distanceMorphResult=0,distanceMorphRefusal=0;
    void MarkUncertain(cfgdonorhunt::UncertainAction kind,std::uint64_t now){
        health.Uncertain(kind,now);quarantined=true;
        // Consume this action, not the Jason. Existing donor retry clocks
        // prevent retrying its ambiguous effect after the1000ms settle.
        if(kind==cfgdonorhunt::UncertainAction::Teleport){
            chase.LastMorphTeleportAt=now;chase.NextDistanceTeleportAt=now+chase.MorphCooldownMs;
            chase.InitialTeleportAttempted=true;additional.NextMorphAt=now+20000;
            navigation={};startupFinishPending=false;startup.Active=false;
        }
        if(kind==cfgdonorhunt::UncertainAction::Projection){navigation={};chase.NextDistanceTeleportAt=now+3000;}
        if(kind==cfgdonorhunt::UncertainAction::Slash||kind==cfgdonorhunt::UncertainAction::Release)
            combat.NextCombatAttemptAt=now+1050;
        if(kind==cfgdonorhunt::UncertainAction::Grab||kind==cfgdonorhunt::UncertainAction::GrabKill){
            combat.GrabKillReadyAt=0;combat.NextCombatAttemptAt=now+850;additional.NextGrabAt=now+4000;
        }
    }
    bool ClearSettledUncertainty(std::uint64_t now){
        if(!quarantined||!health.Settled(now))return false;
        quarantined=false;navigation={};frame.Invalidate();
        std::uintptr_t noPath=0;unsigned consecutive=0;
        cfgdonorhunt::DonorResetStuckSamplingAfterNativeInteraction(chase,now,noPath,consecutive);
        additional.Target=0;additional.NextMoveAt=now;return true;
    }
    bool QueueHostedStartup(bool& owed,bool current){
        if(!current||!owed||!bound||!birth.adoption.valid)return false;
        owed=false;startup.Active=false;startupFinishPending=true;return true;
    }
    bool SameBirth(const cfgjasontick::Owner& owner)const {
        return bound&&birth.source.module==owner.source.module&&birth.source.threadId==owner.source.threadId&&
            birth.source.objects==owner.source.objects&&birth.actor.pawn==owner.actor.pawn&&
            birth.actor.killerController==owner.actor.killerController&&birth.actor.path==owner.actor.path&&
            birth.pawnClass==owner.pawnClass&&birth.defaults==owner.defaults&&
            birth.killerBase==owner.killerBase&&birth.killerAI==owner.killerAI&&birth.pathClass==owner.pathClass;
    }
    // Caller MUST independently prove this CURRENT birth before binding.
    void BindCurrent(const cfgjasontick::Owner& owner,std::uint64_t now,bool extra){
        const bool sameWorld=bound&&birth.source.objects[0]==owner.source.objects[0];
        const auto previous=health;
        *this={};birth=owner;bound=true;isAdditional=extra;
        if(sameWorld){health.uncertainEvents=previous.uncertainEvents;health.permanentFallback=previous.permanentFallback;}
        chase.Active=true;chase.World=owner.source.world;chase.Jason=owner.actor.pawn.object;
        chase.Controller=owner.actor.killerController.object;
        chase.InitialTeleportAt=now+2000; // FEAT4289 opening; trap finish has its own500ms retry
        chase.NextStuckCheckAt=now+1375;
        combat.Jason=chase.Jason;
        additional.World=chase.World;additional.Jason=chase.Jason;additional.Controller=chase.Controller;
    }
};
enum class DonorNativePreparation {Ready,Yield,Exclusive,Refused,Quarantined};
