#ifndef CFG_VEHICLE_INTERACTION_MEMBER_FIXTURE_ONLY
    std::array<cfgvehicle::RosterProof,2> carProof_{}; // BundleAccess is callback-local.
    bool CarOccupancy(std::uintptr_t car,cfgvehicleoccupancy::Proof& proof){
        std::uintptr_t seatClass=0;
        return NativeClass(cfgvehicleinteraction::disk::SCVehicleSeatComponent,
            cfgvehicleinteraction::disk::SCVehicleSeatComponentConstructor,seatClass)&&
            cfgvehicleoccupancy::Observe(r_,context.objectArray,car,seatClass,bundleCounselorClass,proof)!=cfgvehicleoccupancy::Result::Unknown;
    }
    bool CarOccupancyCurrent(const cfgvehicleoccupancy::Proof& proof){
        std::uintptr_t seatClass=0;
        return NativeClass(cfgvehicleinteraction::disk::SCVehicleSeatComponent,
            cfgvehicleinteraction::disk::SCVehicleSeatComponentConstructor,seatClass)&&
            cfgvehicleoccupancy::Current(r_,context.objectArray,seatClass,bundleCounselorClass,proof);
    }
    bool CarExtraction(bool& active){
        active=false;using namespace cfgaction::disk;
        std::uintptr_t component=0,victim=0,type=0,componentVictim=0;
        if(!r_.Read(bundleJason.pawn.object+ExtractionComponent,component)||
           !r_.Read(bundleJason.pawn.object+ExtractionVictim,victim))return false;
        if(!component)return cfgobserver::Same(r_,bundleJason.pawn.object+ExtractionComponent,std::uintptr_t{});
        cfgprobe::lifetime::SlotObservation slot{},victimSlot{};
        // A native extraction may make its victim terminal before clearing
        // the component. Preserve that native owner through completion, while
        // refusing a retired/reused victim rather than requiring living=true.
        if(!bundleVehicleActions.driver||victim!=bundleVehicleActions.driver||bundleVehicleActions.driverSerial<=0||
           !cfgspawn::ObjectSlot(r_,context.objectArray,victim,victimSlot)||victimSlot.serial!=bundleVehicleActions.driverSerial||
           !cfgspawn::ObjectIs(r_,victim,bundleCounselorClass)||
           !NativeClass(cfgkill::disk::SCContextKillComponentReflection,cfgkill::disk::SCContextKillComponentConstructor,type)||
           !cfgspawn::ObjectIs(r_,component,type)||!cfgspawn::ObjectSlot(r_,context.objectArray,component,slot)||!slot.HasAssignedSerial()||
           !r_.Read(component+cfgkill::disk::Victim,componentVictim)||(componentVictim&&componentVictim!=victim)||
           !cfgjason::SameSlot(r_,context.objectArray,slot)||!cfgjason::SameSlot(r_,context.objectArray,victimSlot)||
           !cfgobserver::Same(r_,bundleJason.pawn.object+ExtractionComponent,component)||
           !cfgobserver::Same(r_,bundleJason.pawn.object+ExtractionVictim,victim))return false;
        active=true;
        bundleVehicleActions.ObserveExtractionActive(component,slot.serial,victim,victimSlot.serial);
        return true; // ContextKillComponent is NOT ILLInteractable; no invented car-Outer relationship
    }
    void CarExtractionFinishObserved(){
        auto& state=bundleVehicleActions;
        if(!state.extractionActiveObserved||!state.nativeExtractionVictim||state.nativeExtractionVictimSerial<=0)return;
        cfgprobe::lifetime::SlotObservation victim{};std::uintptr_t seat=0;float health=0;cfgescape::State counselor=cfgescape::State::Unknown;
        const auto target=state.nativeExtractionVictim;
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,target,victim)||!victim.HasAssignedSerial()||
           victim.serial!=state.nativeExtractionVictimSerial||!cfgspawn::ObjectIs(r_,target,bundleCounselorClass)||
           !r_.Read(target+cfgvehicleoccupancy::disk::CurrentSeat,seat)||
           !r_.Read(target+cfghunter::disk::Health,health)||!std::isfinite(health)||
           !cfgescape::Counselor(r_,context.objectArray,plan.module,target,bundleCounselorClass,counselor)||
           counselor==cfgescape::State::Unknown||!cfgjason::SameSlot(r_,context.objectArray,victim))return;
        if(!seat&&(health<=0||cfgescape::Terminal(counselor)))state.RecordExtractionCompleted(true);
        state.EndExtractionObservation();
    }
    bool CarNativeDrain(std::uint64_t now,bool& active){
        active=false;if(!bundleVehicleActions.car)return true;
        // Read-only sentinel, not permission to dispatch. The common no-native-
        // extraction path continues to the existing full Context/Jason guards
        // without duplicating two cold ownership walks in the same budget.
        std::uintptr_t component=0;
        if(!r_.Read(bundleJason.pawn.object+cfgaction::disk::ExtractionComponent,component))return false;
        if(!component)return cfgobserver::Same(r_,bundleJason.pawn.object+cfgaction::disk::ExtractionComponent,std::uintptr_t{});
        if(!Context()||!Jason()||!CarExtraction(active))return false;
        if(active)bundleCarPursuit.Renew(now);
        return true; // no boost, Morph, input, path or synthetic completion callback
    }
    bool CarStopObservation(std::uintptr_t car,std::uintptr_t hood,bool& started,bool& enabled,bool& slammed,float& doorSpeed,float& slamSpeed){
        return cfgvehiclestop::Observe(r_,plan.module,car,hood,started,enabled,slammed,doorSpeed,slamSpeed);
    }
