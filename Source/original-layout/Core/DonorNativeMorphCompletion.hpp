#pragma once
#include "DonorJasonNavContinuation.hpp"
#include "DonorAdditionalJasonHunt.hpp"
namespace cfgdonorhunt {
enum class MorphCompletion {Idle,Pending,Completed,Cancelled,Quarantined};
// Dispatch a *returned* native continuation back to its original donor call
// site. Pending admission is never failure/success/cooldown consumption.
// Native safety forces this return across callbacks rather than one C++ stack.
inline MorphCompletion DonorConsumeNativeMorph(NavRingContinuation& nav,ChaseState& chase,
    AdditionalHuntState& additional,std::uint64_t now){
    if(nav.stage==NavRingStage::Idle)return MorphCompletion::Idle;
    if(nav.Pending())return MorphCompletion::Pending;
    if(nav.stage==NavRingStage::Uncertain)return MorphCompletion::Quarantined;
    if(nav.stage==NavRingStage::Invalid){nav={};return MorphCompletion::Cancelled;}
    const bool success=nav.stage==NavRingStage::Succeeded;
    if(nav.purpose==NavRingPurpose::Additional){
        additional.NextMorphAt=now+(success?20000:6000);
        if(success){additional.StalledSamples=0;additional.HasProgressSample=false;additional.NextMoveAt=now+1000;}
    }else if(nav.purpose==NavRingPurpose::StartupFinish){
        // FEAT12078: finish trap defense with a hunt Morph; unlike the
        // ordinary opening, an actual failed finish retries after500ms.
        chase.InitialTeleportAttempted=success;
        if(success){chase.Target=nav.identity.slots[3].object;DonorMarkJasonAIMorphTeleportUsed(chase,now);}
        else chase.InitialTeleportAt=now+500;
    }else if(success){
        chase.Target=nav.identity.slots[3].object;
        if(nav.purpose==NavRingPurpose::Stuck)DonorResetChaseAfterMorph(chase);
        else{
            DonorMarkJasonAIMorphTeleportUsed(chase,now);
            if(nav.purpose==NavRingPurpose::Initial){chase.PathLocked=false;chase.PathLockUntil=0;}
            else DonorResetChaseAfterMorph(chase);
        }
    }else if(nav.purpose==NavRingPurpose::Distance||nav.purpose==NavRingPurpose::SharedFifty){
        chase.NextDistanceTeleportAt=now+3000;
    }else if(nav.purpose==NavRingPurpose::Stuck){
        chase.ConsecutiveStuckChecks=1;chase.NextStuckCheckAt=now+4750;
    }
    nav={};return MorphCompletion::Completed;
}
}
