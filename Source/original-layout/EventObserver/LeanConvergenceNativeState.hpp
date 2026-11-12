#pragma once
#include "../Core/LeanConvergencePolicy.hpp"
#include "HostedNativeSweaterObservation.hpp"
struct J21LeanBot {
    cfgprobe::lifetime::SlotObservation pawn{},controller{},blackboard{};
    cfglean::Cache cache{};std::uint64_t fearAt=0;std::uintptr_t weapon=0;
    unsigned pendingKey=cfgconvergence::KeyCount;std::uintptr_t pendingValue=0;
};
struct J21LeanState {
    cfgprobe::lifetime::SlotObservation world{};cfglean::Schedule schedule{};
    std::array<J21LeanBot,cfgconvergence::MaximumBots> bots{};
    std::uintptr_t axe=0,machete=0;cfglean::Counters counters{};
    cfgprobe::lifetime::SlotObservation botSweaterOwner{};
    cfgprobe::lifetime::SlotObservation axeClass{},macheteClass{};
    cfglean::SweaterScan sweaterScan{};
    std::uint64_t botSweaterNextProof=0;
    std::uint64_t hostedHumanNextProof=0;bool hostedHumanSweater=false;
} bundleJ21Lean;
thread_local bool bundleJ21LeanDriving=false;
struct J21LeanDrivingScope {
    bool previous=bundleJ21LeanDriving;
    J21LeanDrivingScope(){bundleJ21LeanDriving=true;}
    ~J21LeanDrivingScope(){bundleJ21LeanDriving=previous;}
};
bool J21LeanLatched(){
    bool human=cfgj21::EarnedSweater(bundleSweaterPolicy.nativeOwnershipEarned,
        bundleSweaterPolicy.ownershipObserved,bundleSweaterPolicy.ownershipHas);
#ifdef CFG_H1_JASON
    if(HostedRoutineActive()){
        auto& lean=bundleJ21Lean;const auto now=GetTickCount64();
        if(!(lean.world==plan.objects[0])){lean={};lean.world=plan.objects[0];}
        if(now>=lean.hostedHumanNextProof){
            lean.hostedHumanNextProof=now+211;Reader r;r.cap=256;
            cfgprobe::lifetime::ArrayHeader array{};
            const auto& pawn=bundleHumanTarget;
            lean.hostedHumanSweater=cfgprobe::lifetime::ReadArrayHeader(r,plan.module+cfgprobe::lifetime::ObjectArrayRva,array)&&
                cfghostedsweater::Present(r,array,pawn,plan.controller,bundleCounselorClass);
        }
        // Native pickup is already earned even if the separate power-binding
        // lane has not yet adopted that ability. It never grants a kill/window.
        human=human||lean.hostedHumanSweater;
    }
#endif
#if CFG_J3_BOT_SWEATER || CFG_J215_BOT_SWEATER
    const bool enabled=CFG_J3_BOT_SWEATER||(CFG_J215_BOT_SWEATER&&
        cfgpractice::BotSweaterAutomaticOrSelected(bundleConfiguration.counselorPractice));
    if(CFG_J216_FINISHING)bundleReport.j3BotSweaterOwner=enabled?bundleJ21Lean.botSweaterOwner.object:0;
    return cfglean::SweaterLatched(human,enabled,
        bundleJ21Lean.world==plan.objects[0]&&plan.objects[0].HasAssignedSerial(),
        bundleJ21Lean.botSweaterOwner.HasAssignedSerial());
#else
    return human;
#endif
}
bool J21LeanBlocked(std::uint64_t now){return !cfglean::Driving(true,bundleKill.policy.Exclusive(now),
    bundleKill.policy.interactionAttempted,bundleDeathEnd.confirmedAt||bundleCompletionReady);}
bool J21LeanDue(const cfgjasontick::Owner& owner,std::uint64_t now){
#if CFG_J3_BOT_SWEATER || CFG_J215_BOT_SWEATER
    // Sample only in the already-owned, active primary-Jason game phase.
    // No scan, native dispatch or write is introduced when the flag is OFF.
    const bool botSweaterEnabled=CFG_J3_BOT_SWEATER||(CFG_J215_BOT_SWEATER&&
        cfgpractice::BotSweaterAutomaticOrSelected(bundleConfiguration.counselorPractice));
    if(botSweaterEnabled&&cfgj21::LeanEnabled&&owner.actor.pawn==bundleJason.pawn&&bundleSchedule.active&&
       plan.objects[0].HasAssignedSerial()&&!J21LeanBlocked(now)){
        auto& lean=bundleJ21Lean;
        if(!(lean.world==plan.objects[0])){lean={};lean.world=plan.objects[0];}
        if(lean.botSweaterOwner.HasAssignedSerial()&&now>=lean.botSweaterNextProof){
            lean.botSweaterNextProof=now+211;Reader proof;proof.cap=256;
            cfgprobe::lifetime::ArrayHeader registry{};std::uint8_t has=0;float health=0;
            const auto slot=lean.botSweaterOwner;
            if(!cfgprobe::lifetime::ReadArrayHeader(proof,plan.module+cfgprobe::lifetime::ObjectArrayRva,registry)||
               !cfgjason::SameSlot(proof,registry,slot)||!cfgspawn::ObjectIs(proof,slot.object,bundleCounselorClass)||
               !proof.Read(slot.object+cfghunter::disk::Health,health)||!std::isfinite(health)||health<=0||
               !proof.Read(slot.object+cfgfeature::disk::HasSweater,has)||has!=1||!cfgjason::SameSlot(proof,registry,slot))
                lean.botSweaterOwner={};
        }
        if(!J21LeanLatched()&&bundleReport.bots&&bundleReport.bots<=cfgconvergence::MaximumBots&&
           bundleReport.bots<=bundlePawnSlots.size()&&bundleCounselorClass){
            lean.sweaterScan.Step(now,bundleReport.bots,[&](unsigned index){
                const auto slot=bundlePawnSlots[index];
                if(!slot.HasAssignedSerial()||slot.object==plan.pawn||slot.object==bundleHumanTarget.object)return false;
                Reader r;r.cap=256;cfgprobe::lifetime::ArrayHeader registry{};
                std::uint8_t has=0;float health=0;
                if(!cfgprobe::lifetime::ReadArrayHeader(r,plan.module+cfgprobe::lifetime::ObjectArrayRva,registry)||
                   !cfgjason::SameSlot(r,registry,slot)||!cfgspawn::ObjectIs(r,slot.object,bundleCounselorClass)||
                   !r.Read(slot.object+cfghunter::disk::Health,health)||!std::isfinite(health)||health<=0||
                   !r.Read(slot.object+cfgfeature::disk::HasSweater,has)||has!=1||
                   !cfgjason::SameSlot(r,registry,slot))return false;
                lean.botSweaterOwner=slot;return true;
            });
        }
    }
#endif
    return cfgj21::LeanEnabled&&bundleJ21Lean.schedule.Due(now,owner.actor.pawn==bundleJason.pawn,
        J21LeanLatched(),J21LeanBlocked(now));
}
bool J21LeanOwedCleanup(){
    if(!cfgj21::LeanEnabled||!(bundleJ21Lean.world==plan.objects[0])||
       (!bundleKill.policy.interactionAttempted&&!bundleCompletionReady&&!bundleDeathEnd.confirmedAt))return false;
    for(const auto& travel:bundleConvergenceTravel)if(travel.policy.owned)return true;return false;
}
