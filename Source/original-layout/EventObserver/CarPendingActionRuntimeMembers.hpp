    template<class Pending> bool CarPendingAction(const Pending& pending,std::uint64_t now){
        std::uintptr_t held=0;bool stunned=false,attacking=false;
        if(!CarActorBindings()||!Manager()||!Held(held)||
           !NativeBool(cfghunter::disk::Stunned,stunned)||!NativeBool(cfghunter::disk::Attacking,attacking))return false;
        if(held||stunned||attacking)return true;
        cfgescape::State alive;cfgspawn::Vec3 position{};
        if(!cfgjason::SameSlot(r_,context.objectArray,pending.carSlot)||!cfgjason::SameSlot(r_,context.objectArray,pending.driverSlot)||
           !CarSource(pending.source,pending.car)||
           !cfgescape::Counselor(r_,context.objectArray,plan.module,pending.driver,bundleCounselorClass,alive)||!cfgescape::Driving(alive)||
           !Location(pending.car,position))return false;
        const auto dx=position.x-pending.position.x,dy=position.y-pending.position.y;
        // A commitment is not input authority: the frozen interaction repeats
        // reciprocal occupancy/native stop/restart/lock proofs before routing.
        const bool committed=bundleVehicleActions.extractionCommitted&&
            bundleVehicleActions.car==pending.car&&bundleVehicleActions.driver==pending.driver;
        if(!pending.moving&&dx*dx+dy*dy>25.f*25.f&&!committed)return true;
        if(!CarInteraction(pending.car,pending.driver,position,pending.moving,now,false,pending.velocity))++bundleVehicleActions.refusals;
        return true;
    }
