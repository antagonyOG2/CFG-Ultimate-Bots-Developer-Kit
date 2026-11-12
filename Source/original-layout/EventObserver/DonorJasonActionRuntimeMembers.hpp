#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
    bool DonorNativeMovement(const cfgjasontick::Owner& owner,cfgprobe::lifetime::SlotObservation& slot,std::array<float,4>& speeds){
        slot={};speeds={};std::uintptr_t movement=0,type=0;
        const auto pawn=owner.actor.pawn.object;const auto& binding=cfghook::disk::Types[cfghook::disk::Movement];
        if(!DonorNativeOwner(owner)||!r_.Read(pawn+cfghook::disk::MovementComponent,movement)||
           !NativeClass(binding.reflection,binding.constructor,type)||!cfgspawn::ObjectIs(r_,movement,type)||
           !cfgspawn::ObjectSlot(r_,context.objectArray,movement,slot)||
           !cfgobserver::Same(r_,movement+0x20,pawn)||!cfgobserver::Same(r_,movement+0xE0,pawn))return false;
        constexpr std::uintptr_t fields[]{cfghook::disk::Walk,cfghook::disk::Sprint,cfghook::disk::Run,cfghook::disk::SlowRun};
        for(unsigned i=0;i<4;++i)if(!r_.Read(movement+fields[i],speeds[i])||!cfgdonorhunt::DonorValidPursuitSpeed(speeds[i]))return false;
        return cfgjason::OwnedComponentSlot(r_,context.objectArray,slot)&&
            cfgobserver::Same(r_,pawn+cfghook::disk::MovementComponent,movement);
    }
    bool DonorNativeWriteSpeeds(const cfgjasontick::Owner& owner,const cfgprobe::lifetime::SlotObservation& expected,
        const std::array<float,4>& desired,unsigned mask){
        cfgprobe::lifetime::SlotObservation slot{};std::array<float,4> current{};
        if(!mask||mask>15||!DonorNativeMovement(owner,slot,current)||!(slot==expected))return false;
        constexpr std::uintptr_t fields[]{cfghook::disk::Walk,cfghook::disk::Sprint,cfghook::disk::Run,cfghook::disk::SlowRun};
        for(unsigned i=0;i<4;++i)if(mask&(1u<<i)){
            if(!cfgdonorhunt::DonorValidPursuitSpeed(desired[i])||
               !NativeEnvironmentAllowed()||!cfgjason::OwnedComponentSlot(r_,context.objectArray,slot)||
               !cfgobserver::Same(r_,owner.actor.pawn.object+cfghook::disk::MovementComponent,slot.object)||
               !CarSpeedWrite(slot.object+fields[i],current[i],desired[i]))return false;
        }
        return true;
    }
    bool J3HardMovement(const cfgjasontick::Owner& owner,DonorNativeJasonRuntime& state){
#ifdef CFG_H1_JASON
        HostedLaneEnter(cfgh1::Lane::Hard);
#endif

        const bool enabled=CFG_J3_HARD_MOVEMENT||(CFG_J215_HARD_MOVEMENT&&
            cfgpractice::HardAutomaticOrSelected(bundleConfiguration.counselorPractice));
        if(!enabled||bundleConfiguration.difficulty!=2||state.hardMovementAttempted)return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Hard,1)
#else
 false
#endif
;
        cfgprobe::lifetime::SlotObservation movement{};std::array<float,4> speeds{};
        if(!DonorNativeMovement(owner,movement,speeds))return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Hard,2)
#else
 false
#endif
;
        // Exactly the existing human-Jason +3D0 / +720,+724 native fields.
        const float sprint=speeds[1]*2,run=speeds[2]*2;
        if(!cfgdonorhunt::DonorValidPursuitSpeed(sprint)||!cfgdonorhunt::DonorValidPursuitSpeed(run)||
            !DonorNativeOwner(owner)||!cfgjason::OwnedComponentSlot(r_,context.objectArray,movement))return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Hard,3)
#else
 false
