    bool Manager(){
#ifdef CFG_H1_JASON
        if(HostedRoutineActive()){
            // This is a read-only identity proof, not native-call admission.
            // Do not confuse the phase's spent budget with an invalid manager.
            Reader fresh;fresh.cap=768;cfgobserver::Plan current{};
            unsigned failedStage=0;
            const auto refused=[&](unsigned stage){RecordBundleManagerFailure(stage,fresh.reads,fresh.cap);return false;};
            if(GetCurrentThreadId()!=plan.threadId)return refused(101);
            if(!cfgprimary::LookupVetoClearForObservation(fresh,plan.module))return refused(102);
            if(!cfgprimary::RetainedWorld(fresh,plan,current))return refused(103);
            if(current.world!=plan.world)return refused(104);
            if(current.mode!=plan.mode)return refused(105);
            if(current.controller!=plan.controller)return refused(106);
            if(!cfghosted::adoption::Current(fresh,current.objectArray,hostedAdoptionReceipt,plan))return refused(107);
            if(!cfgobserver::Same(fresh,plan.world+cfgprobe::WorldGameState,spawnAssets.gameState))return refused(108);
            cfgprobe::lifetime::SlotObservation currentClass{};
            if(!cfgbundle::Manager(fresh,plan.module,current.objectArray,hostedAdoptionReceipt.pawn.object,bundleManagerClass,bundleManagerClassSlot,&failedStage,&currentClass))
                return refused(200+failedStage);
            bundleManagerClassSlot=currentClass;
            return true;
        }
#endif
        return cfgbundle::Manager(r_,plan.module,context.objectArray,bundleJason.pawn.object,bundleManagerClass,bundleManagerClassSlot);
    }
    BundleManagerGateResult ManagerForAction(std::uint64_t now){
#ifdef CFG_H1_JASON
        return BundleManagerGate(now,HostedRoutineActive(),[&](){return Manager();});
#else
        (void)now;return Manager()?BundleManagerGateResult::Ready:BundleManagerGateResult::Failed;
#endif
    }
