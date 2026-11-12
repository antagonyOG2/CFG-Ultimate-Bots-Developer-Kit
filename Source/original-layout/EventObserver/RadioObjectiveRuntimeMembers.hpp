// Included in BundleAccess: real GameState cache publication BEFORE FindCBRadio.
    bool ObjectiveRadioGetterReady(cfgprobe::lifetime::ArrayHeader& registry,
        cfgprobe::lifetime::SlotObservation& before,std::uintptr_t& table){
        using namespace cfgradioobjective::disk;
        return ObjectiveScope()&&cfgprimary::LookupSafe(r_,plan.module)&&
            cfgprobe::lifetime::ReadArrayHeader(r_,plan.module+cfgprobe::lifetime::ObjectArrayRva,registry)&&
            cfgspawn::ObjectSlot(r_,registry,spawnAssets.gameState,before)&&before.HasAssignedSerial()&&
            r_.Read(spawnAssets.gameState,table)&&cfgprobe::ObjectPointer(table)&&
            cfgobserver::Same(r_,table+GetterSlot,plan.module+Getter)&&
            cfgbundle::Code(r_,plan.module,Getter,GetterCode)&&
            cfgbundle::Code(r_,plan.module,Executor,ExecutorCode)&&calls_<cfgbundle::CallCap;
    }
    bool ObjectiveRadioGetter(std::uintptr_t& out){
        using namespace cfgradioobjective::disk;out=0;
        cfgprobe::lifetime::ArrayHeader registry;cfgprobe::lifetime::SlotObservation before{},after{};
        std::uintptr_t table=0,again=0;
        if(!ObjectiveRadioGetterReady(registry,before,table))return false;
        ++calls_;
        if(!NativeEnvironmentAllowed())return false;
        __try{out=reinterpret_cast<std::uintptr_t(__fastcall*)(void*)>(plan.module+Getter)(reinterpret_cast<void*>(spawnAssets.gameState));}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;return false;}
        ++bundleReport.radioObjectiveGetterReturns;bundleReport.radioObjectiveGetterValue=out;
        return ObjectiveScope()&&cfgprimary::LookupSafe(r_,plan.module)&&
            cfgspawn::ObjectSlot(r_,registry,spawnAssets.gameState,after)&&after==before&&
            r_.Read(spawnAssets.gameState,again)&&again==table&&
            cfgobserver::Same(r_,table+GetterSlot,plan.module+Getter)&&
            cfgobserver::Same(r_,spawnAssets.gameState+Cache,out);
    }
    bool ObjectiveRadioPhase(std::uint64_t now){
#ifdef CFG_H1_JASON
        HostedLaneEnter(cfgh1::Lane::TommyRadio);
#endif

        if(bundleRadioObjectiveFinished||now<bundleRadioObjectiveNext)return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::TommyRadio,1)
#else
 false
#endif
;
        bundleRadioObjectiveNext=now+25;
        if(!bundleRadioObjectiveBegun){bundleRadioObjectiveBegun=true;bundleRadioObjectiveSince=now;}
        if(now<bundleRadioObjectiveSince||now-bundleRadioObjectiveSince>=90000){
            bundleReport.radioObjectiveFailure=8;bundleRadioObjectiveFinished=true;return true;}
        cfgprobe::lifetime::ArrayHeader registry;
        if(!ObjectiveScope()||!cfgprimary::LookupSafe(r_,plan.module)||
           !cfgprobe::lifetime::ReadArrayHeader(r_,plan.module+cfgprobe::lifetime::ObjectArrayRva,registry))return true;
        if(!bundleRadioObjective.Done()){
            bundleRadioObjective.Step(r_,registry,plan.module,plan.world,spawnAssets.gameState);
            bundleReport.radioObjectiveStage=bundleRadioObjective.stage;
            bundleReport.radioObjectiveFailure=bundleRadioObjective.failure;
            bundleReport.radioObjectiveSlices=bundleRadioObjective.slices;
            bundleReport.radioObjectiveActor=bundleRadioObjective.radio;
            bundleReport.radioObjectiveCandidate=bundleRadioObjective.candidate;
            bundleReport.radioObjectiveCandidateSerial=bundleRadioObjective.candidateSerial;
            bundleReport.radioObjectiveTypedStage=bundleRadioObjective.typedStage;
            if(bundleRadioObjective.stage==3){
                // Only read-only acquisition retries; a native publication never does.
                const auto reason=bundleRadioObjective.failure;
                if(cfgradioretry::Allowed(CFG_J23_RADIO_STREAMING_RETRY!=0,bundleRadioObjectiveAttempted,reason,
                    bundleReport.radioObjectiveRetries,now-bundleRadioObjectiveSince)){
                    ++bundleReport.radioObjectiveRetries;
                    const auto delay=cfgradioretry::Delay(CFG_J23_RADIO_STREAMING_RETRY!=0,reason,bundleReport.radioObjectiveRetries);
                    bundleRadioObjective={};bundleRadioObjectiveNext=now+delay;
                }else bundleRadioObjectiveFinished=true;
            }
            return true;
        }
        bundleReport.radioObjectiveStage=4;bundleReport.radioObjectiveFailure=8;
        // A fresh callback rechecks the exact source/world/registry/type before
        // the one empty-pointer CAS. Never replace a nonempty native cache.
        struct Host{BundleAccess& a;
            bool Current(){return a.ObjectiveScope()&&cfgprimary::LookupSafe(a.r_,plan.module);}
            bool Prepare(){cfgprobe::lifetime::ArrayHeader objects;cfgprobe::lifetime::SlotObservation state;std::uintptr_t table=0;
                return a.ObjectiveRadioGetterReady(objects,state,table);}
            bool Exchange(std::uintptr_t at,std::uintptr_t value){
                if(bundleRadioObjectiveAttempted)return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::TommyRadio,2)
#else
 false
#endif
;
                bundleRadioObjectiveAttempted=true;++bundleReport.radioObjectiveAttempts;
                return cfgobjective::PublishEmptyPointer(at,value);}
            bool Getter(std::uintptr_t& value){return a.ObjectiveRadioGetter(value);}
        } host{*this};
        if(cfgradioobjective::Publish(r_,registry,bundleRadioObjective,host)){
            bundleReport.radioObjectiveConfirmed=1;bundleReport.radioObjectiveFailure=0;
        }
        // Post-CAS ambiguity is preserved, not rolled back/replayed; stock UI
        // remains responsible for its native completion status and radio row.
        bundleRadioObjectiveFinished=true;return true;
    }