#endif
;
        state.hardMovementAttempted=true;state.hardMovement=movement; // consume BEFORE either write
        state.hardBefore={{speeds[1],speeds[2]}};state.hardDesired={{sprint,run}};
        constexpr std::uintptr_t fields[]{cfghook::disk::Sprint,cfghook::disk::Run};
        for(unsigned i=0;i<2;++i){
            if(!NativeEnvironmentAllowed()||!DonorNativeOwner(owner)||!cfgjason::OwnedComponentSlot(r_,context.objectArray,movement)||
               !CarSpeedWrite(movement.object+fields[i],state.hardBefore[i],state.hardDesired[i])){
                ++bundleReport.j3HardMovementRefused;return true;
            }
            state.hardOwned|=1u<<i;
        }
        ++bundleReport.j3HardMovementWrites[owner.actor.pawn==bundleJason.pawn?0u:1u];return true;
    }
    bool J215RestoreHard(DonorNativeJasonRuntime& state){
        if(!state.hardOwned)return false;
        const auto owner=state.birth;cfgprobe::lifetime::SlotObservation movement{};std::array<float,4> speeds{};
        if(!(owner.source.objects[0]==plan.objects[0])||!DonorNativeMovement(owner,movement,speeds)||!(movement==state.hardMovement)){
            ++bundleReport.j215Practice.speedRestoreRefused;return false;
        }
        constexpr std::uintptr_t fields[]{cfghook::disk::Sprint,cfghook::disk::Run};bool changed=false;
        for(unsigned i=0;i<2;++i)if(state.hardOwned&(1u<<i)){
            // Another speed owner/native change wins; never overwrite it.
            if(speeds[i+1]!=state.hardDesired[i]){state.hardOwned&=~(1u<<i);continue;}
            if(!NativeEnvironmentAllowed()||!DonorNativeOwner(owner)||!cfgjason::OwnedComponentSlot(r_,context.objectArray,movement)||
               !CarSpeedWrite(movement.object+fields[i],state.hardDesired[i],state.hardBefore[i])){
                ++bundleReport.j215Practice.speedRestoreRefused;continue;
            }
            state.hardOwned&=~(1u<<i);++bundleReport.j215Practice.speedRestores;changed=true;
        }
        return changed;
    }
    enum class DonorQuery {Stunned,Attacking,CanAttack,SpecialMove};
    void DonorNativePredicateBegin(const cfgjasontick::Owner& owner,unsigned kind)noexcept{
#ifdef CFG_H1_JASON
        if(owner.adoption.valid){++hostedDriverTrace.adoptedPredicateChecks;hostedDriverTrace.adoptedPredicateKind=kind;hostedDriverTrace.adoptedPredicateClause=0;}
#else
        (void)owner;(void)kind;
#endif
    }
    bool DonorNativePredicateRefuse(const cfgjasontick::Owner& owner,unsigned clause)noexcept{
#ifdef CFG_H1_JASON
        if(owner.adoption.valid){++hostedDriverTrace.adoptedPredicateRefusals;hostedDriverTrace.adoptedPredicateClause=clause;}
#else
        (void)owner;(void)clause;
#endif
        return false;
    }
    bool DonorNativeQuery(const cfgjasontick::Owner& owner,DonorQuery query,bool& value)noexcept{
        const unsigned kind=static_cast<unsigned>(query)+1;DonorNativePredicateBegin(owner,kind);
        value=false;std::uintptr_t entry=0;bool code=false;
        if(!DonorNativeOwner(owner))return DonorNativePredicateRefuse(owner,1);
        if(!NativeCallAllowed())return DonorNativePredicateRefuse(owner,2);
        switch(query){
        case DonorQuery::Stunned:entry=cfghunter::disk::Stunned;code=cfgbundle::Code(r_,plan.module,entry,cfghunter::disk::StunnedBytes);break;
        case DonorQuery::Attacking:entry=cfghunter::disk::Attacking;code=cfgbundle::Code(r_,plan.module,entry,cfghunter::disk::AttackingBytes);break;
        case DonorQuery::CanAttack:entry=cfghunter::disk::CanAttack;code=cfgbundle::Code(r_,plan.module,entry,cfghunter::disk::CanAttackBytes);break;
        case DonorQuery::SpecialMove:{using namespace cfgcompat::cfg35::j1a;std::uintptr_t table=0,target=0;
            entry=SpecialMove;code=r_.Read(owner.actor.pawn.object,table)&&r_.Read(table+SpecialMoveSlot,target)&&
                target==plan.module+entry&&cfgbundle::Code(r_,plan.module,entry,SpecialMoveCode);break;}
        }
        if(!code)return DonorNativePredicateRefuse(owner,3);
        if(!RetainBundleActors(owner.actor.pawn.object))return DonorNativePredicateRefuse(owner,4);
        if(!NativeCallAllowed())return DonorNativePredicateRefuse(owner,5);
        if(!BeginBundleNative())return DonorNativePredicateRefuse(owner,6);
        ++calls_;unsigned char result=0;
        __try{result=reinterpret_cast<unsigned char(__fastcall*)(void*)>(plan.module+entry)(reinterpret_cast<void*>(owner.actor.pawn.object));}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;BundleRetentionFailed();return DonorNativePredicateRefuse(owner,7);}
        if(!FinishBundleNative())return DonorNativePredicateRefuse(owner,8);
        if(result>1)return DonorNativePredicateRefuse(owner,9);
        value=result!=0;return true;
    }
    bool DonorNativeHeld(const cfgjasontick::Owner& owner,std::uintptr_t& held){
        held=0;return DonorNativeOwner(owner)&&r_.Read(owner.actor.pawn.object+cfghunter::disk::Held,held)&&
            cfgobserver::Same(r_,owner.actor.pawn.object+cfghunter::disk::Held,held);
    }
    bool DonorNativeRunState(const cfgjasontick::Owner& owner,DonorNativeJasonRuntime& state,std::uint64_t now,
        cfgdonorhunt::NativeOutcome& outcome){
        outcome=cfgdonorhunt::NativeOutcome::NotEntered;
        if(now<state.runStateAt||!DonorNativeOwner(owner))return false;
        const auto pawn=owner.actor.pawn.object;std::uint8_t running=0;
        std::uintptr_t water=0,held=0,function=0,type=0;
        if(!r_.Read(pawn+cfgrun::disk::Running,running)||running>1||
           !r_.Read(pawn+cfgswim::disk::WaterVolume,water)||!r_.Read(pawn+cfghunter::disk::Held,held))return false;
        auto& movement=bundleReport.donorMovement[state.isAdditional?1u:0u];
        if(running){if(state.runningLastAt&&now>=state.runningLastAt&&now-state.runningLastAt<=500)movement.runningMs+=now-state.runningLastAt;
            state.runningLastAt=now;}else state.runningLastAt=0;
        if(running||water||held){state.runStateAt=now+250;return false;}
        bool busy=false;
        if(!DonorNativeQuery(owner,DonorQuery::SpecialMove,busy)||busy)return false;
        using namespace cfgrun::disk;
        if(!FeatureFunction(ServerSetRunning,function,type)||!cfgspawn::ObjectIs(r_,pawn,type)||
           !cfgobserver::Same(r_,function+0xB0,plan.module+Executor)||
           !cfgbundle::Code(r_,plan.module,Executor,ExecutorCode)||
           !cfgbundle::Code(r_,plan.module,Implementation,ImplementationCode)||
           !cfgbundle::Code(r_,plan.module,Toggle,ToggleCode)||!cfgbundle::Code(r_,plan.module,SetBit,SetBitCode)||
           !DonorNativeOwner(owner)||!cfgescape::Same(r_,pawn+Running,std::uint8_t{}))return false;
        std::uint8_t desired=1;bundleInvocationStage_=0;
        const bool returned=Invoke(pawn,function,&desired);
        if(bundleInvocationStage_>=5){state.runStateAt=now+250;outcome=cfgdonorhunt::NativeOutcome::Uncertain;}
        if(returned){outcome=cfgdonorhunt::NativeOutcome::ReturnedTrue;return true;}return false;
    }
    bool DonorNativeObserveRunning(const cfgjasontick::Owner& owner,DonorNativeJasonRuntime& state,std::uint64_t now){
        if(now<state.runningObserveAt||!DonorNativeOwner(owner))return false;
        state.runningObserveAt=now<=UINT64_MAX-250?now+250:UINT64_MAX;
        std::uint8_t running=0;
        if(!r_.Read(owner.actor.pawn.object+cfgrun::disk::Running,running)||running>1)return false;
        auto& movement=bundleReport.donorMovement[state.isAdditional?1u:0u];
        if(running){if(state.runningLastAt&&now>=state.runningLastAt&&now-state.runningLastAt<=500)movement.runningMs+=now-state.runningLastAt;
            state.runningLastAt=now;}else state.runningLastAt=0;
        return true;
    }
    bool DonorNativeLocked(const cfgjasontick::Owner& owner,bool& locked){
        DonorNativePredicateBegin(owner,5);
        locked=false;std::uintptr_t manager=0,type=0,component=0;cfgprobe::lifetime::SlotObservation slot{};
        if(!DonorNativeOwner(owner))return DonorNativePredicateRefuse(owner,1);
        if(!r_.Read(owner.actor.pawn.object+cfghunter::disk::Manager,manager))return DonorNativePredicateRefuse(owner,2);
        if(!NativeClass(cfghunter::disk::ManagerReflection,cfghunter::disk::ManagerConstructor,type))return DonorNativePredicateRefuse(owner,3);
        if(!cfgspawn::ObjectIs(r_,manager,type))return DonorNativePredicateRefuse(owner,4);
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,manager,slot))return DonorNativePredicateRefuse(owner,5);
        if(!cfgobserver::Same(r_,manager+0x20,owner.actor.pawn.object)||!cfgobserver::Same(r_,manager+0xE0,owner.actor.pawn.object))return DonorNativePredicateRefuse(owner,6);
        if(!r_.Read(manager+cfgvehicleinteraction::disk::LockedComponent,component))return DonorNativePredicateRefuse(owner,7);
        if(!cfgobserver::Same(r_,manager+cfgvehicleinteraction::disk::LockedComponent,component))return DonorNativePredicateRefuse(owner,8);
        if(!cfgjason::OwnedComponentSlot(r_,context.objectArray,slot))return DonorNativePredicateRefuse(owner,9);
        locked=component!=0;return true;
    }
    bool DonorNativeSelectedDoor(const cfgjasontick::Owner& owner,bool& selected){
        selected=false;std::uintptr_t doorClass=0,componentClass=0;
        if(!DonorNativeOwner(owner)||!NativeClass(cfgdoor::disk::SCDoorReflection,cfgdoor::disk::SCDoorConstructor,doorClass)||
           !NativeClass(cfgdoor::disk::ILLInteractableComponentReflection,cfgdoor::disk::ILLInteractableComponentConstructor,componentClass))return false;
        cfgdoor::Selected value{};
        selected=cfgdoor::ObserveSelected(r_,context.objectArray,owner.actor.pawn.object,doorClass,componentClass,value);
        return r_.reads<r_.cap;
    }
    bool DonorNativeReadDriver(const cfgjasontick::Owner& owner,std::uintptr_t& driver){
        using namespace cfgcompat::cfg35::j1a;driver=0;std::uintptr_t table=0;
        return DonorNativeOwner(owner)&&r_.Read(owner.actor.pawn.object,table)&&
            r_.Read(table+DriverSlot,driver)&&driver==plan.module+Driver&&cfgbundle::Code(r_,plan.module,Driver,DriverCode);
    }
