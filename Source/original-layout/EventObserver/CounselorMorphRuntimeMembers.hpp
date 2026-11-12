    // Shared stall sampler: used by CounselorMorph and (H2.46) by the lease-mode queue pre-check,
    // so the same 2 s / 8 s stall evidence exists whether or not the donor lease is published.
    bool MorphProgressSample(std::uint64_t now,std::uintptr_t target){
        if(bundlePursuitProgress.target==target&&!bundlePursuitProgress.Due(now))return true;
        cfgspawn::Vec3 pawn{};cfgprobe::lifetime::SlotObservation targetSlot{};
        if(!Location(bundleJason.pawn.object,pawn)||!cfgspawn::ObjectSlot(r_,context.objectArray,target,targetSlot)||!targetSlot.HasAssignedSerial()){
            bundlePursuitProgress.Clear();return false;
        }
        bundlePursuitProgress.Observe(target,targetSlot.serial,pawn,now);
        // Accepted MoveTo is not evidence that its path is still active.
        // Replan after measured stagnation, outside animation/door owners.
        if(bundlePursuitProgress.deferrals)bundleFollowPath.Clear();
        return true;
    }
    cfgpursuit::Result CounselorMorph(std::uint64_t now,unsigned targetIndex,std::uintptr_t target,float squared,
                                    const cfgspawn::Vec3* guardGoal=nullptr){
        using cfgpursuit::Result;
        // Native attack/hold/stun gates were checked by the hunt lane. Also
        // retain the base mod's shared car cooldown and door traversal grace.
        const bool urgent=bundleTrapPriority.Active(now)&&!bundleTrapPriority.morphed&&bundleTrapPriority.victim==target&&
            cfgobserver::Same(r_,target+cfgtrappriority::disk::TriggeredTrap,bundleTriggeredTrapSlot.object)&&
            cfgjason::SameSlot(r_,context.objectArray,bundleTriggeredTrapSlot);
        if(!MorphProgressSample(now,target))return Result::Unavailable;
        bool structureStalled=false;
        if(bundlePursuitProgress.Stalled()&&!bundleDoorRecovery.breaking&&!bundleDoorRecovery.phase){
            cfgspawn::Vec3 here{},there{};
            structureStalled=Location(bundleJason.pawn.object,here)&&Location(target,there)&&
                bundleCabins.cache.Encloses(plan.world,here)&&ExteriorLanding(there);
        }
        const bool morphDue=cfgpursuit::MorphDue(bundleCarMorph,now,squared,urgent,bundlePursuitProgress.Stalled(),structureStalled);
        if(!morphDue||now<bundleDoorRecovery.traversalUntil||(!structureStalled&&now<bundleMoveAt)||
            bundleCabins.cache.stage!=cfgcabin::Stage::Ready)return Result::None;
        if(now<bundlePursuit.retryAt)return Result::None;
        ++bundleReport.pursuitChecks;
        cfgspawn::Vec3 position{};
        if(!Location(target,position)){bundlePursuit.Refused(now);return Result::Unavailable;}
        cfgspawn::Vec3 counselorActual=position;
        if(guardGoal)position=*guardGoal;
        if(bundlePursuit.target&&!bundlePursuit.Current(target,position)){
            bundlePursuit.Refused(now);return Result::None; // walking resumes; never restart candidate0 every hunt
        }
        if(!bundlePursuit.target&&!(guardGoal?bundlePursuit.BeginExact(target,position,now):bundlePursuit.Begin(target,position,now)))return Result::None;
        if(bundlePursuit.exact!=(guardGoal!=nullptr)){bundlePursuit.Refused(now);return Result::None;}
        if(!bundlePursuit.projected){
            std::uintptr_t function=0,navClass=0,system=0;cfgprobe::lifetime::SlotObservation slot{};
            if(!FeatureFunction(cfgvehicle::disk::Project,function,navClass)||
               !r_.Read(plan.world+cfgtrap::disk::WorldNavigationSystem,system)||!cfgspawn::ObjectIs(r_,system,navClass)||
               !cfgspawn::ObjectSlot(r_,context.objectArray,system,slot)){bundlePursuit.Refused(now);return Result::Unavailable;}
            cfgvehicle::Projection projection;projection.world=plan.world;projection.point=bundlePursuit.Candidate();projection.extent={350,350,110};
            std::uintptr_t current=0;bool alive=false;
            if(!Context()||!Jason()||!Known(targetIndex,current)||current!=target||!Alive(target,alive)||!alive||
               !Invoke(system,function,&projection,false)||!cfgjason::OwnedComponentSlot(r_,context.objectArray,slot)||
               !cfgobserver::Same(r_,plan.world+cfgtrap::disk::WorldNavigationSystem,system)){bundlePursuit.Refused(now);return Result::Unavailable;}
            ++bundleReport.pursuitProjections;
            bool accepted=projection.result==1&&bundlePursuit.Projected(projection.projected);
            if(accepted&&CFG_J25_ROOF_GUARD){
                auto probe=[&](cfgspawn::Vec3 query,cfgspawn::Vec3& out){
                    cfgvehicle::Projection lower;lower.world=plan.world;lower.point=query;lower.extent={100,100,110};
                    std::uintptr_t checked=0;bool live=false;
                    if(!Context()||!Jason()||!Known(targetIndex,checked)||checked!=target||!Alive(target,live)||!live||
                       !cfgjason::OwnedComponentSlot(r_,context.objectArray,slot)||
                       !cfgobserver::Same(r_,plan.world+cfgtrap::disk::WorldNavigationSystem,system))return cfgroofguard::Observation::Unsafe;
                    const bool invoked=Invoke(system,function,&lower,false);
                    if(!invoked||lower.result>1||!cfgjason::OwnedComponentSlot(r_,context.objectArray,slot)||
                       !cfgobserver::Same(r_,plan.world+cfgtrap::disk::WorldNavigationSystem,system))return cfgroofguard::Observation::Unsafe;
                    if(lower.result==0)return cfgroofguard::Observation::Absent;
                    if(!cfgspawn::Finite(lower.projected))return cfgroofguard::Observation::Unsafe;
                    out=lower.projected;return cfgroofguard::Observation::Present;
                };
                accepted=cfgroofguard::Safe(bundlePursuit.roof,bundlePursuit.surface,
                    {counselorActual.x,counselorActual.y,counselorActual.z-90.f},probe);
            }
            if(!accepted)bundlePursuit.FailedCandidate(now);
            return Result::Pending; // teleport remains a separate native phase
        }
        std::uintptr_t function=0,current=0,held=0;bool alive=false;
        if(!TeleportFunction(function)||!Context()||!Jason()||!Manager()||!Known(targetIndex,current)||current!=target||
            !Alive(target,alive)||!alive||!Held(held)||held||!Location(target,position)){
            bundlePursuit.Refused(now);return Result::Unavailable;
        }
        counselorActual=position;
        if(guardGoal)position=*guardGoal;
        if(!bundlePursuit.Current(target,position)){bundlePursuit.Refused(now);return Result::Unavailable;}
        if(!ExteriorLanding(bundlePursuit.surface)){bundlePursuit.FailedCandidate(now);return Result::None;}
        cfgpursuit::TeleportPacket packet;packet.location=bundlePursuit.ActorDestination();++bundleReport.pursuitAttempts;
        if(CFG_J25_ROOF_GUARD){bundlePursuit.roof.Destination(1,packet.location,counselorActual,now);
            cfgclosedtrace::Mark("counselor-exterior-morph-destination",target,now);}
        if(!Invoke(bundleJason.pawn.object,function,&packet)){bundlePursuit.Refused(now);return Result::Unavailable;}
        if(packet.result==0){bundlePursuit.FailedCandidate(now);return Result::Pending;}
        // A native nonzero/ambiguous return may have moved Jason. Consume the
        // shared cooldown before any readback; never repeat a side effect.
        bundleCarMorph.Attempted(now);if(urgent)bundleTrapPriority.morphed=true;

        bundlePursuit.Clear();bundleFollowPath.Clear();bundlePathCooldown.Clear();
        bundlePursuitProgress.Clear();
        bundleDoorRecovery={};bundleDoorBreakSelection={};bundleMoveAt=now+300;
        cfgspawn::Vec3 actual{};
        if(packet.result!=1||!Context()||!Jason()||!Location(bundleJason.pawn.object,actual))return Result::Unavailable;
        const auto dx=actual.x-packet.location.x,dy=actual.y-packet.location.y,dz=actual.z-packet.location.z;
        if(!std::isfinite(dx*dx+dy*dy+dz*dz)||dx*dx+dy*dy+dz*dz>500.f*500.f)return Result::Unavailable;
        ++bundleReport.pursuitReturns;return Result::Teleported;
    }
    // H2.47: the donor combat-busy predicate (DonorIsJasonBridgeCombatBusy, same template) for the legacy
    // lease-mode Morph queue/service and burst start. Host seam = values the legacy lane can observe.
    bool H247LegacyCombatBusy(std::uint64_t now,std::uint32_t* causes=nullptr){
        cfgh247::LegacyBusyView v;DonorBusyStateForLegacy(v);
        const auto pawn=bundleJason.pawn.object;
        v.jason=pawn;v.actorReadable=Context()&&Jason();
        v.busy.KnifeSequenceActive=bundleKnife.Busy();v.busy.DoorBreakActive=bundleDoorRecovery.breaking;
        v.busy.StartupTrapSetupActive=bundleTraps.Busy();
        bool grab=false;v.specialReadable=GrabBusy(grab);v.special=grab;
        v.heldReadable=Held(v.held);
        bool stunned=false;v.stunReadable=NativeBool(cfghunter::disk::Stunned,stunned);v.stunned=stunned;
        v.flagsReadable=r_.Read(pawn+cfgcompat::cfg35::j1a::DriverState0,v.flags[0])&&r_.Read(pawn+cfgcompat::cfg35::j1a::DriverState1,v.flags[1])&&
            r_.Read(pawn+cfgcompat::cfg35::j1a::MorphActive,v.flags[2])&&r_.Read(pawn+cfgcompat::cfg35::j1a::ShiftActive,v.flags[3]);
        return cfgh247::LegacyCombatBusy(v,now,causes);
    }
    bool CounselorMorphService(std::uint64_t now,std::uintptr_t target,const cfgspawn::Vec3* guardGoal=nullptr){
#ifdef CFG_H1_JASON
        HostedLaneEnter(cfgh1::Lane::Morph);
#endif

        std::uintptr_t current=0,held=0,seat=0,water=0;bool living=false,stunned=false,attacking=false,busy=false;float squared=0;
        if(!Context()||!Jason()||!Manager()||!NativeBindings()||!Known(bundleTargetIndex,current)||current!=target||
           !Alive(target,living)||!living||!Held(held)||held||
           !r_.Read(target+cfgvehicleoccupancy::disk::CurrentSeat,seat)||seat||
           !r_.Read(target+cfgswim::disk::WaterVolume,water)||water||
           !r_.Read(bundleJason.pawn.object+cfgswim::disk::WaterVolume,water)||water||
           !NativeBool(cfghunter::disk::Stunned,stunned)||stunned||
           !NativeBool(cfghunter::disk::Attacking,attacking)||attacking||!GrabBusy(busy)||busy||!Distance(target,squared)){
            bundlePursuitProgress.Clear();bundlePursuit.Clear();return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Morph,1)
#else
 false
#endif
;}
        const auto result=CounselorMorph(now,bundleTargetIndex,target,squared,guardGoal);
        if(result==cfgpursuit::Result::Pending){bundleCounselorMorphTarget=target;bundleSchedule.huntAt=now+100;}
        if(result==cfgpursuit::Result::Teleported)cfgclosedtrace::Mark("counselor-pursuit-exterior-morph",target,now);
        return true;
    }
