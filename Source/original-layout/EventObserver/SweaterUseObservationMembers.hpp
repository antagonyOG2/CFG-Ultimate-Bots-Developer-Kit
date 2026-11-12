// Included in BundleAccess; no native call, allocation or raw-object resurrection.
    void RecordSweaterUse(unsigned source){
        ++bundleReport.sweaterUses;bundleReport.sweaterUseEdgeSource=source;
#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
        RecordBundleSweaterEvent(true,GetTickCount64());
#endif
        if(source==1)++bundleReport.sweaterOwnershipUses;
        else ++bundleReport.sweaterAbilityUses;
    }
    bool RetireBoundSweaterAbility(){
        if(!bundleSweaterAbility.object||bundleSweaterPolicy.publicationStarted||
           !cfgsweater::Retired(r_,plan.module,bundleSweaterAbility)||!SweaterContext()||!SweaterAlive())return false;
        // Positive registry retirement, not SweaterBinding failure or read exhaustion.
        bundleSweaterRetiredAbility=bundleSweaterAbility;bundleSweaterAbility={};
        bundleSweaterSource={};bundleSweaterSourceIndex=~0u;
        bundleReport.sweaterBound=0;++bundleReport.sweaterRetiredAbilities;
        return true; // keep use/kill/rearm/constructor history
    }
    bool SweaterRearmPrevious(std::uintptr_t previous){
        if(!previous||previous==bundleSweaterAbility.object)return true;
        return previous==bundleSweaterRetiredAbility.object&&
            cfgsweater::RetiredInPlace(r_,plan.module,bundleSweaterRetiredAbility);
        // A valid new native ability (including address reuse) always wins.
    }
    bool NativeSweaterReplacement(std::uintptr_t current){
        if(!current||!bundleSweaterAbility.object)return false;
        if(current!=bundleSweaterAbility.object)return true;
        cfgprobe::lifetime::SlotObservation fresh{};
        return cfgspawn::ObjectSlot(r_,context.objectArray,current,fresh)&&!(fresh==bundleSweaterAbility);
    }
