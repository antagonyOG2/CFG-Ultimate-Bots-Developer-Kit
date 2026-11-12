// Included inside the guarded vehicle access and the focused read-only fixture.
    bool CarStopBeforeRoad(const cfgvehicleoccupancy::Proof& proof,cfgspawn::Vec3 carPosition,
        cfgspawn::Vec3 jasonPosition,cfgspawn::Vec3 velocity,std::uint64_t now,bool& stopOwns){
        stopOwns=false;auto& state=bundleVehicleActions;
        if(proof.result!=cfgvehicleoccupancy::Result::Occupied||!cfgspawn::Finite(carPosition)||
           !cfgspawn::Finite(jasonPosition)||!cfgspawn::Finite(velocity))return false;
        const auto car=proof.carSlot.object;std::uintptr_t hood=0;
        bool started=false,enabled=false,slammed=false;float doorSpeed=0,slamSpeed=0;
        if(!r_.Read(car+cfgvehicleinteraction::disk::JasonCarInteractComponent,hood)||
           (hood&&!cfgobserver::Same(r_,hood+0xE0,car))||
           !CarStopObservation(car,hood,started,enabled,slammed,doorSpeed,slamSpeed)||!CarOccupancyCurrent(proof))return false;
        // A different occupied-car survey must not erase an extraction lease.
        if(state.car&&state.car!=car&&state.StopOwnsEpisode()){stopOwns=true;return true;}
        state.Select(car,proof.driver);
        const float dx=carPosition.x-jasonPosition.x,dy=carPosition.y-jasonPosition.y;
        const float speed=std::sqrt(velocity.x*velocity.x+velocity.y*velocity.y);
        if(!state.ObserveStop(started,enabled,slammed,speed,doorSpeed,slamSpeed,dx*dx+dy*dy,now))return false;
        stopOwns=state.StopOwnsEpisode()||slammed||!started;
        return true;
    }
