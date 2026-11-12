// J23 B-LEAN-only near path. Accepted legacy travel remains byte-identical.
    bool J23ConvergenceTravel(std::uint64_t now){
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
        if(CFG_J23_BOT_NEAR_FOLLOW&&cfgj21::LeanEnabled){auto& d=bundleReport.j23Arrival[index];
            if(d.pawn&&d.pawn!=pawn)d={};
            d.at=now;d.pawn=pawn;d.controller=bundleReport.roster[index].controller;d.distance=distance;}
        const auto action=t.policy.Decide(distance,now,true,here.x,here.y,goal.x,goal.y,
            CFG_J23_BOT_NEAR_FOLLOW&&cfgj21::LeanEnabled);
        using A=cfgconvergencetravel::Action;
        if(action==A::Follow){
            auto& d=bundleReport.j23Arrival[index];std::uintptr_t weapon=0,type=0;
            d.weaponKnown=d.held=d.brainKnown=0;d.weaponClass=0;
            if(!r_.Read(pawn+cfgloadout::restricteddisk::EquippedItem,weapon)){++bundleReport.j23NearFollowRefused;return true;}
            d.weapon=weapon;d.weaponAt=now;d.weaponKnown=1;
            if(!weapon||!HeldWeapon(pawn,weapon)||!r_.Read(weapon+cfgprobe::ObjectClass,type))return true;
            d.held=1;d.weaponClass=type; // actual hand/owner proof, not inventory membership
            bool enabled=false;
            if(!TravelTick(index,false,enabled)){++bundleReport.j23NearFollowRefused;return true;}
            d.brainKnown=1;d.brainAt=now;d.brainEnabled=enabled?1u:0u;
            if(!enabled){++bundleReport.j23NearFollowDisabled;return true;} // never claim somebody else's disabled brain
            // Same projected native location path as travel, with the bot's
            // native brain still ON. No synthetic attack/human controller.
            t.policy.FollowAttempt(now); // failed/ambiguous Move cannot spam the next optional turn
            if(TravelMove(index,goal)){t.policy.Followed(now,goal.x,goal.y);++bundleTravelMoves;++bundleReport.j23NearFollowMoves;++d.nearMoves;}
            else ++bundleReport.j23NearFollowRefused;
            return true;
        }
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
