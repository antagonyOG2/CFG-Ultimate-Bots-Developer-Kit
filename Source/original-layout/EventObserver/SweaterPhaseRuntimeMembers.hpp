    bool SweaterPhase(std::uint64_t now){
        const auto recordRefusal=[&](unsigned caller){if(bundleSweaterPolicy.stage==cfgsweater::Stage::Refused)
            cfgbundle::RecordSweaterFirstRefusal(bundleReport,caller,bundleSweaterPolicy.lastFailure,
                static_cast<unsigned>(bundleSweaterPolicy.stage),bundleSweaterOwner.object,now,bundleReport.sweaterFreshFailure);};
#ifdef CFG_H1_JASON
        HostedLaneEnter(cfgh1::Lane::Sweater);
#endif

        // Idle callbacks pay no native reads. A current-target change may
        // migrate the owner even if the previous wearer's policy retired;
        // all actual mutation paths still repeat SweaterContext proof.
        const bool staleRefusal=bundleSweaterPolicy.stage==cfgsweater::Stage::Refused&&bundleSweaterAbility.object&&
            bundleSweaterPolicy.lastFailure>=112&&bundleSweaterPolicy.lastFailure<=114;
        if(bundleSweaterOwner==bundleHumanTarget&&
           ((!bundleSweaterPolicy.CanPoll()&&!staleRefusal)||now<bundleSweaterPolicy.next))return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Sweater,1)
#else
 false
#endif
;
        if(!RefreshSweaterOwner(now))return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Sweater,2)
#else
 false
#endif
;
        if(staleRefusal){
            bundleSweaterPolicy.next=now<=UINT64_MAX-500?now+500:UINT64_MAX;
            if(RetireBoundSweaterAbility())RetryLateMask(now);
            return true;
        }
        // Donor kill-team health trigger is required AFTER arming too. Keep
        // its fresh-budget phase ahead of Due: a health-only turn must not
        // consume the next earned sweater/recharge opportunity.
        if(SweaterKillArmPhase(now,true))return true;
        if(!bundleSweaterPolicy.Due(now))return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Sweater,3)
#else
 false
#endif
;
        ++bundleReport.sweaterChecks;bool alive=false;std::uintptr_t pawn=0,current=0;
        unsigned char has=0;
        if(!Known(0,pawn)||pawn!=bundleSweaterOwner.object||!Alive(pawn,alive)||!alive)return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Sweater,4)
#else
 false
