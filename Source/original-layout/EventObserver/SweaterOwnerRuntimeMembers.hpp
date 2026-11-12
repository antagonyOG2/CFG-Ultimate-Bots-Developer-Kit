// Included inside BundleAccess: shares its owner-thread reader/call budget.
    bool SweaterContext(){
        std::uintptr_t pawn=0;
        return Context()&&Known(0,pawn)&&bundleSweaterOwner.HasAssignedSerial()&&
            pawn==bundleSweaterOwner.object&&cfgjason::SameSlot(r_,context.objectArray,bundleSweaterOwner)&&
            cfgobserver::Same(r_,plan.controller+cfgprobe::ControllerPawn,pawn)&&
            cfgobserver::Same(r_,pawn+cfgescape::disk::Controller,plan.controller);
    }
    bool SweaterAlive(){bool living=false;return bundleSweaterOwner.object&&Alive(bundleSweaterOwner.object,living)&&living;}
    bool RefreshSweaterOwner(std::uint64_t now){
        std::uintptr_t pawn=0;
        if(!Context()||!Known(0,pawn)||!bundleHumanTarget.HasAssignedSerial()||
           !cfgjason::SameSlot(r_,context.objectArray,bundleHumanTarget)||
           !cfgobserver::Same(r_,plan.controller+cfgprobe::ControllerPawn,pawn)||
           !cfgobserver::Same(r_,pawn+cfgescape::disk::Controller,plan.controller))return false;
        if(bundleSweaterOwner==bundleHumanTarget)return true;
        // Never transfer a consumed ability, mask, publication or kill admission
        // to Tommy. He earns his own native pickup and independent use history.
        if(bundleSweaterPolicy.publicationStarted||bundleKill.policy.Exclusive(now))return false;
#ifndef CFG_SPECTATOR_MEMBER_FIXTURE_ONLY
        cfgbundle::RecordSweaterOwnerChange(bundleReport,bundleSweaterOwner.object,bundleHumanTarget.object,now);
#endif
        bundleSweaterOwner=bundleHumanTarget;bundleSweaterPolicy={};
        cfgh241mask::memo.Clear();cfgh241sweater::memo.Clear();
        bundleLateMaskRetryUsed=false;
        bundleSweaterAbility={};bundleSweaterSource={};bundleSweaterRetiredAbility={};bundleSweaterSourceIndex=~0u;
        bundleSweaterCursor=bundleSweaterSettingsCursor=0;bundleSweaterFreshPending=false;bundleSweaterFreshRequested=false;
        bundleSweaterFreshUse=0;bundleSweaterFreshGrantAttempted=false;
        // Discovery attempts belong to this incarnation too. The preceding
        // wearer's reciprocal class anchor can expire when native cleanup
        // clears its ability; it must not suppress Tommy's independent search.
        bundleSweaterClassAttempted=false;bundleSweaterClassPending=false;
        bundleReport.sweaterClassAttempts=0;
        // An earned class lease has its own independent positive lifetime proof;
        // retain it, not the previous wearer's ability. Current() gates any use.
        bundleReport.sweaterBound=0;bundleReport.sweaterStage=0;
        return true;
    }
    bool RetryLateMask(std::uint64_t now){
        // One fresh acquisition attempt for a later actual mask pickup after
        // cold discovery retired. Never reset an earned/used/ambiguous ability or
        // accepted recharge/kill-window history. No allocation in this phase.
        if(bundleLateMaskRetryUsed||bundleSweaterPolicy.stage!=cfgsweater::Stage::Refused||
           bundleSweaterPolicy.lastUse||
           bundleSweaterPolicy.publicationStarted||bundleSweaterFreshGrantAttempted||
           bundleSweaterAbility.object||bundleKill.policy.Exclusive(now))return false;
        bool acquired=false;
        if(!SweaterContext()||!SweaterAlive())return false;
        if(!HasPamelaTrigger(acquired)||!acquired){
            bundleSweaterPolicy.next=now<=UINT64_MAX-500?now+500:UINT64_MAX;
            return false;
        }
        bundleLateMaskRetryUsed=true;bundleSweaterPolicy={};
        bundleSweaterClassAttempted=false;bundleSweaterClassPending=false;
        bundleSweaterCursor=0;bundleReport.sweaterStage=0;bundleReport.sweaterClassAttempts=0;
        return true;
    }
