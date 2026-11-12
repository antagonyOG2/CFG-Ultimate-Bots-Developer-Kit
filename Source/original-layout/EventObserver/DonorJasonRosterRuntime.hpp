#pragma once
#include "DonorJasonRuntimeState.hpp"
#include "../Core/H247CombatPolicy.hpp"
#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
struct DonorNativeRosterRuntime {
    cfgprobe::lifetime::SlotObservation world{};
    std::uintptr_t module=0;
    cfgdonorhunt::CounselorRoster roster{};
    std::array<cfgprobe::lifetime::SlotObservation,64> pawns{},controllers{};
    cfgkillerstart::Header levels{};
    unsigned level=0,actor=0,controllerCursor=0,pruneCursor=0,hintCursor=0,fallbackEnd=0;
    bool pruning=false,fallback=false;
    std::array<cfgprobe::lifetime::SlotObservation,64> pendingPawns{},pendingControllers{};
    bool censusComplete=false;
    std::uint64_t revision=0,localAt=0,maintenanceAt=0,controllersAt=0,cells=0,refusals=0;
    std::uint64_t trapSampleAt=0,trapSampleUntil=0;std::uintptr_t trapSampleVictim=0;
};
DonorNativeRosterRuntime bundleDonorRoster;
cfgj21::Backoff bundleDonorSurveyBackoff;
std::uint64_t bundleObjectiveTurnAt=0;bool bundleObjectiveWasLast=false;
DonorNativeJasonRuntime bundleDonorPrimary,bundleDonorAdditional;
cfgdonorgrab::State bundleDonorHeldPrimary,bundleDonorHeldAdditional;
DonorConvergenceNativeState bundleDonorConvergence;
struct DonorPostGrab {cfgdonorgrab::Hold hold{};std::uint64_t until=0;};
std::array<DonorPostGrab,2> bundleDonorPostGrab{};
std::array<cfgdonorturn::WaterCounter,2> bundleDonorWaterCounters{};
std::array<cfgprobe::lifetime::SlotObservation,2> bundleDonorWaterBirths{};
void DonorRecordWaterRoute(const cfgjasontick::Owner& owner,bool known,bool original,bool water){
    const auto role=owner.actor.pawn==bundleJason.pawn?0u:1u;
    if(!(bundleDonorWaterBirths[role]==owner.actor.pawn)){
        bundleDonorWaterBirths[role]=owner.actor.pawn;bundleDonorWaterCounters[role]={};
    }
    auto& counter=bundleDonorWaterCounters[role];
    if(counter.Observe(known,original,water))cfgclosedtrace::Mark("donor-water-original",owner.actor.pawn.object,counter.ticks);
}
bool DonorHeldDecisionInstalled(){return bundleNativeJasonDonorService&&BundleNativeJasonControllerTickCodeCurrent(plan.module);}
bool DonorHeldWaterHint(const cfgjasontick::Owner& owner,bool& water){
    water=true;Reader r;r.cap=160;std::uintptr_t jasonWater=0,victim=0,victimWater=0;
    if(!r.Read(owner.actor.pawn.object+cfgswim::disk::WaterVolume,jasonWater))return false;
    if(jasonWater)return true;
    if(!r.Read(owner.actor.pawn.object+cfghunter::disk::Held,victim))return false;
    if(!victim){water=false;return true;}
    cfgprobe::lifetime::ArrayHeader registry{};cfgprobe::lifetime::SlotObservation slot{};
    if(!cfgprobe::lifetime::ReadArrayHeader(r,owner.source.module+cfgprobe::lifetime::ObjectArrayRva,registry)||
        !cfgspawn::ObjectSlot(r,registry,victim,slot)||!slot.HasAssignedSerial()||!cfgspawn::ObjectIs(r,victim,bundleCounselorClass)||
        !r.Read(victim+cfgswim::disk::WaterVolume,victimWater)||
        !cfgobserver::Same(r,owner.actor.pawn.object+cfghunter::disk::Held,victim))return false;
    water=victimWater!=0;return true;
}
bool DonorHeldStopTickHint(const cfgjasontick::Owner& owner){
    const auto& state=owner.actor.pawn==bundleJason.pawn?bundleDonorPrimary:bundleDonorAdditional;
    return state.heldStopPending&&state.heldStopOwner.source.objects[0]==owner.source.objects[0]&&
        state.heldStopOwner.actor.pawn==owner.actor.pawn&&state.heldStopOwner.actor.killerController==owner.actor.killerController;
}
bool DonorPrimaryOrdinaryHeldHint(){
    if(!DonorHeldDecisionInstalled())return false;
    cfgjasontick::Owner waterOwner{};waterOwner.source=plan;waterOwner.actor=bundleJason;
    bool water=true;if(!DonorHeldWaterHint(waterOwner,water)||water)return false;
    if(bundleDonorPrimary.heldStopPending&&bundleDonorPrimary.heldStopOwner.source.objects[0]==plan.objects[0]&&
        bundleDonorPrimary.heldStopOwner.actor.pawn==bundleJason.pawn)return true;
    Reader r;r.cap=128;std::uintptr_t victim=0,seat=0,extraction=0;
    cfgprobe::lifetime::ArrayHeader registry{};cfgprobe::lifetime::SlotObservation slot{};
    return r.Read(bundleJason.pawn.object+cfghunter::disk::Held,victim)&&victim&&
        cfgprobe::lifetime::ReadArrayHeader(r,plan.module+cfgprobe::lifetime::ObjectArrayRva,registry)&&
        cfgspawn::ObjectSlot(r,registry,victim,slot)&&slot.HasAssignedSerial()&&cfgspawn::ObjectIs(r,victim,bundleCounselorClass)&&
        r.Read(victim+cfgvehicleoccupancy::disk::CurrentSeat,seat)&&!seat&&
        r.Read(bundleJason.pawn.object+cfgaction::disk::ExtractionComponent,extraction)&&!extraction&&
        cfgobserver::Same(r,bundleJason.pawn.object+cfghunter::disk::Held,victim);
}
// This read-only hint chooses base vs full original Tick; it is no call permit.
bool DonorHeldTickHint(const cfgjasontick::Owner& owner,bool waterKnown,bool water){
    if(!waterKnown||water)return false;
    Reader r;r.cap=128;std::uintptr_t held=0;cfgprobe::lifetime::SlotObservation slot{};cfgprobe::lifetime::ArrayHeader registry{};
    return r.Read(owner.actor.pawn.object+cfghunter::disk::Held,held)&&held&&
        cfgprobe::lifetime::ReadArrayHeader(r,owner.source.module+cfgprobe::lifetime::ObjectArrayRva,registry)&&
        cfgspawn::ObjectSlot(r,registry,held,slot)&&slot.HasAssignedSerial()&&
        cfgspawn::ObjectIs(r,held,bundleCounselorClass)&&
        cfgobserver::Same(r,owner.actor.pawn.object+cfghunter::disk::Held,held);
}
bool DonorNativeDeathExclusiveState(bool hostedOwner,const cfgkill::Policy& policy,std::uint64_t now,std::uint64_t deathConfirmedAt){
    return (policy.pamela&&now>=policy.usedAt&&now<policy.until)||policy.Exclusive(now)||
        (hostedOwner&&policy.hostedNativeContextRecovery&&policy.RecoveryPending(now))||deathConfirmedAt;
}
bool DonorNativeDeathExclusive(std::uint64_t now){
    return DonorNativeDeathExclusiveState(HostedAdoptionOwnsDriver(),bundleKill.policy,now,bundleDeathEnd.confirmedAt);
}
bool DonorNativeDriverReady(std::uintptr_t controller){
    if(DonorNativeDeathExclusive(GetTickCount64()))return false;
    if(!controller||!bundleDonorRoster.censusComplete||!(bundleDonorRoster.world==plan.objects[0]))return false;
    if(controller==bundleJason.killerController.object)return bundleJason.killerController.HasAssignedSerial()&&
        !bundleDonorPrimary.health.permanentFallback&&!bundleSchedule.pressed;
    return bundleAdditional.actor.killerController.HasAssignedSerial()&&controller==bundleAdditional.actor.killerController.object&&
        !bundleDonorAdditional.health.permanentFallback&&bundleAdditional.policy.stage==cfgadditional::Stage::Active&&
        !bundleAdditional.policy.pressed&&!bundleAdditional.journal.uncertain;
}
DonorNativeJasonRuntime* DonorNativeRuntimeForController(std::uintptr_t controller){
    if(controller&&controller==bundleJason.killerController.object)return &bundleDonorPrimary;
    if(controller&&controller==bundleAdditional.actor.killerController.object)return &bundleDonorAdditional;
    return nullptr;
}
bool DonorNativeLeaseActive(DonorNativeJasonRuntime& state,std::uint64_t now){
    if(state.health.ObserveLapse(now)){
        const auto reason=static_cast<unsigned>(state.health.refusal);
        auto& counts=bundleReport.donorDrivers[state.isAdditional?1u:0u].lapses;
        if(reason<counts.size())++counts[reason];
        state.frame.Invalidate();
        // Only unentered navigation continuation can be abandoned. Entered
        // uncertain effects have already been consumed by MarkUncertain.
        if(state.navigation.Pending())state.navigation={};
        std::uintptr_t noPath=0;unsigned consecutive=0;
        cfgdonorhunt::DonorResetStuckSamplingAfterNativeInteraction(state.chase,now,noPath,consecutive);
        if(state.isAdditional)bundleAdditional.policy.next=now;
        else {bundleSchedule.huntAt=now;bundleSchedule.selectAt=now;bundleMoveAt=now;}
        cfgclosedtrace::Mark("donor-handoff-lapsed",state.birth.actor.pawn.object,static_cast<unsigned>(state.health.refusal));
    }
    return state.bound&&state.health.Active(now);
}
void DonorNativeBeginHandoff(std::uintptr_t controller,std::uint64_t now){
    if(auto* state=DonorNativeRuntimeForController(controller))if(state->bound){
        DonorNativeLeaseActive(*state,now);state->health.Begin(now);
    }
}
void DonorNativeRefused(std::uintptr_t controller,cfgdonorhunt::DriverRefusal why){
    if(auto* state=DonorNativeRuntimeForController(controller))if(state->bound){
        state->health.Refused(why);DonorNativeLeaseActive(*state,GetTickCount64());
    }
}
void DonorNativeDecisionReturned(DonorNativeJasonRuntime& state,std::uint64_t now,bool intendedFiniteYield=false){
    const auto reason=state.health.refusal;
    if(state.health.Decision(now,state.decisionTargetLive||intendedFiniteYield)){
        auto& counts=bundleReport.donorDrivers[state.isAdditional?1u:0u].resumes;
        if(static_cast<unsigned>(reason)<counts.size())++counts[static_cast<unsigned>(reason)];
        // Lapse already retired the old path. Do not erase the NEW decision's
        // just-accepted path or its6107ms preservation timestamp here.
        cfgclosedtrace::Mark("donor-handoff-resumed",state.birth.actor.pawn.object,static_cast<unsigned>(reason));
    }
    // Empty but otherwise successful donor decisions cannot exclude the
    // accepted PE route forever. A live selection alone re-arms this lease.
    if(!state.decisionTargetLive&&!intendedFiniteYield)DonorNativeLeaseActive(state,now);
}
bool NativePrimaryHuntingActive(){const bool lease=DonorNativeLeaseActive(bundleDonorPrimary,GetTickCount64());
    return lease&&bundleNativeJasonDonorService&&DonorNativeDriverReady(bundleJason.killerController.object)&&
        BundleNativeJasonControllerTickCodeCurrent(plan.module);}
