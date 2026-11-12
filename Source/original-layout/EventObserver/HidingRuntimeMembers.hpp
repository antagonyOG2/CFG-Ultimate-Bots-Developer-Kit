// Included inside BundleAccess. Native hidden-counselor interaction, not slash
// damage through walls. The verified reciprocal CurrentHidingSpot chain replaces
// the donor's nearby registry search for already registered offline counselors.
    void HidingRefused(unsigned reason)noexcept{
        ++bundleHidingRefused;HostedLaneRefuse(cfgh1::Lane::Hiding,reason);
    }
    bool HidingSelection(std::uintptr_t pawn,std::uintptr_t& spot,std::uintptr_t& component,std::uintptr_t& locked,cfgspawn::Vec3& point){
        using namespace cfgaction::disk;spot=component=locked=0;point={};
        ++bundleReport.hidingSelectionChecks;bundleReport.hidingSelectionStage=1;
        bundleReport.hidingCandidate=pawn;bundleReport.hidingCandidateSpot=0;
        std::uintptr_t type=0,manager=0,best=0,owner=0,hidden=0,scene=0;
        cfgprobe::lifetime::SlotObservation spotSlot{},compSlot{};
        bool living=false;
#ifdef CFG_H1_JASON
        if(HostedRoutineActive()){
            cfgdonorhunt::CounselorObservation observation{};
#if CFG_H233_HIDING_ROSTER
            living=HostedRosterCounselor(pawn,observation);
            hostedH1Report.hidingRosterProof=living?0u:observation.failure?observation.failure:7u;
#else
            living=DonorConvergenceRead(pawn,observation)&&observation.Live();
#endif
        }else living=KnownLiving(pawn);
#else
        living=KnownLiving(pawn);
#endif
        if(!living){bundleReport.hidingSelectionStage=2;return false;}
        if(!r_.Read(pawn+CurrentHidingSpot,spot)){bundleReport.hidingSelectionStage=3;return false;}
        bundleReport.hidingCandidateSpot=spot;
        if(!spot){bundleReport.hidingSelectionStage=4;return cfgobserver::Same(r_,pawn+CurrentHidingSpot,std::uintptr_t{});}
        bundleReport.hidingSelectionStage=5;
        const auto invalid=[&](unsigned detail){
#if CFG_H233_HIDING_ROSTER
            if(HostedRoutineActive())hostedH1Report.hidingDetail=detail;
#else
            (void)detail;
#endif
            bundleReport.hidingSelectionStage=6;return false;
        };
        if(!NativeClass(HidingReflection,HidingConstructor,type)||!cfgspawn::ObjectIs(r_,spot,type))return invalid(1);
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,spot,spotSlot)||!spotSlot.HasAssignedSerial())return invalid(2);
        if(!r_.Read(spot+HidingCounselor,hidden)||hidden!=pawn)return invalid(3);
        if(!Manager())return invalid(4);
        if(!r_.Read(bundleJason.pawn.object+cfghunter::disk::Manager,manager)||
           !r_.Read(manager+cfgvehicleinteraction::disk::LockedComponent,locked)||!r_.Read(manager+0x210,best))return invalid(5);
        // Preserve native side-specific bed selection. Only a typed component
        // owned by THIS hiding actor can replace its authored killer component.
        const auto belongs=[&](std::uintptr_t candidate){std::uintptr_t native=0,actual=0;
            return candidate&&NativeClass(cfgvehicleinteraction::disk::ILLInteractableComponent,
                cfgvehicleinteraction::disk::ILLInteractableComponentConstructor,native)&&
                cfgspawn::ObjectIs(r_,candidate,native)&&r_.Read(candidate+0xE0,actual)&&actual==spot;};
        if(bundleHiding.Active()&&bundleHiding.counselor==pawn&&bundleHiding.spot==spot&&belongs(bundleHiding.component))component=bundleHiding.component;
        else if(belongs(locked))component=locked;
        else if(belongs(best))component=best;
        else if(!r_.Read(spot+KillerHidingComponent,component)||!belongs(component)){bundleReport.hidingSelectionStage=7;return false;}
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,component,compSlot)||
           !r_.Read(component+0x20,owner)||owner!=spot||
           !NativeClass(cfgposition::disk::SceneClassCache,cfgposition::disk::SceneConstructorCache,scene)||
           !cfgspawn::ObjectIs(r_,component,scene)||!r_.Read(component+cfgposition::disk::SceneWorldTranslation,point)||!cfgspawn::Finite(point)||
           !cfgjason::SameSlot(r_,context.objectArray,spotSlot)||!cfgjason::OwnedComponentSlot(r_,context.objectArray,compSlot)||
           !cfgobserver::Same(r_,component+0x20,spot)||!cfgobserver::Same(r_,component+0xE0,spot)||
           !cfgobserver::Same(r_,pawn+CurrentHidingSpot,spot)||!cfgobserver::Same(r_,spot+HidingCounselor,pawn)){
            bundleReport.hidingSelectionStage=8;return false;}
        bundleReport.hidingSelectionStage=9;
        return true;
    }
