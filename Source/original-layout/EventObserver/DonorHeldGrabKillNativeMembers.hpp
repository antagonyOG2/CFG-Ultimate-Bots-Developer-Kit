// Included in BundleAccess. The same current native-Tick receiver serves both
// Jasons; none of these gateways swaps the primary singleton to F1.
#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
    static_assert(cfgdonorgrab::Slots==cfgheld35::Slots,"Revalidate held policy capacity against the selected CFG binding");
    using HeldOutcome=cfgdonorgrab::Outcome;
    cfgj21c::HeldDiagnostics* heldDiagnostics_=nullptr;
    bool heldObserveUncertain_=false;
    bool DonorHeldDrainStop(const cfgjasontick::Owner& owner,DonorNativeJasonRuntime& driver,std::uint64_t now){
        if(!driver.heldStopPending)return false;
        if(!(driver.heldStopOwner.source.objects[0]==owner.source.objects[0])||
            !(driver.heldStopOwner.actor.pawn==owner.actor.pawn)||!(driver.heldStopOwner.actor.killerController==owner.actor.killerController)){
            driver.heldStopPending=false;driver.heldStopOwner={};return false; // retire receipt; never stop a replacement birth
        }
        auto outcome=cfgdonorhunt::NativeOutcome::NotEntered;DonorNativeStopMovement(owner,outcome);
        if(outcome==cfgdonorhunt::NativeOutcome::NotEntered)return true;
        driver.heldStopPending=false;driver.heldStopOwner={};
        if(outcome==cfgdonorhunt::NativeOutcome::Uncertain)driver.MarkUncertain(cfgdonorhunt::UncertainAction::Stop,now);
        return outcome==cfgdonorhunt::NativeOutcome::Uncertain;
    }
    void DonorHeldLogSlots(const cfgjasontick::Owner&,const cfgdonorgrab::Hold& hold){
        auto& row=bundleReport.j2Held[hold.jason==bundleJason.pawn.object?0u:1u];row={};row.jason=hold.jason;row.victim=hold.victim;
        Reader log;log.cap=4096;
        for(unsigned i=0;i<cfgheld35::Slots;++i){std::uintptr_t actor=0,type=0;
            if(log.Read(hold.jason+cfgheld35::GrabActors+8*i,actor)&&actor&&log.Read(actor+cfgprobe::ObjectClass,type)){
                row.actors[i]=actor;row.classes[i]=type;cfgobjectpath::Observe(log,plan.module,type,row.classPaths[i]);
                cfgclosedtrace::Mark("j2-grab-slot",actor,(static_cast<std::uint64_t>(i)<<32));
                cfgclosedtrace::Mark("j2-grab-class",type,i);
            }
        }
        std::uintptr_t state=0,playerStateClass=0;
        struct List {std::uintptr_t data=0;std::int32_t count=0,capacity=0;} picked{},again{};
        if(log.Read(hold.jason+cfgescape::disk::PlayerState,state)&&state&&
            log.Read(plan.module+cfgescape::disk::SCPlayerStateReflection,playerStateClass)&&
            cfgobserver::Same(log,plan.module+cfgescape::disk::SCPlayerStateConstructor,playerStateClass)&&cfgspawn::ObjectIs(log,state,playerStateClass)&&
            log.Read(state+cfgheld35::PickedGrabKills,picked)&&picked.count>=0&&picked.count<=64&&picked.capacity>=picked.count&&picked.capacity<=64&&
            (!picked.count||cfgprobe::ObjectPointer(picked.data))){
            row.pickedCount=static_cast<unsigned>(picked.count);
            for(unsigned i=0;i<row.pickedCount&&i<7;++i)if(log.Read(picked.data+8*i,row.picked[i])&&row.picked[i])
                cfgobjectpath::Observe(log,plan.module,row.picked[i],row.pickedPaths[i]);
            if(!log.Read(state+cfgheld35::PickedGrabKills,again)||again.data!=picked.data||again.count!=picked.count||again.capacity!=picked.capacity){
                row.pickedCount=0;row.picked={};row.pickedPaths={};
            }
        }
    }
    static HeldOutcome HeldResult(cfgdonorhunt::NativeOutcome result){
        return result==cfgdonorhunt::NativeOutcome::NotEntered?HeldOutcome::NotEntered:
            result==cfgdonorhunt::NativeOutcome::Uncertain?HeldOutcome::Uncertain:HeldOutcome::Returned;
    }
    bool DonorHeldCurrent(const cfgjasontick::Owner& owner,const cfgdonorgrab::Hold& hold){
        cfgprobe::lifetime::ArrayHeader current{};cfgprobe::lifetime::SlotObservation victim{},level{};std::uintptr_t levelType=0;
        return hold.Valid()&&hold.world==plan.world&&hold.jason==owner.actor.pawn.object&&
            hold.jasonSerial==owner.actor.pawn.serial&&hold.jasonIndex==owner.actor.pawn.index&&DonorNativeOwner(owner)&&
            cfgprobe::lifetime::ReadArrayHeader(r_,plan.module+cfgprobe::lifetime::ObjectArrayRva,current)&&
            cfgspawn::ObjectSlot(r_,current,hold.victim,victim)&&victim.HasAssignedSerial()&&victim.serial==hold.victimSerial&&victim.index==hold.victimIndex&&
            cfgkill::ActorWorldCurrent(r_,current,plan.module,hold.victim,plan.world,level,levelType)&&
            cfgspawn::ObjectIs(r_,hold.victim,bundleCounselorClass)&&
            cfgobserver::Same(r_,hold.jason+cfghunter::disk::Held,hold.victim);
    }
    template<class Binding,std::size_t N> HeldOutcome DonorHeldQueryCurrent(const cfgjasontick::Owner& owner,const cfgdonorgrab::Hold& hold,
        const Binding& binding,std::uintptr_t executor,const std::array<unsigned char,N>& code,void* packet,bool holdMayChange=false){
        if(heldDiagnostics_)heldDiagnostics_->queryStage=1;
        std::uintptr_t function=0,type=0;
        if(!DonorHeldCurrent(owner,hold)||!FeatureFunction(binding,function,type)||!cfgspawn::ObjectIs(r_,hold.jason,type)||
            !cfgobserver::Same(r_,function+0xB0,plan.module+executor)||!cfgbundle::Code(r_,plan.module,executor,code))return HeldOutcome::NotEntered;
        if(heldDiagnostics_)heldDiagnostics_->queryStage=2;
        if(!RetainPrimaryObjects(r_,std::array<std::uintptr_t,2>{hold.victim,owner.actor.killerController.object}))return HeldOutcome::NotEntered;
        if(heldDiagnostics_)heldDiagnostics_->queryStage=3;
        bundleInvocationStage_=0;const auto queryStart=J21Micros();const bool returned=Invoke(hold.jason,function,packet);
        J21CostAdd(bundleReport.h291HeldQuery,queryStart); // H2.91 timing only
        if(!returned)return bundleInvocationStage_>=5?HeldOutcome::Uncertain:HeldOutcome::NotEntered;
        return (holdMayChange?DonorNativeOwner(owner):DonorHeldCurrent(owner,hold))?HeldOutcome::Returned:HeldOutcome::Uncertain;
    }
    template<class Binding,std::size_t N> HeldOutcome DonorHeldQuery(const cfgjasontick::Owner& owner,const cfgdonorgrab::Hold& hold,
        const Binding& binding,std::uintptr_t executor,const std::array<unsigned char,N>& code,void* packet,bool holdMayChange=false){
        if(!CFG_J21C_HELD_RECOVERY)return DonorHeldQueryCurrent(owner,hold,binding,executor,code,packet,holdMayChange);
        // Fresh bounded observation, SAME root lease/callback and shared call debt.
        // A spent class/flag query reader must not make every slot0 permanently unreachable.
        BundleAccess query(cfgbundle::ReadCap,calls_);query.heldDiagnostics_=heldDiagnostics_;
        const auto result=query.DonorHeldQueryCurrent(owner,hold,binding,executor,code,packet,holdMayChange);
        calls_+=query.Calls();exception_=exception_||query.Exception();
        bundleReport.reads+=query.Reads();bundleReport.maxReads=(std::max)(bundleReport.maxReads,query.Reads());
        if(heldDiagnostics_){heldDiagnostics_->queryReads+=query.Reads();
            heldDiagnostics_->readBudget+=query.Reads()>=cfgbundle::ReadCap;heldDiagnostics_->callBudget+=calls_>=cfgbundle::CallCap;
            if(result==HeldOutcome::NotEntered)++heldDiagnostics_->notEntered[heldDiagnostics_->queryStage];}
        heldObserveUncertain_=result==HeldOutcome::Uncertain;return result;
    }
    HeldOutcome DonorHeldSlot(const cfgjasontick::Owner& owner,const cfgdonorgrab::Hold& hold,unsigned slot,bool& available){
        available=false;if(slot>=cfgheld35::Slots||!cfgbundle::Code(r_,plan.module,cfgheld35::AvailabilityBound,cfgheld35::AvailabilityBoundCode))return HeldOutcome::NotEntered;
        struct Packet{std::int32_t slot;std::uint8_t result=0,padding[3]{};} packet{static_cast<std::int32_t>(slot)};
        static_assert(sizeof(Packet)==8&&offsetof(Packet,result)==4);
        const auto result=DonorHeldQuery(owner,hold,cfggrab::disk::IsGrabKillAvailable,cfggrab::disk::IsGrabKillAvailableExecutor,
            cfggrab::disk::IsGrabKillAvailableCode,&packet);
        if(result!=HeldOutcome::Returned)return result;
        if(packet.result>1)return HeldOutcome::Uncertain;available=packet.result!=0;return result;
    }
    // H2.47: CurrentSeat is a SEAT only when it is a proven SCVehicleSeatComponent of a vehicle that lists this
    // counselor (reciprocal CounselorInSeat, or LeavingSeat during an extraction pull). Any other nonzero value
    // (non-vehicle or stale pointer) is not a car seat and must not hold an ordinary grab kill. Returns false only
    // when CurrentSeat itself is unreadable. A failed proof keeps the old conservative behaviour (blocks).
    bool DonorHeldSeatState(std::uintptr_t jason,std::uintptr_t victim,bool& blocks){
        blocks=false;std::uintptr_t seat=0;
        if(!r_.Read(victim+cfgvehicleoccupancy::disk::CurrentSeat,seat))return false;
        if(!seat)return true;
        blocks=true;
#if CFG_H247_SEAT_WAIT
        auto& row=bundleReport.h247Held[jason==bundleJason.pawn.object?0u:1u];
        std::uintptr_t type=0,parent=0,occupant=0;std::uint8_t leaving=0;
        if(!NativeClass(cfgvehicleinteraction::disk::SCVehicleSeatComponent,cfgvehicleinteraction::disk::SCVehicleSeatComponentConstructor,type)){
            ++row.seatProofFailures;return true;}
        if(!cfgspawn::ObjectIs(r_,seat,type)){++row.staleSeatIgnored;blocks=false;return true;}
        if(!r_.Read(seat+cfgvehicleinteraction::disk::ParentVehicle,parent)||!r_.Read(seat+cfgvehicleinteraction::disk::CounselorInSeat,occupant)||
           !r_.Read(victim+cfgvehicleoccupancy::disk::LeavingSeat,leaving)){++row.seatProofFailures;return true;}
        if(!parent||(occupant!=victim&&!leaving)){++row.staleSeatIgnored;blocks=false;}
#else
        (void)jason;
#endif
        return true;
    }
    bool DonorHeldObserve(const cfgjasontick::Owner& owner,cfgdonorgrab::Observation& out){
        heldObserveUncertain_=false;if(heldDiagnostics_)heldDiagnostics_->observeStage=1;
        out={};std::uintptr_t victim=0;bool seatBlocks=false;cfgprobe::lifetime::SlotObservation slot{};
        if(!DonorNativeOwner(owner)||!r_.Read(owner.actor.pawn.object+cfghunter::disk::Held,victim))return false;
        if(!victim)return true;
        if(heldDiagnostics_)heldDiagnostics_->observeStage=2;
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,victim,slot)||!slot.HasAssignedSerial()||
            !cfgspawn::ObjectIs(r_,victim,bundleCounselorClass))return false;
        out.hold={plan.world,owner.actor.pawn.object,victim,owner.actor.pawn.serial,slot.serial,owner.actor.pawn.index,slot.index};
        if(heldDiagnostics_)heldDiagnostics_->observeStage=3;
        cfgescape::State state{};float health=0;std::uint8_t struggling=0;std::uintptr_t water=0,victimWater=0;bool extracting=false;
        if(!cfgescape::Counselor(r_,context.objectArray,plan.module,victim,bundleCounselorClass,state)||state==cfgescape::State::Unknown||
            !r_.Read(victim+cfghunter::disk::Health,health)||!std::isfinite(health)||
            !DonorHeldSeatState(owner.actor.pawn.object,victim,seatBlocks)||
            !DonorHeldExtractionActive(out.hold,extracting)||
            !cfgbundle::Code(r_,plan.module,cfgheld35::StrugglingSetter,cfgheld35::StrugglingSetterCode)||
            !r_.Read(victim+cfgheld35::Struggling,struggling)||struggling>1||
            !r_.Read(victim+cfgswim::disk::WaterVolume,victimWater)||!r_.Read(out.hold.jason+cfgswim::disk::WaterVolume,water))return false;
        out.water=water||victimWater;
        out.victimDead=health<=0;out.dead=out.victimDead||cfgescape::Terminal(state);out.seat=seatBlocks;out.extracting=extracting;out.struggling=struggling!=0;
        out.finalContext=DonorNativeDeathExclusive(GetTickCount64());
        if(heldDiagnostics_)heldDiagnostics_->observeStage=4;
        cfgspawn::Vec3 point{};if(!DonorNativePosition(owner.actor.pawn,point))return false;
        out.location={point.x,point.y,point.z};
        if(out.dead||out.seat||out.extracting||out.finalContext){
            if(heldDiagnostics_)heldDiagnostics_->observeStage=7;return DonorHeldCurrent(owner,out.hold);}
        if(heldDiagnostics_)heldDiagnostics_->observeStage=5;
        std::uint8_t killing=0,allowed=0;
        if(DonorHeldQuery(owner,out.hold,cfggrab::disk::IsGrabKilling,cfggrab::disk::IsGrabKillingExecutor,
                cfggrab::disk::IsGrabKillingCode,&killing)!=HeldOutcome::Returned||killing>1)return false;
        out.killing=killing!=0;if(out.killing)return true;
        if(heldDiagnostics_)heldDiagnostics_->observeStage=6;
        if(DonorHeldQuery(owner,out.hold,cfggrab::disk::CanGrabKill,cfggrab::disk::CanGrabKillExecutor,
                cfggrab::disk::CanGrabKillCode,&allowed)!=HeldOutcome::Returned||allowed>1)return false;
        out.allowed=allowed!=0;if(heldDiagnostics_)heldDiagnostics_->observeStage=0;return true;
    }
    bool DonorHeldExtractionActive(const cfgdonorgrab::Hold& hold,bool& active){
        active=false;std::uintptr_t component=0,victim=0;
        if(!r_.Read(hold.jason+cfgaction::disk::ExtractionComponent,component)||
           !r_.Read(hold.jason+cfgaction::disk::ExtractionVictim,victim)||
           !cfgobserver::Same(r_,hold.jason+cfgaction::disk::ExtractionComponent,component)||
           !cfgobserver::Same(r_,hold.jason+cfgaction::disk::ExtractionVictim,victim))return false;
        // A resident ContextKillComponent is not itself evidence that a car
        // extraction still owns this hold. The donor waits on the matching
        // seated/held counselor; validate the native component only when its
        // recorded victim is this exact held counselor.
        if(!component||victim!=hold.victim)return true;
        return CarExtraction(active)&&active;
    }
    HeldOutcome DonorHeldInput(const cfgjasontick::Owner& owner,const cfgdonorgrab::Hold& hold,unsigned slot)noexcept{
        if(heldDiagnostics_)heldDiagnostics_->inputStage=1;
        bool available=false;
        const auto checked=DonorHeldSlot(owner,hold,slot,available);if(checked!=HeldOutcome::Returned)return checked;
        std::uint8_t allowed=0;
        if(heldDiagnostics_)heldDiagnostics_->inputStage=2;
        const auto can=DonorHeldQuery(owner,hold,cfggrab::disk::CanGrabKill,cfggrab::disk::CanGrabKillExecutor,cfggrab::disk::CanGrabKillCode,&allowed);
        if(can!=HeldOutcome::Returned)return can;
        if(allowed>1)return HeldOutcome::Uncertain;if(!allowed)return HeldOutcome::NotEntered;
        bool seatBlocks=false,extracting=false;
        if(heldDiagnostics_)heldDiagnostics_->inputStage=3;
        if(!available||!DonorHeldCurrent(owner,hold)||!DonorHeldSeatState(hold.jason,hold.victim,seatBlocks)||seatBlocks||
            !DonorHeldExtractionActive(hold,extracting)||extracting||
            !cfgbundle::Code(r_,plan.module,cfggrab::disk::GrabKillInput,cfggrab::disk::GrabKillInputCode))return HeldOutcome::NotEntered;
        if(heldDiagnostics_)heldDiagnostics_->inputStage=4;
        if(!NativeCallAllowed())return HeldOutcome::NotEntered;
        if(heldDiagnostics_)heldDiagnostics_->inputStage=5;
        if(!RetainBundleActors(hold.jason)||!RetainPrimaryObjects(r_,std::array<std::uintptr_t,3>{hold.victim,
            owner.actor.killerController.object,owner.actor.path.object}))return HeldOutcome::NotEntered;
        if(heldDiagnostics_)heldDiagnostics_->inputStage=6;
        if(!NativeCallAllowed()||!BeginBundleNative())return HeldOutcome::NotEntered;
        ++calls_;const auto inputStart=J21Micros();
        __try{reinterpret_cast<void(__fastcall*)(void*,std::int32_t)>(plan.module+cfggrab::disk::GrabKillInput)(
            reinterpret_cast<void*>(hold.jason),static_cast<std::int32_t>(slot));}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;BundleRetentionFailed();return HeldOutcome::Uncertain;}
        J21CostAdd(bundleReport.h291HeldInput,inputStart); // H2.91 timing only: the native call itself
        return FinishBundleNative()?HeldOutcome::Returned:HeldOutcome::Uncertain;
    }
    bool DonorHeldDrive(const cfgjasontick::Owner& owner,cfgdonorgrab::State& state,DonorNativeJasonRuntime& driver,std::uint64_t now){
        heldDiagnostics_=&state.diagnostics;
        // Returned native Tick/session already established the owner scope. An
        // absent hold is only a no-work hint, never native-call admission.
        std::uintptr_t hint=0;
        if(state.stage==cfgdonorgrab::Stage::Idle&&!driver.heldStopPending&&
            r_.Read(owner.actor.pawn.object+cfghunter::disk::Held,hint)&&!hint)return false;
        // No expensive seven-slot/native observation before the literal donor
        // deadline. Loss/replacement of Held still retires motion immediately.
        if(state.stage!=cfgdonorgrab::Stage::Idle&&!driver.heldStopPending&&bundleNativeJasonWaterKnown&&
           !bundleNativeJasonWater&&!DonorNativeDeathExclusive(now)&&!state.CanFallback(now)&&now<state.nextProbeAt&&
           r_.Read(owner.actor.pawn.object+cfghunter::disk::Held,hint)&&hint==state.hold.victim)return true;
        struct Host {
            BundleAccess& a;const cfgjasontick::Owner& owner;DonorNativeJasonRuntime& driver;cfgdonorgrab::State& motion;bool offlineF1=false,randomValid=true;
            bool HeldKillAllowed()const{return !offlineF1;} // H2.82
            bool Observe(cfgdonorgrab::Observation& out){const bool result=a.DonorHeldObserve(owner,out);
                if(!result){++motion.diagnostics.observeFailures[(std::min)(motion.diagnostics.observeStage,15u)];
                    if(a.heldObserveUncertain_){motion.stage=cfgdonorgrab::Stage::Uncertain;++motion.uncertain;}}
                return result;}
            void HoldReleased(const cfgdonorgrab::Hold& old,std::uint64_t at){
                if(motion.motionOwned){driver.heldStopPending=true;driver.heldStopOwner=owner;a.DonorHeldDrainStop(owner,driver,at);}
                bundleDonorPostGrab[old.jason==bundleJason.pawn.object?0u:1u]={old,at+2500};
                const auto index=bundleDonorRoster.roster.DonorFindJasonAITargetIndex(old.victim);
                if(index>=0){auto& until=bundleDonorRoster.roster.blockedUntil[static_cast<unsigned>(index)];until=(std::max)(until,at+2500);}
                driver.frame.Invalidate();driver.navigation={};cfgclosedtrace::Mark("j2-hold-released",old.jason,old.victim);
#if CFG_H283_F1_KILL_DIAG
                if(motion.committedAt){auto& k=bundleReport.h283Kill[old.jason==bundleJason.pawn.object?0u:1u];Reader log;log.cap=4;float health=0;std::uint8_t context=255;
                    if(log.Read(old.victim+cfgdonorgrab::h283::Health,health)){k.releaseHealth=health;++(health>0?k.releasedAlive:k.releasedDead);
                        cfgclosedtrace::Mark(health>0?"h283-held-release-alive":"h283-held-release-dead",old.jason,old.victim);}
                    if(log.Read(old.victim+cfgdonorgrab::h283::ContextKill,context))k.releaseContext=context;}
#endif
            }
            void ClearSpeculativeCombatTimer(){if(!CFG_J21C_HELD_RECOVERY)driver.combat.GrabKillReadyAt=0;driver.frame.Invalidate();driver.navigation={};}
            void LogSlots(const cfgdonorgrab::Hold& hold){a.DonorHeldLogSlots(owner,hold);}
            void PreserveExtraction(const cfgdonorgrab::Observation&,std::uint64_t){} // accepted seat transaction remains owner
            HeldOutcome ProbeSlot(const cfgdonorgrab::Hold& h,unsigned i,bool& value){return a.DonorHeldSlot(owner,h,i,value);}
            HeldOutcome Kill(const cfgdonorgrab::Hold& h,unsigned i){
                if(!randomValid){++motion.diagnostics.notEntered[15];return HeldOutcome::NotEntered;}
                // Complete-mask observation can exhaust its read slice. The
                // one input gets fresh bounded proofs in the SAME callback/root
                // capability, inheriting all calls already spent this turn.
                BundleAccess input(cfgbundle::ReadCap,a.calls_);
                input.heldDiagnostics_=&motion.diagnostics;
                const auto result=input.DonorHeldInput(owner,h,i);a.calls_+=input.Calls();a.exception_=a.exception_||input.Exception();
                if(result==HeldOutcome::NotEntered)++motion.diagnostics.notEntered[8+motion.diagnostics.inputStage];
                bundleReport.reads+=input.Reads();bundleReport.maxReads=(std::max)(bundleReport.maxReads,input.Reads());return result;
            }
            HeldOutcome StopHolding(const cfgdonorgrab::Hold& h){
                if(!a.DonorHeldCurrent(owner,h))return HeldOutcome::NotEntered;
                auto result=cfgdonorhunt::NativeOutcome::NotEntered;a.DonorNativeStopMovement(owner,result);return HeldResult(result);
            }
            HeldOutcome ReleaseStale(const cfgdonorgrab::Hold& h,std::uint64_t){
                ++motion.diagnostics.staleReleases;
                std::uintptr_t extracting=0,manager=0,type=0,lock=0;bool hiding=false,seatBlocks=false;
                if(!a.DonorHeldCurrent(owner,h)||!a.DonorHeldSeatState(h.jason,h.victim,seatBlocks)||seatBlocks||
                    !a.r_.Read(h.jason+cfgaction::disk::ExtractionComponent,extracting)||extracting||
                    !a.DonorNativeLocked(owner,hiding)||hiding||
                    !a.r_.Read(h.jason+cfghunter::disk::Manager,manager)||
                    !a.NativeClass(cfghunter::disk::ManagerReflection,cfghunter::disk::ManagerConstructor,type)||
                    !cfgspawn::ObjectIs(a.r_,manager,type)||!cfgobserver::Same(a.r_,manager+0x20,h.jason)||
                    !a.r_.Read(manager+cfgvehicleinteraction::disk::LockedComponent,lock)||lock)return HeldOutcome::NotEntered;
                return a.DonorHeldQuery(owner,h,cfgaction::disk::CLIENT_CancelInteractAttempt,
                    cfgaction::disk::CLIENT_CancelInteractAttemptExecutor,cfgaction::disk::CLIENT_CancelInteractAttemptCode,nullptr,true);
            }
            HeldOutcome ProbeOpenDirection(const cfgdonorgrab::Hold& h,const cfgdonorgrab::Position& origin,
                unsigned direction,float distance,float& free,cfgdonorgrab::Position& point,bool& safe){
                safe=false;free=0;std::uintptr_t water=0,heldWater=0;
                if(direction>=8||!a.DonorHeldCurrent(owner,h)||!a.r_.Read(h.jason+cfgswim::disk::WaterVolume,water)||water||
                    !a.r_.Read(h.victim+cfgswim::disk::WaterVolume,heldWater)||heldWater)return HeldOutcome::NotEntered;
                constexpr float diagonal=0.70710678118f;
                constexpr float dx[]{1,diagonal,0,-diagonal,-1,-diagonal,0,diagonal};
                constexpr float dy[]{0,diagonal,1,diagonal,0,-diagonal,-1,-diagonal};
                cfgspawn::Vec3 target{origin.X+dx[direction]*distance,origin.Y+dy[direction]*distance,origin.Z},projected{};
                auto result=cfgdonorhunt::NativeOutcome::NotEntered;
                const bool ok=a.DonorNativeProject(owner,target,{80,80,120},projected,&result);
                if(result==cfgdonorhunt::NativeOutcome::NotEntered||result==cfgdonorhunt::NativeOutcome::Uncertain)return HeldResult(result);
                if(!ok)return HeldOutcome::Returned;
                const float x=projected.x-target.x,y=projected.y-target.y,z=projected.z-origin.Z;
                const float shortfall=std::sqrt(x*x+y*y);
                safe=std::isfinite(shortfall)&&shortfall<=100&&std::fabs(z)<=120;
                free=safe?(std::max)(0.f,distance-shortfall):0.f;point={projected.x,projected.y,projected.z};return HeldOutcome::Returned;
            }
            HeldOutcome MoveHolding(const cfgdonorgrab::Hold& h,const cfgdonorgrab::Position& point){
                if(!a.DonorHeldCurrent(owner,h))return HeldOutcome::NotEntered;
                auto result=cfgdonorhunt::NativeOutcome::NotEntered;unsigned status=255;
                const bool returned=a.DonorNativeMoveLocation(owner,{point.X,point.Y,point.Z},status,40,&result,true);
                if(returned&&!status)return HeldOutcome::NotEntered;return HeldResult(result);
            }
            std::uint32_t Random(){
                std::uint32_t value=0;
                if(BCryptGenRandom(nullptr,reinterpret_cast<PUCHAR>(&value),sizeof(value),BCRYPT_USE_SYSTEM_PREFERRED_RNG)<0)randomValid=false;
                return value; // OS RNG failure defers the input, never guesses a slot
            }
            void LogAttempt(const cfgdonorgrab::Hold& h,unsigned mask,unsigned slot,HeldOutcome result){
                cfgclosedtrace::Mark("j2-held-kill",h.jason,(static_cast<std::uint64_t>(mask)<<32)|(slot<<8)|static_cast<unsigned>(result));
            }
            void CombatDelayed(std::uintptr_t,std::uint64_t until){driver.combat.NextCombatAttemptAt=until;driver.additional.NextCombatAt=until;}
        } host{*this,owner,driver,state,CFG_H282_F1_OFFLINE_NO_HELD_KILL&&!(owner.actor.pawn==bundleJason.pawn)&&!HostedRoutineActive()&&!(CFG_H283_F1_KILLER_CLASS&&bundleAdditional.killerClass)};
        const bool owned=state.Drive(host,now);
#if CFG_H283_F1_KILL_DIAG
        static_assert(cfgdonorgrab::h283::GrabActors==cfgheld35::GrabActors&&cfgdonorgrab::h283::KillComponent==cfgkill::disk::ContextKillComponent&&
            cfgdonorgrab::h283::Health==cfghunter::disk::Health,"H2.83 gate reads use the pinned CFG 3.5 bindings");
        {auto& k=bundleReport.h283Kill[owner.actor.pawn==bundleJason.pawn?0u:1u]; // once per committed hold, after the native kill has bound its component
         if(state.stage==cfgdonorgrab::Stage::Committed&&state.committedAt&&now>=state.committedAt+1500&&k.snapCommit!=state.committedAt&&state.lastUsed>=0){
            k.snapCommit=state.committedAt;++k.snapshots;Reader log;log.cap=192;
            cfgdonorgrab::h283::ObserveKillGates(log,state.hold.jason,state.hold.victim,static_cast<unsigned>(state.lastUsed),k);
            cfgclosedtrace::Mark("h283-kill-gates",state.hold.jason,(static_cast<std::uint64_t>(k.ownerRole&0xFF)<<24)|((k.moveKillFlag&0xFF)<<16)|((k.compVictim&0xFF)<<8)|(k.clause&0xFF));}}
#endif
        auto& heldRow=bundleReport.j2Held[owner.actor.pawn==bundleJason.pawn?0u:1u];
        heldRow.jason=owner.actor.pawn.object;heldRow.victim=state.hold.victim;heldRow.stage=static_cast<unsigned>(state.stage);
#ifdef CFG_H1_JASON
        auto& reached=bundleReport.j2HeldStageReached[owner.actor.pawn==bundleJason.pawn?0u:1u];
        if(HostedRoutineActive()){
            if(state.stage==cfgdonorgrab::Stage::Probing)++reached[0];
            else if(state.stage==cfgdonorgrab::Stage::Scanning)++reached[1];
            else if(state.stage==cfgdonorgrab::Stage::Walking)++reached[2];
            else if(state.stage==cfgdonorgrab::Stage::Committed)++reached[3];
        }
#endif
        heldRow.queried=state.queriedMask;heldRow.available=state.availableMask;heldRow.before=state.beforeMask;heldRow.after=state.afterMask;
        heldRow.slot=state.lastUsed<0?7u:static_cast<unsigned>(state.lastUsed);heldRow.scans=state.scanCount;heldRow.travelled=state.travelled;
        heldRow.attempts=state.attempts;heldRow.returns=state.returns;heldRow.uncertain=state.uncertain;heldRow.probes=state.probes;heldRow.holds=state.holds;
        bundleReport.j21cHeld[owner.actor.pawn==bundleJason.pawn?0u:1u]=state.diagnostics;
        {auto& split=bundleReport.h247Held[owner.actor.pawn==bundleJason.pawn?0u:1u];
         split.seatWaitsSeat=state.seatWaitsSeat;split.seatWaitsExtracting=state.seatWaitsExtracting;split.scanSkippedStruggling=state.scanSkippedStruggling;
         split.commitGuardWaits=state.commitGuardWaits;split.postCommitReleases=state.postCommitReleases;split.postCommitKills=state.postCommitKills;split.offlineF1Releases=state.offlineF1Releases;}
        bundleReport.grabFinished[owner.actor.pawn==bundleJason.pawn?0u:1u]=state.grabFinishedCount;
        if(CFG_J23_GRAB_TIMING)bundleReport.j23GrabTiming[owner.actor.pawn==bundleJason.pawn?0u:1u]=state.timing.report;
        return owned;
    }
#endif
