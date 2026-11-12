    void FinishHelperBirth(){
        using namespace cfgbundle;
        const auto birth=bundleHelperBirth;
        const auto& a=spawnAssets.resources;
        std::uintptr_t controller=0;
        bundleReport.helperPreparation=6;
        if(!birth.Current(r_,plan,GetTickCount64())||
           !cfgspawn::ObserveCounselorSpawnResultScoped(r_,plan,spawnAssets.gameState,birth.pawnClass,bundleCounselorClass,
               a.classes[cfgspawn::CounselorAI],a.classes[cfgspawn::Brain],birth.pawn.object,controller,
               [&](const cfgobserver::Plan&){return HelperContext();})||
           !birth.Current(r_,plan,GetTickCount64())||
           !cfgprobe::lifetime::ReadArrayHeader(r_,plan.module+cfgprobe::lifetime::ObjectArrayRva,context.objectArray)){
            bundleHelpers.Refuse(HelperFailure::Result,true);return;
        }
        for(unsigned i=0;i<bundleReport.bots;++i)if(bundleReport.roster[i].pawn==birth.pawn.object||bundleReport.roster[i].controller==controller){
            bundleHelpers.Refuse(HelperFailure::Result,true);return;
        }
        const auto index=bundleReport.bots;
        if(index>=bundlePawnSlots.size()||
           !cfgspawn::ObjectSlot(r_,context.objectArray,birth.pawn.object,bundlePawnSlots[index])||!(bundlePawnSlots[index]==birth.pawn)||
           !cfgspawn::ObjectSlot(r_,context.objectArray,controller,bundleControllerSlots[index])||!bundleControllerSlots[index].HasAssignedSerial()){
            bundleHelpers.Refuse(HelperFailure::Slots,true);return;
        }
        bundleReport.roster[index]={birth.pawn.object,controller,bundlePawnSlots[index].serial,bundleControllerSlots[index].serial};
        bundleReport.helperProjected=bundleHelpers.surface;bundleReport.helperRequested=birth.requested;
        cfgspawn::Vec3 actual{};
        if(Location(birth.pawn.object,actual)){bundleReport.helperActual=actual;++bundleReport.helperPositionSamples;}
        ++bundleReport.bots;++bundleReport.spawned;++bundleSchedule.spawned;
        bundleRandomCounselors.Confirm(birth.pawnClass,birth.requested);
        bundleRandomCounselors.NextSelection();bundleHelperResidentPass={};
        bundleHelperSelected=false;bundleHelperLocationAttempts=0;bundleHelperStaleRetries=0;
        bundleHelpers.Confirm(bundleReport.spawned,bundleSchedule.requestedBots);
        bundleHelperBirth={};bundleHelperReinforcement=false;
    }
    void DispatchHelperBirth(cfgspawn::SpawnAIParameters& p){
        using namespace cfgbundle;
        const auto& a=spawnAssets.resources;
        // Full tree/class/source/marker/world proof has already run on THIS
        // callback. Reserve only the immediate positive-return journal here;
        // full post-birth ownership proof belongs to the next fresh callback.
        if(bundleHelperBirth.pending||r_.reads>ReadCap-cfghelperbirth::PendingCounselorBirth::CaptureReserve){
            bundleHelpers.Refuse(HelperFailure::Budget);return;
        }
        ++bundleReport.spawnAttempts;bundleReport.helperDispatched=1;
#if CFG_H277_HELPER_SPAWN_SAFE
        h277HelperSpawn_=true;const bool dispatched=Invoke(a.helperDefault,a.spawnFunction,&p,false);h277HelperSpawn_=false;
        if(!dispatched){bundleHelpers.Refuse(HelperFailure::Dispatch,true);return;}
#else
        if(!Invoke(a.helperDefault,a.spawnFunction,&p,false)){bundleHelpers.Refuse(HelperFailure::Dispatch,true);return;}
#endif
        if(!p.result){++bundleReport.helperNulls;bundleHelpers.Next(HelperFailure::NullResult);return;}
        if(p.result==plan.pawn||p.result==bundleJason.pawn.object||
           !bundleHelperBirth.Capture(r_,plan,p.result,bundleHelperTemplate.pawnClass,p.location,GetTickCount64())){
            bundleHelpers.Refuse(HelperFailure::Result,true);return;
        }
        bundleSchedule.spawnAt=GetTickCount64()+100;
        cfgclosedtrace::Mark("helper-birth-journaled",p.result,r_.reads);
    }