#if CFG_H264_F1_INTERACTIONS
    // H2.64: donor FJB TryAdditionalJasonNativeDoor over the same proven natives the primary DoorRecovery uses.
    struct DonorAdditionalDoorFacts {cfgdoor::Selected selected{};cfgdoor::BreakState state{};cfgspawn::Vec3 anchor{};std::uintptr_t doorClass=0,componentClass=0;std::uint8_t permitted=0;bool attackComponent=false;};
    bool DonorNativeAdditionalDoorFacts(const cfgjasontick::Owner& owner,DonorAdditionalDoorFacts& f){
        f={};std::uintptr_t barricade=0,scene=0,attack=0,manager=0,best=0,fallback=0,locked=0;const auto pawn=owner.actor.pawn.object;
        if(!r_.Read(pawn+cfghunter::disk::Manager,manager)||!manager||!r_.Read(manager+cfgdoor::disk::Best,best)||!r_.Read(manager+cfgdoor::disk::Fallback,fallback)||(!best&&!fallback))return false; // cheap miss first
        if(!r_.Read(manager+cfgvehicleinteraction::disk::LockedComponent,locked)||locked)return false;
        if(!DonorNativeOwner(owner)||!Context()||
           !NativeClass(cfgdoor::disk::SCDoorReflection,cfgdoor::disk::SCDoorConstructor,f.doorClass)||
           !NativeClass(cfgdoor::disk::ILLInteractableComponentReflection,cfgdoor::disk::ILLInteractableComponentConstructor,f.componentClass)||
           !cfgdoor::ObserveSelected(r_,context.objectArray,pawn,f.doorClass,f.componentClass,f.selected)||
           !cfgdoor::ObserveBreak(r_,context.objectArray,pawn,f.selected,f.doorClass,f.state)||
           !NativeClass(cfgdoor::breakdisk::BarricadeReflection,cfgdoor::breakdisk::BarricadeConstructor,barricade)||
           !cfgdoor::ObserveBarricade(r_,context.objectArray,f.selected,barricade,f.state.barricaded)||
           !NativeClass(cfgposition::disk::SceneClassCache,cfgposition::disk::SceneConstructorCache,scene)||
           !(f.state.Blocked()?cfgdoor::ObserveAnchor(r_,context.objectArray,f.selected,f.componentClass,scene,f.anchor):
             cfgdoor::ObserveSelectedAnchor(r_,context.objectArray,f.selected,scene,f.anchor))||
           !NativeClass(cfgdoor::breakdisk::ComponentReflection,cfgdoor::breakdisk::ComponentConstructor,attack))return false;
        f.attackComponent=cfgspawn::ObjectIs(r_,f.selected.component,attack);
        return !f.attackComponent||(r_.Read(f.selected.component+cfgdoor::breakdisk::CanAttackToInteract,f.permitted)&&f.permitted<=1);
    }
    bool DonorNativeAdditionalFace(const cfgjasontick::Owner& owner,const cfgspawn::Vec3& point){
        cfgspawn::Vec3 jason{};if(!Location(owner.actor.pawn.object,jason))return false;
        cfgtrap::RotationParameters body;body.rotation.y=std::atan2(point.y-jason.y,point.x-jason.x)*57.2957795131f;
        return CarCall(cfgtrap::disk::K2_SetActorRotation,cfgtrap::disk::K2_SetActorRotationExecutor,cfgtrap::disk::K2_SetActorRotationCode,owner.actor.pawn.object,&body)&&body.result==1&&
            CarCall(cfgtrap::disk::SetControlRotation,cfgtrap::disk::SetControlRotationExecutor,cfgtrap::disk::SetControlRotationCode,owner.actor.killerController.object,&body.rotation);
    }
    // Locked doors use the pinned generic press; unlocked doors use the explicit component press (PID10400).
    // H2.66: why = 1 face, 2 owner, 3 selection changed, 4 generic press would pick something else, 5 break not requestable, 6 blocked or owned, 7 function, 8 native.
    bool DonorNativeAdditionalDoorPress(const cfgjasontick::Owner& owner,const DonorAdditionalDoorFacts& f,bool& breakStarted,bool& opened,unsigned& why){
        breakStarted=opened=false;why=0;const auto pawn=owner.actor.pawn.object;cfgdoor::Selected fresh{};
        if(!DonorNativeAdditionalFace(owner,f.anchor)){why=1;return false;}
        if(!DonorNativeOwner(owner)){why=2;return false;}
        if(!cfgdoor::ObserveSelected(r_,context.objectArray,pawn,f.doorClass,f.componentClass,fresh)||!cfgdoor::Same(f.selected,fresh)){why=3;return false;}
        bool returned=false;
        if(f.state.Blocked()&&f.attackComponent&&f.permitted==1){
            if(!cfgdoor::NativeSelectionMatches(r_,fresh)){why=4;return false;} // only the generic press depends on the selection order
            if(!f.state.CanRequestNative(f.permitted)){why=5;return false;}
#if CFG_H268_F1_DOOR_PRESS
            // Live H2.67: 5 of 5 locked-door presses failed the owner proof at this exact call. Never repeat a press that was entered.
            {BundleAccess press(cfgbundle::ReadCap,calls_);BundleAccess::ChildAccounting accounting(*this,press);
             auto outcome=cfgdonorhunt::NativeOutcome::NotEntered;
             returned=press.DonorNativeInput(owner,false,&outcome);
             if(!returned&&outcome==cfgdonorhunt::NativeOutcome::NotEntered){why=9;returned=press.Context()&&press.AdditionalLinked()&&press.AdditionalInput(false);}
             if(returned)why=0;}
#else
            returned=DonorNativeInput(owner,false);
#endif
        }else{
            if(f.state.Blocked()||f.state.target||f.state.killer){why=6;return false;}
            std::uintptr_t function=0,functionOwner=0;
            if(!FeatureFunction(cfgdoor::disk::Interact,function,functionOwner)||!cfgspawn::ObjectIs(r_,pawn,functionOwner)||
               !cfgobserver::Same(r_,function+cfgdoor::disk::FunctionExecutor,plan.module+cfgdoor::disk::Executor)||
               !cfgbundle::Code(r_,plan.module,cfgdoor::disk::Executor,cfgdoor::disk::ExecutorCode)){why=7;return false;}
            cfgdoor::InteractPacket packet;packet.component=f.selected.component;
            returned=Invoke(pawn,function,&packet);
        }
        if(!returned){if(!why)why=8;return false;}
        cfgdoor::BreakState after;
        if(cfgdoor::ObserveBreak(r_,context.objectArray,pawn,f.selected,f.doorClass,after)){breakStarted=after.Owned(pawn,f.selected.door);opened=after.open!=0;}
        return true;
    }
    // H2.64: donor FindNearbyOccupiedHidingSpotForJason over the reciprocal CurrentHidingSpot chain.
    bool DonorNativeAdditionalHider(const cfgjasontick::Owner& owner,cfgdonorhunt::Targets& roster,std::uint64_t now,
        std::uintptr_t& outSpot,std::uintptr_t& outHidden,std::uintptr_t& outComponent,cfgspawn::Vec3& outPoint,std::uintptr_t chasing=0){
        using namespace cfgaction::disk;outSpot=outHidden=outComponent=0;outPoint={};const auto pawn=owner.actor.pawn.object;
        const unsigned total=roster.count>0?(std::min)(static_cast<unsigned>(roster.count),64u):0u;bool any=false;
        for(unsigned i=0;i<total&&!any;++i){std::uintptr_t spot=0;any=roster.actors[i]&&r_.Read(roster.actors[i]+CurrentHidingSpot,spot)&&spot;}
        if(!any||!DonorNativeOwner(owner)||!Context())return false;
        std::uintptr_t type=0,native=0,scene=0,manager=0,locked=0,best=0;cfgspawn::Vec3 from{};
        if(!NativeClass(HidingReflection,HidingConstructor,type)||!NativeClass(cfgvehicleinteraction::disk::ILLInteractableComponent,cfgvehicleinteraction::disk::ILLInteractableComponentConstructor,native)||
           !NativeClass(cfgposition::disk::SceneClassCache,cfgposition::disk::SceneConstructorCache,scene)||!Location(pawn,from)||
           !r_.Read(pawn+cfghunter::disk::Manager,manager)||!r_.Read(manager+cfgvehicleinteraction::disk::LockedComponent,locked)||!r_.Read(manager+0x210,best))return false;
        float nearest=(std::numeric_limits<float>::max)();
        for(unsigned i=0;i<total;++i){
            const auto candidate=roster.actors[i];std::uintptr_t spot=0,hidden=0,component=0,owner2=0;cfgprobe::lifetime::SlotObservation slot{};cfgspawn::Vec3 point{};
            if(!candidate||roster.DonorIsJasonAITargetBlocked(candidate,now)||!r_.Read(candidate+CurrentHidingSpot,spot)||!spot)continue;
            if(!cfgspawn::ObjectIs(r_,spot,type)||!cfgspawn::ObjectSlot(r_,context.objectArray,spot,slot)||!slot.HasAssignedSerial()||!r_.Read(spot+HidingCounselor,hidden)||hidden!=candidate)continue;
            const auto belongs=[&](std::uintptr_t c){std::uintptr_t actual=0;return c&&cfgspawn::ObjectIs(r_,c,native)&&r_.Read(c+0xE0,actual)&&actual==spot;};
            if(belongs(locked))component=locked;else if(belongs(best))component=best;else if(!r_.Read(spot+KillerHidingComponent,component)||!belongs(component))continue;
            if(!r_.Read(component+0x20,owner2)||owner2!=spot||!cfgspawn::ObjectIs(r_,component,scene)||!r_.Read(component+cfgposition::disk::SceneWorldTranslation,point)||!cfgspawn::Finite(point))continue;
            const float dx=point.x-from.x,dy=point.y-from.y,squared=dx*dx+dy*dy;
            if(!std::isfinite(squared)||squared>=nearest||(squared>=600.f*600.f&&!(CFG_H266_HIDER_APPROACH&&candidate==chasing)))continue; // 6 m as the donor, or whoever he is already hunting
            nearest=squared;outSpot=spot;outHidden=candidate;outComponent=component;outPoint=point;
        }
        return outSpot!=0;
    }
