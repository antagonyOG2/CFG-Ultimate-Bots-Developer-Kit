    bool ConvergenceFearPhase(){
        if(cfgj21::ConvergenceEnabled&&DonorHeldDecisionInstalled())return false;
        if(cfgj21::LeanEnabled)return false;
        if(!bundleConvergenceFearPending)return false;
        const auto index=bundleConvergenceFearPending-1;bundleConvergenceFearPending=0;
        std::uintptr_t pawn=0,current=0;cfgconvergencefear::Evidence e;
        if(!bundleConvergence.active||index>=bundleReport.bots||!bundleConvergence.bots[index].published||
            !Known(index+1,pawn)||!cfgconvergencefear::Capture(r_,context.objectArray,plan.module,pawn,e)||
            !Known(index+1,current)||current!=pawn||!cfgconvergencefear::Current(r_,context.objectArray,plan.module,e)){
            ++bundleConvergenceFearRefused;return true;
        }
        // Same donor gameplay field, not the separate manual-hook No Fear toggle.
        if(e.amount!=0){
            if(CarSpeedWrite(e.manager.object+cfghook::disk::FearValue,e.amount,0.f))++bundleConvergenceFearWrites;
            else ++bundleConvergenceFearRefused;
        }
        return true; // independent fresh-budget phase; no navigation/native call
    }
    bool TravelIdentity(unsigned index,bool restore){
        if(index>=bundleReport.bots)return false;
        auto& t=bundleConvergenceTravel[index];
        const auto controller=bundleReport.roster[index].controller;
        cfgprobe::lifetime::SlotObservation slot{};
        if(controller!=bundleControllerSlots[index].object||
           !cfgconvergencetravel::Brain(r_,context.objectArray,bundleControllerSlots[index],
               spawnAssets.resources.classes[cfgspawn::CounselorAI],spawnAssets.resources.classes[cfgspawn::Brain],slot)){
            ++bundleReport.travelIdentityRefused;return false;
        }
        if(restore||t.policy.owned){
            return slot==t.brain&&t.controller==bundleControllerSlots[index]&&
                cfgobserver::Same(r_,controller+0x418,t.brain.object);
        }
        t.brain=slot;t.controller=bundleControllerSlots[index];return true;
    }
    bool TravelTick(unsigned index,bool set,bool& enabled){
        namespace d=cfgconvergencetravel::disk;
        if(!TravelIdentity(index,set&&bundleConvergenceTravel[index].policy.owned))return false;
        std::uint8_t value=enabled?1:0;
        const auto brain=bundleConvergenceTravel[index].brain.object;
        const bool ok=set?
            CarCall(d::SetComponentTickEnabled,d::SetComponentTickEnabledExecutor,d::SetComponentTickEnabledCode,brain,&value):
            CarCall(d::IsComponentTickEnabled,d::IsComponentTickEnabledExecutor,d::IsComponentTickEnabledCode,brain,&value);
        if(!ok||!TravelIdentity(index,true)||value>1)return false;
        enabled=value!=0;return true;
    }
    bool TravelResume(unsigned index,std::uint64_t now,bool recover){
        auto& t=bundleConvergenceTravel[index];if(!t.policy.owned)return true;
        if(!TravelIdentity(index,true)){t={};++bundleTravelLost;return true;} // never adopt a replacement brain
        bool enabled=false;
        if(!TravelTick(index,false,enabled))return false;
        if(!enabled){enabled=true;if(!TravelTick(index,true,enabled))return false;}
        t.policy.Resumed(now,recover);++bundleTravelResumes;return true;
    }
    bool TravelCleanup(std::uint64_t now){
#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
        if(cfgj21::ConvergenceEnabled&&DonorHeldDecisionInstalled()&&DonorConvergenceCleanup())return true;
#endif
        for(unsigned i=0;i<bundleReport.bots;++i)if(bundleConvergenceTravel[i].policy.owned){
            if(!Context()){bundleConvergenceTravel[i]={};++bundleTravelLost;return true;}
            TravelResume(i,now,false);return true; // one original brain per callback, before cinematic ownership
        }
        return false;
    }
    bool TravelMove(unsigned index,cfgspawn::Vec3 goal){
        auto p=cfgconvergencetravel::MovePacket(goal);
        // Donor uses a 100cm world-location path, not MoveToActor's additional
        // capsule-overlap radius (which could stop outside the220cm release).
        // Native projected/pathfinding MoveToLocation is already disk-pinned.
        bundleReport.travelMoveFailure=1;bundleReport.travelMoveNativeResult=255;
        if(!cfgspawn::Finite(goal)||!TravelIdentity(index,true))return false;
        bundleReport.travelMoveFailure=2;
        if(!CarCall(cfgtrap::disk::MoveToLocation,cfgtrap::disk::MoveToLocationExecutor,
            cfgtrap::disk::MoveToLocationCode,bundleReport.roster[index].controller,&p))return false;
        bundleReport.travelMoveNativeResult=p.result;bundleReport.travelMoveFailure=3;
        if(!p.result||p.result>2)return false;
        bundleReport.travelMoveFailure=4;
        if(!TravelIdentity(index,true))return false;
        bundleReport.travelMoveFailure=0;return true;
    }
    bool ConvergenceTravel(std::uint64_t now){
        if(cfgj21::ConvergenceEnabled&&DonorHeldDecisionInstalled())return false;
        if(cfgj21::LeanEnabled&&!bundleJ21LeanDriving)return false;
        if(!bundleConvergence.active||!bundleReport.bots||now<bundleTravelAt)return false;
        // Alternate complete, fresh-budget callbacks with publication; a long publication cannot starve travel.
        bundleTravelTurn=!bundleTravelTurn;if(!bundleTravelTurn)return false;
        bundleTravelAt=now+333;
        const unsigned index=bundleTravelCursor++%bundleReport.bots;
        if(BundleEmoteHeld(index))return true;
        auto& t=bundleConvergenceTravel[index];
        std::uintptr_t pawn=0;const bool eligible=bundleConvergence.bots[index].published&&Known(index+1,pawn);
        if(!eligible){if(t.policy.owned)TravelResume(index,now,false);return true;}
        cfgspawn::Vec3 here{},goal{};
        if(!Location(pawn,here)||!Location(bundleJason.pawn.object,goal))return true;
        const float x=here.x-goal.x,y=here.y-goal.y;
        const float distance=std::sqrt(x*x+y*y);if(!std::isfinite(distance))return true;
        const auto action=t.policy.Decide(distance,now,true,here.x,here.y,goal.x,goal.y);
        using A=cfgconvergencetravel::Action;
        if(action==A::Acquire){
            bool enabled=false;
            if(!TravelTick(index,false,enabled)){++bundleReport.travelTickRefused;return true;}
            if(!enabled){++bundleReport.travelAlreadyDisabled;return true;} // do not claim an externally disabled brain
            enabled=false;
            // Retain cleanup ownership even if post-dispatch verification refuses.
            t.policy.Acquired(distance,now,here.x,here.y);
            if(TravelTick(index,true,enabled)){++bundleTravelAcquires;bundleTravelCursor=index;}
            else {++bundleReport.travelTickRefused;TravelResume(index,now,true);}
            return true; // movement has its own next callback
        }
        if(action==A::Resume||action==A::Recover){TravelResume(index,now,action==A::Recover);return true;}
        if(action==A::Move){
            if(TravelMove(index,goal)){t.policy.Moved(now,goal.x,goal.y);++bundleTravelMoves;}
            else {++bundleReport.travelMoveRefused;TravelResume(index,now,true);}
        }
        return true;
    }
