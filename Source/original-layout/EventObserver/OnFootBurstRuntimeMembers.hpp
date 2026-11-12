    bool FootBurstMovement(){
        const auto& binding=cfghook::disk::Types[cfghook::disk::Movement];std::uintptr_t movement=0,type=0;
        cfgprobe::lifetime::SlotObservation slot{};
        return !bundleCarSpeed.active&&Context()&&Jason()&&
            r_.Read(bundleJason.pawn.object+cfghook::disk::MovementComponent,movement)&&
            NativeClass(binding.reflection,binding.constructor,type)&&cfgspawn::ObjectIs(r_,movement,type)&&
            cfgobserver::Same(r_,movement+0x20,bundleJason.pawn.object)&&
            cfgspawn::ObjectSlot(r_,context.objectArray,movement,slot)&&slot.HasAssignedSerial()&&
            ((!bundleFootBurst.Owned()&&(bundleFootMovement=slot,true))||slot==bundleFootMovement);
    }
    bool FootBurstField(unsigned index,float expected,float desired,bool restoring){
        static constexpr std::uintptr_t fields[]{cfghook::disk::Walk,cfghook::disk::Sprint,cfghook::disk::Run,cfghook::disk::SlowRun};
        float current=0;
        if(index>=4||!FootBurstMovement()||!r_.Read(bundleFootMovement.object+fields[index],current)||!std::isfinite(current))return false;
        if(restoring&&current!=expected)return true; // foreign later value is not ours
        if(current!=expected||!cfgprimary::LookupSafe(r_,plan.module)||
           !cfgjason::SameSlot(r_,context.objectArray,bundleFootMovement)||
           !cfgobserver::Same(r_,bundleJason.pawn.object+cfghook::disk::MovementComponent,bundleFootMovement.object)||
           !cfgobserver::Same(r_,plan.viewport+cfgprobe::ViewportWorld,plan.world))return false;
        return CarSpeedWrite(bundleFootMovement.object+fields[index],expected,desired);
    }
    bool FootBurstService(std::uint64_t now,bool interrupt){
        struct Host {BundleAccess& a;
            bool Exchange(unsigned i,float before,float after){return a.FootBurstField(i,before,after,false);}
            bool Restore(unsigned i,float before,float after){return a.FootBurstField(i,before,after,true);}
        }host{*this};
        return bundleFootBurst.Step(host,now,interrupt);
    }
    bool FootBurstObserve(std::array<float,4>& values){
                std::uintptr_t target=0,seat=0,water=0,held=0;float squared=0;bool alive=false,stunned=false,attacking=false,busy=false;
                if(!FootBurstMovement()||!Known(bundleTargetIndex,target)||target!=bundleReport.target||
                   !Alive(target,alive)||!alive||!Held(held)||held||!Distance(target,squared)||
                   squared<400.f*400.f||(!CFG_H267_BURST_ANY_RANGE&&squared>2000.f*2000.f)||!std::isfinite(squared)||
                   !r_.Read(target+cfgvehicleoccupancy::disk::CurrentSeat,seat)||seat||
                   !r_.Read(bundleJason.pawn.object+cfgswim::disk::WaterVolume,water)||water||
                   !cfgbundle::Code(r_,plan.module,cfghunter::disk::Stunned,cfghunter::disk::StunnedBytes)||
                   !cfgbundle::Code(r_,plan.module,cfghunter::disk::Attacking,cfghunter::disk::AttackingBytes)||
                   !NativeBool(cfghunter::disk::Stunned,stunned)||stunned||
                   !NativeBool(cfghunter::disk::Attacking,attacking)||attacking||!GrabBusy(busy)||busy)return false;
                bundleFootBurstSquared=squared; // H2.90 (audit #2): chase recovery uses the distance proved at start, not at the request
                static constexpr std::uintptr_t fields[]{cfghook::disk::Walk,cfghook::disk::Sprint,cfghook::disk::Run,cfghook::disk::SlowRun};
                for(unsigned i=0;i<4;++i)if(!r_.Read(bundleFootMovement.object+fields[i],values[i]))return false;
                return FootBurstMovement();
    }
    bool FootBurstBegin(std::uint64_t now,bool underLease=false){
        if(bundleFootBurst.Owned()||now<bundleFootBurst.readyAt)return false;
        struct Host {BundleAccess& a;
            bool Observe(std::array<float,4>& values){return a.FootBurstObserve(values);}
        }host{*this};
        bundleFootBurstSquared=-1.f; // unknown until Observe proves the distance (the request-time value already gated admission)
        const bool began=bundleFootBurst.Begin(host,now,bundleReport.target,underLease,&bundleFootBurstSquared);
        bundleReport.h285Starts[0]=bundleFootBurst.starts;bundleReport.h285ChaseStarts[0]=bundleFootBurst.chaseStarts;return began;
    }
    bool FootBurstVehicleTurn(std::uint64_t now){
        if(!bundleFootBurst.Owned())return false;
        // An empty periodic vehicle poll is not evidence of a car chase. Keep
        // a freshly proved on-foot burst for its bounded3s duration. No car
        // action/boost runs until all owned movement fields are restored.
        std::array<float,4> values{};
        if(bundleFootBurst.stage==cfgfootburst::Stage::Hold&&now<bundleFootBurst.until&&
           FootBurstObserve(values))return true;
        FootBurstService(now,true);return true;
    }