#if CFG_H266_F1_SWIM
    // H2.66: same native pair as SwimPhase; the game chooses the eligible swimmer. Ordinary chase closes the distance.
    bool DonorNativeAdditionalSwim(const cfgjasontick::Owner& owner,std::uintptr_t target,std::uint64_t now,std::uint64_t& next){
        using namespace cfgswim::disk;if(now<next)return false;
        const auto pawn=owner.actor.pawn.object;std::uintptr_t water=0,can=0,action=0,functionOwner=0,held=0;
        if(!r_.Read(target+WaterVolume,water)||!water)return false;
        next=now+500;cfgspawn::Vec3 from{},at{};
        if(!DonorNativeOwner(owner)||!Context()||!Location(pawn,from)||!Location(target,at))return false;
        const float dx=at.x-from.x,dy=at.y-from.y,dz=at.z-from.z;
        if(!std::isfinite(dx*dx+dy*dy+dz*dz)||dx*dx+dy*dy+dz*dz>600.f*600.f||calls_+6>cfgbundle::CallCap)return false;
        if(!FeatureFunction(CanInteractUnderwater,can,functionOwner)||!cfgspawn::ObjectIs(r_,pawn,functionOwner)||
           !cfgobserver::Same(r_,can+0xB0,plan.module+CanInteractUnderwaterExecutor)||!cfgbundle::Code(r_,plan.module,CanInteractUnderwaterExecutor,CanInteractUnderwaterCode)||
           !cfgbundle::Code(r_,plan.module,CanInteractUnderwaterBody,CanInteractUnderwaterBodyCode)||
           !FeatureFunction(OnInteractUnderwater,action,functionOwner)||!cfgspawn::ObjectIs(r_,pawn,functionOwner)||
           !cfgobserver::Same(r_,action+0xB0,plan.module+OnInteractUnderwaterExecutor)||!cfgbundle::Code(r_,plan.module,OnInteractUnderwaterExecutor,OnInteractUnderwaterCode)||
           !cfgbundle::Code(r_,plan.module,OnInteractUnderwaterBody,OnInteractUnderwaterBodyCode))return false;
        struct Admission {std::uintptr_t character=0;cfgspawn::Vec3 location{},rotation{};std::int32_t result=0;std::uint32_t pad=0;} query;
        struct Action {std::uintptr_t interactor=0;std::uint8_t method=0,pad[7]{};} packet;
        query.character=pawn;query.location=from;
        if(!Invoke(pawn,can,&query)||query.result!=1)return false;
        bool stunned=true,attacking=true;
        if(!r_.Read(pawn+cfghunter::disk::Held,held)||held||!DonorNativeQuery(owner,DonorQuery::Stunned,stunned)||stunned||
           !DonorNativeQuery(owner,DonorQuery::Attacking,attacking)||attacking||!cfgobserver::Same(r_,plan.module+OnInteractUnderwater.cache,action))return false;
        packet.interactor=pawn;next=now+3000;
        const bool sent=Invoke(pawn,action,&packet);
        cfgclosedtrace::Mark(sent?"f1-h266-swim":"f1-h266-swim-refused",target,pawn);return sent;
    }
