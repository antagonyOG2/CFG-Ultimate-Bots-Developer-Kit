// Active donor bounded obstacle recovery through verified CFG native adapters.
    bool CarObstaclePhase(std::uint64_t now){
#ifdef CFG_H1_JASON
        HostedLaneEnter(cfgh1::Lane::Stuck);
#endif

        auto& p=bundleCarObstacle;bundleSchedule.vehicleAt=now+100;
        if(!p.stage)return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Stuck,1)
#else
 false
#endif
;
        if(bundleCompletionReady||bundleKill.policy.Exclusive(now)){p.Reset();return true;}
        if(!Context()||!Jason()||!CarActorBindings()||!Manager()){p.Reset();return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Stuck,2)
#else
 false
#endif
;}
        std::uintptr_t manager=0,locked=0,held=0;bool stunned=false,attacking=false,living=false;
        cfgvehicleoccupancy::Proof proof{};cfgspawn::Vec3 current{};
        if(!Held(held)||!Alive(bundleJason.pawn.object,living)||!NativeBool(cfghunter::disk::Stunned,stunned)||
           !NativeBool(cfghunter::disk::Attacking,attacking)||!r_.Read(bundleJason.pawn.object+cfghunter::disk::Manager,manager)||
           !r_.Read(manager+cfgvehicleinteraction::disk::LockedComponent,locked)||!CarSource(p.source,p.car)||
           !CarOccupancy(p.car,proof)||!Location(bundleJason.pawn.object,current)){p.Reset();return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Stuck,3)
#else
 false
#endif
;}
        if(!living||held||stunned||attacking||locked||bundleVehicleActions.extractionCommitted||bundleVehicleActions.hoodAt||
           proof.result!=cfgvehicleoccupancy::Result::Occupied||proof.driver!=p.driver){p.Reset();return true;}
        const float dx=current.x-p.origin.x,dy=current.y-p.origin.y;
        if(dx*dx+dy*dy>100.f*100.f||std::fabs(current.z-p.origin.z)>100.f){p.Reset();return true;}
        if(p.stage==4){
            // Active donor DriveVehicleObstacleRecovery's missing second half.
            // Keep the ordinary shared Morph timer, owned occupied-car proof,
            // native combat/lock gates and native TeleportTo collision test.
            // Waiting for recharge must not monopolize callback polling.
            const auto ready=(std::max)(bundleCarMorph.next,bundleCarIntercept.next);
            if(now<ready){p.stage=0;p.next=ready;return true;}
            const auto destination=p.projected;
            const float x=destination.x-current.x,y=destination.y-current.y;
            if(!p.detourIssued||!cfgspawn::Finite(destination)||std::fabs(destination.z-current.z)>100.f||
               x*x+y*y<100.f*100.f||x*x+y*y>400.f*400.f||!ExteriorLanding(destination)||!CarOccupancyCurrent(proof)){
                p.Reset();return true;
            }
            cfgpursuit::TeleportPacket packet;packet.location=destination;
            cfgspawn::Vec3 carPosition{};
            if(!Location(p.car,carPosition)){p.Reset();return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Stuck,4)
#else
 false
#endif
;}
            packet.rotation.y=std::atan2(carPosition.y-current.y,carPosition.x-current.x)*57.2957795f;
            // Consume before dispatch; no replay after ambiguous native entry.
            p.Reset();bundleCarMorph.Attempted(now);bundleCarIntercept.Attempted(now);++bundleReport.morphAttempts;
            if(!CarCall(cfgtrap::disk::K2_TeleportTo,cfgtrap::disk::K2_TeleportToExecutor,
                cfgtrap::disk::K2_TeleportToCode,bundleJason.pawn.object,&packet)||packet.result!=1)return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Stuck,5)
#else
 false
#endif
;
            ++bundleReport.morphReturns;bundleReport.carStage=15;bundleCarPursuit.Renew(now);
            bundleVehicleActions.Morphed();bundleFollowPath.Clear();return true;
        }
        if(p.stage==1){
            std::uintptr_t function=0,type=0,system=0;cfgprobe::lifetime::SlotObservation slot{};
            if(!FeatureFunction(cfgvehicle::disk::Project,function,type)||
               !r_.Read(plan.world+cfgtrap::disk::WorldNavigationSystem,system)||!cfgspawn::ObjectIs(r_,system,type)||
               !cfgspawn::ObjectSlot(r_,context.objectArray,system,slot)||!slot.HasAssignedSerial()){p.stage=0;return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Stuck,6)
#else
 false
#endif
;}
            cfgvehicle::Projection packet;packet.world=plan.world;packet.point=p.candidate;packet.extent={80,80,100};
            if(!CarOccupancyCurrent(proof)||!Invoke(system,function,&packet,false)||packet.result!=1||
               !cfgjason::SameSlot(r_,context.objectArray,slot)||
               !cfgobserver::Same(r_,plan.world+cfgtrap::disk::WorldNavigationSystem,system)){p.stage=0;return true;}
            p.Projected(packet.projected);return true;
        }
        if(!CarOccupancyCurrent(proof)){p.Reset();return true;}
        if(p.stage==2){
            if(!CarCall(cfgtrap::disk::StopMovement,cfgtrap::disk::StopMovementExecutor,cfgtrap::disk::StopMovementCode,bundleJason.killerController.object,nullptr)){p.stage=0;return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Stuck,7)
#else
 false
#endif
;}
            p.stage=3;return true;
        }
        cfgtrap::MoveLocationParameters packet;packet.destination=p.projected;packet.acceptance=65.f;packet.result=255;
        cfgvehicleaction::NavigationOptions(false,packet);
        if(!CarCall(cfgtrap::disk::MoveToLocation,cfgtrap::disk::MoveToLocationExecutor,cfgtrap::disk::MoveToLocationCode,bundleJason.killerController.object,&packet)){p.stage=0;return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Stuck,8)
#else
 false
#endif
;}
        if(packet.result!=0&&packet.result<=2){p.Accepted(now);++bundleReport.carMoves;bundleReport.carStage=15;
            bundleCarPursuit.Renew(now);bundleVehicleActions.haveHoodGoal=false;bundleFollowPath.Clear();}
        else p.stage=0;
        return true;
    }
