// Donor360cm chassis clearances with geometric on-foot admission. Separate
// route: near corner then far corner, measured arrival and bounded recovery.
// The accepted occupied-car extraction path is not changed.
// This callback is separate from Hunt so its native proofs get a fresh budget.
    static std::uint64_t FootCarCoordinate(float value){
        if(!std::isfinite(value))return 0;
        const double scaled=std::round(static_cast<double>(value)*10.0);
        if(std::fabs(scaled)>9.0e15)return 0;
        const auto signedValue=static_cast<std::int64_t>(scaled);
        const auto magnitude=static_cast<std::uint64_t>(signedValue<0?-signedValue:signedValue);
        return (signedValue<0?(std::uint64_t{1}<<63):0)|magnitude; // signed tenths of a centimetre.
    }
    bool FootCarRetryEnd(std::uint64_t now,std::uintptr_t target,cfgspawn::Vec3 here,cfgfootcar::Failure why){
        auto& p=bundleFootObstacle;
        const float failedEnd=p.route.end;
        const bool alternate=bundleFootObstacleHistory.Fail(failedEnd,why,now);
        cfgclosedtrace::Mark("foot-car-end-failed",p.car,
            (static_cast<std::uint64_t>(cfgfootcar::RouteHistory::EndIndex(failedEnd))<<32)|static_cast<unsigned>(why));
        if(alternate){
            const float next=bundleFootObstacleHistory.NextEnd(-failedEnd);
            if(next&&p.route.ReplanEnd(next,here,now)){
                p.candidate=p.route.goal;p.stage=1;bundleFootObstacleTarget=target;
                bundleSchedule.huntAt=now+100;bundleFootObstacleAlternate=true;
                cfgclosedtrace::Mark("foot-car-opposite-end",p.car,static_cast<std::uint64_t>(next>0?1:2));
                return true;
            }
        }
        cfgclosedtrace::Mark("foot-car-route-exhausted",target,
            (static_cast<std::uint64_t>(bundleFootObstacleHistory.attemptedEnds)<<32)|bundleFootObstacleHistory.episodes);
        p.Reset();bundleFootObstacleTarget=0;bundleFootObstacleCheckAt=now+5000;
        bundleMoveAt=now;bundleFollowPath.Clear();return false;
    }
    bool FootCarObstacle(std::uint64_t now,std::uintptr_t target){
        auto& p=bundleFootObstacle;bundleFootObstacleTarget=0;
        std::uintptr_t freshTarget=0,seat=0,held=0,manager=0,locked=0;
        cfgprobe::lifetime::SlotObservation targetSlot{};
        bool living=false,stunned=false,attacking=false;cfgspawn::Vec3 here{},goal{};
        if(!Context()||!Jason()||!FootObstacleBindings()||!Manager()||!Known(bundleTargetIndex,freshTarget)||freshTarget!=target||
           !Alive(target,living)||!living||!Held(held)||held||
           !r_.Read(target+cfgvehicleoccupancy::disk::CurrentSeat,seat)||
           !NativeBool(cfghunter::disk::Stunned,stunned)||stunned||
           !NativeBool(cfghunter::disk::Attacking,attacking)||attacking||
           !cfgspawn::ObjectSlot(r_,context.objectArray,target,targetSlot)||!targetSlot.HasAssignedSerial()||
           !r_.Read(bundleJason.pawn.object+cfghunter::disk::Manager,manager)||
           !r_.Read(manager+cfgvehicleinteraction::disk::LockedComponent,locked)||locked||
           !Location(bundleJason.pawn.object,here)||!Location(target,goal)) {p.Reset();return false;}
        if(seat){p.Reset();bundleFootObstacleHistory.Reset();return false;}
        if(!p.stage){
            std::uintptr_t car=0,source=0,type=0,ctor=0;cfgspawn::Vec3 at{};
            if(!r_.Read(plan.module+cfgvehicle::disk::VehicleReflection,type)||
               !r_.Read(plan.module+cfgvehicle::disk::VehicleConstructor,ctor)||!cfgprobe::MatchingClassCache(type,ctor)){p.Reset();return false;}
            for(const auto offset:{cfgvehicle::disk::Car2,cfgvehicle::disk::Car4}){
                std::uintptr_t candidate=0;cfgprobe::lifetime::SlotObservation slot{};
                if(!CarResolve(offset,candidate)||!candidate||!cfgspawn::ObjectSlot(r_,context.objectArray,candidate,slot)||
                   !slot.HasAssignedSerial()||!cfgspawn::ObjectIs(r_,candidate,type)||!Location(candidate,at))continue;
                const float x=at.x-here.x,y=at.y-here.y;
                if(x*x+y*y<=650.f*650.f&&std::fabs(at.z-here.z)<180.f){car=candidate;source=offset;bundleFootObstacleCarSlot=slot;break;}
            }
            if(!car){p.Reset();return false;}
            // Use the counselor direction when the car blocks an ON-FOOT
            // chase. Driver/hood routes and their accepted state are untouched.
            if(!p.Observe(car,target,source,here,goal,now))return false;
            p.stage=5; // heading gets its own independently budgeted turn
            bundleFootObstacleTarget=target;bundleSchedule.huntAt=now+100;return true;
        }
        if(!CarSource(p.source,p.car)||!cfgjason::SameSlot(r_,context.objectArray,bundleFootObstacleCarSlot)) {
            p.Reset();bundleFootObstacleHistory.Reset();return false;}
        if(p.stage==5){
            cfgspawn::Vec3 at{},forward{};
            if(!Location(p.car,at)||!CarCall(cfgtrap::disk::GetActorForwardVector,cfgtrap::disk::GetActorForwardVectorExecutor,
                cfgtrap::disk::GetActorForwardVectorCode,p.car,&forward)||!p.route.Begin(at,forward,here,goal,now)){
                p.Reset();bundleFootObstacleCheckAt=now+3000;return false;
            }
            if(!bundleFootObstacleHistory.Bind(bundleFootObstacleCarSlot.index,bundleFootObstacleCarSlot.serial,
                targetSlot.index,targetSlot.serial,at,p.route.axis,p.route.side,here)){
                p.Reset();bundleFootObstacleHistory.Reset();bundleFootObstacleCheckAt=now+3000;return false;
            }
            cfgclosedtrace::Mark("foot-car-route-acquired",p.car,
                (static_cast<std::uint64_t>(static_cast<std::uint32_t>(bundleFootObstacleCarSlot.index))<<32)|
                static_cast<std::uint32_t>(bundleFootObstacleCarSlot.serial));
            cfgclosedtrace::Mark("foot-car-target-slot",target,
                (static_cast<std::uint64_t>(static_cast<std::uint32_t>(targetSlot.index))<<32)|
                static_cast<std::uint32_t>(targetSlot.serial));
            if(bundleFootObstacleHistory.exhausted){
                cfgclosedtrace::Mark("foot-car-route-exhausted-suppressed",p.car,bundleFootObstacleHistory.attemptedEnds);
                p.Reset();bundleFootObstacleCheckAt=now+5000;return false;
            }
            const float eligibleEnd=bundleFootObstacleHistory.NextEnd(p.route.end);
            if(!eligibleEnd||!p.route.ReplanEnd(eligibleEnd,here,now)){
                p.Reset();bundleFootObstacleCheckAt=now+5000;return false;
            }
            cfgclosedtrace::Mark("foot-car-route-chosen-end",p.car,static_cast<std::uint64_t>(eligibleEnd>0?1:2));
            p.candidate=p.route.goal;p.stage=1;
        }else if(p.stage==4){
            cfgspawn::Vec3 at{};
            if(!Location(p.car,at)||!p.route.Current(at,goal)){
                cfgclosedtrace::Mark("foot-car-route-invalidated",p.car,target);
                p.Reset();bundleFootObstacleHistory.Reset();bundleMoveAt=now;return false;}
            const auto oldProgressAt=p.route.progressAt;
            const auto oldLeg=p.route.leg;
            const auto action=p.route.Observe(here,now);
            if(p.route.progressAt!=oldProgressAt||p.route.leg!=oldLeg){
                const float remaining=std::sqrt(cfgfootcar::Route::Squared(here,p.route.arrival));
                cfgclosedtrace::Mark("foot-car-route-progress",p.car,
                    (static_cast<std::uint64_t>(p.route.leg)<<32)|static_cast<std::uint32_t>(now-p.route.progressAt));
                cfgclosedtrace::Mark("foot-car-route-remaining-tenth-cm",p.car,FootCarCoordinate(remaining));
                cfgclosedtrace::Mark("foot-car-route-best-tenth-cm",p.car,FootCarCoordinate(p.route.best));
            }
            if(action==2){p.Reset();bundleFootObstacleHistory.Reset();bundleFootObstacleCheckAt=now+3000;
                bundleMoveAt=now;bundlePathCooldown.Clear();cfgclosedtrace::Mark("foot-car-cleared",target,now);return true;}
            if(action==3){cfgclosedtrace::Mark("foot-car-route-stalled",target,
                (static_cast<std::uint64_t>(p.route.leg)<<32)|static_cast<std::uint32_t>(now-p.route.progressAt));
                return FootCarRetryEnd(now,target,here,cfgfootcar::Failure::Stall);}
            if(action==1){p.candidate=p.route.goal;p.stage=1;}
        }else if(p.stage==1){
            std::uintptr_t function=0,type=0,system=0;cfgprobe::lifetime::SlotObservation slot{};
            if(!FeatureFunction(cfgvehicle::disk::Project,function,type)||
               !r_.Read(plan.world+cfgtrap::disk::WorldNavigationSystem,system)||!cfgspawn::ObjectIs(r_,system,type)||
               !cfgspawn::ObjectSlot(r_,context.objectArray,system,slot)||!slot.HasAssignedSerial())
                return FootCarRetryEnd(now,target,here,cfgfootcar::Failure::NavigationOwner);
            // Keep exact native projection evidence so geometry rejection can
            // be distinguished from native refusal in the bounded trace.
            cfgclosedtrace::Mark("foot-car-project-nav-slot",system,
                (static_cast<std::uint64_t>(static_cast<std::uint32_t>(slot.index))<<32)|static_cast<std::uint32_t>(slot.serial));
            cfgclosedtrace::Mark("foot-car-project-candidate-x-tenth-cm",p.car,FootCarCoordinate(p.candidate.x));
            cfgclosedtrace::Mark("foot-car-project-candidate-y-tenth-cm",p.car,FootCarCoordinate(p.candidate.y));
            cfgclosedtrace::Mark("foot-car-project-candidate-z-tenth-cm",p.car,FootCarCoordinate(p.candidate.z));
            cfgclosedtrace::Mark("foot-car-project-world-nav",plan.world,
                (static_cast<std::uint64_t>(static_cast<std::uint32_t>(slot.index))<<32)|static_cast<std::uint32_t>(slot.serial));
            cfgclosedtrace::Mark("foot-car-project-extent-cm",system,160u|(160ull<<16)|(180ull<<32));
            // Donor driver-corner projection uses160/160/180. Ground the query
            // at the car instead of searching100cm below a103cm capsule center.
            cfgvehicle::Projection packet;packet.world=plan.world;packet.point=p.candidate;packet.extent={160,160,180};
            const bool invoked=Invoke(system,function,&packet,false);
            const bool current=cfgjason::SameSlot(r_,context.objectArray,slot)&&
                cfgobserver::Same(r_,plan.world+cfgtrap::disk::WorldNavigationSystem,system);
            if(!invoked||!current){
                cfgclosedtrace::Mark(!invoked?"foot-car-project-call-refused":"foot-car-project-owner-changed",p.car,Reads());
                if(!current){p.Reset();bundleFootObstacleHistory.Reset();bundleFootObstacleCheckAt=now+3000;bundleMoveAt=now;return false;}
                return FootCarRetryEnd(now,target,here,cfgfootcar::Failure::ProjectionCall);
            }
            if(packet.result!=1||!p.route.AcceptProjection(packet.projected,now)){
                const auto reason=packet.result!=1?cfgfootcar::Route::ProjectionFailure::None:p.route.ProjectionReason(packet.projected);
                cfgclosedtrace::Mark(packet.result!=1?"foot-car-project-no-ground":"foot-car-project-rejected",
                    p.car,packet.result!=1?packet.result:static_cast<unsigned>(reason));
                if(packet.result==1){
                    cfgclosedtrace::Mark("foot-car-project-x-tenth-cm",p.car,FootCarCoordinate(packet.projected.x));
                    cfgclosedtrace::Mark("foot-car-project-y-tenth-cm",p.car,FootCarCoordinate(packet.projected.y));
                    cfgclosedtrace::Mark("foot-car-project-z-tenth-cm",p.car,FootCarCoordinate(packet.projected.z));
                }
                return FootCarRetryEnd(now,target,here,packet.result!=1?cfgfootcar::Failure::NoGround:
                    cfgfootcar::Failure::ProjectionGeometry);
            }
            cfgclosedtrace::Mark("foot-car-project-accepted",p.car,packet.result);
            cfgclosedtrace::Mark("foot-car-project-x-tenth-cm",p.car,FootCarCoordinate(packet.projected.x));
            cfgclosedtrace::Mark("foot-car-project-y-tenth-cm",p.car,FootCarCoordinate(packet.projected.y));
            cfgclosedtrace::Mark("foot-car-project-z-tenth-cm",p.car,FootCarCoordinate(packet.projected.z));
            p.projected=packet.projected;p.stage=2;
        }else if(p.stage==2){
            const bool stopped=CarCall(cfgtrap::disk::StopMovement,cfgtrap::disk::StopMovementExecutor,cfgtrap::disk::StopMovementCode,
                bundleJason.killerController.object,nullptr);
            cfgclosedtrace::Mark(stopped?"foot-car-stop-accepted":"foot-car-stop-refused",p.car,now);
            if(!stopped)
                return FootCarRetryEnd(now,target,here,cfgfootcar::Failure::StopMovement);
            p.stage=3;
        }else if(p.stage==3){
            cfgtrap::MoveLocationParameters packet;packet.destination=p.projected;packet.acceptance=65.f;packet.result=255;
            cfgvehicleaction::NavigationOptions(false,packet);
            const bool requested=CarCall(cfgtrap::disk::MoveToLocation,cfgtrap::disk::MoveToLocationExecutor,cfgtrap::disk::MoveToLocationCode,
                bundleJason.killerController.object,&packet);
            cfgclosedtrace::Mark(requested?"foot-car-moveto-result":"foot-car-moveto-refused",p.car,packet.result);
            if(!requested||packet.result==0||packet.result>2)
                return FootCarRetryEnd(now,target,here,cfgfootcar::Failure::MoveTo);
            p.stage=4;p.protectUntil=0;bundleFollowPath.Clear();bundleMoveAt=now+100;
            // A successful native request is not progress; stage4 measures actual arrival.
            bundleFootObstacleTarget=target;bundleSchedule.huntAt=now+100;
            cfgclosedtrace::Mark("foot-car-detour",p.car,target);return true;
        }
        if(p.stage){bundleFootObstacleTarget=target;bundleSchedule.huntAt=now+100;}
        return true;
    }