#endif
#if CFG_F1_KNIFE
    // F1 knife natives: every access targets the F1 pawn (owner.actor.pawn), never bundleJason.
    bool DonorNativeAdditionalKnifeRead(const cfgjasontick::Owner& owner,std::int32_t& count,std::uint8_t& pressed,std::uintptr_t& driver){
        count=0;pressed=0;driver=0;const auto pawn=owner.actor.pawn.object;
        return DonorNativeOwner(owner)&&r_.Read(pawn+cfgknife::disk::Count,count)&&r_.Read(pawn+cfgknife::disk::Pressed,pressed)&&r_.Read(pawn+cfgknife::disk::Driver,driver);
    }
    bool DonorNativeAdditionalKnifeCall(const cfgjasontick::Owner& owner,std::uintptr_t entry){
        if(!NativeCallAllowed()||!plan.module||!DonorNativeOwner(owner))return false;
        const bool code=entry==cfgknife::disk::Press?cfgbundle::Code(r_,plan.module,entry,cfgknife::disk::PressCode):
            entry==cfgknife::disk::Release?cfgbundle::Code(r_,plan.module,entry,cfgknife::disk::ReleaseCode):
            entry==cfgknife::disk::Finalize&&cfgbundle::Code(r_,plan.module,entry,cfgknife::disk::FinalizeCode);
        if(!code)return false;
        const auto pawn=owner.actor.pawn.object;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        std::uintptr_t driver=0; // H2.72: same retention bracket as KnifeSimpleCall, on the F1 pawn
        if(!RetainBundleActors(pawn)||!r_.Read(pawn+cfgknife::disk::Driver,driver)||
           !RetainPrimaryObjects(r_,std::array<std::uintptr_t,1>{driver})||!BeginBundleNative())return false;
#endif
        ++calls_;
        const auto savedReleaseDepth=bundleKnifeReleaseDepth;
        if(entry==cfgknife::disk::Release)++bundleKnifeReleaseDepth;
        __try{reinterpret_cast<void(__fastcall*)(void*)>(plan.module+entry)(reinterpret_cast<void*>(pawn));}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;bundleKnifeReleaseDepth=savedReleaseDepth;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
            BundleRetentionFailed();
#endif
            return false;}
        bundleKnifeReleaseDepth=savedReleaseDepth;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        return FinishBundleNative();
