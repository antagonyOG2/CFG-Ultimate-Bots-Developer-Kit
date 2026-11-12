// Included inside BundleAccess. Dedicated preparation/projection/Morph phases;
// never a straight-tangent fallback or the counselor50m teleport gate.
    bool PrepareRoads(){
        if(!Context())return false;
        bundleRoads.Step(r_,plan.module,plan.world,true,cfgbundle::ReadCap-r_.reads);
        return true;
    }
    bool CarRoadPhase(std::uint64_t now){
        auto& pending=bundleRoadMorph;bundleSchedule.vehicleAt=now+100;
        if(!pending.stage)return false;
        if((now<bundleCarIntercept.next&&!pending.exitPriority)||bundleKill.policy.Exclusive(now)||bundleCompletionReady){pending={};return true;}
        if(!Context()||!Jason()||!Manager()){pending={};return false;}
        std::uintptr_t held=0,manager=0,locked=0;bool stunned=false,attacking=false,living=false;
        if(!Alive(bundleJason.pawn.object,living)||!living||!Held(held)||
           !NativeBool(cfghunter::disk::Stunned,stunned)||!NativeBool(cfghunter::disk::Attacking,attacking)||
           !r_.Read(bundleJason.pawn.object+cfghunter::disk::Manager,manager)||
           !r_.Read(manager+cfgvehicleinteraction::disk::LockedComponent,locked)){pending={};return false;}
        if(held||stunned||attacking||locked||bundleVehicleActions.extractionCommitted){pending={};return true;}
        cfgvehicleoccupancy::Proof occupancy{};cfgspawn::Vec3 car{},jason{};
        if(!CarSource(pending.source,pending.car)||!CarOccupancy(pending.car,occupancy)){pending={};return false;}
        if(occupancy.result!=cfgvehicleoccupancy::Result::Occupied||occupancy.driver!=pending.driver){
            const auto vacant=pending.car;CarVacant(vacant,now);pending={};return true;}
        if(!Location(pending.car,car)||!Location(bundleJason.pawn.object,jason)){pending={};return false;}
        bool stopOwns=false;
        // Fresh flags on BOTH projection and teleport slices cancel an older
        // moving request; never let exitPriority bypass a newly observed stop.
        if(!CarStopBeforeRoad(occupancy,car,jason,pending.velocity,now,stopOwns)){pending={};return false;}
        if(stopOwns){pending={};bundleSchedule.vehicleAt=now;bundleCarPursuit.Renew(now);return true;}
        const auto speed=std::sqrt(pending.velocity.x*pending.velocity.x+pending.velocity.y*pending.velocity.y);
        if(!std::isfinite(speed)||speed<100.f){pending={};return true;}
        const cfgroad::Vector direction{pending.velocity.x/speed,pending.velocity.y/speed,0};
        // Retire stale motion after a large turn/backtrack rather than Morph
        // using a driver pointer left over from the previous occupied car.
        const float dx=car.x-pending.position.x,dy=car.y-pending.position.y;
        if(dx*dx+dy*dy>1500.f*1500.f||dx*direction.x+dy*direction.y < -100.f){pending={};return true;}
        cfgroad::Candidate fresh;constexpr unsigned MutationReserve=220;
        if(r_.reads+MutationReserve>=cfgbundle::ReadCap||!bundleRoads.Intercept(r_,plan.world,
            {car.x,car.y,car.z},direction,speed,true,fresh,cfgbundle::ReadCap-r_.reads-MutationReserve,pending.car)){
            pending={};return true; // physical pursuit remains available on bounded road refusal
        }
        if(pending.exitPriority&&!fresh.exitBlock){pending={};return true;}
        if(fresh.exitBlock){
            const auto distance=cfgroad::Sub(fresh.location,{jason.x,jason.y,jason.z});
            if(cfgroad::Dot(distance,distance)<=100.f*100.f){
                bundleRoads.blockedCar=pending.car;bundleRoads.blockedExit=fresh.exitIndex;pending={};return true;
            }
        }else{
            if(pending.prolongedTail&&cfgroad::Dot(cfgroad::Sub({jason.x,jason.y,jason.z},{car.x,car.y,car.z}),direction)>=-200.f){pending={};return true;}
            if(!cfgroad::MayMorph({car.x,car.y,car.z},{jason.x,jason.y,jason.z},direction,pending.prolongedTail)){pending={};return true;}
        }
        ++bundleReport.roadIntercepts;
        if(pending.stage==1){
            std::uintptr_t function=0,navClass=0,system=0;cfgprobe::lifetime::SlotObservation slot{};
            if(!FeatureFunction(cfgvehicle::disk::Project,function,navClass)||
               !r_.Read(plan.world+cfgtrap::disk::WorldNavigationSystem,system)||
               !cfgspawn::ObjectIs(r_,system,navClass)||!cfgspawn::ObjectSlot(r_,context.objectArray,system,slot)){
                pending={};return false;}
            cfgvehicle::Projection projection;projection.world=plan.world;
            projection.point={fresh.location.x,fresh.location.y,fresh.location.z};projection.extent={180,180,180};
            if(!CarOccupancyCurrent(occupancy)||!Invoke(system,function,&projection,false)||projection.result!=1||
               !cfgspawn::Finite(projection.projected)||!cfgjason::OwnedComponentSlot(r_,context.objectArray,slot)||
               !cfgobserver::Same(r_,plan.world+cfgtrap::disk::WorldNavigationSystem,system)){pending={};return true;}
            pending.candidate=fresh;pending.projected=projection.projected;pending.stage=2;return true;
        }
        // Validate the graph again at dispatch. A retained projection is only
        // eligible near the newly predicted road centre, before escape gates.
        cfgspawn::Vec3 landing{};const cfgspawn::Vec3 predicted{fresh.location.x,fresh.location.y,fresh.location.z};
        const float lead=(predicted.x-car.x)*direction.x+(predicted.y-car.y)*direction.y;
        cfgroad::Vector roadLanding{};
        if(!cfgroad::Landing(fresh.location,{pending.projected.x,pending.projected.y,pending.projected.z},roadLanding)||
           lead<(fresh.exitBlock?250.f:1200.f)||lead>fresh.requestedLead+1000.f){pending={};return true;}
        landing={roadLanding.x,roadLanding.y,roadLanding.z};
        if(
           !ExteriorLanding(landing)){pending={};return true;}
        std::uintptr_t function=0;cfgobserver::Plan world;
        if(!TeleportFunction(function)||!CarSource(pending.source,pending.car)||!CarOccupancyCurrent(occupancy)||
           !cfgobserver::Same(r_,manager+cfgvehicleinteraction::disk::LockedComponent,std::uintptr_t{})||
           !cfgprimary::LookupSafe(r_,plan.module)||!GameplayWorld(world)||
           !cfgjason::SameSlot(r_,context.objectArray,bundleJason.pawn)||
           !cfgobserver::Same(r_,bundleJason.pawn.object+cfgescape::disk::Controller,bundleJason.killerController.object)){
                pending={};return false;}
        cfgpursuit::TeleportPacket packet;packet.location=landing;
        packet.rotation={0,std::atan2(-fresh.direction.y,-fresh.direction.x)*57.2957795f,0};
        bundleReport.roadRequested=landing;bundleReport.roadActual={};bundleReport.roadLandingObserved=0;
        bundleReport.roadCar=car;bundleReport.roadDirection={fresh.direction.x,fresh.direction.y,fresh.direction.z};
        bundleReport.roadSourceAlignment=fresh.sourceAlignment;bundleReport.roadSourceLane=fresh.sourceLane;
        bundleReport.roadLaneCorrection=fresh.laneCorrection;
        pending={};bundleCarMorph.Attempted(now);bundleCarIntercept.Attempted(now);++bundleReport.morphAttempts;
        if(!Invoke(bundleJason.pawn.object,function,&packet)||packet.result!=1)return false;
        if(fresh.exitBlock){bundleRoads.blockedCar=occupancy.carSlot.object;bundleRoads.blockedExit=fresh.exitIndex;}
        cfgspawn::Vec3 actual{};
        bundleRoadTrace={};
        if(Location(bundleJason.pawn.object,actual)){
            bundleReport.roadActual=actual;bundleReport.roadLandingObserved=1;
            bundleRoadTrace.Begin(occupancy.carSlot.object,occupancy.carSlot.serial,{car.x,car.y,car.z},{actual.x,actual.y,actual.z},now);
        }
        ++bundleReport.morphReturns;bundleReport.carStage=10;bundleCarPursuit.Renew(now);
        // Donor stops the native pre-Morph path, not just our policy caches.
        // Stop owns its NEXT guarded callback; failed teleports never cancel.
        bundleVehicleActions.Select(occupancy.carSlot.object,occupancy.driver);
        bundleVehicleActions.Morphed();bundleCarObstacle.Reset();
        bundlePursuit.Clear();bundleFollowPath.Clear();bundlePathCooldown.Clear();bundleDoorRecovery={};bundleMoveAt=now+300;
        return true;
    }
#include "VehicleObstacleRuntimeMembers.hpp"