bool DonorNativeMovementLeasePublished(){return HostedAdoptionOwnsDriver()||NativePrimaryHuntingActive();}
// H2.46: legacy lease-mode Morph/burst helpers (read-only views of the donor state, plus the donor's own post-Morph chase reset).
bool DonorNavPendingForLegacy(){return bundleDonorPrimary.navigation.Pending();}
bool DonorTrapIntentForLegacy(){return bundleDonorPrimary.trapIntent.victim!=0;}
void DonorChaseResetAfterLegacyMorph(){cfgdonorhunt::DonorResetChaseAfterMorph(bundleDonorPrimary.chase);}
// H2.47: the donor combat-busy bookkeeping (attack pressed, grab-kill ready) for the legacy lease-mode Morph/burst veto.
void DonorBusyStateForLegacy(cfgh247::LegacyBusyView& v){v.busy=bundleDonorPrimary.busy;v.combat=bundleDonorPrimary.combat;}
bool NativeAdditionalHuntingActive(){const bool lease=DonorNativeLeaseActive(bundleDonorAdditional,GetTickCount64());
    return lease&&bundleNativeJasonDonorService&&DonorNativeDriverReady(bundleAdditional.actor.killerController.object)&&
        BundleNativeJasonControllerTickCodeCurrent(plan.module);}
bool DonorFinalVictimOwnsCamera(){return bundleDonorRoster.censusComplete&&bundleDonorRoster.world==plan.objects[0]&&
    bundleDonorRoster.roster.FinalCounselorDeathObserved;}
void ServiceDonorNativeHunting(const cfgjasontick::Owner&)noexcept;
#else
inline bool DonorHeldDecisionInstalled(){return false;}
inline bool DonorPrimaryOrdinaryHeldHint(){return false;}
inline bool NativePrimaryHuntingActive(){return false;}
inline bool DonorNativeMovementLeasePublished(){return false;}
inline bool DonorNavPendingForLegacy(){return false;}
inline bool DonorTrapIntentForLegacy(){return false;}
inline void DonorChaseResetAfterLegacyMorph(){}
inline void DonorBusyStateForLegacy(cfgh247::LegacyBusyView&){}
inline bool NativeAdditionalHuntingActive(){return false;}
inline bool DonorFinalVictimOwnsCamera(){return false;}
#endif
