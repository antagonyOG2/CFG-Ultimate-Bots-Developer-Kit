    bool AdditionalPhase(std::uint64_t now){
        auto& s=bundleAdditional;
        if(s.rollback&&now>=s.policy.next){s.lastTurn=true;s.policy.next=now+100;AdditionalRollback();return true;}
        if(!s.policy.Due(now))return false;s.lastTurn=true;s.policy.next=now+100;
        if(s.policy.stage==cfgadditional::Stage::Select){
#ifdef CFG_H1_JASON
            if(!HostedF1CanRegisterJason()){
                cfgclosedtrace::Mark("hosted-f1-capacity-preflight-refused",bundleJason.pawn.object,Reads());
                return true;
            }
#endif
            const auto selected=AdditionalSelect();
            if(selected==1)s.policy.Advance(now);
            else if(selected==2){s.policy.Fail(false);cfgclosedtrace::Mark("f1-no-compatible-resident-class",s.selection,Reads());}
            return true;
        }
        if(s.policy.BirthPending()){
            const auto before=s.policy.stage;const bool okay=AdditionalBirth(now);
            if(okay&&s.hostedGateWait){s.hostedGateWait=false;return true;} // H2.65: same stage next turn
            bool registered=true;
#ifdef CFG_H1_JASON
            if(okay&&before==cfgadditional::Stage::Navigation)registered=HostedF1RegisterJason();
#endif
            if(okay&&before==cfgadditional::Stage::Project&&!s.journal.captured){
                cfgclosedtrace::Mark("f1-placement-next-candidate",s.placement.candidate,Reads());return true;
            }
#ifdef CFG_H1_JASON
            if(okay&&before==cfgadditional::Stage::Pawn&&hostedF1StartWait){hostedF1StartWait=false;return true;} // H2.58: still locating the native start; no birth yet
#endif
            if(okay&&registered)s.policy.Advance(now);else{
                s.policy.Fail(s.allocationAttempted);
                if(s.actor.pawn.HasAssignedSerial())s.rollback=1;
            }
            cfgclosedtrace::Mark(okay&&registered?"f1-birth-stage":"f1-birth-refused",s.rawPawn,
                (static_cast<std::uint64_t>(before)<<32)|Reads());
            if(s.policy.stage==cfgadditional::Stage::Active)cfgclosedtrace::Mark("f1-jason-active",s.rawPawn,s.rawController);
        }else if(CFG_J22_ALL_SOUND_BLIPS&&AdditionalSoundPending()){
            (void)AdditionalSoundPhase(); // native donor hunting skips legacy AdditionalDrive; keep presentation reachable
        }else if(CFG_H267_F1_BURST&&AdditionalBurstService(now)){
            // H2.67: this turn applied or restored one burst field
        }else if(!NativeAdditionalHuntingActive()&&!AdditionalDrive(now)){
            ++s.driveRefusals; // no retry of birth, no primary BundleStop
            if(s.driveRefusals<=4)cfgclosedtrace::Mark("f1-drive-refused",s.rawPawn,Reads());
        }
        return true;
    }
    void AdditionalRollback(){
        auto& s=bundleAdditional;using namespace cfgadditional::disk;
        const auto phase=s.rollback;
        if(!AdditionalWorld()||!AdditionalPawn()||!AdditionalRestorePrimary()||!AdditionalLifecycle()){
            s.rollback=0;
            cfgclosedtrace::Mark("f1-cleanup-ownership-refused",s.rawPawn,phase);return;
        }
        const auto controller=phase==1?s.actor.killerController:phase==2?s.actor.initialController:cfgprobe::lifetime::SlotObservation{};
        if(phase<=2&&controller.HasAssignedSerial()&&!(phase==2&&s.genericDestroyAttempted)){
            std::uintptr_t ownedPawn=0;
            std::uintptr_t function=0,owner=0;
            if(!cfgjason::SameSlot(r_,context.objectArray,controller)||
               controller.object==bundleJason.killerController.object||controller.object==plan.controller||
               !cfgspawn::ObjectIs(r_,controller.object,phase==1?spawnAssets.resources.classes[cfgspawn::KillerAI]:s.selected.defaultAI)||
               !r_.Read(controller.object+0x370,ownedPawn)||(ownedPawn&&ownedPawn!=s.rawPawn)||
               !FeatureFunction(Destroy,function,owner)||!cfgspawn::ObjectIs(r_,controller.object,owner)||
               !cfgobserver::Same(r_,function+0xB0,plan.module+DestroyExecutor)||
               !cfgbundle::Code(r_,plan.module,DestroyExecutor,DestroyCode)||!AdditionalPrimary()||r_.reads>cfgbundle::ReadCap-400){
                s.rollback=0;cfgclosedtrace::Mark("f1-controller-cleanup-refused",controller.object,Reads());return;
            }
            s.rollback=0; // never repeat an ambiguous native destruction
            const bool returned=AdditionalInvoke(controller.object,function,nullptr);
            const bool restored=AdditionalRestorePrimary();
            if(returned&&restored)s.rollback=phase+1;
            else cfgclosedtrace::Mark("f1-controller-cleanup-indeterminate",controller.object,Reads());
            return;
        }
        if(phase<=2){s.rollback=phase+1;return;}
        std::uintptr_t function=0,owner=0;
        if(!FeatureFunction(Destroy,function,owner)||!cfgspawn::ObjectIs(r_,s.rawPawn,owner)||
           !cfgobserver::Same(r_,function+0xB0,plan.module+DestroyExecutor)||
           !cfgbundle::Code(r_,plan.module,DestroyExecutor,DestroyCode)||!AdditionalPrimary()||r_.reads>cfgbundle::ReadCap-300){
            s.rollback=0;cfgclosedtrace::Mark("f1-pawn-cleanup-refused",s.rawPawn,Reads());return;
        }
        s.rollback=0;
        const bool returned=AdditionalInvoke(s.rawPawn,function,nullptr);
        // Destroyed pawn need not remain readable. Since the primary was
        // restored BEFORE removal, a null/foreign registry now is unexpected.
        const bool primary=AdditionalPrimary();
        cfgclosedtrace::Mark(returned&&primary?"f1-uncommitted-cleanup-complete":"f1-cleanup-registry-indeterminate",s.rawPawn,phase);
    }