#include "VehicleStopRuntimeMembers.hpp"
    bool CarDrain(std::uintptr_t car,const cfgvehicleoccupancy::Proof& proof,std::uint64_t now,bool& held){
#ifdef CFG_H1_JASON
        HostedLaneEnter(cfgh1::Lane::Car);
#endif

        held=false;auto& state=bundleVehicleActions;
        if(state.car!=car)return true;
        if(!CarExtraction(held))return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Car,1)
#else
 false
#endif
;
        if(held){bundleCarPursuit.Renew(now);return true;}
        CarExtractionFinishObserved(); // passive completion proof; does not affect extraction behavior
        namespace disk=cfgvehicleinteraction::disk;
        std::uintptr_t manager=0,locked=0,hood=0;cfgprobe::lifetime::SlotObservation slot{};
        if(!Manager()||!r_.Read(bundleJason.pawn.object+cfghunter::disk::Manager,manager)||
           !r_.Read(manager+disk::LockedComponent,locked)||!r_.Read(car+disk::JasonCarInteractComponent,hood))return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Car,2)
#else
 false
#endif
;
        if(locked&&(locked==proof.seat||locked==hood)){
            if(!cfgspawn::ObjectSlot(r_,context.objectArray,locked,slot)||!slot.HasAssignedSerial()||
               !cfgobserver::Same(r_,locked+0xE0,car)||!CarOccupancyCurrent(proof))return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Car,3)
#else
 false
