#pragma once
#include "DonorJasonMorph.hpp"
#include "DonorJasonCombat.hpp"
#include "DonorJasonBusyCause.hpp"
#include <array>
namespace cfgdonorhunt {
struct PursuitBoost {
    bool active=false;std::uintptr_t movement=0;
    std::array<float,4> base{}; // walk,sprint,run,slowrun, donor write order
    float appliedMultiplier=0;
    std::uint64_t appliedCooldown=0,nextRefreshAt=0;
};
inline bool DonorValidPursuitSpeed(float value){return std::isfinite(value)&&value>25.0f&&value<5000.0f;}
// FJB4974 ACTIVE body (the remaining lines in the brief's4974-5213
// interval are native SEH helpers). Typed native ownership/CAS writes supplied
// by host; an unproved/failed write never publishes a successful boost latch.
template<class Host>void DonorSetJasonHighPriorityPursuitBoost(Host& host,bool enabled,
    float speedMultiplier=2.0f,std::uint64_t morphCooldownMs=10000){
    auto& b=host.Boost();auto& s=host.Chase();const auto now=host.Now();
    if(!enabled&&!b.active)return;
    const auto movement=host.CurrentMovement(s.Jason);
    if(enabled&&movement&&host.ReadableMovement(movement)){
        if(movement!=b.movement||!DonorValidPursuitSpeed(b.base[0])){
            std::array<float,4> speeds{};
            if(!host.ReadMovementSpeeds(movement,speeds))return;
            for(const auto speed:speeds)if(!DonorValidPursuitSpeed(speed))return;
            b.movement=movement;b.base=speeds;
        }
        const float multiplier=std::isfinite(speedMultiplier)&&speedMultiplier>0?speedMultiplier:2.0f;
        const auto cooldown=morphCooldownMs>=2000?morphCooldownMs:2000;
        const bool same=b.active&&std::fabs(b.appliedMultiplier-multiplier)<0.001f&&b.appliedCooldown==cooldown;
        if(same&&now<b.nextRefreshAt)return;
        std::array<float,4> desired{};
        for(unsigned i=0;i<desired.size();++i)desired[i]=b.base[i]*multiplier;
        if(!host.WriteMovementSpeeds(movement,desired,0xFu))return;
        s.MorphCooldownMs=cooldown;b.appliedMultiplier=multiplier;b.appliedCooldown=cooldown;
        b.nextRefreshAt=now+30013;b.active=true;return;
    }
    if(b.movement&&host.ReadableMovement(b.movement)){
        unsigned mask=0;for(unsigned i=0;i<b.base.size();++i)if(DonorValidPursuitSpeed(b.base[i]))mask|=1u<<i;
        if(mask&&!host.WriteMovementSpeeds(b.movement,b.base,mask))return;
    }
    s.MorphCooldownMs=20000;b.appliedMultiplier=0;b.appliedCooldown=0;b.nextRefreshAt=0;b.active=false;
}
struct BusyState {
    bool KnifeSequenceActive=false,DoorBreakActive=false,StartupTrapSetupActive=false,
        StartupTrapAttemptSent=false,StartupTrapCountConsumed=false,StartupTrapPhoneApproachActive=false;
    std::uint64_t CombatBusyUntil=0;
};
// FJB19298-19383. Negative proof never authorizes motion. Raw readability
// failures in the donor become an explicit conservative native-boundary veto.
template<class Host>bool DonorIsJasonBridgeCombatBusy(Host& host,std::uint64_t now,bool includeNearby,std::uint32_t* causes=nullptr){
    if(causes)*causes=0;
    const auto mark=[&](BusyCause cause){if(causes)*causes|=BusyCauseBit(cause);};
    const auto jason=host.Chase().Jason;
    if(!jason){mark(BusyCause::NoJason);return true;}
    if(!host.ReadableActor(jason)){mark(BusyCause::ActorUnreadable);return true;}
    auto& s=host.Busy();const auto& c=host.Combat();
    bool busy=false;
    if(s.KnifeSequenceActive){mark(BusyCause::KnifeSequence);busy=true;}
    if(c.AttackPressed){mark(BusyCause::AttackPressed);busy=true;}
    if(c.GrabKillReadyAt!=0){mark(BusyCause::GrabKillReady);busy=true;}
    if(s.DoorBreakActive){mark(BusyCause::DoorBreak);busy=true;}
    if(s.StartupTrapSetupActive){mark(BusyCause::StartupTrapSetup);busy=true;}
    if(s.StartupTrapAttemptSent){mark(BusyCause::StartupTrapAttempt);busy=true;}
    if(s.StartupTrapCountConsumed){mark(BusyCause::StartupTrapCount);busy=true;}
    if(s.StartupTrapPhoneApproachActive){mark(BusyCause::StartupTrapPhone);busy=true;}
    if(!busy){
        bool lockedReadable=false;const bool locked=host.GetLockedJasonInteractable(jason,lockedReadable);
        if(locked)mark(BusyCause::LockedInteractable);
        if(!lockedReadable){mark(BusyCause::LockedReadFailure);busy=true;}else if(locked)busy=true;
    }
    if(!busy){
        bool specialReadable=false;const bool special=host.IsJasonInNativeSpecialMove(jason,specialReadable);
        if(special)mark(BusyCause::NativeSpecialMove);
        if(!specialReadable){mark(BusyCause::SpecialMoveReadFailure);busy=true;}else if(special)busy=true;
    }
    std::array<std::uint8_t,4> flags{};
    if(!host.ReadBusyFlags(jason,flags)){mark(BusyCause::BusyFlagsReadFailure);return true;}
    for(unsigned i=0;i<flags.size();++i)if(flags[i]!=0){mark(static_cast<BusyCause>(static_cast<unsigned>(BusyCause::BusyFlag0)+i));busy=true;}
    if(host.ModuleReady()){
        bool stunned=false;std::uintptr_t grabbed=0;
        if(!host.SafeIsStunned(jason,stunned)){mark(BusyCause::StunnedReadFailure);return true;}
        if(!host.SafeGetGrabbedCounselor(jason,grabbed)){mark(BusyCause::GrabbedReadFailure);return true;}
        if(stunned){mark(BusyCause::Stunned);busy=true;}
        if(grabbed){mark(BusyCause::GrabbedCounselor);busy=true;}
    }
    Position here{},there{};
    const auto nearest=includeNearby?host.FindNearestJasonAICounselorTarget(jason):0;
    if(nearest&&host.GetJasonAIActorLocation(jason,here)&&host.GetJasonAIActorLocation(nearest,there)){
        const float dx=there.X-here.X,dy=there.Y-here.Y,dz=there.Z-here.Z;
        const float squared=dx*dx+dy*dy+dz*dz;
        if(std::isfinite(squared)&&squared<=300.0f*300.0f){mark(BusyCause::NearbyCounselor);busy=true;}
    }
    if(busy)s.CombatBusyUntil=now+1000;
    const bool latched=includeNearby&&now<s.CombatBusyUntil;
    if(latched)mark(BusyCause::RecentNearbyLatch);
    return busy||latched;
}
inline void DonorResetStuckSamplingAfterNativeInteraction(ChaseState& s,std::uint64_t now,
    std::uintptr_t& noPathTarget,unsigned& consecutiveNoPathTargets){
    s.ConsecutiveStuckChecks=0;s.LastAcceptedMoveAt=0;s.HaveLastLocation=false;s.PathLocked=false;
    s.PathLockUntil=0;s.NextStuckCheckAt=now+1375;noPathTarget=0;consecutiveNoPathTargets=0;
}
}
