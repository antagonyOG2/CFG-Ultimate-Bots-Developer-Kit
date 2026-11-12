    enum class DonorScalarAction:unsigned {Forward,SetLocation,Top,Bottom,Left,Right,Grab};
    bool DonorScalarFunction(DonorScalarAction action,std::uintptr_t& function){
        using namespace cfgcompat::cfg35::j1move;std::uintptr_t owner=0;
        const auto prove=[&](const auto& binding,std::uintptr_t thunk,const auto& code){
            return FeatureFunction(binding,function,owner)&&
                cfgobserver::Same(r_,function+0xB0,plan.module+thunk)&&cfgbundle::Code(r_,plan.module,thunk,code);
        };
        switch(action){
        case DonorScalarAction::Forward:return prove(Forward,ForwardThunk,ForwardThunkCode);
        case DonorScalarAction::SetLocation:return prove(SetLocation,SetLocationThunk,SetLocationThunkCode);
        case DonorScalarAction::Top:return prove(Top,TopThunk,TopThunkCode);
        case DonorScalarAction::Bottom:return prove(Bottom,BottomThunk,BottomThunkCode);
        case DonorScalarAction::Left:return prove(Left,LeftThunk,LeftThunkCode);
        case DonorScalarAction::Right:return prove(Right,RightThunk,RightThunkCode);
        case DonorScalarAction::Grab:return prove(cfggrab::disk::BeginGrab,cfggrab::disk::BeginGrabExecutor,cfggrab::disk::BeginGrabCode);
        default:return false;
        }
    }
    bool DonorScalarInvoke(const cfgjasontick::Owner& owner,DonorScalarAction action,void* packet,
        cfgdonorhunt::NativeOutcome* outcome=nullptr)noexcept{
        if(outcome)*outcome=cfgdonorhunt::NativeOutcome::NotEntered;
        // Closed, disk-proved native schemas: FVector output; SetActorLocation
        // scalar inputs/weak HitResult output; or zero-parameter native inputs.
        // No arbitrary function/raw-frame escape through this gateway.
        if((action==DonorScalarAction::Forward||action==DonorScalarAction::SetLocation)!=static_cast<bool>(packet))return false;
        std::uintptr_t function=0;const auto pawn=owner.actor.pawn.object;
        if(!DonorNativeOwner(owner)||!NativeCallAllowed()||!DonorScalarFunction(action,function))return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!RetainBundleActors(pawn)||!RetainPrimaryObjects(r_,std::array<std::uintptr_t,1>{function})||
           !NativeCallAllowed()||!BeginBundleNative())return false;
#endif
        ++calls_;if(outcome)*outcome=cfgdonorhunt::NativeOutcome::Uncertain;
        __try{originalActorDispatcher(reinterpret_cast<void*>(pawn),reinterpret_cast<void*>(function),packet);}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
            BundleRetentionFailed();
#endif
            return false;}
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        const bool finished=FinishBundleNative();
        if(finished&&outcome)*outcome=cfgdonorhunt::NativeOutcome::ReturnedTrue;
        return finished;
#else
        if(outcome)*outcome=cfgdonorhunt::NativeOutcome::ReturnedTrue;
        return true;