#endif
;
            held=true; // actual native extraction owns Jason; no new path/input
        }else if(proof.result==cfgvehicleoccupancy::Result::Exiting&&
                 state.Leaving(car,proof.driver,now)&&CarOccupancyCurrent(proof))held=true;
        if(held)bundleCarPursuit.Renew(now);
        return true;
    }
    void CarVacant(std::uintptr_t car,std::uint64_t now){
        if(bundleRoads.blockedCar==car){bundleRoads.blockedCar=0;bundleRoads.blockedExit=cfgroad::NoPoint;}
        bundleCarIntercept.Vacant(car);
        // An unrelated empty car must not cancel the currently occupied car.
        if(bundleVehicleActions.car==car||bundleCarActionPending.car==car||bundleRoadMorph.car==car){
            bundleRoadMorph={};CarRetire(now);}
    }
    bool CarResolve(std::uintptr_t source,std::uintptr_t& car){
        if(source!=cfgvehicle::disk::Car2&&source!=cfgvehicle::disk::Car4)return false;
        return cfgvehicle::RosterCar(r_,context.objectArray,plan.module,context.world,context.mode,spawnAssets.gameState,source,car,
            &carProof_[source==cfgvehicle::disk::Car2?1:0]);
    }
    bool CarSource(std::uintptr_t source,std::uintptr_t expected){
        if(source!=cfgvehicle::disk::Car2&&source!=cfgvehicle::disk::Car4)return false;
        auto& proof=carProof_[source==cfgvehicle::disk::Car2?1:0];
        if(!proof.valid){std::uintptr_t car=0;return CarResolve(source,car)&&car==expected;}
        return proof.slot.object==expected&&proof.world==context.world&&proof.mode==context.mode&&
            proof.gameState==spawnAssets.gameState&&cfgvehicle::RosterCurrent(r_,context.objectArray,proof);
    }
    bool CarSpeedWrite(std::uintptr_t at,float expected,float desired)noexcept{
        if(GetCurrentThreadId()!=plan.threadId||at%4)return false;
        LONG oldBits=0,newBits=0;std::memcpy(&oldBits,&expected,4);std::memcpy(&newBits,&desired,4);
        __try{return InterlockedCompareExchange(reinterpret_cast<volatile LONG*>(at),newBits,oldBits)==oldBits;}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;return false;}
    }
    bool CarBoost(bool enable,bool current=false){
        auto& s=bundleCarSpeed;
        if(!enable&&!s.active)return true;
        if((!current&&(!Context()||!Jason()))||GetCurrentThreadId()!=plan.threadId||
           !cfgprimary::LookupSafe(r_,plan.module))return false;
        std::uintptr_t movement=0,type=0;cfgprobe::lifetime::SlotObservation slot{};
        const auto& binding=cfghook::disk::Types[cfghook::disk::Movement];
        if(!r_.Read(bundleJason.pawn.object+cfghook::disk::MovementComponent,movement)||!movement||
           !NativeClass(binding.reflection,binding.constructor,type)||!cfgspawn::ObjectIs(r_,movement,type)||
           !cfgobserver::Same(r_,movement+0x20,bundleJason.pawn.object)||
           !cfgspawn::ObjectSlot(r_,context.objectArray,movement,slot)||!slot.HasAssignedSerial())return false;
        if(s.active&&!(slot==s.slot))return false;
        if(enable&&s.active)return true; // latch; no recurrent float writes
        constexpr std::uintptr_t fields[]{cfghook::disk::Walk,cfghook::disk::Sprint,cfghook::disk::Run,cfghook::disk::SlowRun};
        std::array<float,4> before{},after{};
        for(unsigned i=0;i<4;++i)if(!r_.Read(movement+fields[i],before[i]))return false;
        if(enable){if(!cfgvehicleaction::PursuitSpeeds(before,after))return false;}
        else for(unsigned i=0;i<4;++i)after[i]=before[i]==s.boosted[i]?s.original[i]:before[i]; // never overwrite another owner's later change
        if(!cfgjason::SameSlot(r_,context.objectArray,slot)||!cfgobserver::Same(r_,bundleJason.pawn.object+cfghook::disk::MovementComponent,movement))return false;
        for(unsigned i=0;i<4;++i)if(!CarSpeedWrite(movement+fields[i],before[i],after[i])){
            for(unsigned j=0;j<i;++j)CarSpeedWrite(movement+fields[j],after[j],before[j]);return false;}
        if(enable){s.slot=slot;s.original=before;s.boosted=after;s.active=true;++bundleVehicleActions.speedWrites;}
        else {s.active=false;++bundleVehicleActions.speedRestores;}return true;
    }
    template<class Binding,std::size_t N> bool CarCall(const Binding& binding,std::uintptr_t executor,const std::array<unsigned char,N>& code,std::uintptr_t receiver,void* packet){
        std::uintptr_t function=0,owner=0;
        return FeatureFunction(binding,function,owner)&&cfgspawn::ObjectIs(r_,receiver,owner)&&
            cfgobserver::Same(r_,function+0xB0,plan.module+executor)&&cfgbundle::Code(r_,plan.module,executor,code)&&Invoke(receiver,function,packet,false);
    }
    void CarRetire(std::uint64_t now){
        bundleCarIntercept.ClearEpisode(); // preserve car-only cooldown across occupants
        bundleCarObstacle.Reset();
        const bool hadEpisode=bundleVehicleActions.car||bundleCarActionPending.car||bundleCarPursuit.Active(now);
        bundleCarPursuit.until=0;bundleCarActionPending={};
        if(!hadEpisode)return;
        if(!bundleVehicleActions.Retire())return; // Repeated vacancy must not cancel a resumed counselor chase.
        bundlePursuit.Clear();bundleFollowPath.Clear();bundlePathCooldown.Clear();
        bundleMoveAt=now;bundleSchedule.selectAt=now;bundleSchedule.huntAt=now;bundleReport.carStage=13;
        // StopMovement is deliberately a NEW callback, not stacked on eight
        // typed seat samples. Do not clear a native lock or change possession.
    }