#else
        return true;
#endif
    }
    bool DonorNativeAdditionalKnifeViewWrite(std::uintptr_t pawn,const cfgspawn::Vec3& before,const cfgspawn::Vec3& rotation)noexcept{
        if(GetCurrentThreadId()!=plan.threadId)return false;
        __try{auto* view=reinterpret_cast<void*>(pawn+0x7DC);
            if(std::memcmp(view,&before,sizeof(before))!=0)return false;
            std::memcpy(view,&rotation,sizeof(rotation));return true;}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;return false;}
    }
    bool DonorNativeAdditionalKnifeAim(const cfgjasontick::Owner& owner,std::uintptr_t target){
        cfgspawn::Vec3 jason{},at{},previous{};
        if(!Context()||!DonorNativeOwner(owner)||!target||!Location(owner.actor.pawn.object,jason)||!Location(target,at))return false;
        const float dx=at.x-jason.x,dy=at.y-jason.y,horizontal=std::sqrt(dx*dx+dy*dy);float pitch=0.f;
        if(!std::isfinite(horizontal)||!cfgknife::AimPitch(jason.z,at.z,horizontal,pitch))return false;
        const float yaw=std::atan2(dy,dx)*57.2957795131f;cfgspawn::Vec3 view{pitch,yaw,0.f};
        cfgtrap::RotationParameters body;body.rotation={0.f,yaw,0.f}; // donor: body pitch 0, yaw only; pitch lives in the controller and the throw view
        if(!cfgspawn::Finite(view)||!CarCall(cfgtrap::disk::SetControlRotation,cfgtrap::disk::SetControlRotationExecutor,cfgtrap::disk::SetControlRotationCode,owner.actor.killerController.object,&view)||
           !CarCall(cfgtrap::disk::K2_SetActorRotation,cfgtrap::disk::K2_SetActorRotationExecutor,cfgtrap::disk::K2_SetActorRotationCode,owner.actor.pawn.object,&body)||body.result!=1)return false;
        return cfgbundle::Code(r_,plan.module,cfgknife::disk::ViewTransform,cfgknife::disk::ViewTransformCode)&&
            cfgbundle::Code(r_,plan.module,cfgknife::disk::Velocity,cfgknife::disk::VelocityCode)&&
            r_.Read(owner.actor.pawn.object+0x7DC,previous)&&cfgspawn::Finite(previous)&&
            DonorNativeAdditionalKnifeViewWrite(owner.actor.pawn.object,previous,view);
    }
    bool DonorNativeAdditionalKnifeSerial(std::uintptr_t target,std::int32_t& serial){
        serial=0;cfgknife::Identity id{};
        if(!Context()||!KnifeIdentity(target,id))return false;
        serial=id.serial;return true;
    }
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    // Read-only F1 twin of KnifeDriver for the supplied pawn.
    bool DonorNativeAdditionalKnifeDriverProof(std::uintptr_t pawn,const cfgknife::Identity& expected,bool requireUnthrown){
        using namespace cfgknife::disk;std::uintptr_t driver=0,type=0,ctor=0,table=0,owner=0;unsigned char authority=0,collision=0;cfgknife::Identity id{};
        return r_.Read(pawn+Driver,driver)&&driver&&driver==expected.object&&KnifeIdentity(driver,id)&&id==expected&&
            r_.Read(plan.module+ThrowableReflection,type)&&r_.Read(plan.module+ThrowableConstructor,ctor)&&type&&type==ctor&&
            cfgspawn::ObjectIs(r_,driver,type)&&r_.Read(driver+Role,authority)&&authority==3&&
            r_.Read(driver+ThrownBy,owner)&&(requireUnthrown?owner==0:(!owner||owner==pawn))&&
            r_.Read(driver,table)&&cfgobserver::Same(r_,table+0x5F0,plan.module+MulticastUse)&&
            cfgobserver::Same(r_,table+0x5E8,plan.module+Launch)&&cfgobserver::Same(r_,table+0x608,plan.module+OnStop)&&
            r_.Read(driver+0x86,collision)&&(collision&2)!=0;
    }
    // F1 twin of KnifeReleaseEntryProof/Current; runs inside the native Release detour, read-only on a fresh reader.
    bool DonorNativeAdditionalKnifeEntryCurrent(const cfgknife::ReleaseFrame& frame){
        const auto& w=frame.window;const auto pawn=w.jason.object;
        if(frame.access!=this||frame.lease!=bundleRootLease||!bundleRootLease||!bundleRootLease->Current()||!bundleRootLease->NativeEntered()||
           frame.thread!=plan.threadId||GetCurrentThreadId()!=plan.threadId||frame.depth!=bundleKnifeReleaseDepth||
           frame.entries!=1||frame.inside||frame.ambiguous||!w.armed||bundleKnifeReleaseFrame!=&frame)return false;
        BundleAccess entry(cfgbundle::ReadCap,calls_);entry.context=context;ChildAccounting accounting(*this,entry);
        cfgknife::Identity jason{},target{};bool living=false;std::int32_t count=0;
        return entry.KnifeIdentity(pawn,jason)&&jason==w.jason&&entry.DonorNativeAdditionalKnifeDriverProof(pawn,w.driver,true)&&
            entry.r_.Read(pawn+cfgknife::disk::Count,count)&&count==w.count&&entry.KnifeIdentity(w.target.object,target)&&target==w.target&&
            entry.Alive(w.target.object,living)&&living;
    }
