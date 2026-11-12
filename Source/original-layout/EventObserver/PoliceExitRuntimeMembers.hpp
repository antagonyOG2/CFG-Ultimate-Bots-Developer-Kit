    bool EscapeGuardAllowed(std::uint64_t now)const{
        return cfgpoliceexit::HuntAvailable(bundleCarPursuit.Active(now)||bundleRoadMorph.stage!=0||bundleCarActionPending.active,bundleVehicleActions.StopOwnsEpisode(),
            bundleCarSpeed.active,bundleTraps.Busy()||bundleTrapPriority.Active(now),bundleHiding.Active(),
            bundleKnife.Busy()||bundleKnifeProbePending,bundleDoorRecovery.breaking||bundleDoorRecovery.phase,
            bundleKill.policy.Exclusive(now),bundleDeathEnd.confirmedAt!=0,bundleCompletionReady);
    }
    bool EscapeGuardUnlocked(){
        std::uintptr_t manager=0,locked=0;
        return r_.Read(bundleJason.pawn.object+cfghunter::disk::Manager,manager)&&
            r_.Read(manager+cfgvehicleinteraction::disk::LockedComponent,locked)&&!locked;
    }
    void EscapeGuardSample(unsigned index,std::uintptr_t pawn,cfgspawn::Vec3 at,std::uint64_t now){
        if(index>=bundleEscapeGuard.people.size())return;
        cfgprobe::lifetime::SlotObservation slot{};
        if(cfgspawn::ObjectSlot(r_,context.objectArray,pawn,slot))bundleEscapeGuard.people[index]={slot,at,now};
        else bundleEscapeGuard.people[index]={};
    }
    bool EscapeGuardObserve(std::uint64_t now){
        if(!EscapeGuardAllowed(now))return false;
        struct Host {
            BundleAccess& access;
            bool Current(){return access.Context();}
            bool Location(std::uintptr_t actor,cfgspawn::Vec3& at){return access.Location(actor,at);}
            bool Collision(std::uintptr_t sphere,std::uint8_t& value){namespace d=cfgcompat::cfg35::policeexit;
                return access.CarCall(d::GetCollisionEnabled,d::GetCollisionEnabledExecutor,d::GetCollisionEnabledCode,sphere,&value);}
            void Observed(const cfgpoliceexit::Entry& e){if(bundleEscapeGuard.traces<16){++bundleEscapeGuard.traces;
                cfgclosedtrace::Mark("escape-guard-exit",e.volume.object,e.active?1u:0u);}}
        } host{*this};
        return cfgpoliceexit::Step(r_,host,context.objectArray,plan.module,plan.world,plan.mode,spawnAssets.gameState,bundleEscapeGuard.cache,now);
    }
    bool EscapeGuardCurrent(unsigned index,std::uint64_t now,bool nativeProbe=false){
        auto& c=bundleEscapeGuard.cache;if(index>=c.count||!c.policeCalled)return false;
        auto& e=c.entries[index];
        if(!e.active||!e.sampledAt||now<e.sampledAt||now-e.sampledAt>1500||!Context()||
            !cfgpoliceexit::Current(r_,context.objectArray,plan.module,plan.world,plan.mode,c.source,e))return false;
        if(nativeProbe){
            namespace d=cfgcompat::cfg35::policeexit;std::uint8_t collision=255;
            if(!CarCall(d::GetCollisionEnabled,d::GetCollisionEnabledExecutor,d::GetCollisionEnabledCode,e.sphere.object,&collision)||
                !collision||collision>3||!Context()||
                !cfgpoliceexit::Current(r_,context.objectArray,plan.module,plan.world,plan.mode,c.source,e)){
                e.active=false;return false;
            }
            e.sampledAt=now;
        }
        return true;
    }
    bool EscapeGuardAct(std::uint64_t now,const cfgspawn::Vec3& here,float squared){
        auto& g=bundleEscapeGuard;
        if(cfgpoliceexit::CombatOwns(squared,now,bundleSchedule.meleeUntil,bundleGrab.held,bundleKill.policy.Exclusive(now)))
            return false; // preserve guard hysteresis while native combat owns Jason
        if(!EscapeGuardAllowed(now)){g.ClearOwnership();return false;}
        std::array<cfgescapeguard::Exit,cfgescapeguard::MaximumExits> exits{};
        std::array<cfgescapeguard::Counselor,cfgescapeguard::MaximumCounselors> people{};
        for(unsigned i=0;i<g.cache.count;++i)if(EscapeGuardCurrent(i,now)){
            const auto& p=g.cache.entries[i].at;exits[i]={true,{p.x,p.y,p.z}};
        }
        for(unsigned i=0;i<people.size();++i){const auto& p=g.people[i];
            if(!p.pawn.HasAssignedSerial()||!p.sampledAt||now<p.sampledAt||now-p.sampledAt>1500)continue;
            cfgprobe::lifetime::SlotObservation fresh{};
            if(!cfgspawn::ObjectSlot(r_,context.objectArray,p.pawn.object,fresh)||!(fresh==p.pawn))continue;
            people[i]={true,{p.at.x,p.at.y,p.at.z}};
        }
        const bool morphReady=!bundleFootBurst.Owned()&&!bundleFootObstacle.stage&&now>=bundleDoorRecovery.traversalUntil&&
            now>=bundleMoveAt&&now>=bundlePursuit.retryAt&&bundleCabins.cache.stage==cfgcabin::Stage::Ready&&
            cfgpursuit::MorphDue(bundleCarMorph,now,squared);
        const auto d=g.guard.Decide({here.x,here.y,here.z},exits,people,morphReady,now);
        if(d.action==cfgescapeguard::Action::None)return false;
        const auto index=d.counselor>bundleReport.bots?BundleNativeTargetIndex:d.counselor;
        std::uintptr_t target=0;bool living=false;
        if(!Known(index,target)||target!=g.people[d.counselor].pawn.object||!Alive(target,living)||!living)return false;
        if(target!=bundleReport.target){
            bundleTargetIndex=index;bundleReport.target=target;bundleTargetLockUntil=now+10000;
            bundlePursuit.Clear();bundleFollowPath.Clear();bundlePathCooldown.Clear();bundlePursuitProgress.Clear();bundleMoveAt=now;
        }
        g.goal={d.goal.x,d.goal.y,d.goal.z};g.target=target;g.exit=d.exit;
        if(d.action==cfgescapeguard::Action::Morph&&EscapeGuardUnlocked()){
            g.morphPending=true;bundleCounselorMorphTarget=0;bundlePursuit.Clear();bundleSchedule.huntAt=now+100;return true;
        }
        if(now<g.moveAt)return !g.followFallback;
        g.moveAt=now+450;unsigned result=255;
        if(MoveToPoint(g.goal,result)){
            bundleReport.lastMoveResult=result;bundleFollowPath.Clear();
            if(cfgbundle::ClassifyMove(result)!=cfgbundle::MoveOutcome::Refused){++bundleReport.moves;g.followFallback=false;
                if(g.traces<16){++g.traces;cfgclosedtrace::Mark("escape-guard-hunt",target,d.exit);}return true;}
        }
        g.followFallback=true;return true; // ordinary follow resumes on a fresh callback
    }
    bool EscapeGuardMorphService(std::uint64_t now){
        auto& g=bundleEscapeGuard;const auto target=g.target;bundleCounselorMorphTarget=0;
        if(now<bundleSchedule.meleeUntil||bundleGrab.held||!EscapeGuardAllowed(now)||!EscapeGuardUnlocked()||!EscapeGuardCurrent(g.exit,now,true)){
            g.morphPending=false;bundlePursuit.Clear();g.moveAt=now;return true;
        }
        const auto returns=bundleReport.pursuitReturns;
        CounselorMorphService(now,target,&g.goal);
        g.morphPending=bundleCounselorMorphTarget==target&&bundlePursuit.target==target&&bundlePursuit.exact;
        bundleCounselorMorphTarget=0; // this exact-goal continuation owns the queue
        if(!g.morphPending)g.moveAt=now;
        if(bundleReport.pursuitReturns!=returns&&g.traces<16){++g.traces;cfgclosedtrace::Mark("escape-guard-morph",target,g.exit);}
        return true;
    }