#endif
    // H2.55c diagnostics: the source line of the last CarInteraction refusal, published as H234CarManager Cap.
    bool CarRefused(unsigned line)noexcept{bundleCarRefuseLine=line;return false;}
    bool CarInteraction(std::uintptr_t car,std::uintptr_t driver,cfgspawn::Vec3 position,bool moving,std::uint64_t now,bool occupancyOnly=false,cfgspawn::Vec3 velocity={}){
        namespace disk=cfgvehicleinteraction::disk;auto& state=bundleVehicleActions;
        cfgprobe::lifetime::SlotObservation carSlot{},driverSlot{},seatSlot{},componentSlot{},managerSlot{};
        std::uintptr_t seat=0,hood=0,component=0,manager=0;
        cfgvehicleoccupancy::Proof occupancy{};
        if(!CarOccupancy(car,occupancy))return CarRefused(__LINE__);
        bool extracting=false;if(!CarExtraction(extracting))return CarRefused(__LINE__);
        if(extracting){bundleCarPursuit.Renew(now);return true;}
        if(occupancy.result!=cfgvehicleoccupancy::Result::Occupied){
            bool held=false;if(!CarDrain(car,occupancy,now,held))return CarRefused(__LINE__);
            if(!held)CarVacant(car,now);return true;}
        if(occupancy.driver!=driver||!cfgspawn::ObjectSlot(r_,context.objectArray,driver,driverSlot)||
           !driverSlot.HasAssignedSerial()||!r_.Read(car+disk::JasonCarInteractComponent,hood))return CarRefused(__LINE__);
        carSlot=occupancy.carSlot;seat=occupancy.seat;seatSlot=occupancy.entries[0].slot;
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,seat,seatSlot)||!seatSlot.HasAssignedSerial())return CarRefused(__LINE__);
        if(occupancyOnly){
            if(!CarOccupancyCurrent(occupancy))return CarRefused(__LINE__);
            state.Occupied(car,driver);return true;
        }
        state.Select(car,driver);
        state.driverSerial=driverSlot.serial;
        bool started=false,enabled=false,slammed=false;float doorSpeed=0,slamSpeed=0;cfgspawn::Vec3 currentJason{};
        if(!CarStopObservation(car,hood,started,enabled,slammed,doorSpeed,slamSpeed)||!Location(bundleJason.pawn.object,currentJason))return CarRefused(__LINE__);
        const float cx=position.x-currentJason.x,cy=position.y-currentJason.y;
        const float effective=std::sqrt(velocity.x*velocity.x+velocity.y*velocity.y);
        if(!state.ObserveStop(started,enabled,slammed,effective,doorSpeed,slamSpeed,cx*cx+cy*cy,now))return CarRefused(__LINE__);
        // Donor extraction commitment switches from predictive hood pursuit
        // to the stopped-car authored driver-side route. A transient velocity
        // sample must not apply HoodLead to that door route.
        moving=moving&&!state.extractionCommitted;state.Motion(moving);
        if(slammed||state.HoodWaiting(now)){bundleCarPursuit.Renew(now);return true;}
        if(moving&&!state.extractionCommitted&&cx*cx+cy*cy>1200.f*1200.f){
            // Selected road sources are re-proved within this callback's
            // remaining read budget. Refusal keeps the existing physical lane.
            cfgroad::Candidate road;constexpr unsigned reserve=220;
            if(r_.reads+reserve<cfgbundle::ReadCap&&bundleRoads.ChaseWaypoint(r_,plan.world,
                {currentJason.x,currentJason.y,currentJason.z},{position.x,position.y,position.z},road,
                cfgbundle::ReadCap-r_.reads-reserve)){
                const cfgspawn::Vec3 goal{road.location.x,road.location.y,road.location.z+90.f};
                if(!state.DoorMoveDue(goal,now)){bundleCarPursuit.Renew(now);return true;}
                std::uintptr_t locked=0;
                if(!Manager()||!r_.Read(bundleJason.pawn.object+cfghunter::disk::Manager,manager)||
                   !r_.Read(manager+disk::LockedComponent,locked)||locked||!CarOccupancyCurrent(occupancy))return CarRefused(__LINE__);
                cfgtrap::MoveLocationParameters packet;packet.destination=goal;packet.acceptance=60;packet.result=255;
                cfgvehicleaction::NavigationOptions(true,packet);++bundleCarPursuit.moves;
                if(!CarCall(cfgtrap::disk::MoveToLocation,cfgtrap::disk::MoveToLocationExecutor,
                    cfgtrap::disk::MoveToLocationCode,bundleJason.killerController.object,&packet))return CarRefused(__LINE__);
                if(packet.result!=0&&packet.result<=2){state.DoorMoveAccepted(goal,now);state.haveHoodGoal=false;++bundleReport.carMoves;
                    bundleReport.carStage=12;bundleCarPursuit.Renew(now);bundleFollowPath.Clear();return true;}
                return CarRefused(__LINE__);
            }
        }else if(!state.extractionCommitted)state.haveDoorGoal=false;
        const bool door=state.extractionCommitted;
        if(state.doorPhase!=door){state.phase=0;state.readyAt=0;state.doorPhase=door;state.haveDoorGoal=false;state.route.Reset();}
        component=door?seat:hood;if(!component)return CarRefused(__LINE__);
        std::uintptr_t interactType=0;
        if(!NativeClass(disk::ILLInteractableComponent,disk::ILLInteractableComponentConstructor,interactType)||
           !cfgspawn::ObjectIs(r_,component,interactType)||!cfgobserver::Same(r_,component+0xE0,car)||
           !cfgspawn::ObjectSlot(r_,context.objectArray,component,componentSlot)||!componentSlot.HasAssignedSerial())return CarRefused(__LINE__);
        std::uintptr_t locked=0;
        if(!Manager()||!r_.Read(bundleJason.pawn.object+cfghunter::disk::Manager,manager)||!r_.Read(manager+disk::LockedComponent,locked))return CarRefused(__LINE__);
        if(state.StaleSeatAttempt(now)&&(!locked||locked==seat)){
            // Port donor18L-BQ using CFG's already pinned unlock ABI. Never
            // cancel an actual extraction, a different lock, or a departed
            // occupant. Native extraction was checked above and is checked
            // again after the full reciprocal seat/world proof.
            bool nativeActive=false;
            if(!CarOccupancyCurrent(occupancy)||!CarExtraction(nativeActive))return CarRefused(__LINE__);
            if(nativeActive){bundleCarPursuit.Renew(now);return true;}
            if(locked&&!CarCall(cfgaction::disk::CLIENT_UnlockInteraction,cfgaction::disk::CLIENT_UnlockInteractionExecutor,
                cfgaction::disk::CLIENT_UnlockInteractionCode,manager,&seat))return CarRefused(__LINE__);
            state.RetryDriverApproach();bundleCarPursuit.Renew(now);return true;
        }
        // Native lock/animation owns Jason now. Do not replace a successful
        // hood/seat interaction with another path request on the next pass.
        if(locked){if(locked==seat||locked==hood)bundleCarPursuit.Renew(now);return true;}
        cfgspawn::Vec3 forward{},point{},jason{},componentPosition{},componentAgain{};
        const cfgspawn::Vec3* authored=nullptr;
        // Donor ResolveDriverDoorNavPoint uses the actual driver seat side;
        // its hood resolver projects the component onto the chassis axis.
        // Read only a typed, owned scene component; no donor layout reuse.
        if(!moving){std::uintptr_t sceneClass=0;
            if(!NativeClass(cfgposition::disk::SceneClassCache,cfgposition::disk::SceneConstructorCache,sceneClass)||
               !cfgspawn::ObjectIs(r_,component,sceneClass)||!cfgobserver::Same(r_,component+0x20,car)||
               !r_.Read(component+cfgposition::disk::SceneWorldTranslation,componentPosition)||
               !r_.Read(component+cfgposition::disk::SceneWorldTranslation,componentAgain)||
               componentPosition.x!=componentAgain.x||componentPosition.y!=componentAgain.y||componentPosition.z!=componentAgain.z||
               !cfgjason::SameSlot(r_,context.objectArray,componentSlot))return CarRefused(__LINE__);
            authored=&componentPosition;
        }
        if(!CarCall(cfgtrap::disk::GetActorForwardVector,cfgtrap::disk::GetActorForwardVectorExecutor,cfgtrap::disk::GetActorForwardVectorCode,car,&forward)||
           !cfgvehicleaction::Point(position,forward,door,point,authored)||!Location(bundleJason.pawn.object,jason))return CarRefused(__LINE__);
        if(door&&!DriverDoorPoint(car,seat,position,point,carSlot.serial,seatSlot.serial))return CarRefused(__LINE__);
        const auto interactionPoint=point;
        if(door){cfgspawn::Vec3 routePoint{};
            if(!state.route.Goal(position,forward,point,jason,now,routePoint))return CarRefused(__LINE__);
            point=routePoint; // donor360cm nearer-corner route, then the driver's side
            if(!DriverCornerPoint(point))return CarRefused(__LINE__);
        }else state.route.Reset();
        // Gate interaction on the REAL hood, not the predictive pursuit goal.
        const float threshold=door?135.f:moving?240.f:185.f;
        if(!CarOccupancyCurrent(occupancy))return CarRefused(__LINE__); // native geometry calls may coincide with seat exit
        if(!cfgvehicleaction::AtInteraction(jason,interactionPoint,point,threshold)){
            if(!door&&!state.hoodAt){
                const auto source=car==bundleCarActionPending.car?bundleCarActionPending.source:std::uintptr_t{};
                if(source&&bundleCarObstacle.Observe(car,driver,source,jason,position,now)){bundleCarPursuit.Renew(now);return true;}
                if(bundleCarObstacle.Protected(now)){bundleCarPursuit.Renew(now);return true;}
            }
            if(moving){
                cfgspawn::Vec3 pursuitPoint{};if(!cfgvehicleaction::HoodLead(point,velocity,pursuitPoint))return CarRefused(__LINE__);
                point=pursuitPoint;
                if(!state.HoodMoveDue(point,now)){bundleCarPursuit.Renew(now);return true;}
            }
            // Donor keeps an accepted driver-side path until its goal changes
            // materially or five seconds elapse. Reissuing every callback can
            // continually restart navigation around the chassis.
            if(door&&!state.DoorMoveDue(point,now)){bundleCarPursuit.Renew(now);return true;}
            state.phase=0;state.readyAt=0;cfgtrap::MoveLocationParameters packet;packet.destination=point;packet.acceptance=door?70.f:90.f;
            cfgvehicleaction::NavigationOptions(moving,packet);
            packet.result=255;
            ++bundleCarPursuit.moves;
            if(!CarCall(cfgtrap::disk::MoveToLocation,cfgtrap::disk::MoveToLocationExecutor,cfgtrap::disk::MoveToLocationCode,bundleJason.killerController.object,&packet))return CarRefused(__LINE__);
            if(packet.result!=0&&packet.result<=2){
                if(moving)state.HoodMoveAccepted(point,now);
                if(door)state.DoorMoveAccepted(point,now);
                ++bundleReport.carMoves;bundleReport.carStage=12;bundleCarPursuit.Renew(now);bundleFollowPath.Clear();return true;
            }return CarRefused(__LINE__);
        }
        bundleCarPursuit.Renew(now);
        if(state.phase==0){
            state.haveDoorGoal=false; // a stopped path must be reacquired if the driver moves away
            if(!CarCall(cfgtrap::disk::StopMovement,cfgtrap::disk::StopMovementExecutor,cfgtrap::disk::StopMovementCode,bundleJason.killerController.object,nullptr))return CarRefused(__LINE__);
            state.readyAt=now;state.phase=1;return true;
        }
        if((moving&&state.movingAttempts>=3)||(door&&(now<state.readyAt+650||!state.SeatAttemptDue(now)))||
           (state.attemptAt&&now<state.attemptAt+cfgvehicleaction::AttemptInterval(door,moving)))return true;
        if(state.phase==1){
            cfgspawn::Vec3 rotation{};rotation.y=std::atan2(position.y-jason.y,position.x-jason.x)*57.2957795131f;
            if(!CarCall(cfgtrap::disk::SetControlRotation,cfgtrap::disk::SetControlRotationExecutor,cfgtrap::disk::SetControlRotationCode,bundleJason.killerController.object,&rotation))return CarRefused(__LINE__);
            state.phase=2;return true;
        }
        if(!Context()||!Jason()||!Manager()||!cfgjason::SameSlot(r_,context.objectArray,carSlot)||
           !cfgjason::SameSlot(r_,context.objectArray,driverSlot)||!cfgjason::SameSlot(r_,context.objectArray,seatSlot)||
           !cfgjason::SameSlot(r_,context.objectArray,componentSlot)||!CarOccupancyCurrent(occupancy)||
           !cfgobserver::Same(r_,seat+disk::ParentVehicle,car)||!cfgobserver::Same(r_,seat+disk::CounselorInSeat,driver)||
           !cfgobserver::Same(r_,component+0xE0,car)||(!door&&!cfgobserver::Same(r_,car+disk::JasonCarInteractComponent,component))||
           !r_.Read(bundleJason.pawn.object+cfghunter::disk::Manager,manager)||
           !cfgspawn::ObjectSlot(r_,context.objectArray,manager,managerSlot)||!managerSlot.HasAssignedSerial()||
           !cfgobserver::Same(r_,manager+disk::LockedComponent,std::uintptr_t{}))return CarRefused(__LINE__);
        cfgescape::State alive;if(!cfgescape::Counselor(r_,context.objectArray,plan.module,driver,bundleCounselorClass,alive)||!cfgescape::Driving(alive))return CarRefused(__LINE__);
        cfgvehicleaction::Attempt packet;packet.component=component;state.attemptAt=now;state.phase=0;
        if(door){++state.seatAttempts;state.SeatDispatched(now);}else {++state.hoodAttempts;if(moving)++state.movingAttempts;}
        const bool invoked=CarCall(disk::Attempt,disk::Executor,disk::ExecutorCode,manager,&packet);
        if(invoked&&!door)state.HoodDispatched(moving,now);return invoked;
    }
