    bool HeldRecoveryTargetAllowed(std::uintptr_t pawn,std::uint64_t now){
        if(pawn!=bundleHeldRecovery.released||now>=bundleHeldRecovery.quarantineUntil)return true;
        cfgprobe::lifetime::SlotObservation slot{};
        return cfgspawn::ObjectSlot(r_,context.objectArray,pawn,slot)&&bundleHeldRecovery.Allowed(pawn,slot.serial,now);
    }
    bool HeldRecoveryPhase(std::uint64_t now){
        auto& s=bundleHeldRecovery;if(now<s.next)return false;s.next=now+250;
        std::uintptr_t held=0,extraction=0;
        if(!Context()||!Jason()||!Held(held)||!r_.Read(bundleJason.pawn.object+cfgaction::disk::ExtractionComponent,extraction))return true;
        if(!held&&s.phase==cfgholdrecovery::Phase::Unlock){
            if(!s.car&&!s.lock){s.phase=cfgholdrecovery::Phase::Confirm;return true;}
            std::uintptr_t manager=0,locked=0,type=0;cfgprobe::lifetime::SlotObservation victim{},car{};cfgescape::State state{};bool killing=false;
            if(!s.car||!cfgspawn::ObjectSlot(r_,context.objectArray,s.victim,victim)||victim.serial!=s.serial||
               !cfgspawn::ObjectSlot(r_,context.objectArray,s.car,car)||car.serial!=s.carSerial||
               !cfgescape::Counselor(r_,context.objectArray,plan.module,s.victim,bundleCounselorClass,state)||
               cfgescape::Terminal(state)||state==cfgescape::State::Unknown||!HidingSafe()||
               !GrabBool(cfggrab::disk::IsGrabKilling,cfggrab::disk::IsGrabKillingExecutor,cfggrab::disk::IsGrabKillingCode,killing)||killing||
               !Manager()||!r_.Read(bundleJason.pawn.object+cfghunter::disk::Manager,manager)||
               !r_.Read(manager+cfgvehicleinteraction::disk::LockedComponent,locked)){s.Refuse();return true;}
            if(!locked){s.phase=cfgholdrecovery::Phase::Confirm;return true;}
            if(locked!=s.lock||!NativeClass(cfgvehicleinteraction::disk::SCVehicleSeatComponent,
                cfgvehicleinteraction::disk::SCVehicleSeatComponentConstructor,type)||!cfgspawn::ObjectIs(r_,locked,type)||
               !cfgobserver::Same(r_,locked+cfgvehicleinteraction::disk::ParentVehicle,s.car)||
               (!CarSource(cfgvehicle::disk::Car4,s.car)&&!CarSource(cfgvehicle::disk::Car2,s.car))){s.Refuse();return true;}
            s.phase=cfgholdrecovery::Phase::Confirm;
            if(!CarCall(cfgaction::disk::CLIENT_UnlockInteraction,cfgaction::disk::CLIENT_UnlockInteractionExecutor,
                cfgaction::disk::CLIENT_UnlockInteractionCode,manager,&locked))s.Refuse();
            return true; // owed unlock must not disappear when Cancel clears Held first
        }
        if(!held){
            // Absence alone is not successful detachment. A live extraction
            // component still owns Jason and cannot be retired on this hint.
            if(s.victim&&!extraction&&s.attemptAt){
                std::uintptr_t manager=0,lock=0;
                if(Manager()&&r_.Read(bundleJason.pawn.object+cfghunter::disk::Manager,manager)&&
                   r_.Read(manager+cfgvehicleinteraction::disk::LockedComponent,lock)&&!lock&&HidingSafe()){
                    bool retrySeated=false;
                    if(s.car){
                        cfgvehicleoccupancy::Proof proof{};std::uintptr_t currentSeat=0;
                        if(!CarOccupancy(s.car,proof)||proof.carSlot.serial!=s.carSerial||
                           !CarOccupancyCurrent(proof)||!r_.Read(s.victim+cfgvehicleoccupancy::disk::CurrentSeat,currentSeat))return true;
                        retrySeated=proof.result==cfgvehicleoccupancy::Result::Occupied&&proof.driver==s.victim&&currentSeat==proof.seat;
                        if(!retrySeated){
                            if(proof.result!=cfgvehicleoccupancy::Result::Empty||proof.driver||currentSeat)return true;
                            for(const auto& seat:proof.entries)if(seat.occupant==s.victim)return true;
                        }
                    }
                    s.Released(now);
                    if(retrySeated){bundleVehicleActions.RetryDriverApproach();bundleCarActionPending={};bundleRoadMorph={};bundleCarPursuit.Renew(now);}
                    else CarRetire(now);
                    bundleGrab.Observe(0,now);bundleSchedule.selectAt=now;
                    cfgclosedtrace::Mark("stale-held-native-release-confirmed",s.released,s.confirmed);
                }
            }
            if(!extraction&&!s.attemptAt){s.victim=s.car=0;s.serial=s.carSerial=0;s.since=s.inputAt=0;s.phase=cfgholdrecovery::Phase::Observe;}
            if(s.attemptAt&&now>=s.attemptAt+2000&&s.phase!=cfgholdrecovery::Phase::Refused){
                s.Refuse();cfgclosedtrace::Mark("stale-held-native-cleanup-unconfirmed",s.victim,Reads());}
            return true; // this observation spent a fresh scope; never stack normal work on its budget
        }
        cfgprobe::lifetime::SlotObservation victim{},carSlot{};cfgescape::State state{};
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,held,victim)||!victim.HasAssignedSerial()||
           !cfgescape::Counselor(r_,context.objectArray,plan.module,held,bundleCounselorClass,state)||
           state==cfgescape::State::Unknown||cfgescape::Terminal(state))return true; // preserve dead victim montage
        std::uintptr_t seat=0,car=0,manager=0,lock=0;
        if(!r_.Read(held+cfgvehicleoccupancy::disk::CurrentSeat,seat))return true;
        cfgvehicleoccupancy::Proof occupancy{};
        if(seat){std::uintptr_t type=0;
            if(!NativeClass(cfgvehicleinteraction::disk::SCVehicleSeatComponent,cfgvehicleinteraction::disk::SCVehicleSeatComponentConstructor,type)||
               !cfgspawn::ObjectIs(r_,seat,type)||!r_.Read(seat+cfgvehicleinteraction::disk::ParentVehicle,car)||
               (car!=bundleVehicleActions.car)||!car||!cfgspawn::ObjectSlot(r_,context.objectArray,car,carSlot)||
               !carSlot.HasAssignedSerial()||(!CarSource(cfgvehicle::disk::Car4,car)&&!CarSource(cfgvehicle::disk::Car2,car))||
               !CarOccupancy(car,occupancy)||occupancy.result!=cfgvehicleoccupancy::Result::Occupied||
               occupancy.driver!=held||occupancy.seat!=seat)return true;
        }else if(extraction){
            // Held pointer detached, but the paired native extraction has not
            // relinquished its component. No ordinary grab kill or raw clear.
            return true;
        }
        if(!seat&&!extraction&&s.car&&bundleVehicleActions.driver==held){
            cfgvehicleoccupancy::Proof detached{};
            if(!CarOccupancy(s.car,detached)||detached.result!=cfgvehicleoccupancy::Result::Empty||detached.driver)return true;
            for(const auto& entry:detached.entries)if(entry.occupant==held)return true;
            if(!CarOccupancyCurrent(detached)||!cfgobserver::Same(r_,held+cfgvehicleoccupancy::disk::CurrentSeat,std::uintptr_t{}))return true;
            CarRetire(now);s.car=0;s.carSerial=0;
            cfgclosedtrace::Mark("extraction-seat-detachment-confirmed",held,Reads());return true;
        }
        if(!seat&&!extraction&&DonorHeldDecisionInstalled())return true; // preserve detached-car bookkeeping above; D owns ordinary holding below
        if(!s.Observe(held,victim.serial,car,carSlot.serial,now))return true;
        bool killing=false,allowed=false;
        if(!GrabBool(cfggrab::disk::IsGrabKilling,cfggrab::disk::IsGrabKillingExecutor,cfggrab::disk::IsGrabKillingCode,killing)||
           !GrabBool(cfggrab::disk::CanGrabKill,cfggrab::disk::CanGrabKillExecutor,cfggrab::disk::CanGrabKillCode,allowed)||
           killing||!HidingSafe())return true;
        if(bundleGrab.committed&&bundleGrab.held==held&&!s.inputAt)s.Input(held,now);
        if(s.phase==cfgholdrecovery::Phase::Refused)return true; // one cancellation transaction per positive hold identity
        if(!Manager()||!r_.Read(bundleJason.pawn.object+cfghunter::disk::Manager,manager)||
           !r_.Read(manager+cfgvehicleinteraction::disk::LockedComponent,lock))return true;
        // Only the exact car seat lock may be released. Ordinary holding is
        // admitted with NO manager lock; never cancel an unrelated animation.
        if(lock&&(!seat||lock!=seat))return true;
        if(s.phase==cfgholdrecovery::Phase::Unlock){
            if(lock&&lock!=s.lock){s.Refuse();return true;}
            if(lock){std::uintptr_t component=lock;s.phase=cfgholdrecovery::Phase::Confirm;
                if(!CarCall(cfgaction::disk::CLIENT_UnlockInteraction,cfgaction::disk::CLIENT_UnlockInteractionExecutor,
                    cfgaction::disk::CLIENT_UnlockInteractionCode,manager,&component))s.Refuse();
            }else s.phase=cfgholdrecovery::Phase::Confirm;
            return true;
        }
        if(s.phase==cfgholdrecovery::Phase::Confirm){
            if(now>=s.attemptAt+2000){s.Refuse();cfgclosedtrace::Mark("stale-held-native-cancel-unconfirmed",held,Reads());}
            return true;
        }
        if(!s.Stale(now,seat!=0,allowed,bundleGrab.probed>=4,killing,false))return true;
        if(!cfgjason::SameSlot(r_,context.objectArray,victim)||!cfgobserver::Same(r_,bundleJason.pawn.object+cfghunter::disk::Held,held)||
           !cfgobserver::Same(r_,held+cfgvehicleoccupancy::disk::CurrentSeat,seat)||
           !cfgobserver::Same(r_,manager+cfgvehicleinteraction::disk::LockedComponent,lock)||
           (seat&&!CarOccupancyCurrent(occupancy)))return true;
        s.Attempt(lock,now); // consume before dispatch, even if native result is ambiguous
        if(!CarCall(cfgaction::disk::CLIENT_CancelInteractAttempt,cfgaction::disk::CLIENT_CancelInteractAttemptExecutor,
            cfgaction::disk::CLIENT_CancelInteractAttemptCode,bundleJason.pawn.object,nullptr))s.Refuse();
        cfgclosedtrace::Mark(seat?"partial-extraction-native-cancel":"stale-held-native-cancel",held,Reads());return true;
    }
