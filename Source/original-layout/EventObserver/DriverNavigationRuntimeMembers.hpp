// Donor ResolveDriverDoorNavPoint/ResolveDriverSideDetourPoint adaptation.
// Cache the actual projected point; MoveTo's hidden reprojection must not
// disagree with the point used to measure arrival and send the seat input.
    bool DriverProject(cfgspawn::Vec3 query,float extent,cfgspawn::Vec3& out){
        std::uintptr_t function=0,type=0,system=0;cfgprobe::lifetime::SlotObservation slot{};
        if(!FeatureFunction(cfgvehicle::disk::Project,function,type)||
           !r_.Read(plan.world+cfgtrap::disk::WorldNavigationSystem,system)||!cfgspawn::ObjectIs(r_,system,type)||
           !cfgspawn::ObjectSlot(r_,context.objectArray,system,slot)||!slot.HasAssignedSerial())return false;
        cfgvehicle::Projection p;p.world=plan.world;p.point=query;p.extent={extent,extent,180};
        if(!Invoke(system,function,&p,false)||!cfgjason::SameSlot(r_,context.objectArray,slot)||
           !cfgobserver::Same(r_,plan.world+cfgtrap::disk::WorldNavigationSystem,system)||p.result!=1)return false;
        out=p.projected;return true;
    }
    bool DriverDoorPoint(std::uintptr_t car,std::uintptr_t seat,cfgspawn::Vec3 at,cfgspawn::Vec3& point,int carSerial,int seatSerial){
        auto& cache=bundleDriverNavigation;cache.Select(car,seat,at,carSerial,seatSerial);
        if(cache.car!=car||cache.seat!=seat||carSerial<=0||seatSerial<=0)return false;
        constexpr float radii[]{210.f,180.f,245.f};
        const float x=point.x-at.x,y=point.y-at.y,n=std::sqrt(x*x+y*y);
        if(!std::isfinite(n)||n<1.f)return false;
        if(cache.haveDoor&&(cache.direction.x*x/n+cache.direction.y*y/n<.98f)){
            cache.Clear();cache.Select(car,seat,at,carSerial,seatSerial);
        }
        if(cache.haveDoor){point=cache.door;return true;}
        const auto radius=radii[cache.radius++%3];
        const cfgspawn::Vec3 query{at.x+x/n*radius,at.y+y/n*radius,point.z};cfgspawn::Vec3 projected{};
        if(!DriverProject(query,120.f,projected)||!cache.Door(query,projected))return false;
        cache.direction={x/n,y/n,0};
        point=cache.door;cfgclosedtrace::Mark("driver-door-nav-projected",car,seat);return true;
    }
    bool DriverCornerPoint(cfgspawn::Vec3& point){
        auto& cache=bundleDriverNavigation;auto& route=bundleVehicleActions.route;
        if(!route.active||route.reached)return true;
        if(cache.haveCorner&&cfgdrivernav::Cache::Squared(point,cache.corner)<.01f){point=cache.corner;return true;}
        cfgspawn::Vec3 projected{};
        if(!DriverProject(point,160.f,projected)||!cache.Corner(point,projected))return false;
        point=projected;route.waypoint=projected; // donor measures the PROJECTED detour
        cfgclosedtrace::Mark("driver-corner-nav-projected",cache.car,cache.seat);return true;
    }