#endif
;
        if(!r_.Read(pawn+cfgfeature::disk::HasSweater,has)||has>1){
            bundleSweaterPolicy.Unavailable();++bundleReport.sweaterUnavailable;
            recordRefusal(301);
            bundleReport.sweaterStage=static_cast<unsigned>(bundleSweaterPolicy.stage);return true;
        }
        // Native pickup can retire its ability before HasBeenUsed is callable.
        // This sample belongs to the freshly proved same wearer, not that object.
        if(bundleSweaterPolicy.ObserveOwnership(now,has)){
            RecordSweaterUse(1);bundleReport.sweaterStage=static_cast<unsigned>(bundleSweaterPolicy.stage);return true;
        }
        if(!r_.Read(pawn+cfgfeature::disk::AbilityField,current)){
            bundleSweaterPolicy.Unavailable();++bundleReport.sweaterUnavailable;recordRefusal(302);return true;
        }
        // Native pickup wins over the retained mask ability. This is a fresh
        // pawn->ability->owner chain, not permission to resurrect the old one.
        const bool rebindCandidate=CFG_J25_SWEATER_REBIND&&current&&
            (has||(bundleSweaterPolicy.stage==cfgsweater::Stage::Waiting&&bundleSweaterPolicy.nativeOwnershipEarned))&&
            (bundleSweaterPolicy.stage==cfgsweater::Stage::Owned||bundleSweaterPolicy.stage==cfgsweater::Stage::Waiting||
             bundleSweaterPolicy.stage==cfgsweater::Stage::HudPending)&&
            (!bundleSweaterAbility.object||NativeSweaterReplacement(current));
        if((rebindCandidate||(has&&NativeSweaterReplacement(current)))&&
           bundleSweaterPolicy.stage!=cfgsweater::Stage::Refused&&!bundleSweaterPolicy.publicationStarted){
            const auto oldAbility=bundleSweaterAbility,oldSource=bundleSweaterSource;
            const auto oldIndex=bundleSweaterSourceIndex;
            unsigned char used=255;std::uintptr_t function=0;
            const bool okay=SweaterFunction(false,function,false)&&BindOwnedSweater(current)&&
                cfgobserver::Same(r_,pawn+cfgfeature::disk::AbilityField,current)&&
                cfgobserver::Same(r_,current+cfgfeature::disk::AbilityOwner,pawn)&&
                Invoke(current,function,&used,false)&&used<=1&&SweaterBinding(current)&&
                (rebindCandidate?bundleSweaterPolicy.RebindNative(now,used):bundleSweaterPolicy.ReplaceOwned(now,used));
            if(!okay){bundleSweaterAbility=oldAbility;bundleSweaterSource=oldSource;bundleSweaterSourceIndex=oldIndex;
                bundleSweaterPolicy.Unavailable();++bundleReport.sweaterUnavailable;recordRefusal(303);}
            else {bundleReport.sweaterBound=1;bundleSweaterPolicy.lastFailure=0;
                if(rebindCandidate){bundleSweaterFreshRequested=false;bundleSweaterFreshPending=false;}
                bundleSweaterClassPending=!bundleSweaterClassAttempted;}
            bundleReport.sweaterStage=static_cast<unsigned>(bundleSweaterPolicy.stage);return true;
        }
        const bool retired=(!current||current==bundleSweaterAbility.object)&&RetireBoundSweaterAbility();
        if(retired&&bundleSweaterPolicy.stage==cfgsweater::Stage::Owned&&has&&bundleSweaterPolicy.QueueRetiredOwned(now)){
            bundleSweaterClass={};bundleSweaterClassAttempted=false;bundleSweaterCursor=0;
            bundleReport.sweaterFreshClass=0;bundleReport.sweaterClassAnchor=0;
            bundleSweaterFreshRequested=false;return true;
        }
        // Already acquired sweaters must not wait behind a search for an item
        // native pickup has already removed from ItemList.
        bool preferMask=false;
#ifdef CFG_H1_JASON
        // A resident world sweater search must not postpone an already earned host mask for90s.
        if(HostedRoutineActive()&&bundleSweaterPolicy.stage==cfgsweater::Stage::Dormant&&!has&&!current){
            if(!HasPamelaTrigger(preferMask)){bundleSweaterPolicy.Unavailable();++bundleReport.sweaterUnavailable;recordRefusal(304);return true;}
        }
