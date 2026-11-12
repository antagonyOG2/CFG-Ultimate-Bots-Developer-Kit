#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
    bool DonorNativeMatchPhase(cfgdonorhunt::MatchPhase& phase){
        phase=cfgdonorhunt::MatchPhase::Unknown;cfgnames::Words words{};cfgnames::Text text;
        cfgnames::Observation<Reader> names(r_,plan.module);
        if(!r_.Read(plan.mode+cfgescape::disk::MatchState,words)||!names.Decode(words,text)||
           !cfgescape::Same(r_,plan.mode+cfgescape::disk::MatchState,words))return false;
        const auto name=text.View();
        if(name==u"InProgress")phase=cfgdonorhunt::MatchPhase::InProgress;
        else if(!name.empty()&&name!=u"None")phase=cfgdonorhunt::MatchPhase::NotInProgress;
        return true;
    }
    bool DonorNativeStopMovement(const cfgjasontick::Owner& owner,cfgdonorhunt::NativeOutcome& outcome){
        outcome=cfgdonorhunt::NativeOutcome::NotEntered;
        if(!DonorNativeOwner(owner))return false;
        std::uintptr_t function=0,type=0;const auto controller=owner.actor.killerController.object;
        if(!FeatureFunction(cfgtrap::disk::StopMovement,function,type)||!cfgspawn::ObjectIs(r_,controller,type)||
           !cfgobserver::Same(r_,function+0xB0,plan.module+cfgtrap::disk::StopMovementExecutor)||
           !cfgbundle::Code(r_,plan.module,cfgtrap::disk::StopMovementExecutor,cfgtrap::disk::StopMovementCode))return false;
        bundleInvocationStage_=0;const bool result=Invoke(controller,function,nullptr,false);
        if(bundleInvocationStage_>=5)outcome=cfgdonorhunt::NativeOutcome::Uncertain;
        if(result)outcome=cfgdonorhunt::NativeOutcome::ReturnedTrue;
        return result;
    }