#endif
    }
    bool DonorNativeForward(const cfgjasontick::Owner& owner,cfgspawn::Vec3& forward){
        cfgspawn::Vec3 value{};
        if(!DonorScalarInvoke(owner,DonorScalarAction::Forward,&value)||!cfgspawn::Finite(value))return false;
        forward=value;return true;
    }
    bool DonorNativeSetLocation(const cfgjasontick::Owner& owner,cfgspawn::Vec3 position,bool& moved,
        cfgdonorhunt::NativeOutcome* outcome=nullptr){
        if(outcome)*outcome=cfgdonorhunt::NativeOutcome::NotEntered;
        using namespace cfgcompat::cfg35::j1move;moved=false;
        if(!cfgspawn::Finite(position))return false;
        alignas(16)std::array<unsigned char,SetLocationStorageBytes> packet{};
        std::memcpy(packet.data()+NewLocationOffset,&position,sizeof(position));
        packet[SweepOffset]=0;packet[TeleportOffset]=1;
        if(!DonorScalarInvoke(owner,DonorScalarAction::SetLocation,packet.data(),outcome))return false;
        if(packet[SetLocationReturnOffset]>1){if(outcome)*outcome=cfgdonorhunt::NativeOutcome::Uncertain;return false;}
        moved=packet[SetLocationReturnOffset]!=0;
        if(outcome)*outcome=moved?cfgdonorhunt::NativeOutcome::ReturnedTrue:cfgdonorhunt::NativeOutcome::ReturnedFalse;
        return true;
    }
    bool DonorNativeDirectionalGrab(const cfgjasontick::Owner& owner,unsigned direction){
        if(direction>=4)return false;
        constexpr DonorScalarAction directions[]{DonorScalarAction::Top,DonorScalarAction::Bottom,
            DonorScalarAction::Left,DonorScalarAction::Right};
        return DonorScalarInvoke(owner,directions[direction],nullptr);
    }
    bool DonorNativeMoveLocation(const cfgjasontick::Owner& owner,cfgspawn::Vec3 point,unsigned& result,
        float acceptance=85,cfgdonorhunt::NativeOutcome* outcome=nullptr,bool strafe=false,bool exactCenterline=false){
        if(outcome)*outcome=cfgdonorhunt::NativeOutcome::NotEntered;
        result=255;if(!DonorNativeOwner(owner)||!cfgspawn::Finite(point)||!std::isfinite(acceptance)||acceptance<0||acceptance>1000)return false;
        std::uintptr_t function=0,type=0;const auto controller=owner.actor.killerController.object;
        if(!FeatureFunction(cfgtrap::disk::MoveToLocation,function,type)||!cfgspawn::ObjectIs(r_,controller,type)||
           !cfgobserver::Same(r_,function+0xB0,plan.module+cfgtrap::disk::MoveToLocationExecutor)||
           !cfgbundle::Code(r_,plan.module,cfgtrap::disk::MoveToLocationExecutor,cfgtrap::disk::MoveToLocationCode))return false;
        cfgtrap::MoveLocationParameters packet{};packet.destination=point;packet.acceptance=acceptance;packet.result=255;
        packet.pathfinding=packet.projectNavigation=packet.allowPartial=exactCenterline?0:1;
        packet.strafe=strafe?1:0;
        bundleInvocationStage_=0;const bool invoked=Invoke(controller,function,&packet,false);
        if(outcome&&bundleInvocationStage_>=5)*outcome=cfgdonorhunt::NativeOutcome::Uncertain;
        if(!invoked||packet.result>2)return false;
        result=packet.result;
        if(outcome)*outcome=result?cfgdonorhunt::NativeOutcome::ReturnedTrue:cfgdonorhunt::NativeOutcome::ReturnedFalse;
        return true;
    }
    // H2.45: the J2.19 melee facing pair already used by the door-break and
    // additional-Jason lanes (SetControlRotation on the killer controller, then
    // K2_SetActorRotation on the pawn), pinned like DonorNativeMoveLocation.
    bool DonorNativeFaceYaw(const cfgjasontick::Owner& owner,float yaw){
        if(!DonorNativeOwner(owner)||!std::isfinite(yaw))return false;
        std::uintptr_t function=0,type=0;const auto controller=owner.actor.killerController.object;
        cfgspawn::Vec3 rotation{};rotation.y=yaw;
        if(!FeatureFunction(cfgtrap::disk::SetControlRotation,function,type)||!cfgspawn::ObjectIs(r_,controller,type)||
           !cfgobserver::Same(r_,function+0xB0,plan.module+cfgtrap::disk::SetControlRotationExecutor)||
           !cfgbundle::Code(r_,plan.module,cfgtrap::disk::SetControlRotationExecutor,cfgtrap::disk::SetControlRotationCode)||
           !Invoke(controller,function,&rotation,false))return false;
        const auto pawn=owner.actor.pawn.object;
        cfgtrap::RotationParameters body;body.rotation={0,yaw,0};
        if(FeatureFunction(cfgtrap::disk::K2_SetActorRotation,function,type)&&cfgspawn::ObjectIs(r_,pawn,type)&&
           cfgobserver::Same(r_,function+0xB0,plan.module+cfgtrap::disk::K2_SetActorRotationExecutor)&&
           cfgbundle::Code(r_,plan.module,cfgtrap::disk::K2_SetActorRotationExecutor,cfgtrap::disk::K2_SetActorRotationCode))
            Invoke(pawn,function,&body,false);
        return true;
    }
    bool DonorNativeProject(const cfgjasontick::Owner& owner,cfgspawn::Vec3 point,cfgspawn::Vec3 extent,cfgspawn::Vec3& projected,
        cfgdonorhunt::NativeOutcome* outcome=nullptr){
        if(outcome)*outcome=cfgdonorhunt::NativeOutcome::NotEntered;
        if(!DonorNativeOwner(owner)||!cfgspawn::Finite(point)||!cfgspawn::Finite(extent)||extent.x<0||extent.y<0||extent.z<0)return false;
        std::uintptr_t function=0,navClass=0,system=0;cfgprobe::lifetime::SlotObservation slot{};
        if(!FeatureFunction(cfgvehicle::disk::Project,function,navClass)||
           !r_.Read(plan.world+cfgtrap::disk::WorldNavigationSystem,system)||!cfgspawn::ObjectIs(r_,system,navClass)||
           !cfgspawn::ObjectSlot(r_,context.objectArray,system,slot))return false;
        // FEAT4900 explicitly passes NavData=nullptr, FilterClass=nullptr.
        cfgvehicle::Projection packet;packet.world=plan.world;packet.point=point;packet.extent=extent;
        bundleInvocationStage_=0;
        const bool invoked=Invoke(system,function,&packet,false);
        if(outcome&&bundleInvocationStage_>=5)*outcome=cfgdonorhunt::NativeOutcome::Uncertain;
        if(!invoked||packet.result>1||
           !cfgjason::OwnedComponentSlot(r_,context.objectArray,slot)||
           !cfgobserver::Same(r_,plan.world+cfgtrap::disk::WorldNavigationSystem,system))return false;
        if(!packet.result){if(outcome)*outcome=cfgdonorhunt::NativeOutcome::ReturnedFalse;return false;}
        if(!cfgspawn::Finite(packet.projected))return false;
        if(outcome)*outcome=cfgdonorhunt::NativeOutcome::ReturnedTrue;
        projected=packet.projected;return true;
    }
    bool DonorNativeTeleport(const cfgjasontick::Owner& owner,cfgspawn::Vec3 point,
        cfgdonorhunt::NativeOutcome& outcome){
        outcome=cfgdonorhunt::NativeOutcome::NotEntered;
        if(!DonorNativeOwner(owner)||!cfgspawn::Finite(point))return false;
        // FEAT6437: zero-initialized rotation, native bool at24, padded28.
        // Pre-resolve before resetting the entry witness: lazy metadata work
        // must never be confused with entering the requested teleport.
        std::uintptr_t function=0,type=0;
        if(!FeatureFunction(cfgtrap::disk::K2_TeleportTo,function,type)||
           !cfgspawn::ObjectIs(r_,owner.actor.pawn.object,type)||
           !cfgobserver::Same(r_,function+0xB0,plan.module+cfgtrap::disk::K2_TeleportToExecutor)||
           !cfgbundle::Code(r_,plan.module,cfgtrap::disk::K2_TeleportToExecutor,cfgtrap::disk::K2_TeleportToCode))return false;
        cfgpursuit::TeleportPacket packet;packet.location=point;bundleInvocationStage_=0;
        const bool invoked=Invoke(owner.actor.pawn.object,function,&packet,false);
        if(bundleInvocationStage_>=5)outcome=cfgdonorhunt::NativeOutcome::Uncertain;
        if(!invoked||packet.result>1)return false;
        outcome=packet.result?cfgdonorhunt::NativeOutcome::ReturnedTrue:cfgdonorhunt::NativeOutcome::ReturnedFalse;
        return packet.result!=0;
    }
    cfgdonorhunt::NavProof DonorNativeNavCurrent(const cfgjasontick::Owner& owner,
        const cfgdonorhunt::NavRingIdentity& key){
        using cfgdonorhunt::NavProof;
        if(r_.reads>=r_.cap||calls_>=cfgbundle::CallCap)return NavProof::Unavailable;
        if(!key.Valid()||key.module!=plan.module||key.thread!=GetCurrentThreadId()||
           !(key.slots[0]==plan.objects[0])||!(key.slots[1]==owner.actor.pawn)||
           !(key.slots[2]==owner.actor.killerController))return NavProof::Changed;
        cfgdonorhunt::CounselorObservation target;
        const bool current=DonorNativeCounselor(owner,key.slots[3],target)&&target.Live();
        return current?NavProof::Current:(r_.reads>=r_.cap?NavProof::Unavailable:NavProof::Changed);
    }
    bool DonorNativeNavBegin(const cfgjasontick::Owner& owner,const cfgprobe::lifetime::SlotObservation& target,
        cfgdonorhunt::NavRingPurpose purpose,cfgdonorhunt::NavRingContinuation& state){
        cfgdonorhunt::NavRingIdentity key;key.module=plan.module;key.thread=GetCurrentThreadId();
        key.slots={plan.objects[0],owner.actor.pawn,owner.actor.killerController,target};
        cfgspawn::Vec3 destination{};
        const bool begun=DonorNativeNavCurrent(owner,key)==cfgdonorhunt::NavProof::Current&&
            DonorNativePosition(target,destination)&&state.Begin(key,{destination.x,destination.y,destination.z},purpose);
#if CFG_H284_MORPH_CUTOFF
        if(begun&&(purpose==cfgdonorhunt::NavRingPurpose::Distance||purpose==cfgdonorhunt::NavRingPurpose::SharedFifty)){ // pursuit Morphs only
            auto& c=bundleReport.h284Cutoff[owner.actor.pawn.object==bundleJason.pawn.object?0u:1u];++c.pursuitMorphs;float vx=0,vy=0;
            if(bundleCutoff.Heading(target.object,GetTickCount64(),vx,vy)&&state.Lead(vx,vy)){++c.led;
                cfgclosedtrace::Mark("h284-morph-cutoff",target.object,static_cast<std::uint64_t>(std::sqrt(vx*vx+vy*vy)));}
            else ++c.noHeading;}
#endif
        return begun;
    }
    cfgroofguard::Diagnostics<cfgspawn::Vec3>& DonorRoofDiagnostics(){return bundlePursuit.roof;}
    cfgdonorhunt::NavRingStage DonorNativeNavStep(const cfgjasontick::Owner& owner,
        cfgdonorhunt::NavRingContinuation& state){
        struct Host {
            BundleAccess& access;const cfgjasontick::Owner& owner;cfgbundle::Report::DonorMovementRows& stats;
            cfgbundle::Report::H246NavRows& h246;cfgdonorhunt::NavRingContinuation* nav=nullptr;
            cfgdonorhunt::NavProof Current(const cfgdonorhunt::NavRingIdentity& key){
                return access.DonorNativeNavCurrent(owner,key);
            }
            cfgdonorhunt::NativeOutcome Project(const cfgdonorhunt::NavRingIdentity& key,cfgdonorhunt::Position point,
                cfgdonorhunt::Position extent,cfgdonorhunt::Position& projected){
                return ProjectWith(access,key,point,extent,projected);
            }
            cfgdonorhunt::NativeOutcome ProjectWith(BundleAccess& slice,const cfgdonorhunt::NavRingIdentity& key,
                cfgdonorhunt::Position point,cfgdonorhunt::Position extent,cfgdonorhunt::Position& projected){
                cfgdonorhunt::NativeOutcome result=cfgdonorhunt::NativeOutcome::NotEntered;cfgspawn::Vec3 out{};
                slice.DonorNativeProject(owner,{point.X,point.Y,point.Z},{extent.X,extent.Y,extent.Z},out,&result);
                ++stats.navProjectOutcomes[static_cast<unsigned>(result)];
                if(result!=cfgdonorhunt::NativeOutcome::ReturnedTrue)return result;
                if(CFG_J25_ROOF_GUARD){
                    cfgspawn::Vec3 counselor{};
                    if(slice.DonorNativeNavCurrent(owner,key)!=cfgdonorhunt::NavProof::Current||
                       !slice.DonorNativePosition(key.slots[3],counselor))return cfgdonorhunt::NativeOutcome::NotEntered;
                    bool uncertain=false,starved=false;
                    auto probe=[&](cfgspawn::Vec3 query,cfgspawn::Vec3& lower){
                        auto status=cfgdonorhunt::NativeOutcome::NotEntered;
                        const auto project=[&](BundleAccess& phase){
                            return phase.DonorNativeNavCurrent(owner,key)==cfgdonorhunt::NavProof::Current&&
                                phase.DonorNativeProject(owner,query,{100,100,110},lower,&status);
                        };
                        const bool okay=project(slice);
                        ++stats.navProjectOutcomes[static_cast<unsigned>(status)];
                        if(status==cfgdonorhunt::NativeOutcome::NotEntered)starved=true;
                        if(status==cfgdonorhunt::NativeOutcome::ReturnedFalse)return cfgroofguard::Observation::Absent;
                        if(status==cfgdonorhunt::NativeOutcome::ReturnedTrue&&okay)return cfgroofguard::Observation::Present;
                        if(status==cfgdonorhunt::NativeOutcome::Uncertain)uncertain=true;
                        return cfgroofguard::Observation::Unsafe;
                    };
                    ++h246.roofChecks;
                    if(!cfgroofguard::Safe(access.DonorRoofDiagnostics(),out,
                        {counselor.x,counselor.y,counselor.z-90.f},probe)){
                        if(uncertain)return cfgdonorhunt::NativeOutcome::Uncertain;
                        // H2.46: a probe that could not be ENTERED (call/read budget) is not evidence about the
                        // landing. The first-level Project already treats NotEntered as retry; the probes did not,
                        // so a budget-starved callback burned a candidate (16 in a row = ring Failed, no Teleport).
                        if(nav)return nav->RoofRejected(starved);
                        return cfgdonorhunt::NativeOutcome::ReturnedFalse;
                    }
                }
                projected={out.x,out.y,out.z};return result;
            }
            cfgdonorhunt::NativeOutcome Teleport(const cfgdonorhunt::NavRingIdentity& key,cfgdonorhunt::Position point){
                return TeleportWith(access,key,point);
            }
            cfgdonorhunt::NativeOutcome TeleportWith(BundleAccess& slice,const cfgdonorhunt::NavRingIdentity& key,cfgdonorhunt::Position point){
                cfgdonorhunt::NativeOutcome result=cfgdonorhunt::NativeOutcome::NotEntered;
                if(CFG_J25_ROOF_GUARD){cfgspawn::Vec3 counselor{};
                    if(slice.DonorNativeNavCurrent(owner,key)!=cfgdonorhunt::NavProof::Current||
                       !slice.DonorNativePosition(key.slots[3],counselor)){
                        ++stats.navTeleportOutcomes[static_cast<unsigned>(result)];return result;}
                    slice.DonorRoofDiagnostics().Destination(2,{point.X,point.Y,point.Z},counselor,GetTickCount64());
                    cfgclosedtrace::Mark("donor-nav-ring-morph-destination",key.slots[3].object,GetTickCount64());
                }
                slice.DonorNativeTeleport(owner,{point.X,point.Y,point.Z},result);
                ++stats.navTeleportOutcomes[static_cast<unsigned>(result)];return result;
            }
        } host{*this,owner,bundleReport.donorMovement[owner.actor.pawn.object==bundleJason.pawn.object?0u:1u],
            bundleReport.h246Nav[owner.actor.pawn.object==bundleJason.pawn.object?0u:1u],&state};
        const bool wasPending=state.Pending();const auto before=state; // counters only; ring state itself is never restored
        const auto result=cfgdonorhunt::DonorContinueNavRing(host,state);
        {auto& h=host.h246;const auto p=static_cast<unsigned>(state.purpose);
            h.dzRejects+=state.dzRejects-before.dzRejects;h.horizontalRejects+=state.horizontalRejects-before.horizontalRejects;
            h.roofRejects+=state.roofRejects-before.roofRejects;h.probeStarved+=state.probeStarved-before.probeStarved;
            h.probeStarvedRetried+=state.probeStarvedRetried-before.probeStarvedRetried;
            h.probeStarvedGaveUp+=state.probeStarvedGaveUp-before.probeStarvedGaveUp;
#if CFG_H284_MORPH_CUTOFF
            if(wasPending&&state.lead&&result==cfgdonorhunt::NavRingStage::Succeeded){
                auto& c=bundleReport.h284Cutoff[owner.actor.pawn.object==bundleJason.pawn.object?0u:1u];
                if(state.Index()>=16)++c.landedClose;else ++c.landedRing;}
#endif
            if(wasPending&&p<h.ringSucceeded.size()){
                if(result==cfgdonorhunt::NavRingStage::Succeeded)++h.ringSucceeded[p];
                else if(result==cfgdonorhunt::NavRingStage::Failed)++h.ringFailed[p];}}
        auto& diagnostic=bundleReport.donorMovement[owner.actor.pawn.object==bundleJason.pawn.object?0u:1u];
        diagnostic.navStage=static_cast<unsigned>(result);diagnostic.navCandidate=state.candidate;
        diagnostic.navPurpose=static_cast<unsigned>(state.purpose);return result;
    }
