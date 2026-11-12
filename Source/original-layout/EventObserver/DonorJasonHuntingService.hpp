// Included after BundleAccess. Exactly one callback-scoped host per native
// controller turn; no saved root lease. Review208 liveness restores the accepted
// PE driver only while the donor lease has lapsed; root-busy callbacks cannot
// run both, and successful new donor decisions renew the exclusive lease.
#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
void BundleEmoteReport();
unsigned ServiceDonorNativeEmotes(const cfgjasontick::Owner& owner,std::uint64_t now,unsigned callDebt){
    #ifdef CFG_H1_JASON
    HostedLaneEnter(cfgh1::Lane::Emote);
#endif
    if(!cfgj21::EmotesEnabled||!bundleEmotes.NeedsService(now))return 0; // NO reads or native scope work when idle/disabled
    J21Timing timing(cfgj21::Emotes,true);
    // Independent of Ready/External/Census/lease and full-original/base choice.
    // Only this returned native Tick's proved lexical capability admits calls.
    if(!(bundleSchedule.active||HostedAdoptionOwnsDriver())||bundleCompletionReady||bundleDeathEnd.confirmedAt||bundleKill.policy.Exclusive(now))return 0;
    if(callDebt>=cfgbundle::CallCap){++bundleReport.j2EmoteBudgetDeferrals;return 0;}
    BundleAccess access(cfgbundle::ReadCap,callDebt);cfgdonorhunt::MatchPhase phase;
    if(!access.DonorNativeOwner(owner)||!access.DonorNativeMatchPhase(phase)||phase!=cfgdonorhunt::MatchPhase::InProgress)return 0;
    if(bundleEmotes.report.prepared<2)access.MirrorPrepare();
    else access.MirrorService(now);
    bundleReport.reads+=access.Reads();bundleReport.calls+=access.Calls();
    bundleReport.maxReads=(std::max)(bundleReport.maxReads,access.Reads());
    bundleReport.maxCalls=(std::max)(bundleReport.maxCalls,access.Calls());BundleEmoteReport();
    return access.Calls();
}
void ServiceDonorNativeHunting(const cfgjasontick::Owner& owner)noexcept{
    const auto now=GetTickCount64();
    auto& state=owner.actor.pawn==bundleJason.pawn?bundleDonorPrimary:bundleDonorAdditional;
    // Never pay this callback's heartbeat with a previous selection. Busy,
    // startup and owed/nav-only paths may return without querying a target.
    state.decisionTargetLive=false;
    BundleAccess access;
    const auto uncertainBefore=state.health.uncertainEvents;
    const auto gate=[&](unsigned why){
#ifdef CFG_H1_JASON
        if(owner.adoption.valid){auto& d=hostedDriverTrace;d.gate=why;if(why<d.gates.size())++d.gates[why];
            d.exclusiveMask=(bundleKill.policy.Exclusive(now)?1u:0u)|(bundleCompletionReady?2u:0u)|
                (bundleDeathEnd.confirmedAt?4u:0u)|(bundleSchedule.active?0u:8u)|(!bundleNativeJasonTickUsedBase?16u:0u)|
                ((bundleKill.policy.pamela&&now>=bundleKill.policy.usedAt&&now<bundleKill.policy.until)?32u:0u)|
                (!bundleNativeJasonWaterKnown?64u:0u)|(bundleNativeJasonWater?128u:0u);
            if(!state.isAdditional&&state.stallTarget&&now>=state.stallNextSampleAt){
                state.stallNextSampleAt=now<=UINT64_MAX-500?now+500:UINT64_MAX;
                const auto index=bundleDonorRoster.roster.DonorFindJasonAITargetIndex(state.stallTarget);
                if(bundleDonorRoster.censusComplete&&bundleDonorRoster.world==plan.objects[0]&&index>=0&&
                   static_cast<unsigned>(index)<bundleDonorRoster.pawns.size()&&
                   bundleDonorRoster.pawns[static_cast<unsigned>(index)].object==state.stallTarget){
                    cfgdonorhunt::Position jason{},target{};bool targetLive=false;
                    const bool observed=access.DonorNativeStallObservation(owner,
                        bundleDonorRoster.pawns[static_cast<unsigned>(index)],jason,target,targetLive);
                    const float dx=target.X-jason.X,dy=target.Y-jason.Y,dz=target.Z-jason.Z;
                    const float distance=std::sqrt(dx*dx+dy*dy+dz*dz);
                    cfgh1::DriverTrace::StallSample sample{};
                    const cfgh1::StallTracker::Position position{jason.X,jason.Y,jason.Z};
                    if(observed&&targetLive&&std::isfinite(distance)&&state.hostedStall.Observe(
                        now,state.stallTarget,distance,position,true,true,sample,
                        state.runningLastAt&&now>=state.runningLastAt&&now-state.runningLastAt<=500)){
                        sample.gate=why;sample.externalMask=state.stallExternalMask;
                        sample.navStage=static_cast<unsigned>(state.navigation.stage);sample.pathMoveResult=bundleReport.lastMoveResult;
                        sample.distanceMorphResult=state.distanceMorphResult;sample.distanceMorphRefusal=state.distanceMorphRefusal;
                        sample.morphNow=now;sample.nextDistanceTeleportAt=state.chase.NextDistanceTeleportAt;
                        sample.sharedMorphNext=bundleCarMorph.next;sample.lastMorphTeleportAt=state.chase.LastMorphTeleportAt;
                        sample.morphReadyAt=cfgdonorhunt::DonorJasonAIMorphReadyAt(state.chase);
                        sample.lastAcceptedMoveAt=state.chase.LastAcceptedMoveAt;sample.pathLockUntil=state.chase.PathLockUntil;
                        sample.pathLocked=state.chase.PathLocked?1u:0u;sample.moveRequests=state.moveRequests;
                        sample.lastMoveReturn=state.lastMoveReturn;sample.held=state.stallHeld;
                        sample.phase=static_cast<unsigned>(state.preparedPhase);
                        if(!d.stallSamples)d.stallFirst=sample;d.stallLatest=sample;
                        if(d.stallSamples<65535)++d.stallSamples;
                    }else if(!observed||!targetLive||!std::isfinite(distance))state.hostedStall.Reset();
                }else state.hostedStall.Reset();
            }}
#else
        (void)why;
#endif
    };
    InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
    auto& held=owner.actor.pawn==bundleJason.pawn?bundleDonorHeldPrimary:bundleDonorHeldAdditional;
#if CFG_H281_UNSTICK
    // H2.81: Jason is never frozen. water=true runs from Turn::HuntExclusive (the hunt is skipped while he is in water, so this is the
    // only place a ring can begin or step there); water=false runs at the drive point, i.e. only once every external/owned lane is idle.
    // Both use the existing nav-ring teleport (UnstickRing = RequestRing) on this returned native Tick's own proved session.
    const auto unstick=[&](bool water)->bool{
        auto& u=state.unstick;namespace us=cfgunstick;us::Action what=us::Action::None;
        // H2.90 (audit #1): an unstick ring that exhausts its cells marks its counselor, whichever mode began it and whichever step
        // (the initial RequestRing step or a later continuation, in water or by the hunt) consumed the terminal result.
        // Only an exhausted ring (ringFailed) counts; uncertain/invalid/succeeded outcomes clear the pending record.
        auto& nav=bundleReport.h246Nav[state.isAdditional?1u:0u];
        const auto settle=[&]{
            if(!u.pendingTarget)return;const auto p=u.pendingPurpose;
            if(p<nav.ringFailed.size()&&nav.ringFailed[p]>u.pendingBase){u.RingFailed(u.pendingTarget,now);u.pendingTarget=0;}
            else if(state.navigation.stage==cfgdonorhunt::NavRingStage::Idle)u.pendingTarget=0;
        };
        settle();
        if(water){
            if(!bundleDonorRoster.censusComplete||!(bundleDonorRoster.world==plan.objects[0]))return false;
            // The hunt's PrepareActor (which steps a pending ring) does not run in water: step our own ring here.
            if(state.navigation.stage!=cfgdonorhunt::NavRingStage::Idle){(void)access.DonorNativePrepareActor(owner,state,now);settle();return false;}
            Reader w;w.cap=32;std::uintptr_t wet=0,victim=0;
            if(!w.Read(owner.actor.pawn.object+cfgswim::disk::WaterVolume,wet)||!w.Read(owner.actor.pawn.object+cfghunter::disk::Held,victim))return false;
            if(!wet||victim){u.Water(now,false,-1);return false;}
            what=u.Water(now,true,-1);if(what==us::Action::None)return false;
            if(access.DonorNativePrepareActor(owner,state,now)!=DonorNativePreparation::Ready)return false; // phase/exclusive/owner/quarantine
        }else{
            if(!u.SampleDue(now))return false;
            cfgspawn::Vec3 here{};if(!access.DonorNativePosition(owner.actor.pawn,here))return false;
            what=u.Frozen(now,here.x,here.y,here.z);if(what==us::Action::None)return false;
        }
        auto source=access.DonorRosterSource(owner);
        cfgdonorhunt::Targets roster=static_cast<const cfgdonorhunt::Targets&>(bundleDonorRoster.roster);
        BundleAccess::DonorNativeHuntingHost host{access,source,state,roster,now};
        const auto refuse=[&](us::Reason why){u.Refused(what,now,why);cfgclosedtrace::Mark("h281-unstick-refused",owner.actor.pawn.object,static_cast<unsigned>(why));return false;};
        if(access.DonorNativeDecisionFrame(source,state.frame)!=cfgdonorframe::State::Ready)return false; // collects across callbacks, no clock charged
        const auto avoid=u.Avoid(now);
        auto target=host.FindNearestJasonAICounselorTarget(host.Jason(),avoid); // H2.88: skip a counselor with no walkable landing
        if(avoid&&target)++u.otherTargets;
        if(!target&&avoid)target=host.FindNearestJasonAICounselorTarget(host.Jason()); // only that counselor left: try again
        if(!target)return refuse(us::Reason::NoTarget);
        if(water){
            Reader t;t.cap=16;std::uintptr_t targetWater=0;int wet=-1;
            if(t.Read(target+cfgswim::disk::WaterVolume,targetWater))wet=targetWater?1:0;
            what=u.Water(now,true,wet);if(what!=us::Action::WaterEscape)return false; // target in water: native swim kill may run, wait 20 s
        }else{
            const auto& f=state.additional;
            if((state.isAdditional&&(f.AttackPressed||f.KnifeActive||f.OccupiedCar||now<f.DoorInteractionUntil||now<f.HidingInteractionUntil))||
               host.IsJasonBridgeCombatBusy(now,false))return refuse(us::Reason::Busy);
        }
        const auto ringPurpose=static_cast<unsigned>(state.isAdditional?cfgdonorhunt::NavRingPurpose::Additional:cfgdonorhunt::NavRingPurpose::Stuck);
        const auto failedBase=ringPurpose<nav.ringFailed.size()?nav.ringFailed[ringPurpose]:0;
        const bool begun=host.UnstickRing(target);
        if(ringPurpose<nav.ringFailed.size()&&nav.ringFailed[ringPurpose]>failedBase)u.RingFailed(target,now); // exhausted in the initial step
        else if(begun&&state.navigation.stage!=cfgdonorhunt::NavRingStage::Idle){u.pendingTarget=target;u.pendingPurpose=ringPurpose;u.pendingBase=failedBase;}
        state.frame.Consumed();
        if(!begun)return refuse(us::Reason::Refused);
        u.Accepted(what,now);
        cfgclosedtrace::Mark(water?"h281-water-escape":"h281-frozen-escape",owner.actor.pawn.object,target);
        return true;
    };
#endif
    const auto work=[&](){
        gate(0);
        if(CFG_J21C_HELD_RECOVERY&&held.legacyFallback){
            // No target/nav work or second owner: use the frozen J1a.2 combat
            // body in THIS returned native Tick with a fresh bounded reader.
            BundleAccess fallback(cfgbundle::ReadCap,access.Calls());
            BundleAccess::ChildAccounting accounting(access,fallback);
            if(!held.CanFallback(now)||!fallback.DonorHeldCurrent(owner,held.hold)||
                fallback.DonorNativePrepareActor(owner,state,now)!=DonorNativePreparation::Ready||
                state.preparedPhase!=cfgdonorhunt::MatchPhase::InProgress||state.quarantined||DonorNativeDeathExclusive(now))return;
            auto source=fallback.DonorRosterSource(owner);
            cfgdonorhunt::Targets roster=static_cast<const cfgdonorhunt::Targets&>(bundleDonorRoster.roster);
            BundleAccess::DonorNativeHuntingHost host{fallback,source,state,roster,now};
            if(!state.combat.GrabKillReadyAt)state.combat.GrabKillReadyAt=now;
            host.RunJasonAICombatOnGameThread();
            if(host.inputEntered){++held.attempts;held.committedAt=now;
                held.stage=state.quarantined?cfgdonorgrab::Stage::Uncertain:cfgdonorgrab::Stage::Committed;
                held.nextProbeAt=now+350;}
            return; // accounting also runs on every preparation/refusal exit
        }
        {Reader hint;hint.cap=8;std::uintptr_t victim=0;
            if(!hint.Read(owner.actor.pawn.object+cfghunter::disk::Held,victim)){state.stallHeld=2;gate(2);return;}
            state.stallHeld=victim?1u:0u;
#ifdef CFG_H1_JASON
            if(owner.adoption.valid)hostedDriverTrace.heldHint=victim;
#endif
            if(victim){gate(3);return;}}
        // The actor preparation drains an owed slash release before ANY
        // roster, phase, target or navigation work, including terminal turns.
        const bool first=!state.SameBirth(owner);
        const auto prepared=access.DonorNativePrepareActor(owner,state,now);
        #ifdef CFG_H1_JASON
        if(owner.adoption.valid)hostedDriverTrace.preparation=static_cast<unsigned>(prepared);
#endif
        if(prepared!=DonorNativePreparation::Ready){
            gate(4);
            if(prepared==DonorNativePreparation::Yield&&state.intendedYield)DonorNativeDecisionReturned(state,now,true);
            return;
        }
        const bool primary=!state.isAdditional;
        if(primary){
#ifdef CFG_H1_JASON
            if(owner.adoption.valid)(void)access.DonorNativeObserveRunning(owner,state,now);
#endif
            // Existing trap/car adapters already reserve this shared20s
            // charge. Import its actual reservation, never guess engine time.
            if(bundleCarMorph.next>=20000&&bundleCarMorph.next>state.chase.NextDistanceTeleportAt){
                state.chase.LastMorphTeleportAt=bundleCarMorph.next-20000;
                state.chase.NextDistanceTeleportAt=bundleCarMorph.next;
            }
            if(bundleTraps.Busy())state.startup.Active=true;
            else if(state.startup.Active||(first&&bundleTraps.report.stage==cfgtrap::startup::Stage::Complete)
#ifdef CFG_H1_JASON
                ||(first&&owner.adoption.valid&&!bundleTraps.Busy()) // terminal/unsupported trap map: still leave the cabin
#endif
                ){
                state.startup.Active=false;state.startupFinishPending=true;
            }
        }
        if(!bundleDonorRoster.censusComplete||!(bundleDonorRoster.world==plan.objects[0])){
            #ifdef CFG_H1_JASON
            if(owner.adoption.valid){++hostedDriverTrace.censusWaits;hostedDriverTrace.stage=9;}
#endif
            gate(5);state.health.Refused(cfgdonorhunt::DriverRefusal::Census);access.DonorRosterSurvey(owner,now);return;
        }
        // During census/handoff full original Tick already ran. Its return
        // is a valid observation scope, never permission to run two AIs.
        if(!bundleNativeJasonTickUsedBase){gate(6);return;}
        std::uint32_t externalMask=0;state.trapSlashTurn=false;
        if(primary){
            if(bundleCarPursuit.Active(now))externalMask|=1u<<0;
            if(bundleVehicleActions.StopOwnsEpisode())externalMask|=1u<<1;
            if(bundleCarActionPending.active)externalMask|=1u<<2;
            if(bundleRoadMorph.stage)externalMask|=1u<<3;
            if(bundleCarSpeed.active)externalMask|=1u<<4;
            if(bundleHiding.Active()||(CFG_H266_HIDER_APPROACH&&now<bundleHiderApproach.until))externalMask|=1u<<5;
            if(bundleDoorRecovery.phase)externalMask|=1u<<6;
            if(bundleFootObstacle.stage)externalMask|=1u<<7;
            if(bundleFootObstacleTarget)externalMask|=1u<<8;
            if(bundleFootBurst.ExternalOwned())externalMask|=1u<<9; // H2.46: lease-mode burst does not stop the donor
            if(bundleCounselorMorphTarget)externalMask|=1u<<10;
            if(bundleEscapeGuard.morphPending)externalMask|=1u<<11;
            if(access.DonorExitGuardOwns(owner,now))externalMask|=1u<<12;
            // H2.43: Travel stage only; trap placement stages stay exclusive.
            state.trapSlashTurn=CFG_H243_TRAP_SLASH&&bundleTraps.Busy()&&!bundleTraps.Paused()&&
                bundleTraps.report.stage==cfgtrap::startup::Stage::Travel;
            if(bundleTraps.Busy()&&!state.trapSlashTurn)externalMask|=1u<<13;
            if(bundleGrab.held)externalMask|=1u<<14;
            if(bundleHeldRecovery.attemptAt)externalMask|=1u<<15;
            if(bundleKnifeProbePending)externalMask|=1u<<16;
            if(bundleKnife.Busy())externalMask|=1u<<17;
        }
        const bool external=externalMask!=0;
        state.stallExternalMask=externalMask;
        if(externalMask!=state.externalReasonMask||(externalMask&&now>=state.externalReasonAt&&now-state.externalReasonAt>=5000)){
            if(externalMask||state.externalReasonMask)
                cfgclosedtrace::Mark("donor-external-reasons",owner.actor.pawn.object,externalMask);
            state.externalReasonMask=externalMask;state.externalReasonAt=now;
        }
        if(external){gate(7);state.health.Refused(cfgdonorhunt::DriverRefusal::External);state.interactionOwned=true;state.frame.Invalidate();return;}
        if(state.interactionOwned){
            std::uintptr_t noPath=0;unsigned consecutive=0;
            cfgdonorhunt::DonorResetStuckSamplingAfterNativeInteraction(state.chase,now,noPath,consecutive);
            state.interactionOwned=false;
        }
        if(access.DonorRosterRetire(now)){gate(8);return;}
        auto source=access.DonorRosterSource(owner);
        cfgdonorhunt::Targets targets=static_cast<const cfgdonorhunt::Targets&>(bundleDonorRoster.roster);
        if(primary&&state.frame.Status()==cfgdonorframe::State::Ready&&state.frame.Inputs().Same(source)){
            const auto target=access.DonorNativeNearest(state,targets,now);
            // Metadata bridge only. Every existing knife/door/vehicle user
            // still re-proves the selected pawn before invoking the engine.
            bool published=false;
#ifdef CFG_H1_JASON
            if(owner.adoption.valid){
                // Projection into the legacy/player-character metadata list is
                // optional for the donor's own roster. Give it a fresh bounded
                // reader so a failed projection cannot spend the core hunt's
                // remaining read debt. It inherits this turn's native-call debt.
                BundleAccess publication(cfgbundle::ReadCap,access.Calls());
                publication.context=access.context;
                BundleAccess::ChildAccounting accounting(access,publication);
                published=publication.DonorPublishPrimaryTarget(target);
                if(!published){
                    // Retire only the stale legacy selection/hint. The donor
                    // decision frame and its selected target remain intact;
                    // native effects re-prove both from that frame.
                    bundleReport.target=0;bundleTargetIndex=0;bundleNativeTargetHint=0;bundleNativeTargetHintIndex=0;
                    if(bundleNativeTarget.pawn.object)bundleNativeTarget={};
                }
            }else{
                published=access.DonorPublishPrimaryTarget(target);
            }
#else
            published=access.DonorPublishPrimaryTarget(target);
#endif
            if(!published){gate(9);
#ifdef CFG_H1_JASON
                if(!cfgh226::ContinueDonorAfterTargetProjectionFailure(owner.adoption.valid))return;
#else
                return;
#endif
            }
        }
#if CFG_H281_UNSTICK
        const bool unstuck=unstick(false); // frozen backstop: reached only when no external lane owns him; a begun ring replaces this turn's drive
#else
        const bool unstuck=false;
#endif
        const auto outcome=unstuck?DonorNativePreparation::Yield:primary?access.DonorNativeDrivePrimary(source,state,targets,now,true):
            access.DonorNativeDriveAdditional(source,state,targets,now,true);
        gate(10);
        if(outcome==DonorNativePreparation::Ready||
           (outcome==DonorNativePreparation::Yield&&state.intendedYield))
            DonorNativeDecisionReturned(state,now,outcome==DonorNativePreparation::Yield&&state.intendedYield);
        if(outcome==DonorNativePreparation::Quarantined)
            cfgclosedtrace::Mark("donor-hunt-quarantined",owner.actor.pawn.object,state.decisions);
    };
    struct Turn {
        BundleAccess& access;const cfgjasontick::Owner& owner;DonorNativeJasonRuntime& state;
        cfgdonorgrab::State& held;const decltype(work)& hunt;const decltype(gate)& trace;std::uint64_t now;
#if CFG_H281_UNSTICK
        const decltype(unstick)& unstickTurn;
#endif
        unsigned debt=0;
        bool huntEligible=true,chosen=false;unsigned selected=3;
        bool Optional(unsigned lane){
            if(lane==0)return true; // Hunt is core donor work, never B/C budget debt.
            if(!J21HasBudget(1000))return false;
            if(lane==2&&CFG_H256_EMOTE_PRIORITY&&cfgj21::EmotesEnabled&&bundleEmotes.Due(now))return true; // donor pumps emotes every tick; due work never competes with convergence
            if(!chosen){chosen=true;unsigned due=0; // only B/C share optional turns
                if(cfgj21::ConvergenceLaneEnabled(cfgj21::ConvergenceEnabled,HostedRoutineActive())&&
                   (now>=bundleDonorConvergence.donor.NextCounselorConvergenceAt||now>=bundleDonorConvergence.publishAt))due|=2;
                if(J21LeanDue(owner,now))due|=2;
                if(cfgj21::EmotesEnabled&&bundleEmotes.NeedsService(now))due|=4;
                selected=bundleJ21Budget.Choose(state.isAdditional?1u:0u,due);}
            return selected==lane&&bundleJ21Budget.Slice();
        }
        bool DrainOwedRelease(){
            const bool extra=owner.actor.pawn==bundleAdditional.actor.pawn;
            const auto result=extra?access.DonorNativeDrainOwedRelease(owner,state.additional,now):
                access.DonorNativeDrainOwedRelease(owner,state.combat,now);
            if(result==cfgdonorhunt::ReleaseDrain::Uncertain)state.MarkUncertain(cfgdonorhunt::UncertainAction::Release,now);
            const bool blocked=result==cfgdonorhunt::ReleaseDrain::Deferred||result==cfgdonorhunt::ReleaseDrain::Uncertain||
                access.DonorHeldDrainStop(owner,state,now)||access.J21LeanCleanup(owner,now);
            if(blocked)trace(1);return blocked;
        }
        bool DriveHeld(){
#ifdef CFG_H1_JASON
            if(owner.adoption.valid&&now>=hostedNativeDeathMotionCheckAt&&!hostedNativeDeathMotion){
                BundleAccess death(cfgbundle::ReadCap,access.Calls());
                BundleAccess::ChildAccounting accounting(access,death);
                (void)death.HostedNativeDeathOwnsMotion(owner,now);
            }
            if(owner.adoption.valid&&hostedNativeDeathMotion){huntEligible=false;trace(11);return true;}
#endif
            J21Timing timing(cfgj21::Held);
            if(!CFG_J21C_HELD_RECOVERY)state.combat.GrabKillReadyAt=0;
            const bool owns=access.DonorHeldDrive(owner,held,state,now)||state.heldStopPending;
            if(owns){huntEligible=false;trace(3);}return owns;
        }
        void Hunt(){
            const auto last=state.health.decided?state.health.lastDonorDecisionAt:state.health.handoffAt;
            // First handoff and an overdue decision are owed opportunities,
            // bounded100ms apart. Budget/parity never freeze either Jason.
            if(!bundleJ21Budget.OwedHunt(state.isAdditional?1u:0u,now,last,state.bound)&&!Optional(0))return;
            J21Timing timing(cfgj21::Hunt);hunt();
        }
        bool HuntExclusive(){
            const bool exclusive=DonorNativeDeathExclusive(now)||!bundleNativeJasonWaterKnown||bundleNativeJasonWater;
            if(exclusive){huntEligible=false;trace(11);}
#if CFG_H281_UNSTICK
            // H2.81: in water the hunt is skipped (native swim/kill owns him); a Jason stuck there for 8+ s is teleported out. Death/kill exclusivity re-proved in PrepareActor.
            if(exclusive&&bundleNativeJasonWaterKnown&&bundleNativeJasonWater&&!DonorNativeDeathExclusive(now))(void)unstickTurn(true);
#endif
            return exclusive; // exact Tick observation, never a native permission
        }
        void Convergence(){debt=access.Calls();if(!cfgj21::ConvergenceLaneEnabled(cfgj21::ConvergenceEnabled,HostedRoutineActive())&&!cfgj21::LeanEnabled)return;
            if(!Optional(1))return;
            if(cfgj21::LeanEnabled){
                if(debt>=cfgbundle::CallCap)return;
                BundleAccess a(cfgbundle::ReadCap,debt);a.J21LeanConvergence(owner,now);debt+=a.Calls();
                bundleReport.reads+=a.Reads();bundleReport.calls+=a.Calls();
                bundleReport.maxReads=(std::max)(bundleReport.maxReads,a.Reads());bundleReport.maxCalls=(std::max)(bundleReport.maxCalls,debt);
                return;
            }
            J21Timing timing(cfgj21::Convergence,true);
            if(debt>=cfgbundle::CallCap){++bundleDonorConvergence.deferrals;return;}
            BundleAccess a(cfgbundle::ReadCap,debt);const bool publish=a.DonorKillTeamConvergence(owner,now);debt+=a.Calls();
            bundleReport.reads+=a.Reads();bundleReport.calls+=a.Calls();
            bundleReport.maxReads=(std::max)(bundleReport.maxReads,a.Reads());
            bundleReport.maxCalls=(std::max)(bundleReport.maxCalls,a.Calls());
            if(publish&&debt<cfgbundle::CallCap){BundleAccess continuation(cfgbundle::ReadCap,debt);
                bundleDonorConvergence.publishAt=now+25;
                if(continuation.DonorNativeOwner(owner))continuation.DonorConvergencePublish();debt+=continuation.Calls();
                bundleReport.reads+=continuation.Reads();bundleReport.calls+=continuation.Calls();
                bundleReport.maxReads=(std::max)(bundleReport.maxReads,continuation.Reads());}}
        void Emotes(){if(!cfgj21::EmotesEnabled||!Optional(2))return;
            debt+=ServiceDonorNativeEmotes(owner,now,debt);bundleReport.maxCalls=(std::max)(bundleReport.maxCalls,debt);}
    } turn{access,owner,state,held,work,gate,now
#if CFG_H281_UNSTICK
        ,unstick
#endif
    };
    cfgdonorturn::Service(turn);
    const auto& convergence=bundleDonorConvergence;
    bundleReport.j2ConvergenceTurns=convergence.turns;bundleReport.j2ConvergenceGrants=convergence.grants;
    bundleReport.j2ConvergenceAmbiguous=convergence.ambiguousGrants;bundleReport.j2ConvergenceDeferrals=convergence.deferrals;
    bundleReport.j2ConvergencePublications=convergence.publications;bundleReport.j2ConvergenceBrainResumes=convergence.brainResumes;
    bundleReport.j2ConvergenceBrainLost=convergence.brainLost;bundleReport.j2ConvergenceActive=convergence.donor.CounselorConvergenceActive?1u:0u;
    bundleReport.j2ConvergenceUncertainPublications=convergence.uncertainPublications;
    bundleReport.j2ConvergenceTravel=static_cast<unsigned>(convergence.donor.ConvergenceTravelCounselorCount);
#if CFG_H281_UNSTICK
    {auto& row=bundleReport.h281Unstick[state.isAdditional?1u:0u];const auto& u=state.unstick.rows;
     row.waterEscapes=u.waterEscapes;row.waterRefused=u.waterRefused;row.frozenEscapes=u.frozenEscapes;row.frozenRefused=u.frozenRefused;row.lastReason=u.lastReason;
     bundleReport.h288FailedRings[state.isAdditional?1u:0u]=state.unstick.failedRings;bundleReport.h288OtherTargets[state.isAdditional?1u:0u]=state.unstick.otherTargets;}
#endif
    bundleReport.donorCensusComplete=bundleDonorRoster.censusComplete?1u:0u;
    bundleReport.donorCensusCells=bundleDonorRoster.cells;
    bundleReport.donorRegistered=static_cast<unsigned>(bundleDonorRoster.roster.count);
    bundleReport.donorLive=0;
    for(unsigned i=0;i<bundleReport.donorRosterRows;++i){auto& row=bundleReport.donorRoster[i];
        row.blocked=bundleDonorRoster.roster.DonorIsJasonAITargetBlocked(row.pawn,now)?1u:0u;
        row.kind=row.pawn==bundleDonorRoster.roster.LocalCounselorTarget?1u:row.kind;
        if(row.live&&bundleDonorRoster.roster.DonorFindJasonAITargetIndex(row.pawn)>=0)++bundleReport.donorLive;
    }
    if(state.health.uncertainEvents!=uncertainBefore){
        cfgclosedtrace::Mark("donor-action-uncertain",owner.actor.pawn.object,
            (static_cast<std::uint64_t>(state.health.uncertainAction)<<32)|state.health.uncertainEvents);
        if(!state.isAdditional&&state.health.uncertainAction==cfgdonorhunt::UncertainAction::Teleport)
            bundleCarMorph.next=(std::max)(bundleCarMorph.next,state.chase.NextDistanceTeleportAt);
    }
    bundleReport.reads+=access.Reads();bundleReport.calls+=access.Calls();
    bundleReport.maxReads=(std::max)(bundleReport.maxReads,access.Reads());
    bundleReport.maxCalls=(std::max)(bundleReport.maxCalls,access.Calls());bundleReport.error=access.Error();
    MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
}
#endif