#endif
    // H2.72: Release through the primary's native geometry window. A refused correction never suppresses the native release.
    bool DonorNativeAdditionalKnifeRelease(const cfgjasontick::Owner& owner,std::uintptr_t target,std::int32_t targetSerial,unsigned& flags){
        flags=0;
#ifndef CFG_RETAINED_GAMEPLAY_BUNDLE
        (void)target;(void)targetSerial;return DonorNativeAdditionalKnifeCall(owner,cfgknife::disk::Release);
#else
        cfgknife::ReleaseFrame frame;const auto pawn=owner.actor.pawn.object;auto& w=frame.window;
        std::int32_t count=0;std::uint8_t pressed=0;std::uintptr_t driver=0;cfgknife::Identity jasonId{},driverId{},targetId{};
        bool living=false;cfgspawn::Vec3 j{},t{};float speed=0;
        const bool prepared=calls_+3<=cfgbundle::CallCap&&Context()&&DonorNativeOwner(owner)&&bundleKnifeReleaseAdapterEnabled&&KnifeNotifyBindings()&&
            !bundleKnifeReleaseFrame&&bundleRootLease&&!bundleRootLease->NativeEntered()&&GetCurrentThreadId()==plan.threadId&&target&&
            r_.Read(pawn+cfgknife::disk::Count,count)&&count>0&&r_.Read(pawn+cfgknife::disk::Pressed,pressed)&&pressed==1&&
            r_.Read(pawn+cfgknife::disk::Driver,driver)&&driver&&
            KnifeIdentity(pawn,jasonId)&&KnifeIdentity(driver,driverId)&&KnifeIdentity(target,targetId)&&targetId.serial==targetSerial&&
            Alive(target,living)&&living&&DonorNativeAdditionalKnifeDriverProof(pawn,driverId,true)&&
            Location(pawn,j)&&Location(target,t)&&r_.Read(pawn+cfgknife::disk::Speed,speed)&&
            cfgknife::WorldLaunch({j.x,j.y,j.z},{t.x,t.y,t.z},speed,w.geometry)&&
            RetainPrimaryObjects(r_,std::array<std::uintptr_t,3>{pawn,driver,target});
        if(!prepared){flags=256;return DonorNativeAdditionalKnifeCall(owner,cfgknife::disk::Release);}
        w.jason=jasonId;w.driver=driverId;w.target=targetId;w.count=count;w.geometryReady=true;w.armed=true;
        frame.lease=bundleRootLease;frame.access=this;frame.thread=plan.threadId;frame.depth=bundleKnifeReleaseDepth+1;
        frame.returnPc=plan.module+cfgcompat::cfg35::constructBoundary::ReturnRva;
        frame.current=[](void* source,const cfgknife::ReleaseFrame& value){return static_cast<BundleAccess*>(source)->DonorNativeAdditionalKnifeEntryCurrent(value);};
        struct Scope {
            cfgknife::ReleaseFrame* oldFrame;cfgknife::NotifyWindow* oldWindow;
            explicit Scope(cfgknife::ReleaseFrame& value):oldFrame(bundleKnifeReleaseFrame),oldWindow(bundleKnifeNotifyWindow){
                bundleKnifeReleaseFrame=&value;bundleKnifeNotifyWindow=&value.window;
            }
            ~Scope(){bundleKnifeReleaseFrame=oldFrame;bundleKnifeNotifyWindow=oldWindow;}
        } scope(frame);
        const bool released=DonorNativeAdditionalKnifeCall(owner,cfgknife::disk::Release);
        flags=(frame.entries==1?1u:0u)|(frame.returned?2u:0u)|(w.multicast?4u:0u)|(w.launched?8u:0u)|(w.geometryUsed?16u:0u)|(w.stopped?32u:0u)|(frame.ambiguous?64u:0u);
        if(frame.faulted){exception_=true;BundleRetentionFailed();return false;}
        return released;
#endif
    }
    bool DonorNativeAdditionalKnifeRefill(const cfgjasontick::Owner& owner){
        std::int32_t count=0;std::uint8_t pressed=0;std::uintptr_t driver=0;
        if(!DonorNativeAdditionalKnifeRead(owner,count,pressed,driver)||driver||pressed)return false;
        if(count>0)return true;
        if(count<0)return false;
        __try{return InterlockedCompareExchange(reinterpret_cast<volatile LONG*>(owner.actor.pawn.object+cfgknife::disk::Count),6,count)==count;}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;return false;}
    }
    bool DonorNativeAdditionalKnifeLocked(const cfgjasontick::Owner& owner){
        std::uintptr_t manager=0,locked=0;
        if(!DonorNativeOwner(owner)||!r_.Read(owner.actor.pawn.object+cfghunter::disk::Manager,manager)||!manager)return true; // unreadable: refuse
        return !r_.Read(manager+cfgvehicleinteraction::disk::LockedComponent,locked)||locked!=0;
    }