#endif
        if(bundleSweaterPolicy.stage==cfgsweater::Stage::Dormant&&!has&&!current&&!preferMask&&SweaterPickupPhase(now))return true;
        bool trigger=false;
        if(bundleSweaterPolicy.repairingOwned&&!bundleSweaterClassAttempted){
            // Owned-instance retirement is a different path from native use.
            // Discover on its own bounded callback BEFORE scheduling allocation;
            // otherwise Waiting would repeatedly expire an empty class lease.
            if(now>=bundleSweaterPolicy.deadline){bundleSweaterPolicy.lastFailure=104;bundleSweaterPolicy.Refuse();
                cfgbundle::RecordSweaterFirstRefusal(bundleReport,8,bundleSweaterPolicy.lastFailure,
                    static_cast<unsigned>(bundleSweaterPolicy.stage),bundleSweaterOwner.object,now,bundleReport.sweaterFreshFailure);return true;}
            const bool discovered=DiscoverSweater();
            if(discovered&&bundleSweaterClassAttempted)bundleSweaterFreshRequested=true;
            else {bundleSweaterPolicy.Unavailable();++bundleReport.sweaterUnavailable;recordRefusal(305);}
            bundleReport.sweaterStage=static_cast<unsigned>(bundleSweaterPolicy.stage);return true;
        }
        if(cfgsweater::AcceptOwnedDuringDiscovery(bundleSweaterPolicy.stage,bundleSweaterPolicy.searchExpired)){
            if(has&&current){
                if(!BindOwnedSweater(current)){bundleSweaterPolicy.Unavailable();++bundleReport.sweaterUnavailable;recordRefusal(306);
                    bundleReport.sweaterStage=static_cast<unsigned>(bundleSweaterPolicy.stage);return true;}
                if(!bundleSweaterPolicy.AdoptNativeOwned())return true;
                bundleReport.sweaterBound=1;
            }else if(!bundleSweaterPolicy.searchExpired&&!HasPamelaTrigger(trigger)){bundleSweaterPolicy.Unavailable();++bundleReport.sweaterUnavailable;recordRefusal(307);
                bundleReport.sweaterStage=static_cast<unsigned>(bundleSweaterPolicy.stage);return true;}
        }
        const auto action=bundleSweaterPolicy.Decide(now,trigger,has!=0,bundleSweaterAbility.object!=0);
        if((action==cfgsweater::Action::Publish||action==cfgsweater::Action::Observe)&&
           !bundleSweaterClassAttempted&&bundleReport.sweaterClassAttempts<8)
            bundleSweaterClassPending=true; // finite retries; observation/publication must still run
        if(action==cfgsweater::Action::Rearm&&!bundleSweaterFreshPending&&!SweaterBinding(bundleSweaterAbility.object)){
            bundleSweaterFreshRequested=true;bundleReport.sweaterStage=static_cast<unsigned>(bundleSweaterPolicy.stage);return true;
        }
        // One cold factory per independent phase. Publication can only use
        // already initialized caches, never combine cold factories with writes.
        if((action==cfgsweater::Action::Publish||action==cfgsweater::Action::Rearm)&&bundleSweaterWarm<2){
            std::uintptr_t function=0;
            if(!SweaterFunction(bundleSweaterWarm==1,function)){bundleSweaterPolicy.Unavailable();++bundleReport.sweaterUnavailable;recordRefusal(308);}
            else ++bundleSweaterWarm;
            bundleReport.sweaterStage=static_cast<unsigned>(bundleSweaterPolicy.stage);return true;
        }
        bool success=true;
        switch(action){
        case cfgsweater::Action::Discover:
            success=DiscoverSweater();
            if(success&&!bundleSweaterAbility.object&&(bundleSweaterClassAttempted||trigger)&&!bundleSweaterFreshGrantAttempted){
                // Cannot borrow AJ's worn sweater for Tommy. Only an independently
                // current, earned exact-class lease permits his own native object.
                bundleSweaterFreshRequested=true;
            }
            break;
        case cfgsweater::Action::Publish:success=PublishSweater(false,now);break;
        case cfgsweater::Action::Rearm:success=PublishSweater(true,now);break;
        case cfgsweater::Action::PublishHud:success=PublishSweaterHud();break;
        case cfgsweater::Action::Observe:{
            unsigned char used=0;std::uintptr_t function=0;
            success=SweaterBinding(bundleSweaterAbility.object);
            if(success){bundleSweaterPolicy.lastFailure=18;success=SweaterFunction(false,function,false)&&
                Invoke(bundleSweaterAbility.object,function,&used,false)&&used<=1;if(success)bundleSweaterPolicy.lastFailure=0;}
            if(success&&bundleSweaterPolicy.ObserveUsed(now,used))RecordSweaterUse(2);
            break;
        }
        default:break;
        }
        if(!success){if(action==cfgsweater::Action::PublishHud)bundleSweaterPolicy.hudProofFailure=bundleSweaterPolicy.lastFailure;
            bundleSweaterPolicy.Unavailable();++bundleReport.sweaterUnavailable;}
        if(bundleSweaterPolicy.stage==cfgsweater::Stage::Refused)
            cfgbundle::RecordSweaterFirstRefusal(bundleReport,action==cfgsweater::Action::None?1u:
                2u+static_cast<unsigned>(action),bundleSweaterPolicy.lastFailure,
                static_cast<unsigned>(bundleSweaterPolicy.stage),bundleSweaterOwner.object,now,bundleReport.sweaterFreshFailure);
        bundleReport.sweaterStage=static_cast<unsigned>(bundleSweaterPolicy.stage);
        return action!=cfgsweater::Action::None;
    }
