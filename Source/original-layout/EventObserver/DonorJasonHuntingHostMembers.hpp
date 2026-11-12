#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
    std::uintptr_t DonorNativeNearest(DonorNativeJasonRuntime& state,cfgdonorhunt::Targets& roster,
        std::uint64_t now,std::uintptr_t excluded=0){
        auto nearest=state.frame.Nearest(roster,now,excluded);
        {const auto& st=cfgdonorframe::Frame::Stats();bundleReport.h286NearestEmpty=st.empty;bundleReport.h286NearestSkips=st.skips;bundleReport.h286NearestChosen=st.chosen;}
        if(CFG_J3_LATE_HUNT&&!nearest&&!state.quarantined){
            nearest=state.frame.Nearest(roster,now,excluded,true);
            if(nearest)++bundleReport.j3LateTargetRetries;
        }
        // FJB's temporary trap-victim restriction does not alter the full
        // registered census or the independent reinforcement's target set.
        const auto victim=bundleTrapPriority.victim;
        const bool priority=CFG_J214_TRAP_PRIORITY?
           (!state.isAdditional&&bundleTrapPriority.Active(now)&&victim!=excluded&&
            roster.DonorFindJasonAITargetIndex(victim)>=0&&state.frame.ObservedLive(victim)):
           (nearest&&!state.isAdditional&&bundleTrapPriority.Active(now)&&victim!=excluded&&
            roster.DonorFindJasonAITargetIndex(victim)>=0&&!roster.DonorIsJasonAITargetBlocked(victim,now)&&
            state.frame.ObservedLive(victim));
        bool nearbyHuman=false;
        if(CFG_J22_F1_CLOSE_HUMAN&&state.isAdditional&&excluded&&excluded==bundleDonorRoster.roster.LocalCounselorTarget){
            float humanDistance=0;const bool measured=state.frame.ObservedDistance(excluded,humanDistance);
            nearbyHuman=cfgj22::NearbyHuman(true,true,true,state.frame.ObservedLive(excluded),
                roster.DonorIsJasonAITargetBlocked(excluded,now),measured,humanDistance);
        }
        const auto chosen=nearbyHuman?excluded:priority?victim:nearest;
        state.decisionTargetLive=chosen&&state.frame.ObservedLive(chosen);
        auto& log=bundleReport.donorDrivers[state.isAdditional?1u:0u];float distance=0;
        const bool measured=state.frame.ObservedDistance(chosen,distance);
        if(chosen&&measured&&std::isfinite(distance)&&distance>=0){
            auto& buckets=bundleReport.donorMovement[state.isAdditional?1u:0u].nearestDistanceBuckets;
            const unsigned bucket=distance<2000.f?0u:distance<5000.f?1u:distance<9000.f?2u:3u;
            ++buckets[bucket];
        }
        const unsigned reason=nearbyHuman?5u:priority?2u:state.isAdditional?(excluded?3u:4u):1u;
        const auto previous=(log.cursor+15u)%16u;
        if(!log.count||log.selected[previous].at!=now||log.selected[previous].pawn!=chosen||log.selected[previous].reason!=reason){
            log.selected[log.cursor]={now,chosen,distance,reason};log.cursor=(log.cursor+1u)%16u;
            log.count=(std::min)(log.count+1u,16u);
        }
#ifdef CFG_H1_JASON
        if(state.birth.adoption.valid&&!state.isAdditional&&chosen&&measured&&std::isfinite(distance)){
            state.stallTarget=chosen;state.stallTargetAt=now;state.stallDistance=distance;
        }else{state.stallTarget=0;state.stallTargetAt=0;state.hostedStall.Reset();}
#endif
        auto& shared=bundleDonorRoster;
        if(shared.trapSampleAt&&now>=shared.trapSampleAt&&shared.trapSampleVictim==victim){
            const auto end=(std::min)(now,shared.trapSampleUntil);
            if(end>shared.trapSampleAt)bundleReport.donorTrapActiveMs+=end-shared.trapSampleAt;
        }
        shared.trapSampleAt=now;shared.trapSampleUntil=bundleTrapPriority.until;shared.trapSampleVictim=victim;
        bundleReport.donorTrapVictim=bundleTrapPriority.Active(now)?victim:0;
        return chosen;
    }
    struct DonorNativeHuntingHost {
        using Position=cfgdonorhunt::Position;
        BundleAccess& a;const cfgdonorframe::Source& source;DonorNativeJasonRuntime& state;
        cfgdonorhunt::Targets& roster;std::uint64_t now;
        cfgdonorhunt::NavRingPurpose purpose=cfgdonorhunt::NavRingPurpose::Initial;
        bool deferred=false,inputEntered=false,moveDenied=false,haveAuthored=false,runApplied=false,startupMayFinish=false;
        std::array<float,4> authored{};
        struct PositionValue {std::uintptr_t actor=0;unsigned generation=~0u;Position value{};};
        std::array<PositionValue,2> positions{};
        auto& Chase(){return state.chase;}auto& Combat(){return state.combat;}auto& Boost(){return state.boost;}
        auto& Busy(){return state.busy;}std::uint64_t Now(){return now;}
        auto& Startup(){return state.startup;}
        void ResolveCounselorRouteStartupObjectives(std::uintptr_t){
            // The accepted typed trap runtime already resolved this source;
            // do not repeat a whole-world objective census in the Tick hook.
            state.startup.ObjectivesReady=bundleTrapSources.discovered;
        }
        int ReadJasonAITrapCount(std::uintptr_t actor){int count=-1;
            if(actor!=Jason()||!a.r_.Read(actor+cfgtrap::disk::TrapCount,count)||count<0||count>32){Denied();return -1;}return count;
        }
        void StopJasonAIMovementForKnifeOnGameThread(){
            auto result=cfgdonorhunt::NativeOutcome::NotEntered;a.DonorNativeStopMovement(source.owner,result);Outcome(result,false,cfgdonorhunt::UncertainAction::Stop);
            if(result==cfgdonorhunt::NativeOutcome::NotEntered)deferred=true;
        }
        void FinishJasonAIStartupTrapSetupOnGameThread(bool){
#ifdef CFG_H1_JASON
        HostedLaneEnter(cfgh1::Lane::CabinExit);
#endif
startupMayFinish=true;}
        std::uintptr_t World(){return source.owner.source.world;}std::uintptr_t Jason(){return state.chase.Jason;}
        bool ModuleReady(){const bool okay=a.NativeEnvironmentAllowed();
            if(!okay){deferred=true;state.health.Refused(cfgdonorhunt::DriverRefusal::Retention);}return okay;}
        bool NavigationPending(){return deferred||state.quarantined||state.navigation.stage!=cfgdonorhunt::NavRingStage::Idle;}
        const cfgprobe::lifetime::SlotObservation* Slot(std::uintptr_t actor){
            if(actor==source.owner.actor.pawn.object)return &source.owner.actor.pawn;
            for(unsigned i=0;i<source.count;++i)if(source.targets[i].object==actor)return &source.targets[i];return nullptr;
        }
        bool ReadableActor(std::uintptr_t actor){return Slot(actor)!=nullptr;}
        bool ReadableActorAndClass(std::uintptr_t actor){return ReadableActor(actor);}
        bool ReadableController(std::uintptr_t actor){return actor==source.owner.actor.killerController.object;}
        bool ReadableControllerAndClass(std::uintptr_t actor){return ReadableController(actor);}
        void Denied(){if(a.r_.reads>=a.r_.cap||a.calls_>=cfgbundle::CallCap)deferred=true;}
        void Outcome(cfgdonorhunt::NativeOutcome result,bool input,cfgdonorhunt::UncertainAction kind){
            if(result==cfgdonorhunt::NativeOutcome::NotEntered){
                // A proved no-op/throttle of Running is not a failed action.
                if(kind==cfgdonorhunt::UncertainAction::Running&&now<state.runStateAt)return;
                deferred=true;state.health.Refused(cfgdonorhunt::DriverRefusal::NativeEntry);
            }
            else if(input)inputEntered=true;
            if(result==cfgdonorhunt::NativeOutcome::Uncertain){state.MarkUncertain(kind,now);deferred=true;}
        }
        bool GetJasonAIActorLocation(std::uintptr_t actor,Position& point){
            const auto slot=Slot(actor);if(!slot)return false;
            const unsigned index=actor==Jason()?0u:1u;auto& cached=positions[index];
            if(cached.actor==actor&&cached.generation==a.Calls()){point=cached.value;return true;}
            if(actor!=Jason()){
                cfgdonorhunt::CounselorObservation current;
                if(!a.DonorNativeCounselor(source.owner,*slot,current)||!current.Live()){
                    Denied();state.frame.Invalidate();return false;
                }
            }
            cfgspawn::Vec3 value{};if(!a.DonorNativePosition(*slot,value)){Denied();return false;}
            point={value.x,value.y,value.z};cached={actor,a.Calls(),point};return true;
        }
        std::uintptr_t FindNearestJasonAICounselorTarget(std::uintptr_t actor,std::uintptr_t excluded=0){
            if(actor!=Jason())return 0;return a.DonorNativeNearest(state,roster,now,excluded);
        }
        bool MoveToLocationFunction(){const bool okay=a.NativeCallAllowed();
            if(!okay){deferred=true;state.health.Refused(cfgdonorhunt::DriverRefusal::NativeEntry);}return okay;}
        bool SafeMoveToLocation(cfgtrap::MoveLocationParameters& packet){
            ++state.moveRequests;
            auto result=cfgdonorhunt::NativeOutcome::NotEntered;unsigned returned=255;
            const bool called=a.DonorNativeMoveLocation(source.owner,packet.destination,returned,packet.acceptance,&result,
                packet.strafe!=0,packet.pathfinding==0);
            Outcome(result,false,cfgdonorhunt::UncertainAction::Move);moveDenied=result==cfgdonorhunt::NativeOutcome::NotEntered&&deferred;
            state.lastMoveReturn=static_cast<std::uint8_t>(returned);
            packet.result=static_cast<std::uint8_t>(returned);
            if(called&&returned==2){++bundleReport.moves;bundleReport.lastMoveResult=returned;}
            return called;
        }
#if CFG_H286_BURST_LEAD
        bool ChaseBursting(){return state.isAdditional?(CFG_H267_F1_BURST&&bundleAdditional.burst.Owned()):bundleFootBurst.Owned();}
        void ChaseLead(std::uintptr_t goal,Position& point){ // heading from the proven position samples the lane already took
            if(!ChaseBursting()||!goal||goal==Jason())return;
            auto& row=bundleReport.h286Lead[state.isAdditional?1u:0u];float vx=0,vy=0;Position jason{};
            if(!bundleCutoff.Heading(goal,GetTickCount64(),vx,vy)||!GetJasonAIActorLocation(Jason(),jason)||
               !cfgchaselead::Lead(point.X,point.Y,jason.X,jason.Y,vx,vy,point.X,point.Y)){++row.unled;return;}
            ++row.led;
        }
#endif
        void IssueStockReflectedJasonMoveOnGameThread(std::uintptr_t actor){
            cfgdonorhunt::DonorIssueStockReflectedJasonMoveOnGameThread(*this,actor);
        }
        bool ReadAuthoredMovementSpeeds(float& walk,float& run){
            // H2.46: a lease-mode Shift burst temporarily multiplies these fields. Never capture boosted
            // values as the donor's authored speed; the caller already advanced its 30 s timer, so retry soon.
            if(CFG_H246_BURST&&(state.isAdditional?(CFG_H267_F1_BURST&&bundleAdditional.burst.Owned()):bundleFootBurst.Owned())){state.chase.nextRunSpeedCheckAt=now+1000;return false;}
            std::array<float,4> values{};if(!a.DonorNativeMovement(source.owner,state.movement,values)){Denied();return false;}
            authored=values;haveAuthored=true;walk=values[0];run=values[2];return true;
        }
        std::uintptr_t CurrentMovement(std::uintptr_t actor){
            std::array<float,4> values{};
            if(actor!=Jason()||!a.DonorNativeMovement(source.owner,state.movement,values)){Denied();return 0;}
            return state.movement.object;
        }
        bool ReadableMovement(std::uintptr_t movement){return movement&&movement==CurrentMovement(Jason());}
        bool ReadMovementSpeeds(std::uintptr_t movement,std::array<float,4>& values){
            cfgprobe::lifetime::SlotObservation slot{};
            if(!a.DonorNativeMovement(source.owner,slot,values)||slot.object!=movement){Denied();return false;}
            state.movement=slot;return true;
        }
        bool WriteMovementSpeeds(std::uintptr_t movement,const std::array<float,4>& values,unsigned mask){
            if(state.movement.object!=movement)return false;
            const bool okay=a.DonorNativeWriteSpeeds(source.owner,state.movement,values,mask);if(!okay)Denied();return okay;
        }
        bool GetLockedJasonInteractable(std::uintptr_t actor,bool& readable){
            bool locked=false;readable=actor==Jason()&&a.DonorNativeLocked(source.owner,locked);
            if(!readable){Denied();return true;}return locked;
        }
        bool IsJasonInNativeSpecialMove(std::uintptr_t actor,bool& readable){
            bool busy=false;readable=actor==Jason()&&a.DonorNativeQuery(source.owner,DonorQuery::SpecialMove,busy);
            if(!readable){Denied();return true;}return busy;
        }
        bool ReadBusyFlags(std::uintptr_t actor,std::array<std::uint8_t,4>& flags){
            if(actor!=Jason()||!a.DonorNativeBusyFlags(source.owner,flags)){Denied();return false;}return true;
        }
        bool SafeIsStunned(std::uintptr_t actor,bool& stunned){
            if(actor!=Jason())return false;const bool okay=a.DonorNativeQuery(source.owner,DonorQuery::Stunned,stunned);if(!okay)Denied();return okay;
        }
        bool SafeGetGrabbedCounselor(std::uintptr_t actor,std::uintptr_t& held){
            if(actor!=Jason())return false;const bool okay=a.DonorNativeHeld(source.owner,held);if(!okay)Denied();return okay;
        }
        bool IsJasonBridgeCombatBusy(std::uint64_t time,bool nearby,std::uint32_t* causes=nullptr){
            state.busy.KnifeSequenceActive=KnifeSequenceActive();state.busy.DoorBreakActive=DoorBreakActive();
            state.busy.StartupTrapSetupActive=StartupTrapSetupActive();
            const bool busy=cfgdonorhunt::DonorIsJasonBridgeCombatBusy(*this,time,nearby,causes);
            if(busy&&causes&&purpose==cfgdonorhunt::NavRingPurpose::SharedFifty){
                auto& counts=bundleReport.donorMovement[state.isAdditional?1u:0u].busyCauseHits;
                for(unsigned i=0;i<counts.size();++i)if(*causes&(std::uint32_t{1}<<i))++counts[i];
            }
            return busy;
        }
        bool TrySharedFiftyMeterMorph(){
            const auto previous=purpose;purpose=cfgdonorhunt::NavRingPurpose::SharedFifty;
            const auto before=state.chase;
            const bool candidate=state.stallDistance>5000.f&&state.stallDistance<=9000.f;
            unsigned refusal=0;const bool result=cfgdonorhunt::DonorTrySharedFiftyMeterHuntMorph(*this,now,&refusal);purpose=previous;
            auto& stats=bundleReport.donorMovement[state.isAdditional?1u:0u];
            if(result){++stats.fiftyMorphAttempts;}
            else if(candidate&&refusal<stats.fiftyMorphRefusals.size())++stats.fiftyMorphRefusals[refusal];
            if(deferred&&state.navigation.stage==cfgdonorhunt::NavRingStage::Idle){
                state.chase.nextFiftyMeterProbeAt=before.nextFiftyMeterProbeAt;
                state.chase.NextDistanceTeleportAt=before.NextDistanceTeleportAt;
            }
            return result;
        }
        void UseAuthoredRunSpeed(float run){
            if(!haveAuthored)return;auto values=authored;values[0]=run;
            runApplied=a.DonorNativeWriteSpeeds(source.owner,state.movement,values,1);
            if(!runApplied)Denied();
        }
        bool RequestRing(std::uintptr_t target,cfgdonorhunt::NavRingPurpose requested){
            if(NavigationPending())return false;
            const auto slot=Slot(target);
            if(!slot||target==Jason())return false;
            auto result=cfgdonorhunt::NavRingStage::Idle;
            if(!a.DonorNativeNavBegin(source.owner,*slot,requested,state.navigation)){Denied();return false;}
            ++bundleReport.h246Nav[state.isAdditional?1u:0u].ringBegun[static_cast<unsigned>(requested)];
            result=a.DonorNativeNavStep(source.owner,state.navigation);
            state.intendedYield=true; // new finite donor navigation intent, not repeated read refusal
            if(result==cfgdonorhunt::NavRingStage::Succeeded){state.navigation={};state.chase.Target=target;return true;}
            if(result==cfgdonorhunt::NavRingStage::Failed||result==cfgdonorhunt::NavRingStage::Invalid)state.navigation={};
            if(result==cfgdonorhunt::NavRingStage::Uncertain)state.MarkUncertain(state.navigation.uncertainTeleport?
                cfgdonorhunt::UncertainAction::Teleport:cfgdonorhunt::UncertainAction::Projection,now);
            return false;
        }
        bool RunOfflineBotsInitialTeleportOnGameThread(){return RequestRing(FindNearestJasonAICounselorTarget(Jason()),purpose);}
#if CFG_H281_UNSTICK
        // H2.81 water/frozen backstop: the SAME RequestRing (Begin + Step) the Distance/Additional morphs use. True = a ring
        // was begun or already finished; it charges the shared 20 s Morph clock exactly like a Stuck/Additional ring would.
        bool UnstickRing(std::uintptr_t target){
            if(!target||NavigationPending())return false;
            const bool done=RequestRing(target,state.isAdditional?cfgdonorhunt::NavRingPurpose::Additional:cfgdonorhunt::NavRingPurpose::Stuck);
            const bool begun=done||state.navigation.stage!=cfgdonorhunt::NavRingStage::Idle||state.quarantined;
            if(!begun)return false;
            if(state.isAdditional){state.additional.NextMorphAt=now+20000;
                if(done){state.additional.StalledSamples=0;state.additional.HasProgressSample=false;state.additional.NextMoveAt=now+1000;}}
            else{bundleCarMorph.Attempted(now);if(done)cfgdonorhunt::DonorResetChaseAfterMorph(state.chase);}
            return true;
        }
#endif
        void MarkJasonAIMorphTeleportUsed(){
            cfgdonorhunt::DonorMarkJasonAIMorphTeleportUsed(state.chase,now);
            auto& stats=bundleReport.donorMovement[state.isAdditional?1u:0u];
            if(purpose==cfgdonorhunt::NavRingPurpose::Distance)++stats.distanceTeleportConfirmed;
            else if(purpose==cfgdonorhunt::NavRingPurpose::SharedFifty)++stats.fiftyMorphConfirmed;
            if(!state.isAdditional)bundleCarMorph.Attempted(now);
        }
        std::uint64_t JasonAIMorphReadyAt(){return cfgdonorhunt::DonorJasonAIMorphReadyAt(state.chase);}
        bool RunJasonAIDistanceTeleportOnGameThread(){
            const auto previous=purpose;purpose=cfgdonorhunt::NavRingPurpose::Distance;
            const auto retry=state.chase.NextDistanceTeleportAt;
            const bool candidate=state.stallDistance>9000.f;
            const bool result=cfgdonorhunt::DonorRunJasonAIDistanceTeleportOnGameThread(*this);purpose=previous;
            auto& stats=bundleReport.donorMovement[state.isAdditional?1u:0u];
            if(result){++stats.distanceTeleportAttempts;}
            else if(candidate&&state.distanceMorphRefusal<stats.distanceTeleportRefusals.size())
                ++stats.distanceTeleportRefusals[state.distanceMorphRefusal];
            if(deferred&&state.navigation.stage==cfgdonorhunt::NavRingStage::Idle)state.chase.NextDistanceTeleportAt=retry;
            state.distanceMorphResult=result?1u:0u;
            state.distanceMorphRefusal=result?0u:state.navigation.Pending()?1u:
                now<state.chase.NextDistanceTeleportAt?2u:state.stallDistance<=9000.0f?3u:4u;
            return result;
        }
        bool GetActorForwardVector(std::uintptr_t actor,Position& value){
            if(actor!=Jason())return false;cfgspawn::Vec3 result{};
            if(!a.DonorNativeForward(source.owner,result)){Denied();return false;}value={result.x,result.y,result.z};return true;
        }
        bool ProjectJasonAINavPointOnGameThread(Position point,Position extent,Position& out){
            cfgspawn::Vec3 value{};auto result=cfgdonorhunt::NativeOutcome::NotEntered;
            const bool okay=a.DonorNativeProject(source.owner,{point.X,point.Y,point.Z},{extent.X,extent.Y,extent.Z},value,&result);
            if(result==cfgdonorhunt::NativeOutcome::NotEntered){deferred=true;state.health.Refused(cfgdonorhunt::DriverRefusal::NativeEntry);}
            if(result==cfgdonorhunt::NativeOutcome::Uncertain){state.MarkUncertain(cfgdonorhunt::UncertainAction::Projection,now);deferred=true;}
            if(okay)out={value.x,value.y,value.z};return okay;
        }
        bool SetActorLocation(std::uintptr_t actor,Position point,bool sweep,bool teleport,bool& moved){
            if(actor!=Jason()||sweep||!teleport)return false;auto result=cfgdonorhunt::NativeOutcome::NotEntered;
            const bool okay=a.DonorNativeSetLocation(source.owner,{point.X,point.Y,point.Z},moved,&result);Outcome(result,false,cfgdonorhunt::UncertainAction::SetLocation);return okay;
        }
        bool TryOfflineBotsFailsafeTeleportOnGameThread(Position point){
#ifdef CFG_H1_JASON
            HostedLaneEnter(cfgh1::Lane::Stuck);
#endif

            purpose=cfgdonorhunt::NavRingPurpose::Stuck;
            ++bundleReport.h246Nav[state.isAdditional?1u:0u].stuckFailsafeCalls;
            return cfgdonorhunt::DonorTryOfflineBotsFailsafeTeleportOnGameThread(*this,point);
        }
#ifdef CFG_H1_JASON
        void StuckRecoveryStage(unsigned stage){
            if(!source.owner.adoption.valid)return;
            if(stage==1)++hostedH1Report.stuckRecoveryAttempts;
            if(stage==7)++hostedH1Report.stuckRecoveryReturns;
            hostedH1Report.stuckRecoveryStage=stage;hostedH1Report.stuckRecoveryReads=a.Reads();
        }
#endif
        bool StartupTrapSetupActive(){return !state.isAdditional&&bundleTraps.Busy();}
        // H2.43: during a trap-slash turn the bundle lane (with its held/stun/
        // attack TrapMotionAllowed gate) stays the sole trap driver.
        bool TrapSlashTurn(){return state.trapSlashTurn;}
        // H2.45: shared J2.19 facing pair (DonorNativeFaceYaw), failure never blocks the swing.
        void FaceJasonAtYaw(float yaw){a.DonorNativeFaceYaw(source.owner,yaw);}
        void RunJasonAIStartupTrapSetupOnGameThread(){if(state.trapSlashTurn)return;if(bundleTraps.Due(now)&&!bundleTraps.Paused())a.TrapPhase(now);}
        bool DoorBreakActive(){return !state.isAdditional&&bundleDoorRecovery.breaking;}
        bool TickJasonAIDoorBreakOnGameThread(){
            return DoorBreakActive()&&a.DoorRecovery(now,0,state.chase.Target)!=cfgdoor::Result::None;
        }
        bool RunJasonAIDoorInteractionOnGameThread(){
            if(state.isAdditional)return false;
            ++bundleReport.h246Nav[0].stuckDoorCalls;
            const auto target=FindNearestJasonAICounselorTarget(Jason());Position here{},there{};
            if(!target||!GetJasonAIActorLocation(Jason(),here)||!GetJasonAIActorLocation(target,there))return false;
            const float x=there.X-here.X,y=there.Y-here.Y,z=there.Z-here.Z;
            const auto result=a.DoorRecovery(now,x*x+y*y+z*z,target);Denied();return result==cfgdoor::Result::Acted;
        }
        bool KnifeSequenceActive(){return !state.isAdditional&&bundleKnife.Busy();}
#include "DonorKnifeContinuationMembers.hpp"
        bool TryStartJasonAIKnifeThrowOnGameThread(std::uintptr_t target,float distance,std::uint64_t time){
#ifdef CFG_H1_JASON
            HostedLaneEnter(cfgh1::Lane::Knife);
#endif

            if(state.isAdditional||distance<850||distance>1550||time<bundleKnife.nextAt)return false;
            if(bundleReport.target!=target)return false; // legacy knife owner is bridged by the integration caller
            if(!bundleKnifeProbePending){bundleKnifeProbePending=true;bundleKnifeProbeIndex=bundleTargetIndex;bundleKnifeProbeAt=time;}
            return true; // existing fresh-budget knife lane owns dispatch; no duplicate throw
        }
        bool AttackReleaseReadable(){return true;}bool AttackPressReadable(){const bool okay=a.NativeCallAllowed();
            if(!okay){deferred=true;state.health.Refused(cfgdonorhunt::DriverRefusal::NativeEntry);}return okay;}
        void ReleaseAttack(){auto result=cfgdonorhunt::NativeOutcome::NotEntered;a.DonorNativeInput(source.owner,true,&result);Outcome(result,true,cfgdonorhunt::UncertainAction::Release);}
        bool PressAttack(){
            auto& pressed=state.isAdditional?state.additional.AttackPressed:state.combat.AttackPressed;
            auto& releaseAt=state.isAdditional?state.additional.AttackReleaseAt:state.combat.AttackReleaseAt;
            const bool previous=pressed;const auto previousAt=releaseAt;pressed=true;releaseAt=now+120;
            auto result=cfgdonorhunt::NativeOutcome::NotEntered;const bool okay=a.DonorNativeInput(source.owner,false,&result);
            if(result==cfgdonorhunt::NativeOutcome::NotEntered){pressed=previous;releaseAt=previousAt;}
            Outcome(result,true,cfgdonorhunt::UncertainAction::Slash);if(okay)++bundleReport.pressReturns;return okay;
        }
        bool GrabReadable(){return AttackPressReadable();}
        bool TryGrab(){
#ifdef CFG_H1_JASON
            HostedLaneEnter(cfgh1::Lane::Grab);
#endif
            auto result=cfgdonorhunt::NativeOutcome::NotEntered;
            bool okay=false;
#ifdef CFG_H1_JASON
            if(source.owner.adoption.valid){
                BundleAccess grab(cfgbundle::ReadCap,a.calls_);
                BundleAccess::ChildAccounting accounting(a,grab);
                JasonIdentity identity{};
                if(!grab.Context())++hostedH1Report.grabAdmissionRefusals[0];
                else if(!grab.JasonAdmission(identity))++hostedH1Report.grabAdmissionRefusals[1];
                else if(!identity.hosted)++hostedH1Report.grabAdmissionRefusals[2];
                else okay=grab.DonorScalarInvoke(source.owner,DonorScalarAction::Grab,nullptr,&result);
                ++hostedH1Report.grabEntry[static_cast<unsigned>(result)];
            }else
#endif
#ifdef CFG_H1_JASON
            {if(HostedRoutineActive())++hostedH1Report.grabAdmissionRefusals[3];
             okay=a.DonorScalarInvoke(source.owner,DonorScalarAction::Grab,nullptr,&result);}
#else
            okay=a.DonorScalarInvoke(source.owner,DonorScalarAction::Grab,nullptr,&result);
#endif
            Outcome(result,true,cfgdonorhunt::UncertainAction::Grab);return okay;
        }
        void UseReflectedGrabKill(unsigned direction){
#ifdef CFG_H1_JASON
            HostedLaneEnter(cfgh1::Lane::Grab);
#endif

            if(direction>=4)return;constexpr DonorScalarAction actions[]{DonorScalarAction::Top,DonorScalarAction::Bottom,DonorScalarAction::Left,DonorScalarAction::Right};
            auto result=cfgdonorhunt::NativeOutcome::NotEntered;a.DonorScalarInvoke(source.owner,actions[direction],nullptr,&result);Outcome(result,true,cfgdonorhunt::UncertainAction::GrabKill);
#ifdef CFG_H1_JASON
            if(HostedRoutineActive())++hostedH1Report.grabKillEntry[static_cast<unsigned>(result)];
#endif
            if(CFG_J23_GRAB_TIMING){auto& held=state.isAdditional?bundleDonorHeldAdditional:bundleDonorHeldPrimary;
                held.timing.Frozen(direction,result!=cfgdonorhunt::NativeOutcome::NotEntered,
                    result==cfgdonorhunt::NativeOutcome::ReturnedTrue||result==cfgdonorhunt::NativeOutcome::ReturnedFalse,
                    result==cfgdonorhunt::NativeOutcome::Uncertain);
                bundleReport.j23GrabTiming[state.isAdditional?1u:0u]=held.timing.report;}
        }
        bool SelectedInteractableIsSCDoor(){bool selected=false;if(!a.DonorNativeSelectedDoor(source.owner,selected))Denied();return selected;}
        bool RunJasonAIChaseUpdateOnGameThread(){
            if(state.frame.Status()!=cfgdonorframe::State::Ready){deferred=true;return true;}
            const auto before=state.chase;const bool result=cfgdonorhunt::DonorRunJasonAIChaseUpdateOnGameThread(*this);
            if(moveDenied){state.chase.Target=before.Target;state.chase.PathLocked=before.PathLocked;state.chase.PathLockUntil=before.PathLockUntil;}
            if(deferred&&state.navigation.stage==cfgdonorhunt::NavRingStage::Idle&&!before.InitialTeleportAttempted)
                state.chase.InitialTeleportAttempted=false;
            if(deferred&&!runApplied)state.chase.nextRunSpeedCheckAt=before.nextRunSpeedCheckAt;
            return result;
        }
        void RunJasonAICombatOnGameThread(){
            const auto before=state.combat;inputEntered=false;cfgdonorhunt::DonorRunJasonAICombatOnGameThread(*this);
            if(deferred&&!inputEntered&&!state.quarantined)state.combat=before;
        }
        bool ReadDriverBlockingStates(std::uintptr_t actor,std::uint8_t& first,std::uint8_t& second){
            std::array<std::uint8_t,4> flags{};if(actor!=Jason()||!a.DonorNativeBusyFlags(source.owner,flags)){Denied();return false;}
            first=flags[0];second=flags[1];return true;
        }
        bool ReadVerifiedDriverSlot(std::uintptr_t actor,std::uintptr_t& driver){return actor==Jason()&&a.DonorNativeReadDriver(source.owner,driver);}
        bool SafeJasonDriverCall(std::uintptr_t actor,std::uintptr_t){return actor==Jason()&&a.DonorNativeDriver(source.owner);}
        bool DriverTailLogged(){return state.driverTailLogged;}void SetDriverTailLogged(){state.driverTailLogged=true;}
        void LogDriverTail(bool okay){cfgclosedtrace::Mark(okay?"donor-native-driver":"donor-native-driver-refused",Jason(),now);}
    };
    bool DonorNativeTrapMorph(DonorNativeHuntingHost& h,std::uint64_t now){
        struct Host {
            DonorNativeHuntingHost& h;
            bool Primary(){return !h.state.isAdditional&&!h.state.quarantined;}
            bool Usable(std::uintptr_t pawn){
                const auto* slot=h.Slot(pawn);cfgdonorhunt::CounselorObservation current;
                if(!slot||!(*slot==bundleTrapVictimSlot))return false;
                if(!h.a.DonorNativeCounselor(h.source.owner,*slot,current)){h.Denied();return false;}
                return current.Live();
            }
            bool Uncertain(){return h.state.quarantined;}
            std::uint64_t LastMorph(){return h.state.chase.LastMorphTeleportAt;}
            std::uint64_t MorphRemaining(std::uint64_t time){
                const auto donor=cfgdonorhunt::DonorJasonAIMorphRemainingMs(h.state.chase,time);
                const auto shared=bundleCarMorph.next>time?bundleCarMorph.next-time:0;
                return (std::max)(donor,static_cast<std::uint64_t>(shared));
            }
            bool Busy(std::uint64_t time){return h.IsJasonBridgeCombatBusy(time,true);}
            bool Deferred(){return h.NavigationPending();}
            bool NavigationPending(){return h.NavigationPending();}
            bool Stop(){auto result=cfgdonorhunt::NativeOutcome::NotEntered;
                const bool called=h.a.DonorNativeStopMovement(h.source.owner,result);
                h.Outcome(result,false,cfgdonorhunt::UncertainAction::Stop);
                return called&&!h.state.quarantined;
            }
            cfgj214trap::MorphResult Morph(std::uintptr_t victim){
                using Result=cfgj214trap::MorphResult;
                // Same bounded exterior ring/roof guard; only its target is
                // scoped to the victim, like donor's temporary roster restriction.
                const bool okay=h.RequestRing(victim,cfgdonorhunt::NavRingPurpose::Distance);
                if(h.state.quarantined)return Result::Uncertain;
                if(okay)return Result::ReturnedTrue;
                if(h.state.navigation.Pending())return Result::Pending;
                return h.deferred?Result::NotEntered:Result::ReturnedFalse;
            }
            void MarkMorph(std::uint64_t){h.MarkJasonAIMorphTeleportUsed();cfgdonorhunt::DonorResetChaseAfterMorph(h.state.chase);}
        } host{h};
        const bool result=cfgj214trap::Service(h.state.trapIntent,host,now);
        if(h.state.trapIntent.completed&&!h.state.trapIntent.ambiguous)bundleTrapPriority.morphed=true;
        if(!h.state.trapIntent.victim)bundleTrapPriority.Clear();
        return result;
    }
    DonorNativePreparation DonorNativeDrivePrimary(const cfgdonorframe::Source& source,DonorNativeJasonRuntime& state,
        cfgdonorhunt::Targets& roster,std::uint64_t now,bool actorPrepared=false){
        if(source.count>64||roster.count<0||roster.count>64)return DonorNativePreparation::Refused;
        const bool interaction=bundleTraps.Busy()||bundleDoorRecovery.breaking||bundleKnife.Busy();
        // H2.43: a trap-slash turn needs the donor value frame (counselor
        // positions) but stays an `interaction`, so no Morph/chase runs.
        const bool trapSlash=state.trapSlashTurn&&bundleTraps.Busy();
        const auto prepared=(interaction&&!trapSlash)?(actorPrepared?DonorNativePreparation::Ready:DonorNativePrepareActor(source.owner,state,now)):
            DonorNativePrepareDecision(source,state,now,actorPrepared);
        if(prepared!=DonorNativePreparation::Ready)return prepared;
        if(state.isAdditional)return DonorNativePreparation::Refused;
        DonorNativeHuntingHost host{*this,source,state,roster,now};
        ++state.decisions;
        if(CFG_J214_TRAP_PRIORITY&&DonorNativeTrapMorph(host,now)){
            state.frame.Consumed();
            return state.quarantined?DonorNativePreparation::Quarantined:
                host.NavigationPending()?DonorNativePreparation::Yield:DonorNativePreparation::Ready;
        }
        if(state.startupFinishPending){
            if(bundleTraps.report.stage==cfgtrap::startup::Stage::Complete&&cfgdonorhunt::DonorJasonAIMorphReadyAt(state.chase)!=0){
                auto& startup=state.startup;startup.Active=true;startup.ObjectivesReady=true;
                startup.ObjectiveIndex=bundleTraps.report.objective;startup.ObjectiveCount=bundleTraps.report.objectives;
                const auto before=startup;
                cfgdonorhunt::DonorPrepareNonAggroStartupTick(host);
                if(host.NavigationPending()){startup=before;return state.quarantined?DonorNativePreparation::Quarantined:DonorNativePreparation::Yield;}
                if(!host.startupMayFinish)return DonorNativePreparation::Yield;
            }
            const auto target=host.FindNearestJasonAICounselorTarget(host.Jason());
            const auto slot=host.Slot(target);
            if(slot&&DonorNativeNavBegin(source.owner,*slot,cfgdonorhunt::NavRingPurpose::StartupFinish,state.navigation)){
                ++bundleReport.h246Nav[0].ringBegun[static_cast<unsigned>(cfgdonorhunt::NavRingPurpose::StartupFinish)];
                state.intendedYield=true;
                DonorNativeNavStep(source.owner,state.navigation);
                const bool uncertainTeleport=state.navigation.uncertainTeleport;
                const auto completion=cfgdonorhunt::DonorConsumeNativeMorph(state.navigation,state.chase,state.additional,now);
                if(completion==cfgdonorhunt::MorphCompletion::Quarantined){state.MarkUncertain(uncertainTeleport?
                    cfgdonorhunt::UncertainAction::Teleport:cfgdonorhunt::UncertainAction::Projection,now);return DonorNativePreparation::Quarantined;}
                if(completion==cfgdonorhunt::MorphCompletion::Pending)return DonorNativePreparation::Yield;
                if(completion!=cfgdonorhunt::MorphCompletion::Completed)return DonorNativePreparation::Yield;
                if(state.chase.LastMorphTeleportAt==now)bundleCarMorph.Attempted(now);
            }else{
                if(r_.reads>=r_.cap||calls_>=cfgbundle::CallCap)return DonorNativePreparation::Yield;
                state.chase.InitialTeleportAttempted=false;state.chase.InitialTeleportAt=now+500;
            }
            #ifdef CFG_H1_JASON
                if(state.startupFinishPending&&hostedAdoptionActive&&state.birth.adoption.valid){++hostedDriverTrace.startupCompleted;hostedDriverTrace.stage=10;}
#endif
                state.startupFinishPending=false;state.startup.Active=false;
            state.combat.NextCombatAttemptAt=now+900;bundleKnife.nextAt=now+3500;
            state.frame.Consumed();return DonorNativePreparation::Ready;
        }
        if(!interaction&&host.TrySharedFiftyMeterMorph()){
            state.frame.Consumed();return host.NavigationPending()?DonorNativePreparation::Yield:DonorNativePreparation::Ready;
        }
        if(host.NavigationPending())return state.quarantined?DonorNativePreparation::Quarantined:DonorNativePreparation::Yield;
        const bool handled=cfgdonorhunt::DonorRunOfflineBotsControllerTickOnGameThread(host);
        if(state.quarantined)return DonorNativePreparation::Quarantined;
        if(!handled)return DonorNativePreparation::Refused;
        if(host.NavigationPending())return DonorNativePreparation::Yield;
        state.frame.Consumed();return DonorNativePreparation::Ready;
    }
    struct DonorNativeAdditionalHost:DonorNativeHuntingHost {
        DonorNativeAdditionalHost(BundleAccess& access,const cfgdonorframe::Source& input,
            DonorNativeJasonRuntime& runtime,cfgdonorhunt::Targets& targets,std::uint64_t time):
            DonorNativeHuntingHost{access,input,runtime,targets,time}{}
        auto& Additional(){return state.additional;}std::uintptr_t Controller(){return source.owner.actor.killerController.object;}
        bool Hosted(){return false;}std::uintptr_t EngineWorld(){return World();}
        auto ReadCounselorRouteMatchPhase(){return state.preparedPhase;}
        bool NativeJasonDeathCinematicObserved(){return bundleDeathEnd.confirmedAt!=0;}
        void BindNativeCurrentPrimaryKiller(){if(CFG_H263_F1_DONOR_HUNT&&HostedRoutineActive())return;a.AdditionalRestorePrimary();} // the donor online path never rebinds at match end
        void RunBaseAIControllerTick(){} // already returned at the verified native entry; NEVER call twice
        bool IsHostedAuthorityWorld(){return false;} // offline owner proof rejects hosted worlds
        bool IsPamelaExclusiveKillLaneActive(std::uint64_t time){return bundleKill.policy.Exclusive(time);}
        bool PermanentHumanSweaterActivationActive(std::uint64_t time){return bundleKill.policy.Exclusive(time);}
        void StopMovement(){auto result=cfgdonorhunt::NativeOutcome::NotEntered;a.DonorNativeStopMovement(source.owner,result);Outcome(result,false,cfgdonorhunt::UncertainAction::Stop);}
        bool DriveAdditionalJasonHeldGrab(std::uint64_t){return bundleDonorHeldAdditional.stage!=cfgdonorgrab::Stage::Idle;}
        std::uintptr_t LocalCounselor(){
            // This cached weak value was refreshed by the complete roster. The
            // decision frame freshly proves membership/identity; no second
            // expensive Known(0) query can lose the exclusion under read cap.
            return bundleDonorRoster.roster.LocalCounselorTarget;
        }
        unsigned StateIndex(){return 0;} // this build owns one independent F1 reinforcement
        void EnableAdditionalJasonAuthoredRunSpeed(std::uint64_t time){
            auto result=cfgdonorhunt::NativeOutcome::NotEntered;
            a.DonorNativeRunState(source.owner,state,time,result);Outcome(result,false,cfgdonorhunt::UncertainAction::Running);
            if(NavigationPending())return;
            if(time<state.chase.nextRunSpeedCheckAt)return;const auto before=state.chase.nextRunSpeedCheckAt;
            state.chase.nextRunSpeedCheckAt=time+30000;float walk=0,run=0;
            if(ReadAuthoredMovementSpeeds(walk,run)&&std::isfinite(walk)&&std::isfinite(run)&&
               walk>25&&run>walk+25&&run<1500)UseAuthoredRunSpeed(run);
            if(deferred&&!runApplied)state.chase.nextRunSpeedCheckAt=before;
        }
        void IssueMove(Position point,float acceptance){
            cfgdonorhunt::DonorIssueAIMoveToLocationOnGameThread(*this,Controller(),point,acceptance);
        }
        bool RunJasonAINavRingTeleportOnGameThread(std::uintptr_t actor,std::uintptr_t target){
            return actor==Jason()&&RequestRing(target,cfgdonorhunt::NavRingPurpose::Additional);
        }
        // J1a binds the independent on-foot donor root. These four new F1
        // interaction owners have no accepted-B1 implementation to retain;
        // their donor bodies are the explicitly queued J1b package, not
        // calls against the PRIMARY actor or guessed shared state.
#if CFG_F1_KNIFE
        bool DriveAdditionalJasonKnife(std::uintptr_t target,float distance,std::uint64_t time){
            BundleAccess knife(cfgbundle::ReadCap,a.calls_);BundleAccess::ChildAccounting accounting(a,knife);
            auto& s=state.additional;const auto pawn=source.owner.actor.pawn.object;
            std::int32_t count=0;std::uint8_t pressed=0;std::uintptr_t driver=0;
            if(!knife.DonorNativeAdditionalKnifeRead(source.owner,count,pressed,driver))return s.KnifeActive;
            if(s.KnifeActive){
                if(!s.KnifeReleased&&time>=s.KnifeReleaseAt){
                    unsigned flags=0;
                    s.KnifeCountAtRelease=count;s.KnifeDriverAtRelease=driver;s.KnifeReleased=true;s.KnifeReleasedAt=time;
                    BundleAccess release(cfgbundle::ReadCap,knife.calls_);BundleAccess::ChildAccounting releaseAccounting(knife,release); // same bounded-reader pattern as the primary's hosted release
                    release.DonorNativeAdditionalKnifeRelease(source.owner,s.KnifeTarget,s.KnifeTargetSerial,flags);
                    cfgclosedtrace::Mark("f1-knife-geometry",pawn,static_cast<std::uint64_t>(flags)|(static_cast<std::uint64_t>(release.Reads())<<16));
                    cfgclosedtrace::Mark("f1-knife-released",pawn,static_cast<std::uint64_t>(count));return true;
                }
                if(s.KnifeTarget&&!s.KnifeReleased)knife.DonorNativeAdditionalKnifeAim(source.owner,s.KnifeTarget);
                if(s.KnifeReleased){
                    if(!s.KnifeLaunched&&s.KnifeCountAtRelease>0&&count<s.KnifeCountAtRelease){
                        s.KnifeLaunched=true;cfgclosedtrace::Mark("f1-knife-launched",s.KnifeTarget,pawn);
                    }
                    if(s.KnifeLaunched&&s.KnifeDriverAtRelease&&driver==s.KnifeDriverAtRelease&&time>=s.KnifeReleasedAt+500)
                        knife.DonorNativeAdditionalKnifeCall(source.owner,cfgknife::disk::Finalize);
                    if(s.KnifeLaunched&&s.KnifeDriverAtRelease&&driver!=s.KnifeDriverAtRelease){
                        s.KnifeActive=false;s.KnifeTarget=0;s.NextKnifeAt=time+12000;
                        cfgclosedtrace::Mark("f1-knife-complete",pawn,time);return true;
                    }
                }
                if(time>=s.KnifeEndAt){
                    knife.DonorNativeAdditionalKnifeCall(source.owner,cfgknife::disk::Finalize);
                    s.KnifeActive=false;s.KnifeTarget=0;s.NextKnifeAt=time+12000;
                    cfgclosedtrace::Mark("f1-knife-timeout",pawn,time);
                }
                return true;
            }
            if(!target||time<s.NextKnifeAt||!std::isfinite(distance)||distance<850.f||distance>1550.f)return false;
            if(knife.DonorNativeAdditionalKnifeLocked(source.owner))return false;
            s.NextKnifeAt=time+4000;
            if(!knife.DonorNativeAdditionalKnifeRefill(source.owner))return false;
            std::int32_t serial=0;
            if(!knife.DonorNativeAdditionalKnifeSerial(target,serial))return false;
            if(!knife.DonorNativeAdditionalKnifeCall(source.owner,cfgknife::disk::Press))return false;
            if(!knife.DonorNativeAdditionalKnifeRead(source.owner,count,pressed,driver)||!pressed){
                cfgclosedtrace::Mark("f1-knife-refused",pawn,1);return false;
            }
            const bool aimed=knife.DonorNativeAdditionalKnifeAim(source.owner,target);
            cfgclosedtrace::Mark("f1-knife-aim",pawn,aimed?1u:0u);
            s.KnifeActive=true;s.KnifeReleased=false;s.KnifeLaunched=false;s.KnifeTarget=target;s.KnifeTargetSerial=serial;
            s.KnifeReleaseAt=time+900;s.KnifeEndAt=time+3500;s.NextKnifeAt=time+12000;
            cfgclosedtrace::Mark("f1-knife-armed",target,static_cast<std::uint64_t>(distance));return true;
        }
#else
        bool DriveAdditionalJasonKnife(std::uintptr_t,float,std::uint64_t){return false;}
#endif
#if CFG_F1_CAR
        bool DriveAdditionalJasonOccupiedCar(std::uint64_t time){
            auto& s=state.additional;const auto pawn=source.owner.actor.pawn.object;
            if(time<s.NextCarActionAt)return s.OccupiedCar!=0;
            s.NextCarActionAt=time+451;
            BundleAccess car(cfgbundle::ReadCap,a.calls_);BundleAccess::ChildAccounting accounting(a,car);
            std::uintptr_t driver=0;
            if(s.OccupiedCar&&s.CarSeat&&car.DonorNativeAdditionalCarSeatCheck(source.owner,s.OccupiedCar,s.CarSeat,driver))s.CarOccupant=driver;
            else{
                if(s.OccupiedCar)cfgclosedtrace::Mark("f1-car-lost",s.OccupiedCar,time);
                s.OccupiedCar=s.CarSeat=s.CarOccupant=0;
                std::uintptr_t found=0,seat=0;
                if(!car.DonorNativeAdditionalCarScan(source.owner,found,seat,driver)){s.NextCarActionAt=time+1500;return false;}
                s.OccupiedCar=found;s.CarSeat=seat;s.CarOccupant=driver;
                s.NextCarMorphAt=time;s.HoodInputAt=0;s.DriverReadySince=0;s.LastExtractionAt=0;s.HaveCachedDriverDoorPoint=false;s.CarForwardSpeed=0;s.NextCarSpeedAt=0;
                cfgclosedtrace::Mark("f1-car-acquired",found,pawn);
            }
            const auto vehicle=s.OccupiedCar;Position carAt{},here{};
            if(!GetJasonAIActorLocation(vehicle,carAt)||!GetJasonAIActorLocation(Jason(),here))return true;
            if(time>=s.NextCarSpeedAt){
                cfgspawn::Vec3 velocity{};
                if(!car.DonorNativeAdditionalCarVelocity(source.owner,vehicle,velocity))return true; // fail closed
                // Simplification: horizontal speed magnitude stands in for forward speed.
                s.CarForwardSpeed=std::sqrt(velocity.x*velocity.x+velocity.y*velocity.y);s.NextCarSpeedAt=time+1201;
            }
            bool started=false;
            if(!car.DonorNativeAdditionalCarStarted(vehicle,started))return true;
            const float dx=carAt.X-here.X,dy=carAt.Y-here.Y,carDistance=std::sqrt(dx*dx+dy*dy);
            const bool moving=started&&(std::fabs(s.CarForwardSpeed)>100.f||s.CarForwardSpeed>150.f);
            if(moving){s.DriverReadySince=0;s.HaveCachedDriverDoorPoint=false;}
            std::uintptr_t hood=0;car.DonorNativeAdditionalCarHood(vehicle,hood);
            const cfgspawn::Vec3 carPoint{carAt.X,carAt.Y,carAt.Z};
            // Simplification: F1's proven nav ring teleport replaces the donor road-center intercept.
            if(moving&&carDistance>1600.f&&time>=s.NextCarMorphAt){
                s.NextCarMorphAt=time+6000;
                if(RunJasonAINavRingTeleportOnGameThread(Jason(),vehicle)){
                    s.NextCarMorphAt=time+20000;cfgclosedtrace::Mark("f1-car-morph",vehicle,static_cast<std::uint64_t>(carDistance));return true;
                }
            }
            if(moving||s.HoodInputAt==0){
                if(carDistance>185.f){IssueMove(carAt,185.f);return true;}
                if(hood&&(s.HoodInputAt==0||time>=s.HoodInputAt+5000)&&car.DonorNativeAdditionalCarHoodInteract(source.owner,hood,carPoint)){
                    s.HoodInputAt=time;cfgclosedtrace::Mark("f1-car-hood",vehicle,hood);
                }
                if(moving||hood)return true;
            }
            if(moving)return true;
            if(!hood&&s.HoodInputAt==0)s.HoodInputAt=time;
            if(s.HoodInputAt==0)return true;
            if(carDistance>135.f){s.DriverReadySince=0;IssueMove(carAt,135.f);return true;}
            StopMovement();
            if(s.DriverReadySince==0){s.DriverReadySince=time;return true;}
            if(time>=s.DriverReadySince+650&&(s.LastExtractionAt==0||time>=s.LastExtractionAt+3000)&&
               car.DonorNativeAdditionalCarExtraction(source.owner,s.CarSeat,carPoint)){
                s.LastExtractionAt=time;cfgclosedtrace::Mark("f1-car-extraction",vehicle,s.CarSeat);
            }
            return true;
        }
#else
        bool DriveAdditionalJasonOccupiedCar(std::uint64_t){return false;}
#endif
#if CFG_H264_F1_INTERACTIONS
        std::uintptr_t hiderSpot=0;Position hiderAt{};
        bool GetJasonAIActorLocation(std::uintptr_t actor,Position& point){
            if(actor&&actor==hiderSpot){point=hiderAt;return true;}
            return DonorNativeHuntingHost::GetJasonAIActorLocation(actor,point);
        }
        bool FindNearbyOccupiedHidingSpotForJason(std::uintptr_t,std::uintptr_t& spot,std::uintptr_t& hidden,std::uintptr_t& component){
            BundleAccess find(cfgbundle::ReadCap,a.calls_);BundleAccess::ChildAccounting accounting(a,find);cfgspawn::Vec3 point{};
            auto& s=state.additional;
            if(!find.DonorNativeAdditionalHider(source.owner,roster,now,spot,hidden,component,point,now<s.HiderBlockedUntil?std::uintptr_t{}:s.Target))return false;
            hiderSpot=spot;hiderAt.X=point.x;hiderAt.Y=point.y;hiderAt.Z=point.z;Position here{};
            // H2.68: live, his trip to a far hider could hold him still indefinitely, because this branch runs before the chase and door code.
            if(DonorNativeHuntingHost::GetJasonAIActorLocation(Jason(),here)){const float dx=point.x-here.X,dy=point.y-here.Y,distance=std::sqrt(dx*dx+dy*dy);
                if(distance>600.f){s.HiderStalls=s.HiderLastDistance>=0&&std::fabs(s.HiderLastDistance-distance)<60.f?s.HiderStalls+1:0;s.HiderLastDistance=distance;
                    if(s.HiderStalls>=6){const auto spotCounselor=hidden;s.HiderStalls=0;s.HiderLastDistance=-1.f;spot=hidden=component=0;hiderSpot=0;
                        cfgclosedtrace::Mark("f1-h268-hider-stalled",source.owner.actor.pawn.object,static_cast<std::uint64_t>(distance));
                        // Within 30 m: the F1 teleport ring around that counselor. Otherwise, or on cooldown, back to normal hunting for 6 s.
                        if(distance<=3000.f&&now>=s.NextMorphAt&&!s.AttackPressed){s.NextMorphAt=now+6000;
                            if(RunJasonAINavRingTeleportOnGameThread(Jason(),spotCounselor)){s.NextMorphAt=now+20000;s.HasProgressSample=false;s.NextMoveAt=now+1000;return false;}}
                        s.HiderBlockedUntil=now+6000;s.Target=0;return false;}
                }else{s.HiderStalls=0;s.HiderLastDistance=-1.f;}
            }
            return true;
        }
        bool AttemptJasonHidingSpotComponentForJason(std::uintptr_t,std::uintptr_t component){
            BundleAccess attempt(cfgbundle::ReadCap,a.calls_);BundleAccess::ChildAccounting accounting(a,attempt);
            cfgspawn::Vec3 point{};point.x=hiderAt.X;point.y=hiderAt.Y;point.z=hiderAt.Z;
            const bool sent=attempt.DonorNativeAdditionalHidingAttempt(source.owner,component,point);
            cfgclosedtrace::Mark(sent?"f1-h264-hiding-attempt":"f1-h264-hiding-refused",hiderSpot,component);return sent;
        }
        bool TryAdditionalJasonNativeDoor(std::uintptr_t target,Position here,Position there,std::uint64_t time,bool early){
            auto& s=state.additional;if(time<s.DoorNextAttemptAt)return false;
            BundleAccess door(cfgbundle::ReadCap,a.calls_);BundleAccess::ChildAccounting accounting(a,door);BundleAccess::DonorAdditionalDoorFacts f{};
            if(early){
                if(!door.DonorNativeAdditionalDoorFacts(source.owner,f)||!f.attackComponent||f.permitted!=1||f.state.destroyed||f.state.target||f.state.killer)return false;
                if(!JasonDoorApproachPolicy::IsOnApproach(f.anchor.x-here.X,f.anchor.y-here.Y,f.anchor.z-here.Z,there.X-here.X,there.Y-here.Y,there.Z-here.Z))return false;
                bool stunned=true,attacking=true;
                if(!door.DonorNativeQuery(source.owner,BundleAccess::DonorQuery::Stunned,stunned)||stunned||!door.DonorNativeQuery(source.owner,BundleAccess::DonorQuery::Attacking,attacking)||attacking)return false;
            }
            if(s.DoorSampleTarget!=target){s.DoorSampleTarget=target;s.DoorHasSample=false;s.DoorBlockedSamples=0;}
            const float tx=there.X-here.X,ty=there.Y-here.Y;
            if(s.DoorHasSample&&tx*tx+ty*ty>100.f*100.f){
                const float mx=here.X-s.DoorLastLocation.X,my=here.Y-s.DoorLastLocation.Y;
                s.DoorBlockedSamples=mx*mx+my*my<80.f*80.f?(std::min)(s.DoorBlockedSamples+1,3u):0u;
            }else s.DoorBlockedSamples=0;
            s.DoorLastLocation=here;s.DoorHasSample=true;
            if(!early){if(s.DoorBlockedSamples<2||!door.DonorNativeAdditionalDoorFacts(source.owner,f))return false;
                if(f.selected.door==s.DoorLast&&time<s.DoorSameRetryAt)return false;}
            const float dx=f.anchor.x-here.X,dy=f.anchor.y-here.Y,dz=f.anchor.z-here.Z;
            if(dx*dx+dy*dy+dz*dz>500.f*500.f)return false;
            s.DoorNextAttemptAt=time+(early?2000:8000);bool breakStarted=false,opened=false;
            unsigned why=0;const bool sent=door.DonorNativeAdditionalDoorPress(source.owner,f,breakStarted,opened,why);
            cfgclosedtrace::Mark(sent?"f1-h264-door":"f1-h264-door-refused",f.selected.door,(early?1u:0u)|(breakStarted?2u:0u)|(opened?4u:0u)|(why<<8));
            if(!sent)return false;
            s.DoorBlockedSamples=0;s.DoorLast=f.selected.door;
            const bool retry=early&&!breakStarted&&!opened;
            s.DoorSameRetryAt=time+(retry?2000:30000);if(retry)s.DoorNextAttemptAt=time+2000;
            s.DoorInteractionUntil=time+(breakStarted?6500:2500);return true;
        }
        bool DriveAdditionalJasonSwim(std::uintptr_t target,std::uint64_t time){
#if CFG_H266_F1_SWIM
            BundleAccess swim(cfgbundle::ReadCap,a.calls_);BundleAccess::ChildAccounting accounting(a,swim);
            return swim.DonorNativeAdditionalSwim(source.owner,target,time,state.additional.NextSwimAt);
#else
            (void)target;(void)time;return false;
#endif
        }
#else
        bool DriveAdditionalJasonSwim(std::uintptr_t,std::uint64_t){return false;}
        bool FindNearbyOccupiedHidingSpotForJason(std::uintptr_t,std::uintptr_t& spot,std::uintptr_t& hidden,std::uintptr_t& component){
            spot=hidden=component=0;return false;
        }
        bool AttemptJasonHidingSpotComponentForJason(std::uintptr_t,std::uintptr_t){return false;}
        bool TryAdditionalJasonNativeDoor(std::uintptr_t,Position,Position,std::uint64_t,bool){return false;}
#endif
    };
    DonorNativePreparation DonorNativeDriveAdditional(const cfgdonorframe::Source& source,DonorNativeJasonRuntime& state,
        cfgdonorhunt::Targets& roster,std::uint64_t now,bool actorPrepared=false){
        if(source.count>64||roster.count<0||roster.count>64)return DonorNativePreparation::Refused;
        const auto prepared=DonorNativePrepareDecision(source,state,now,actorPrepared);
        if(prepared!=DonorNativePreparation::Ready)return prepared;
        if(!state.isAdditional)return DonorNativePreparation::Refused;
        DonorNativeAdditionalHost host{*this,source,state,roster,now};const auto before=state.additional;
        ++state.decisions;if(CFG_H263_F1_DONOR_HUNT&&state.decisions==1&&HostedRoutineActive())cfgclosedtrace::Mark("f1-h263-donor-lane",source.owner.actor.pawn.object,Reads());
        const bool handled=cfgdonorhunt::DonorFrozenBridgeDriveAdditionalJason(host);
        if(host.moveDenied){state.additional.Target=before.Target;state.additional.NextMoveAt=before.NextMoveAt;}
        if(host.deferred&&!host.inputEntered&&!state.quarantined){
            state.additional.NextCombatAt=before.NextCombatAt;state.additional.NextGrabAt=before.NextGrabAt;
            state.additional.AttackPressed=before.AttackPressed;state.additional.AttackReleaseAt=before.AttackReleaseAt;
        }
        if(state.quarantined)return DonorNativePreparation::Quarantined;
        if(!handled)return DonorNativePreparation::Refused;
        if(host.NavigationPending())return DonorNativePreparation::Yield;
        state.frame.Consumed();return DonorNativePreparation::Ready;
    }
#endif
