// Compatibility adapter inside the EXISTING BundleAccess. This does not own
// scheduling, targeting, movement, abilities or match lifetime. Only explicit
// input schemas are admitted; an unknown frame is never guessed from bytes.
    inline static bool bundleRetentionUncertain_=false;
    template<class Access>static bool ResetRetentionAfterMenuRecovery(Access& access)noexcept{
        if(bundleRootLease||!access.ProvenSlotWindow())return false;
        bundleRetentionUncertain_=false;return true;
    }
    unsigned bundleNextOutputMaximum_=0;
    unsigned bundleActorRetentionReads_=0;
    unsigned bundleRetentionStage_=0;
    unsigned bundleEnvironmentStage_=0,bundleActorStage_=0,bundleNativeStage_=0;
    void BundleRetentionFailed() noexcept {
        bundleRetentionUncertain_=true;
        if(bundleRootLease)bundleRootLease->Uncertain();
    }
    bool BundleRetentionEnvironment() noexcept {
        Reader eligibility;eligibility.cap=256;
        bundleEnvironmentStage_=1;if(bundleRetentionUncertain_)return false;
        bundleEnvironmentStage_=2;if(GetCurrentThreadId()!=plan.threadId)return false;
        bundleEnvironmentStage_=3;if(!originalActorDispatcher)return false;
        bundleEnvironmentStage_=4;if(!primaryRootTickFunction)return false;
        bundleEnvironmentStage_=5;if(!bundleRootLease)return false;
        bundleEnvironmentStage_=6;if(!bundleRootLease->Current())return false;
        bundleEnvironmentStage_=7;if(!cfgprimary::LookupSafe(eligibility,plan.module))return false;
        bundleEnvironmentStage_=0;return true;
    }
    bool BeginBundleNative(unsigned maximumOutputs=0)noexcept{
        bundleJ21Memo.Clear(); // allocate/reentrant native work cannot reuse prior facts
        bundleNativeStage_=1;if(!BundleRetentionEnvironment())return false;
        bundleNativeStage_=2;if(!PrepareBundleCallbackCall(maximumOutputs))return false;
        bundleNativeStage_=3;if(EnterBundleCallbackCall()){bundleNativeStage_=0;return true;}
        if(!CancelBundleCallbackCall())BundleRetentionFailed();return false;
    }
    bool FinishBundleNative()noexcept{
        const bool completed=CompleteBundleCallbackCall(r_,std::array<std::uintptr_t,0>{});
        if(!completed)BundleRetentionFailed();return completed;
    }
    bool EnterBundleInvocation()noexcept{return BeginBundleNative(bundleNextOutputMaximum_);}
    bool RetainBundleActors(std::uintptr_t actor) noexcept {
        // This is the3.5 native-lifetime adapter, not feature AI work. Like
        // metadata/publication validation, use a separately bounded proof;
        // repeated native gateways must not spend the donor's AI read budget.
        Reader lifetime;lifetime.cap=512;
        struct Meter {Reader& reader;unsigned& total;~Meter(){total+=reader.reads;}}meter{lifetime,bundleActorRetentionReads_};
        std::uintptr_t manager=0,held=0;
        bundleActorStage_=1;if(!BundleRetentionEnvironment())return false;
        bundleActorStage_=2;if(!cfgprimary::RetainedWorld(lifetime,plan,context))return false;
        bundleActorStage_=3;if(!lifetime.Read(bundleJason.pawn.object+cfghunter::disk::Manager,manager))return false;
        bundleActorStage_=4;if(!lifetime.Read(bundleJason.pawn.object+cfghunter::disk::Held,held))return false;
        // A former human/controller/selected target is not an argument of
        // every native operation. Retain the actual receiver and current AI
        // dependencies here; explicit frame schemas retain any real victim/
        // driver/target arguments. Do not make dead cached targets universal.
        bundleActorStage_=5;
        if(!RetainPrimaryObjects(lifetime,std::array<std::uintptr_t,9>{plan.world,plan.mode,spawnAssets.gameState,
            actor,bundleJason.pawn.object,bundleJason.killerController.object,
            bundleJason.path.object,manager,held}))return false;
        bundleActorStage_=0;return true;
    }
    bool RetainBundleReturnedObject(std::uintptr_t object) noexcept {
        const bool retained=object&&BundleRetentionEnvironment()&&
            CompleteBundleCallbackCall(r_,std::array<std::uintptr_t,1>{object});
        if(!retained)BundleRetentionFailed();
        return retained;
    }
    bool RetainBundleInvocation(std::uintptr_t actor,std::uintptr_t function,void* parameters) noexcept {
        bundleNextOutputMaximum_=0;
        bundleRetentionStage_=1;
        if(!RetainBundleActors(actor))return false;
        bundleRetentionStage_=2;
        std::array<std::uintptr_t,6> roots{actor,function,0,0,0,0};
        bool hostedMove=false;
#ifdef CFG_H1_JASON
        hostedMove=cfgh241move::HostedFixedMoveFrame(r_,HostedRoutineActive(),function,plan.module,
            cfgtrap::disk::MoveToLocationExecutor);
#endif
        if(hostedMove){
            if(!parameters)return false;
#ifdef CFG_H1_JASON
            ++hostedDriverTrace.hostedMoveFrameDispatched;
#endif
            bundleRetentionStage_=7;
            if(!RetainPrimaryObjects(r_,roots))return false;
            bundleRetentionStage_=8;return true;
        }
        if(function==spawnAssets.functions[2]){
            if(!parameters)return false; // location: pure FVector output
        }else if(function==spawnAssets.functions[3]){
            if(!parameters)return false;
            const auto& p=*static_cast<const cfgspawn::MoveParameters*>(parameters);
            roots[2]=p.goal;roots[3]=p.filterClass;
        }else if(function==spawnAssets.resources.spawnFunction){
            if(!parameters)return false;
            const auto& p=*static_cast<const cfgspawn::SpawnAIParameters*>(parameters);
            roots[2]=p.world;roots[3]=p.pawnClass;roots[4]=p.behaviorTree;
            bundleNextOutputMaximum_=1;
        }else{
            bool typed=false;
            bundleRetentionStage_=3;
            if(!RetainTypedSoftClassFrame(actor,function,parameters,typed))return false;
            if(typed){bundleRetentionStage_=8;return true;}
            Reader metadata;metadata.cap=4096;
            cfgretainframe::RuntimeHost<Reader> host(metadata,plan.module);
            std::array<std::uintptr_t,32> frameRoots{};
            bundleRetentionStage_=4;if(!RetainPrimaryObjects(r_,std::array<std::uintptr_t,2>{actor,function}))return false;
            bundleRetentionStage_=5;if(!cfgretainframe::Observe(host,function,reinterpret_cast<std::uintptr_t>(parameters),frameRoots))return false;
            bundleRetentionStage_=6;if(!cfgretainframe::MaximumOutputRoots(host,function,bundleNextOutputMaximum_))return false;
            bundleRetentionStage_=7;if(!RetainPrimaryObjects(r_,frameRoots))return false;
            bundleRetentionStage_=8;return true;
        }
        bundleRetentionStage_=7;if(!RetainPrimaryObjects(r_,roots))return false;
        bundleRetentionStage_=8;return true;
    }
    bool RetainBundleResults(std::uintptr_t function,void* parameters) noexcept {
        if(!BundleRetentionEnvironment())return false;
#ifdef CFG_H1_JASON
        if(cfgh241move::HostedFixedMoveFrame(r_,HostedRoutineActive(),function,plan.module,
           cfgtrap::disk::MoveToLocationExecutor)){
            if(!parameters)return false;
            const auto result=static_cast<const cfgtrap::MoveLocationParameters*>(parameters)->result;
            hostedDriverTrace.hostedMoveFrameResult=result;
            const bool finished=FinishBundleNative();
            if(finished)++hostedDriverTrace.hostedMoveFrameRetentionFinished;
            else ++hostedDriverTrace.hostedMoveFrameRetentionRefusals;
            return finished;
        }
#endif
        if(function==spawnAssets.resources.spawnFunction){
            if(!parameters)return false;
            const auto& packet=*static_cast<const cfgspawn::SpawnAIParameters*>(parameters);
            return CompleteBundleCallbackCall(r_,std::array<std::uintptr_t,1>{packet.result});
        }
        if(function==spawnAssets.functions[2]||function==spawnAssets.functions[3])return FinishBundleNative();
        bool typed=false;
        if(!CompleteTypedSoftClassFrame(parameters,typed))return false;
        if(typed)return true;
        Reader metadata;metadata.cap=4096;
        cfgretainframe::RuntimeHost<Reader> host(metadata,plan.module);
        std::array<std::uintptr_t,32> results{};
        return cfgretainframe::Observe(host,function,reinterpret_cast<std::uintptr_t>(parameters),results,
            cfgretainframe::Phase::Outputs)&&CompleteBundleCallbackCall(r_,results,bundleNextOutputMaximum_);
    }