#include "HostedNativeDeathMotionMembers.hpp"
    DonorNativePreparation DonorNativePrepareActor(const cfgjasontick::Owner& owner,
        DonorNativeJasonRuntime& state,std::uint64_t now){
        using namespace cfgdonorhunt;
        state.intendedYield=false;
        if(!DonorNativeOwner(owner)){state.health.Refused(DriverRefusal::Owner);
            return r_.reads>=r_.cap?DonorNativePreparation::Yield:DonorNativePreparation::Refused;}
        const bool extra=owner.actor.pawn==bundleAdditional.actor.pawn&&
            owner.actor.killerController==bundleAdditional.actor.killerController;
        if(!extra&&(!(owner.actor.pawn==bundleJason.pawn)||!(owner.actor.killerController==bundleJason.killerController)))
            return DonorNativePreparation::Refused;
        if(state.bound&&state.birth.source.objects[0]==owner.source.objects[0]&&state.isAdditional!=extra)
            return DonorNativePreparation::Refused; // callers cannot reuse the primary state object for F1
        if(!state.SameBirth(owner)){
            state.BindCurrent(owner,now,extra);
#ifdef CFG_H1_JASON
            // Hosted donor takeover initializes chase cooldown/opening clocks
            // at adoption. The offline J2.19 path retains BindCurrent's values.
            if(owner.adoption.valid&&!extra)
                cfgdonorhunt::DonorInitializeHostedPrimaryChase(state.chase,now);
#endif
        }
        // Both layouts use the same actual gateway; drain before spending any
        // callback budget on phase names, navigation, targets or new actions.
        const auto release=extra?DonorNativeDrainOwedRelease(owner,state.additional,now):
            DonorNativeDrainOwedRelease(owner,state.combat,now);
        if(release==ReleaseDrain::Deferred){state.health.Refused(DriverRefusal::NativeEntry);++state.nativeYields;return DonorNativePreparation::Yield;}
        state.intendedYield=release==ReleaseDrain::Returned||release==ReleaseDrain::Waiting;
        if(release==ReleaseDrain::Uncertain){state.MarkUncertain(UncertainAction::Release,now);return DonorNativePreparation::Quarantined;}
        if(state.ClearSettledUncertainty(now))
            cfgclosedtrace::Mark("donor-quarantine-cleared",owner.actor.pawn.object,static_cast<unsigned>(state.health.uncertainAction));
        if(state.health.permanentFallback){state.health.Refused(DriverRefusal::UncertaintyLimit);return DonorNativePreparation::Refused;}
        if(state.quarantined)return DonorNativePreparation::Quarantined;
        if(extra&&(bundleAdditional.policy.stage!=cfgadditional::Stage::Active||bundleAdditional.journal.uncertain))
            return DonorNativePreparation::Refused;
        MatchPhase phase;
        if(!DonorNativeMatchPhase(phase)){state.health.Refused(DriverRefusal::Phase);
            return r_.reads>=r_.cap?DonorNativePreparation::Yield:DonorNativePreparation::Refused;}
        if(phase==MatchPhase::Unknown){state.health.Refused(DriverRefusal::Phase);return DonorNativePreparation::Refused;}
        state.preparedPhase=phase;
#ifdef CFG_H1_JASON
        if(owner.adoption.valid&&phase==MatchPhase::NotInProgress&&hostedClassRestore.pending&&
           hostedClassRestore.world==plan.world)hostedClassRestore.due=true;
#endif
        const bool exclusive=phase!=MatchPhase::InProgress||bundleKill.policy.Exclusive(now)||
            bundleDeathEnd.confirmedAt||bundleCompletionReady||!(bundleSchedule.active||HostedAdoptionOwnsDriver());
        if(exclusive){
            if(phase!=MatchPhase::InProgress||bundleDeathEnd.confirmedAt||bundleCompletionReady||!(bundleSchedule.active||HostedAdoptionOwnsDriver()))
                (void)J215RestoreHard(state);
            // Projection/pending-unentered teleport has no owed side effect.
            // Never erase an ambiguous entered action to make it retryable.
            if(state.navigation.stage==NavRingStage::Uncertain){state.MarkUncertain(state.navigation.uncertainTeleport?
                UncertainAction::Teleport:UncertainAction::Projection,now);return DonorNativePreparation::Quarantined;}
            state.navigation={};state.frame.Invalidate();
            // The accepted primary Pamela/context lane already owns its stop.
            // F1 has an independent donor one-shot stop latch and own receiver.
            if(extra&&!state.additional.FinalKillStopped){
                auto outcome=NativeOutcome::NotEntered;
                DonorNativeStopMovement(owner,outcome);
                if(outcome==NativeOutcome::NotEntered){++state.nativeYields;return DonorNativePreparation::Yield;}
                state.additional.FinalKillStopped=true;
                if(outcome==NativeOutcome::Uncertain){state.MarkUncertain(UncertainAction::Stop,now);return DonorNativePreparation::Quarantined;}
            }
            return DonorNativePreparation::Exclusive;
        }
        state.additional.FinalKillStopped=false;
        if(J3HardMovement(owner,state))return DonorNativePreparation::Yield;
        if(state.navigation.stage!=NavRingStage::Idle){
            state.frame.Invalidate();
            const auto previousStage=state.navigation.stage;const auto previousCandidate=state.navigation.candidate;
            const auto completedPurpose=state.navigation.purpose;
            if(state.navigation.Pending())DonorNativeNavStep(owner,state.navigation);
            const bool startup=state.navigation.purpose==NavRingPurpose::StartupFinish;
            const bool uncertainTeleport=state.navigation.uncertainTeleport;
            state.intendedYield=state.intendedYield||state.navigation.stage!=previousStage||state.navigation.candidate!=previousCandidate;
            const auto completion=DonorConsumeNativeMorph(state.navigation,state.chase,state.additional,now);
            if(completion==MorphCompletion::Quarantined){state.MarkUncertain(uncertainTeleport?
                UncertainAction::Teleport:UncertainAction::Projection,now);return DonorNativePreparation::Quarantined;}
            if(!extra&&completion==MorphCompletion::Completed&&state.chase.LastMorphTeleportAt==now){
                auto& movement=bundleReport.donorMovement[state.isAdditional?1u:0u];
                if(completedPurpose==NavRingPurpose::Distance)++movement.distanceTeleportConfirmed;
                else if(completedPurpose==NavRingPurpose::SharedFifty)++movement.fiftyMorphConfirmed;
                bundleCarMorph.Attempted(now); // same20s clock used by accepted trap/car owners
            }
            if(startup&&completion==MorphCompletion::Completed){
                #ifdef CFG_H1_JASON
                if(state.startupFinishPending&&hostedAdoptionActive&&state.birth.adoption.valid){++hostedDriverTrace.startupCompleted;hostedDriverTrace.stage=10;}
#endif
                state.startupFinishPending=false;state.startup.Active=false;
                state.combat.NextCombatAttemptAt=now+900;bundleKnife.nextAt=now+3500;
            }
            ++state.nativeYields;return DonorNativePreparation::Yield; // never move in the teleport-return callback
        }
        return DonorNativePreparation::Ready;
    }
    DonorNativePreparation DonorNativePrepareDecision(const cfgdonorframe::Source& source,
        DonorNativeJasonRuntime& state,std::uint64_t now,bool actorPrepared=false){
        if(!actorPrepared){
            const auto preparation=DonorNativePrepareActor(source.owner,state,now);
            if(preparation!=DonorNativePreparation::Ready)return preparation;
        }
        const bool wasReady=state.frame.Status()==cfgdonorframe::State::Ready&&state.frame.Inputs().Same(source);
        switch(DonorNativeDecisionFrame(source,state.frame)){
        case cfgdonorframe::State::Ready:
            // Dispatch on the next admitted callback with a fresh budget.
            // Do not consume an attack's1050ms donor retry merely because
            // the target census left too little budget to enter its input.
            if(!wasReady){state.health.Refused(cfgdonorhunt::DriverRefusal::Frame);++state.readYields;return DonorNativePreparation::Yield;}
            return DonorNativePreparation::Ready;
        case cfgdonorframe::State::Collecting:state.health.Refused(cfgdonorhunt::DriverRefusal::Frame);++state.readYields;return DonorNativePreparation::Yield;
        default:state.health.Refused(cfgdonorhunt::DriverRefusal::Frame);return DonorNativePreparation::Refused;
        }
    }
    DonorNativePreparation DonorNativeSelectTarget(const cfgdonorframe::Source& source,
        DonorNativeJasonRuntime& state,cfgdonorhunt::Targets& roster,std::uint64_t now,
        std::uintptr_t excluded,cfgdonorhunt::CounselorObservation& selected,cfgspawn::Vec3& position){
        selected={};position={};
        const auto prepared=DonorNativePrepareDecision(source,state,now);
        if(prepared!=DonorNativePreparation::Ready)return prepared;
        const auto pawn=state.frame.Nearest(roster,now,excluded);
        if(!pawn)return DonorNativePreparation::Ready;
        const cfgprobe::lifetime::SlotObservation* slot=nullptr;
        for(unsigned i=0;i<source.count;++i)if(source.targets[i].object==pawn){slot=&source.targets[i];break;}
        // Value frames choose; current typed native proof authorizes use.
        // Never treat a successful prior scan as permission after a side effect.
        if(!slot||!DonorNativeCounselor(source.owner,*slot,selected)||!selected.Live()||
           !DonorNativePosition(*slot,position)){
            selected={};position={};state.frame.Invalidate();
            return r_.reads>=r_.cap?DonorNativePreparation::Yield:DonorNativePreparation::Refused;
        }
        return DonorNativePreparation::Ready;
    }
#endif