#endif
#if CFG_F1_CAR
    // F1 car natives: every access targets the F1 pawn (owner.actor.pawn), never bundleJason or primary car state.
    bool DonorNativeAdditionalCarScan(const cfgjasontick::Owner& owner,std::uintptr_t& car,std::uintptr_t& seat,std::uintptr_t& driver){
        car=seat=driver=0;cfgspawn::Vec3 jason{};
        if(!DonorNativeOwner(owner)||!Context()||!Location(owner.actor.pawn.object,jason))return false;
        float nearest=0;
        for(unsigned i=0;i<bundleTrapSources.count&&i<bundleTrapSources.entries.size();++i){
            const auto& e=bundleTrapSources.entries[i];
            if((e.kind!=cfgtrap::startup::Kind::Car2&&e.kind!=cfgtrap::startup::Kind::Car4)||!e.actor)continue;
            cfgvehicleoccupancy::Proof proof{};cfgspawn::Vec3 at{};
            if(!CarOccupancy(e.actor,proof)||proof.result!=cfgvehicleoccupancy::Result::Occupied||!proof.driver||!proof.seat||!Location(e.actor,at))continue;
            const float dx=at.x-jason.x,dy=at.y-jason.y,d=dx*dx+dy*dy;
            if(!std::isfinite(d)||(car&&d>=nearest))continue;
            nearest=d;car=e.actor;seat=proof.seat;driver=proof.driver;
        }
        return car!=0;
    }
    bool DonorNativeAdditionalCarSeatCheck(const cfgjasontick::Owner& owner,std::uintptr_t car,std::uintptr_t seat,std::uintptr_t& driver){
        cfgvehicleoccupancy::Proof proof{};driver=0;
        if(!car||!seat||!DonorNativeOwner(owner)||!Context()||!CarOccupancy(car,proof)||proof.seat!=seat||
           proof.result!=cfgvehicleoccupancy::Result::Occupied||!proof.driver)return false;
        driver=proof.driver;return true;
    }
    bool DonorNativeAdditionalCarVelocity(const cfgjasontick::Owner& owner,std::uintptr_t car,cfgspawn::Vec3& velocity){
        std::uintptr_t function=0,functionOwner=0;velocity={};
        return car&&DonorNativeOwner(owner)&&FeatureFunction(cfgvehicle::disk::Velocity,function,functionOwner)&&Invoke(car,function,&velocity)&&
            std::isfinite(velocity.x)&&std::isfinite(velocity.y)&&std::isfinite(velocity.z);
    }
    bool DonorNativeAdditionalCarStarted(std::uintptr_t car,bool& started){
        std::uint8_t value=0;started=false;
        if(!car||!r_.Read(car+cfgaction::disk::CarStarted,value)||value>1)return false;
        started=value!=0;return true;
    }
    bool DonorNativeAdditionalCarHood(std::uintptr_t car,std::uintptr_t& hood){
        cfgprobe::lifetime::SlotObservation slot{};hood=0;std::uintptr_t value=0;
        if(!car||!Context()||!r_.Read(car+cfgvehicleinteraction::disk::JasonCarInteractComponent,value)||!value||
           !cfgspawn::ObjectSlot(r_,context.objectArray,value,slot))return false;
        hood=value;return true;
    }
    bool DonorNativeAdditionalCarComponentAttempt(const cfgjasontick::Owner& owner,std::uintptr_t component,const cfgspawn::Vec3& point,std::uint8_t autoLock){
        std::uintptr_t manager=0;cfgprobe::lifetime::SlotObservation slot{};
        if(!component||!DonorNativeOwner(owner)||!Context()||!r_.Read(owner.actor.pawn.object+cfghunter::disk::Manager,manager)||!manager||
           !cfgspawn::ObjectSlot(r_,context.objectArray,manager,slot)||!cfgspawn::ObjectSlot(r_,context.objectArray,component,slot)||
           !cfgobserver::Same(r_,manager+cfgvehicleinteraction::disk::LockedComponent,std::uintptr_t{})||!DonorNativeAdditionalFace(owner,point))return false;
        cfgvehicleaction::Attempt p;p.component=component;p.autoLock=autoLock;
        return CarCall(cfgvehicleinteraction::disk::Attempt,cfgvehicleinteraction::disk::Executor,cfgvehicleinteraction::disk::ExecutorCode,manager,&p);
    }
    bool DonorNativeAdditionalCarHoodInteract(const cfgjasontick::Owner& owner,std::uintptr_t hood,const cfgspawn::Vec3& carPoint){
        return DonorNativeAdditionalCarComponentAttempt(owner,hood,carPoint,0);
    }
    bool DonorNativeAdditionalCarExtraction(const cfgjasontick::Owner& owner,std::uintptr_t seat,const cfgspawn::Vec3& seatPoint){
        return DonorNativeAdditionalCarComponentAttempt(owner,seat,seatPoint,1);
    }
#endif
    bool DonorNativeAdditionalHidingAttempt(const cfgjasontick::Owner& owner,std::uintptr_t component,const cfgspawn::Vec3& point){
        std::uintptr_t manager=0;cfgprobe::lifetime::SlotObservation slot{};
        if(!DonorNativeOwner(owner)||!Context()||!r_.Read(owner.actor.pawn.object+cfghunter::disk::Manager,manager)||!manager||
           !cfgspawn::ObjectSlot(r_,context.objectArray,manager,slot)||
           !cfgobserver::Same(r_,manager+cfgvehicleinteraction::disk::LockedComponent,std::uintptr_t{})||!DonorNativeAdditionalFace(owner,point))return false;
        cfgvehicleaction::Attempt p;p.component=component;p.autoLock=1;
        return CarCall(cfgvehicleinteraction::disk::Attempt,cfgvehicleinteraction::disk::Executor,cfgvehicleinteraction::disk::ExecutorCode,manager,&p);
    }
#endif
#endif