#if CFG_H264_HIDING_ROSTER
    // H2.64: live captures always showed the fallback host here while the donor chased someone else, so a hider was never considered.
    // Donor FindNearbyOccupiedHidingSpot looks at every occupied spot within 6 m; HidingSelection still proves the one chosen.
    std::uintptr_t HostedNearestHider(std::uint64_t now){
        using namespace cfgaction::disk;if(bundleHiding.Active())return bundleHiding.counselor;
        auto& roster=bundleDonorRoster.roster;const unsigned total=roster.count>0?(std::min)(static_cast<unsigned>(roster.count),64u):0u;bool any=false;
        for(unsigned i=0;i<total&&!any;++i){std::uintptr_t spot=0;any=roster.actors[i]&&r_.Read(roster.actors[i]+CurrentHidingSpot,spot)&&spot;}
        cfgspawn::Vec3 from{};if(!any||!Location(bundleJason.pawn.object,from))return 0;
        const auto chasing=CFG_H266_HIDER_APPROACH?bundleDonorPrimary.stallTarget:std::uintptr_t{};std::uintptr_t chosen=0;float nearest=(std::numeric_limits<float>::max)();
        for(unsigned i=0;i<total;++i){const auto candidate=roster.actors[i];std::uintptr_t spot=0;cfgspawn::Vec3 at{};
            if(!candidate||roster.DonorIsJasonAITargetBlocked(candidate,now)||!r_.Read(candidate+CurrentHidingSpot,spot)||!spot||!Location(candidate,at))continue;
            const float dx=at.x-from.x,dy=at.y-from.y,squared=dx*dx+dy*dy;
            if(std::isfinite(squared)&&squared<nearest&&(squared<600.f*600.f||candidate==chasing)){nearest=squared;chosen=candidate;}}
        if(chosen&&bundleHiderApproach.marks<32){++bundleHiderApproach.marks;cfgclosedtrace::Mark("h264-hider-near",chosen,static_cast<std::uint64_t>(std::sqrt(nearest)));}
        return chosen;
    }
