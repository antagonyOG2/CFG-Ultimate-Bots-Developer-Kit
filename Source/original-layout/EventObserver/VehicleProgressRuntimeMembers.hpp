    bool VehicleProgressPhase(std::uint64_t now){
        auto& recovery=bundleVehicleProgress;
        if(!recovery.pending||now<recovery.next)return false;
        recovery.next=now+100;auto& episode=recovery.cars[recovery.pending-1];
        // Separate callback before native drain/road continuations. Repeated
        // ownership proof can refuse, but cannot consume the action's budget.
        if(!Context()||!Jason()||bundleVehicleActions.StopOwnsEpisode()){
            recovery.pending=0;return true;
        }
        cfgprobe::lifetime::SlotObservation car{},driver{};cfgvehicleoccupancy::Proof occupancy{};
        const auto source=recovery.pending==2?cfgvehicle::disk::Car2:cfgvehicle::disk::Car4;
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,episode.car,car)||car.serial!=episode.serial||
           !cfgspawn::ObjectSlot(r_,context.objectArray,episode.driver,driver)||driver.serial!=episode.driverSerial||
           !CarSource(source,episode.car)||!CarOccupancy(episode.car,occupancy)||
           occupancy.result!=cfgvehicleoccupancy::Result::Occupied||occupancy.driver!=episode.driver){recovery.pending=0;return true;}
        std::uintptr_t candidate=0;bool alive=false;
        const auto index=recovery.cursor++;
        if(index>bundleReport.bots){recovery.pending=0;
            if(!CarOccupancyCurrent(occupancy)||!cfgjason::SameSlot(r_,context.objectArray,car)||
               !cfgjason::SameSlot(r_,context.objectArray,driver))return true;
            episode.SearchExhausted(now);
            cfgclosedtrace::Mark("car-progress-search-exhausted",episode.car,episode.deadline);return true;}
        if(!Known(index,candidate)||candidate==episode.driver||!Alive(candidate,alive)||!alive)return true;
        std::uintptr_t manager=0,lock=0,held=0;bool extraction=false;
        if(!Manager()||!Held(held)||held||!r_.Read(bundleJason.pawn.object+cfghunter::disk::Manager,manager)||
           !r_.Read(manager+cfgvehicleinteraction::disk::LockedComponent,lock)||lock||!CarExtraction(extraction)||extraction||
           !CarOccupancyCurrent(occupancy)||!cfgjason::SameSlot(r_,context.objectArray,car)||
           !cfgjason::SameSlot(r_,context.objectArray,driver))return true;
        if(!episode.BackOff(now,true,bundleVehicleActions.StopOwnsEpisode())){recovery.pending=0;return true;}
        CarRetire(now);bundleRoadMorph={};bundleCarActionPending={};
        bundleReport.target=candidate;bundleTargetIndex=index;bundleTargetLockUntil=now+15000;
        bundleFollowPath.Clear();bundlePathCooldown.Clear();bundlePursuit.Clear();bundleMoveAt=now;
        recovery.pending=0;
        cfgclosedtrace::Mark("car-measured-progress-backoff",episode.car,candidate);return true;
    }
