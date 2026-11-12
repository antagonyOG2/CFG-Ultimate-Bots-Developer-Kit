// Narrow wrapper around the byte-for-byte frozen stop/extraction adapters.
// Native reciprocal occupancy, locks, driver-side route and input eligibility
// remain in those adapters. No forced engine/seat/possession or collision writes.
    bool StuckCarIdentity(std::uintptr_t car,std::uintptr_t driver,cfgprobe::lifetime::SlotObservation& c,
        cfgprobe::lifetime::SlotObservation& d){
        return cfgspawn::ObjectSlot(r_,context.objectArray,car,c)&&c.HasAssignedSerial()&&
            cfgspawn::ObjectSlot(r_,context.objectArray,driver,d)&&d.HasAssignedSerial();
    }
    bool CarStopBeforeRoad(const cfgvehicleoccupancy::Proof& proof,cfgspawn::Vec3 position,
        cfgspawn::Vec3 jason,cfgspawn::Vec3 velocity,std::uint64_t now,bool& owns){
        auto& state=bundleVehicleActions;const auto car=proof.carSlot.object;
        cfgprobe::lifetime::SlotObservation c{},d{};
        const bool identity=StuckCarIdentity(car,proof.driver,c,d);auto* clock=bundleStuckCars.Find(car);
        if(identity&&clock&&clock->Ready(car,c.serial,proof.driver,d.serial,position,now))velocity={};
        if(!NativeCarStopBeforeRoad(proof,position,jason,velocity,now,owns))return false;
        if(state.car!=car||state.driver!=proof.driver||!identity||!clock)return true;
        const bool stalled=clock->Observe(car,c.serial,proof.driver,d.serial,position,now);
        const float x=position.x-jason.x,y=position.y-jason.y;
        if(stalled&&x*x+y*y<=1500.f*1500.f&&!state.extractionCommitted){
            state.extractionCommitted=true;state.committedAt=now;state.phase=0;state.hoodAt=0;
            state.haveHoodGoal=state.haveDoorGoal=false;state.route.Reset();owns=true;
            cfgclosedtrace::Mark("car-stationary-driver-extraction",car,proof.driver);
        }
        return true;
    }
    bool CarInteraction(std::uintptr_t car,std::uintptr_t driver,cfgspawn::Vec3 position,bool moving,
        std::uint64_t now,bool occupancyOnly=false,cfgspawn::Vec3 velocity={}){
        cfgprobe::lifetime::SlotObservation c{},d{};
        // A fresh measured stalled-car lease is the movement observation. The
        // frozen adapter still reads actual engine/hood/slam flags itself.
        auto* clock=bundleStuckCars.Find(car);
        if(!occupancyOnly&&clock&&bundleVehicleActions.extractionCommitted&&
           bundleVehicleActions.car==car&&bundleVehicleActions.driver==driver&&
           StuckCarIdentity(car,driver,c,d)&&clock->Ready(car,c.serial,driver,d.serial,position,now)){
            moving=false;velocity={};
        }
        return NativeCarInteraction(car,driver,position,moving,now,occupancyOnly,velocity);
    }