#endif
    bool HostedHiderApproach(std::uint64_t now,const cfgspawn::Vec3& point,float squared){
#if CFG_H266_HIDER_APPROACH
        // H2.66: a path to the person inside fails, so the Jason stalled outside search range. Walk to the search point; the donor chase yields meanwhile.
        auto& a=bundleHiderApproach;if(now<a.blockedUntil)return false; // online and offline: the same path failure stalls both
        a.until=now+1500;if(now<a.next)return true;a.next=now+900;
        // H2.68: live, an accepted move left him 14.42 m away for 7 s (a closed door, most likely) and this lane returned before DoorRecovery could act.
        const float distance=std::sqrt(squared);a.stalls=a.lastDistance>=0&&std::fabs(a.lastDistance-distance)<60.f?a.stalls+1:0;a.lastDistance=distance;
        const auto unreachable=[&](const char* tag){a.stalls=0;a.failures=0;a.lastDistance=-1.f;a.until=0;
            cfgclosedtrace::Mark(tag,bundleHiding.counselor,static_cast<std::uint64_t>(distance));
            if(distance<=3000.f){a.forceSearch=true;return true;} // live: paths to the search point failed from 6.5 m; the search phase's own native teleport lands beside the spot
            a.blockedUntil=now+6000;return false;};
        if(a.stalls>=3)return unreachable("h268-hider-approach-stalled");
        (void)RunState(); // the donor chase is yielding, so nothing else keeps him running on this trip
        cfgtrap::MoveLocationParameters p;p.destination=point;p.acceptance=65;p.allowPartial=1;p.result=255;
        if(!CarCall(cfgtrap::disk::MoveToLocation,cfgtrap::disk::MoveToLocationExecutor,cfgtrap::disk::MoveToLocationCode,bundleJason.killerController.object,&p)){a.until=0;return false;}
        if(p.result==0){if(++a.failures>=4)return unreachable("h268-hider-approach-failed");}else a.failures=0;
        return true;
#else
        (void)now;(void)point;(void)squared;return false;
#endif
    }
    bool HostedRosterLive(std::uintptr_t pawn){
#if defined(CFG_H1_JASON)&&defined(CFG_H1_OBSERVE)&&defined(CFG_RETAINED_GAMEPLAY_BUNDLE)&&defined(CFG_FRONTEND_BUNDLE)
        cfgdonorhunt::CounselorObservation observation{};return HostedRosterCounselor(pawn,observation);
#else
        (void)pawn;return false;
#endif
    }
    // H2.66: online this lane's own list is the host and one AI slot, so it went idle once the host died or escaped.
    bool HostedDonorTargetFallback(std::uintptr_t& pawn){
#if defined(CFG_H1_JASON)&&defined(CFG_H1_OBSERVE)&&defined(CFG_RETAINED_GAMEPLAY_BUNDLE)&&defined(CFG_FRONTEND_BUNDLE)&&CFG_H266_TARGET_FALLBACK
        const auto donor=bundleDonorPrimary.stallTarget;const auto at=bundleDonorPrimary.stallTargetAt;const auto now=GetTickCount64();
        if(!HostedRoutineActive()||!donor||!at||now<at)return false;
        if(now-at>2000&&now>=bundleHiderApproach.until&&!bundleHiding.Active())return false; // the donor chase yields during those two, so its target is not refreshed
        cfgdonorhunt::CounselorObservation observation{};if(!HostedRosterCounselor(donor,observation))return false;
        pawn=donor;return true;
#else
        (void)pawn;return false;
#endif
    }
    bool HidingSafe(){
        bundleReport.hidingSafeFailure=0;bool dying=false;cfgkill::Context death;
        // The stock initialized death actor persists while inactive. Its mere
        // presence is not a live finishing kill (and blocked every hiding kill).
        // Full typed/world/owned native context proof still rejects ambiguity.
#ifdef CFG_H1_JASON
        if(HostedRoutineActive()){
            std::uint8_t first=0,again=0;
            if(!HostedContext(false)||!Jason()||!cfgjason::SameSlot(r_,context.objectArray,bundleJason.pawn)||
               !r_.Read(bundleJason.pawn.object+cfgescape::disk::DyingFlags,first)||
               !r_.Read(bundleJason.pawn.object+cfgescape::disk::DyingFlags,again)||first!=again){bundleReport.hidingSafeFailure=1;return false;}
            dying=(first&cfgescape::disk::DyingMask)!=0;
        }else
#endif
        if(!DeathScope(dying)){bundleReport.hidingSafeFailure=1;return false;}
        if(dying){bundleReport.hidingSafeFailure=2;return false;}
        if(!DeathContext(death)){bundleReport.hidingSafeFailure=3;return false;}
        if(death.active){bundleReport.hidingSafeFailure=4;return false;}
        if(bundleKill.policy.interactionAttempted){bundleReport.hidingSafeFailure=5;return false;}
        if(bundleKill.policy.nativeActiveObserved){bundleReport.hidingSafeFailure=6;return false;}
        return true;
    }
    bool HidingAlternative(std::uintptr_t target){
        // Scheduling only: the next selector independently proves liveness.
        // Do not add a full roster scan to a native interaction callback.
        if((bundleHumanTarget.object?bundleHumanTarget.object:plan.pawn)!=target)return true;
        for(unsigned i=0;i<bundleReport.bots;++i)
            if(bundleReport.roster[i].pawn&&bundleReport.roster[i].pawn!=target)return true;
        return false;
    }
    bool HidingTargetAllowed(std::uintptr_t target,std::uint64_t now)const{
        return target!=bundleHiding.ignoredCounselor||now>=bundleHiding.ignoreUntil;
    }
    bool HidingCallRecovery(cfghiding::Phase phase,std::uintptr_t component){
        using namespace cfgaction::disk;
        // Every recovery action, including Cancel and Unlock, must preserve an
        // actual Jason death/finishing context. No synthetic montage cleanup.
        if(!HidingSafe())return false;
        if(!bundleHiding.attempts)return true; // no owned input/lock/stun to clean up
        if(phase==cfghiding::Phase::Cancel){std::uintptr_t manager=0,locked=0;
            if(!Manager()||!r_.Read(bundleJason.pawn.object+cfghunter::disk::Manager,manager)||
               !r_.Read(manager+cfgvehicleinteraction::disk::LockedComponent,locked)||(locked&&locked!=component))return false;
            return CarCall(CLIENT_CancelInteractAttempt,CLIENT_CancelInteractAttemptExecutor,
                CLIENT_CancelInteractAttemptCode,bundleJason.pawn.object,nullptr);}
        if(phase==cfghiding::Phase::Unlock){std::uintptr_t manager=0,locked=0;
            if(!Manager()||!r_.Read(bundleJason.pawn.object+cfghunter::disk::Manager,manager)||
               !r_.Read(manager+cfgvehicleinteraction::disk::LockedComponent,locked))return false;
            if(!locked)return true;
            if(locked!=component)return false;
            return CarCall(CLIENT_UnlockInteraction,CLIENT_UnlockInteractionExecutor,CLIENT_UnlockInteractionCode,manager,&component);
        }
        using namespace cfgofflinerecovery::disk;
        return CarCall(EndStun,EndStunExecutor,EndStunExecutorCode,bundleJason.pawn.object,nullptr);
    }
    bool HidingPhase(std::uintptr_t target,std::uint64_t now){
        HostedLaneEnter(cfgh1::Lane::Hiding);
        bundleReport.hidingPhaseStartStage=1;
        auto& s=bundleHiding;using P=cfghiding::Phase;
        if(s.Active()&&now<s.next)return true;
        std::uintptr_t spot=0,component=0,locked=0;cfgspawn::Vec3 point{},jason{};
        if(!HidingSelection(target,spot,component,locked,point)){
            bundleReport.hidingPhaseStartStage=2;
            if(s.Active()){HidingRefused(2);s.Clear();return true;}return false;}
        if(!spot){bundleReport.hidingPhaseStartStage=3;if(s.Active())s.Clear();return false;}
        // The ignore entry controls new acquisition, never owed cleanup.
        if(!s.Active()&&s.Ignored(target,spot,now)){bundleReport.hidingPhaseStartStage=4;bundleSchedule.selectAt=now;return true;}
        if(!Location(bundleJason.pawn.object,jason)){bundleReport.hidingPhaseStartStage=5;HostedLaneRefuse(cfgh1::Lane::Hiding,14);return true;}
        const float dx=point.x-jason.x,dy=point.y-jason.y;
        const float squared=dx*dx+dy*dy;
        bool motionReady=false;
        if(!s.Active()){
#if CFG_H266_HIDER_APPROACH
            const bool forced=bundleHiderApproach.forceSearch;bundleHiderApproach.forceSearch=false;
#else
            const bool forced=false;
#endif
            if(squared>600.f*600.f&&!forced){bundleReport.hidingPhaseStartStage=6;return HostedHiderApproach(now,point,squared);} // offline: ordinary chase closes donor nearby-acquisition range
            bool stunned=false,attacking=false,busy=false;std::uintptr_t held=0;
            if(!HidingSafe()){bundleReport.hidingPhaseStartStage=7;return true;}
            if(!Held(held)){bundleReport.hidingPhaseStartStage=8;return true;}
            if(!NativeBool(cfghunter::disk::Stunned,stunned)){bundleReport.hidingPhaseStartStage=9;return true;}
            if(!NativeBool(cfghunter::disk::Attacking,attacking)){bundleReport.hidingPhaseStartStage=10;return true;}
            if(!GrabBusy(busy)){bundleReport.hidingPhaseStartStage=11;return true;}
            if(held||stunned||attacking||busy){bundleReport.hidingPhaseStartStage=12;return true;} // do not acquire over an unrelated native action
            s.Begin(target,spot,component,now);motionReady=true;
#if CFG_H266_HIDER_APPROACH
            if(forced&&now>5000){s.started=now-5000;cfgclosedtrace::Mark("h268-hider-forced-search",target,spot);} // go straight to the reposition step
#endif
            bundleReport.hidingPhaseStartStage=13;
        }
        if(s.counselor!=target||s.spot!=spot){s.Clear();return true;}
        s.component=component;
        if(s.Recovery()){
            const auto phase=s.phase;s.next=now+250;
            // Mark phase before any ambiguous native call; never replay it.
            s.phase=phase==P::Cancel?P::Unlock:phase==P::Unlock?P::Recover:P::Retreat;
            if(!HidingCallRecovery(phase,component)){HidingRefused(3);s.Clear();bundleSchedule.selectAt=now;return true;}
            if(phase==P::Recover)++s.recoveries;
            return true;
        }
        if(s.phase==P::Retreat){
            if(!HidingSafe()){HidingRefused(4);s.Clear();return true;}
            const float length=std::sqrt(squared);cfgtrap::MoveLocationParameters p;
            p.destination=length>1?cfgspawn::Vec3{jason.x+dx/length*-450.f,jason.y+dy/length*-450.f,jason.z}:jason;
            p.acceptance=75;p.result=255;s.Clear();
            CarCall(cfgtrap::disk::MoveToLocation,cfgtrap::disk::MoveToLocationExecutor,cfgtrap::disk::MoveToLocationCode,
                bundleJason.killerController.object,&p);bundleSchedule.selectAt=now;return true;
        }
        switch(s.ObserveLock(locked,now)){
        case cfghiding::LockDecision::Owned:s.phase=P::Locked;s.next=now+250;return true;
        case cfghiding::LockDecision::ForeignGrace:s.next=now+250;return true;
        case cfghiding::LockDecision::YieldForeign:
                // CFG Unlock requires the exact owned component. Yield a
                // foreign native owner; never cancel/unlock it speculatively.
                s.Ignore(now,HidingAlternative(target));s.Clear();bundleSchedule.selectAt=now;return true;
        case cfghiding::LockDecision::Expire:s.Abandon(now,HidingAlternative(target));return true;
        case cfghiding::LockDecision::Continue:break;
        }
        // A successful sixth attempt may have acquired a native lock. Check
        // that lock FIRST, using the donor's episode-based 12-second bound.
        s.lockedAt=0;
        if(s.phase==P::Locked||s.phase==P::Confirm)s.phase=P::Approach;
        bool stunned=false,attacking=false,busy=false;std::uintptr_t held=0;
        if(!motionReady&&(!HidingSafe()||!Held(held)||!NativeBool(cfghunter::disk::Stunned,stunned)||
           !NativeBool(cfghunter::disk::Attacking,attacking)||!GrabBusy(busy))){HidingRefused(5);return true;}
        if(held||stunned||attacking||busy){s.next=now+250;return true;}
        // Native TeleportTo performs collision adjustment, unlike a direct
        // transform write. One attempt only; never repeat an ambiguous dispatch.
        if(!s.repositionAttempted&&(now-s.started>=5000||s.attempts>=2)){
            s.repositionAttempted=true;s.phase=P::RepositionStop;
        }
        if(s.phase==P::RepositionStop){
            s.phase=P::Reposition;s.next=now+100;
            if(!CarCall(cfgtrap::disk::StopMovement,cfgtrap::disk::StopMovementExecutor,cfgtrap::disk::StopMovementCode,
                bundleJason.killerController.object,nullptr)){HidingRefused(6);s.phase=P::Approach;}
            return true;
        }
        if(s.phase==P::Reposition){
            std::uintptr_t fn=0;if(!TeleportFunction(fn)){HidingRefused(7);s.phase=P::Approach;return true;}
            constexpr float offsets[4][2]{{95,0},{-95,0},{0,95},{0,-95}};
            const unsigned index=s.repositionIndex++;
            cfgpursuit::TeleportPacket p;p.location={point.x+offsets[index][0],point.y+offsets[index][1],jason.z};
            p.rotation.y=std::atan2(point.y-p.location.y,point.x-p.location.x)*57.2957795131f;
            s.phase=P::Approach;s.next=now+250; // mark BEFORE dispatch; ambiguous return never replays
            if(!Invoke(bundleJason.pawn.object,fn,&p)){HidingRefused(8);return true;}
            s.repositioned=p.result==1;
            if(!s.repositioned&&s.repositionIndex<4)s.phase=P::Reposition;
            return true;
        }
        if(squared>85.f*85.f&&!s.repositioned){
            cfgtrap::MoveLocationParameters p;p.destination=point;p.acceptance=65;p.result=255;
            s.next=now+1000;s.phase=P::Approach;
            if(!CarCall(cfgtrap::disk::MoveToLocation,cfgtrap::disk::MoveToLocationExecutor,cfgtrap::disk::MoveToLocationCode,
                bundleJason.killerController.object,&p))HidingRefused(9);return true;
        }
        if(!s.AttemptDue(now)){s.next=now+100;return true;}
        if(s.phase==P::Approach){s.phase=P::Stop;return true;}
        if(s.phase==P::Stop){s.phase=P::FaceActor;
            if(!CarCall(cfgtrap::disk::StopMovement,cfgtrap::disk::StopMovementExecutor,cfgtrap::disk::StopMovementCode,
                bundleJason.killerController.object,nullptr)){HidingRefused(10);s.phase=P::Approach;}return true;}
        if(s.phase==P::FaceActor){cfgtrap::RotationParameters p;p.rotation.y=std::atan2(dy,dx)*57.2957795131f;s.phase=P::Face;
            if(!CarCall(cfgtrap::disk::K2_SetActorRotation,cfgtrap::disk::K2_SetActorRotationExecutor,cfgtrap::disk::K2_SetActorRotationCode,
                bundleJason.pawn.object,&p)||p.result!=1){HidingRefused(11);s.phase=P::Approach;}return true;}
        if(s.phase==P::Face){cfgspawn::Vec3 rotation{};rotation.y=std::atan2(dy,dx)*57.2957795131f;s.phase=P::Attempt;
            if(!CarCall(cfgtrap::disk::SetControlRotation,cfgtrap::disk::SetControlRotationExecutor,cfgtrap::disk::SetControlRotationCode,
                bundleJason.killerController.object,&rotation)){HidingRefused(12);s.phase=P::Approach;}return true;}
        if(s.phase==P::Attempt){std::uintptr_t manager=0;
            if(!Manager()||!r_.Read(bundleJason.pawn.object+cfghunter::disk::Manager,manager)||
               !cfgobserver::Same(r_,manager+cfgvehicleinteraction::disk::LockedComponent,std::uintptr_t{}))return true;
            // Repeat final facing and owned selection in this same dispatch:
            // native AI yaw changes between the earlier preparation callbacks.
            cfgtrap::RotationParameters body;body.rotation.y=std::atan2(dy,dx)*57.2957795131f;
            if(!CarCall(cfgtrap::disk::K2_SetActorRotation,cfgtrap::disk::K2_SetActorRotationExecutor,cfgtrap::disk::K2_SetActorRotationCode,
                bundleJason.pawn.object,&body)||body.result!=1||
               !CarCall(cfgtrap::disk::SetControlRotation,cfgtrap::disk::SetControlRotationExecutor,cfgtrap::disk::SetControlRotationCode,
                bundleJason.killerController.object,&body.rotation))return true;
            std::uintptr_t currentSpot=0,currentComponent=0,currentLock=0;cfgspawn::Vec3 currentPoint{};
            if(!HidingSelection(target,currentSpot,currentComponent,currentLock,currentPoint)||
               currentSpot!=spot||currentComponent!=component||currentLock)return true;
            cfgvehicleaction::Attempt p;p.component=component;p.autoLock=1;s.MarkAttempt(now);
            if(!CarCall(cfgvehicleinteraction::disk::Attempt,cfgvehicleinteraction::disk::Executor,cfgvehicleinteraction::disk::ExecutorCode,manager,&p))
                HidingRefused(13);return true;
        }
        return true;
    }
