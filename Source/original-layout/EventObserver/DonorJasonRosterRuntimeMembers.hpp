#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
    void DonorRecordRosterObservation(std::uintptr_t pawn,const cfgdonorhunt::CounselorObservation& value,
        bool observed,std::uint64_t now){
        if(!pawn)return;
        auto& rows=bundleReport.donorRoster;auto& count=bundleReport.donorRosterRows;
        unsigned i=0;for(;i<count&&rows[i].pawn!=pawn;++i){}
        if(i==rows.size()){
            // Reuse only a no-longer-registered diagnostic row, so Tommy/new
            // F3 births remain visible after many earlier counselors died.
            for(i=0;i<count;++i)if(bundleDonorRoster.roster.DonorFindJasonAITargetIndex(rows[i].pawn)<0)break;
            if(i==count)return;
        }else if(i==count)++count;
        auto& row=rows[i];if(row.pawn!=pawn)row={};
        row.pawn=pawn;row.at=now;
        row.kind=(row.kind==1||pawn==bundleDonorRoster.roster.LocalCounselorTarget||
            pawn==bundleHumanTarget.object||pawn==plan.objects[3].object)?1u:2u;
        for(unsigned b=0;b<bundleReport.bots&&b<bundlePawnSlots.size();++b)
            if(bundlePawnSlots[b].object==pawn){row.kind=3;break;}
        row.result=observed?0u:value.failure?value.failure:7u;
#if CFG_H248_SWEATER_TARGET
        if(!observed&&value.failure==1&&value.clause){
            const auto k=static_cast<unsigned>(i);
            bundleReport.h248RosterClause[k]=static_cast<std::uint8_t>(value.clause);
            bundleReport.h248RosterCtxRaw[k]=value.rawContext;bundleReport.h248RosterEscRaw[k]=value.rawEscaped;
            if(value.clause<bundleReport.h248ObserveClause.size())++bundleReport.h248ObserveClause[value.clause];
            if(r_.reads>=r_.cap&&bundleReport.h248ObserveBudgetFailures<UINT32_MAX)++bundleReport.h248ObserveBudgetFailures;
#if CFG_H248_TOMMY_SLOT
            if(value.clause==4){
                bundleReport.h248RosterSlotWhy[k]=value.slotWhy;
                if(value.slotWhy<bundleReport.h248SlotWhy.size())++bundleReport.h248SlotWhy[value.slotWhy];
                bundleReport.h248SlotFlags=value.slotFlags;bundleReport.h248SlotFreshSerial=value.slotFreshSerial;
                bundleReport.h248SlotExpectedSerial=value.slotExpectedSerial;
            }
#endif
        }
#endif
        row.live=observed&&value.Live()?1u:0u;row.escaped=observed&&value.escaped?1u:0u;
        if(row.live)++row.liveObservations;
        bundleReport.donorRosterLiveWhy[i]=static_cast<std::uint8_t>(!observed?32u:
            ((value.health>0?0u:1u)|(value.contextKill?2u:0u)|(value.escaped?4u:0u)|
             (value.possessed?0u:8u)|(value.controller.HasAssignedSerial()?0u:16u)));
        row.blocked=bundleDonorRoster.roster.DonorIsJasonAITargetBlocked(pawn,now)?1u:0u;
    }
    void DonorRosterRemove(std::uintptr_t pawn,bool confirmedDeath,std::uint64_t now,
        cfgprobe::lifetime::SlotObservation controller={}){
        auto& shared=bundleDonorRoster;const auto index=shared.roster.DonorFindJasonAITargetIndex(pawn);
        if(index<0)return;
        if(confirmedDeath){for(unsigned i=0;i<64;++i){auto& pending=shared.roster.pendingDeaths[i];
            if(pending.Pawn==pawn)break;
            if(!pending.Pawn){pending={pawn,now+15000};shared.pendingPawns[i]=shared.pawns[static_cast<unsigned>(index)];
                shared.pendingControllers[i]=controller;break;}}}
        const auto before=static_cast<unsigned>(shared.roster.count);
        shared.roster.DonorRemoveCounselorTarget(pawn);
        for(unsigned i=static_cast<unsigned>(index);i+1<before;++i)shared.pawns[i]=shared.pawns[i+1];
        shared.pawns[before-1]={};++shared.revision;
    }
    bool DonorRosterObserve(std::uintptr_t pawn,std::uint64_t now){
        auto& shared=bundleDonorRoster;cfgprobe::lifetime::SlotObservation slot{};cfgdonorhunt::CounselorObservation value;
        if(!pawn)return true;
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,pawn,slot)||!slot.HasAssignedSerial()||
           !cfgdonorhunt::ObserveDonorCounselor(r_,context.objectArray,plan.module,plan.world,slot,bundleCounselorClass,value)){
            DonorRecordRosterObservation(pawn,value,false,now);
            if(r_.reads>=r_.cap)return false;
            DonorRosterRemove(pawn,false,now);return true;
        }
        DonorRecordRosterObservation(pawn,value,true,now);
        if(!value.Live()){DonorRosterRemove(pawn,value.health<=0||value.contextKill,now,value.controller);return true;}
        const auto existing=shared.roster.DonorFindJasonAITargetIndex(pawn);
        if(existing>=0&&!(shared.pawns[static_cast<unsigned>(existing)]==slot))DonorRosterRemove(pawn,false,now);
        struct RegisterHost {std::uintptr_t actor;bool ReadableActor(std::uintptr_t value){return value==actor;}} host{pawn};
        const auto before=shared.roster.count;shared.roster.DonorRegisterJasonAITarget(host,pawn);
        const auto index=shared.roster.DonorFindJasonAITargetIndex(pawn);
        if(index>=0)shared.pawns[static_cast<unsigned>(index)]=slot;
        if(shared.roster.count!=before){++shared.revision;shared.roster.FinalCounselorDeathObserved=false;}
        unsigned controller=0;
        for(;controller<static_cast<unsigned>(shared.roster.controllerCount);++controller)
            if(shared.roster.controllers[controller]==value.controller.object)break;
        if(controller<64){
            if(controller==static_cast<unsigned>(shared.roster.controllerCount))
                shared.roster.controllers[static_cast<unsigned>(shared.roster.controllerCount++)]=value.controller.object;
            shared.controllers[controller]=value.controller;
        }
        return true;
    }
    bool DonorRosterPrune(std::uint64_t now){
        auto& shared=bundleDonorRoster;
        if(!shared.pruning){
            if(now<shared.roster.NextPruneAt)return true;
            shared.pruning=true;shared.pruneCursor=0;
        }
        while(shared.pruneCursor<static_cast<unsigned>(shared.roster.count)){
            if(r_.reads+256>=r_.cap)return false;
            const auto pawn=shared.roster.actors[shared.pruneCursor];
            if(!DonorRosterObserve(pawn,now))return false;
            // Removal compacts all three donor metadata arrays and slots.
            if(shared.pruneCursor<static_cast<unsigned>(shared.roster.count)&&
               shared.roster.actors[shared.pruneCursor]==pawn)++shared.pruneCursor;
        }
        if(!DonorRosterController(now))return false;
        shared.pruning=false;shared.roster.NextPruneAt=now+1000;
        bool pending=false;for(const auto& death:shared.roster.pendingDeaths)pending=pending||death.Pawn!=0;
        if(!shared.roster.count&&pending){
            // Literal final-victim guard: never unpossess the last controller
            // or repair its camera while the native ending takes ownership.
            shared.roster.pendingDeaths={};shared.pendingPawns={};shared.pendingControllers={};
            shared.roster.FinalCounselorDeathObserved=true;bundleDeadSpectatorRecovery={};
        }
        return true;
    }
    bool DonorRosterRetire(std::uint64_t now){
        auto& shared=bundleDonorRoster;if(!shared.roster.count)return false;
        for(unsigned i=0;i<64;++i){auto& death=shared.roster.pendingDeaths[i];
            if(!death.Pawn||now<death.ReadyAt||death.Pawn==bundleGrab.held)continue;
            if(r_.reads+512>=r_.cap)return true; // owed retirement, fresh callback
            const auto pawn=shared.pendingPawns[i],controller=shared.pendingControllers[i];
            cfgdonorhunt::CounselorObservation value;
            const auto consume=[&](){death={};shared.pendingPawns[i]={};shared.pendingControllers[i]={};};
            if(!pawn.HasAssignedSerial()||!controller.HasAssignedSerial()||controller.object==plan.controller||
               !cfgjason::SameSlot(r_,context.objectArray,pawn)||!cfgjason::SameSlot(r_,context.objectArray,controller)||
               !cfgspawn::ObjectIs(r_,controller.object,spawnAssets.resources.classes[cfgspawn::CounselorAI])||
               !cfgdonorhunt::ObserveDonorCounselor(r_,context.objectArray,plan.module,plan.world,pawn,bundleCounselorClass,value)){
                if(r_.reads>=r_.cap)return true;consume();continue;
            }
            if(!(value.controller==controller)||!value.possessed||(value.health>0&&!value.contextKill)){consume();continue;}
            if(bundleAdditional.actor.pawn.HasAssignedSerial()){
                std::uintptr_t held=0;
                if(!cfgjason::SameSlot(r_,context.objectArray,bundleAdditional.actor.pawn)||
                   !r_.Read(bundleAdditional.actor.pawn.object+cfghunter::disk::Held,held))return true;
                if(held==pawn.object)continue;
            }
            if(!AdditionalUnpossessBinding()||!RetainPrimaryObjects(r_,std::array<std::uintptr_t,2>{pawn.object,controller.object})||
               !cfgobserver::Same(r_,controller.object+cfgescape::disk::Pawn,pawn.object)||
               !cfgobserver::Same(r_,pawn.object+cfgescape::disk::Controller,controller.object))return true;
            bundleInvocationStage_=0;const bool returned=Invoke(controller.object,spawnAssets.functions[0],nullptr);
            if(bundleInvocationStage_>=5||returned){
                consume();cfgclosedtrace::Mark(returned?"donor-retired-dead-counselor":"donor-retirement-uncertain",pawn.object,controller.object);
            }
            return true; // never replay an entered uncertain UnPossess
        }
        return false;
    }
    bool DonorRosterLocal(std::uint64_t now){
        auto& shared=bundleDonorRoster;if(now<shared.localAt)return true;
        std::uintptr_t known=0;Known(0,known); // keep accepted terminal-mask/Tommy presentation bookkeeping
        std::uintptr_t pawn=0;
        if(!cfgjason::SameSlot(r_,context.objectArray,plan.objects[2])||
           !r_.Read(plan.controller+cfgescape::disk::Pawn,pawn))return false;
        if(!DonorRosterObserve(pawn,now))return false;
#if CFG_H243_ROSTER
        if(pawn){   // report only: did the controller's current pawn (Tommy after return) enter the one roster?
            auto& rep=bundleReport;
            if(pawn!=rep.donorLocalLastPawn){++rep.donorLocalPawnChanges;rep.donorLocalLastPawn=pawn;}
            const bool in=shared.roster.DonorFindJasonAITargetIndex(pawn)>=0;
            if(in)++rep.donorLocalAdmitted;else ++rep.donorLocalRejected;
            rep.donorLocalLastRegistered=in?1u:0u;rep.donorLocalLastResult=in?0u:99u; // 99 = no diagnostic row
            for(unsigned i=0;i<rep.donorRosterRows;++i)if(rep.donorRoster[i].pawn==pawn){rep.donorLocalLastResult=rep.donorRoster[i].result;break;}
            if(!in&&rep.donorLocalLastResult==99)++rep.donorLocalRowDropped;
        }
#endif
        const auto previous=shared.roster.LocalCounselorTarget;
        if(previous&&previous!=pawn)DonorRosterObserve(previous,now);
        shared.roster.LocalPlayerController=plan.controller;
        const auto index=shared.roster.DonorFindJasonAITargetIndex(pawn);
        shared.roster.LocalCounselorTarget=index>=0?pawn:0;
        if(CFG_J22_ALL_SOUND_BLIPS&&shared.roster.LocalCounselorTarget){
            // This exact replacement human was just observed through the same
            // typed roster/owner proof. Keep suppression valid after Tommy
            // possession without changing AI or touching counselor emitters.
            if(bundleSoundScope.active&&bundleSoundScope.humanController==plan.controller)bundleSoundScope.humanPawn=pawn;
            if(bundleAdditional.soundScope.active&&bundleAdditional.soundScope.humanController==plan.controller)
                bundleAdditional.soundScope.humanPawn=pawn;
        }
        if(shared.roster.LocalCounselorTarget&&previous!=pawn){
            shared.roster.LocalCounselorDeathEvidence=false;shared.roster.NextKillTeamTickAt=0;
        }
        shared.localAt=now+250;return true;
    }
    bool DonorRosterController(std::uint64_t now){
        auto& shared=bundleDonorRoster;const auto count=static_cast<unsigned>(shared.roster.controllerCount);
        if(!count||now<shared.controllersAt)return true;
        while(shared.controllerCursor<count){
            if(r_.reads+192>=r_.cap)return false;
            const auto index=shared.controllerCursor;const auto slot=shared.controllers[index];
            if(slot.HasAssignedSerial()&&cfgjason::SameSlot(r_,context.objectArray,slot)){
                std::uintptr_t pawn=0;if(!r_.Read(slot.object+cfgescape::disk::Pawn,pawn))return false;
                if(pawn&&shared.roster.DonorFindJasonAITargetIndex(pawn)<0&&!DonorRosterObserve(pawn,now))return false;
            }else if(r_.reads>=r_.cap)return false;
            ++shared.controllerCursor;
        }
        shared.controllerCursor=0;shared.controllersAt=now+1000;return true;
    }
    bool DonorRosterNativeList(std::uint64_t now){
        cfgkillerstart::Header list{},again{};
        if(!cfgobserver::Same(r_,plan.world+cfgprobe::WorldGameState,spawnAssets.gameState)||
           !r_.Read(spawnAssets.gameState+cfgescape::disk::PlayerCharacterList,list)||!list.Sane(64)||list.capacity>128){
#if CFG_H243_ROSTER
            ++bundleReport.donorNativeListRefusals; // report only
#endif
            return false;}
#if CFG_H243_ROSTER
        bundleReport.donorNativeListLastCount=static_cast<unsigned>(list.count);
#endif
        for(unsigned i=0;i<static_cast<unsigned>(list.count);++i){
            if(r_.reads+192>=r_.cap)return false;
            std::uintptr_t pawn=0,after=0;const auto cell=list.data+8*i;
            if(!r_.Read(cell,pawn)||!r_.Read(cell,after)||pawn!=after)return false;
            if(!pawn||pawn==bundleJason.pawn.object||pawn==bundleAdditional.actor.pawn.object||
               bundleDonorRoster.roster.DonorFindJasonAITargetIndex(pawn)>=0)continue;
            if(!DonorRosterObserve(pawn,now))return false;
        }
        return r_.Read(spawnAssets.gameState+cfgescape::disk::PlayerCharacterList,again)&&again==list;
    }
    bool DonorRosterSurvey(const cfgjasontick::Owner& owner,std::uint64_t now){
        if(now<bundleDonorSurveyBackoff.next)return false;
        J21Timing timing(cfgj21::Roster);
        const bool success=DonorRosterSurveyWork(owner,now);
        bundleDonorSurveyBackoff.Finish(now,success);
        // The donor's1s MAINTENANCE cadence must not turn a32-cell initial
        // census continuation into minutes of priming. Successful bounded
        // startup slices resume25ms later; failure still backs off250..2000ms.
        if(success&&!bundleDonorRoster.censusComplete)bundleDonorSurveyBackoff.next=now+25;
        bundleDonorRoster.maintenanceAt=bundleDonorSurveyBackoff.next;
        if(!success)++bundleReport.j21RosterFailures;
        return success;
    }
    bool DonorRosterSurveyWork(const cfgjasontick::Owner&,std::uint64_t now){
        auto& shared=bundleDonorRoster;
        // Read-only roster work may use either fully admitted callback. Never
        // require the native Tick publisher just to maintain weak observations.
        if(!NativeEnvironmentAllowed()||!cfgprimary::RetainedWorld(r_,plan,context))return false;
        if(shared.module!=plan.module||!(shared.world==plan.objects[0])){
            shared={};shared.module=plan.module;shared.world=plan.objects[0];shared.revision=1;
        }
        if(!DonorRosterLocal(now)||!DonorRosterPrune(now))return false;
        // Already committed births are cheap hints. The full native value
        // observer still proves type/world/possession before registration.
        if(bundleReport.bots&&bundleReport.bots<=cfgbundle::MaximumBots&&r_.reads+256<r_.cap){
            const unsigned i=shared.hintCursor%bundleReport.bots;
            std::uintptr_t known=0;Known(i+1,known);
            if(!DonorRosterObserve(bundlePawnSlots[i].object,now))return false;
            ++shared.hintCursor;
        }
        if(shared.censusComplete){
            if(!DonorRosterNativeList(now))return false;
            if(shared.roster.controllerCount>0&&!(CFG_J216_FINISHING&&!bundleJ21Lean.axe)){
                shared.fallback=false;shared.maintenanceAt=now+250;return true;}
            if(!shared.fallback&&now<shared.roster.NextRosterRefreshAt){shared.maintenanceAt=now+250;return true;}
        }
        cfgkillerstart::Header levels{};
        if(!r_.Read(plan.world+cfgradarworld::disk::WorldLevels,levels)||!levels.Sane(1024)||!levels.count)return false;
        if(!(levels==shared.levels)){shared.levels=levels;shared.level=shared.actor=0;shared.fallback=false;}
        if(shared.censusComplete&&!shared.fallback){
            // Donor's fallback scans ONE level per60s, only without known
            // controllers. Budget continuation never restarts that level.
            shared.fallback=true;shared.roster.NextRosterRefreshAt=now+60000;
            shared.level=shared.roster.RosterRefreshLevel%static_cast<unsigned>(levels.count);
            shared.roster.RosterRefreshLevel=shared.level+1;shared.actor=0;shared.fallbackEnd=shared.level+1;
        }
        const auto end=shared.fallback?shared.fallbackEnd:static_cast<unsigned>(levels.count);
        unsigned quota=32;
        while(quota&&shared.level<end&&shared.roster.count<64){
            if(r_.reads+256>=r_.cap)return true;
            std::uintptr_t level=0,levelClass=0;cfgkillerstart::Header actors{};
            cfgprobe::lifetime::SlotObservation levelSlot{};
            if(!r_.Read(levels.data+8*shared.level,level)||
               !NativeClass(cfgradarworld::disk::LevelClassCache,cfgradarworld::disk::LevelConstructorCache,levelClass)||
               !cfgspawn::ObjectIs(r_,level,levelClass)||!cfgspawn::ObjectSlot(r_,context.objectArray,level,levelSlot)||
               !cfgobserver::Same(r_,level+cfgkill::disk::OwningWorld,plan.world)||
               !r_.Read(level+cfgradarworld::disk::LevelActors,actors)||!actors.Sane(100000)){++shared.refusals;return false;}
            if(shared.actor>=static_cast<unsigned>(actors.count)){++shared.level;shared.actor=0;continue;}
            while(quota&&shared.actor<static_cast<unsigned>(actors.count)){
                if(r_.reads+256>=r_.cap)return true;
                std::uintptr_t actor=0,again=0;const auto cell=actors.data+8*shared.actor;
                if(!r_.Read(cell,actor)||!r_.Read(cell,again)||actor!=again){++shared.refusals;return false;}
                // Reuse this bounded resident LEVEL census; never a UObject
                // array scan or asset load when the Starting Weapon cache is empty.
                if(CFG_J21C_RESIDENT_WEAPONS&&actor&&(!bundleJ21Lean.axe||!bundleJ21Lean.machete)){
                    std::uintptr_t weaponBase=0,type=0;cfgprobe::lifetime::SlotObservation actorSlot{};
                    if(NativeClass(cfgconvergence::disk::SCWeaponReflection,cfgconvergence::disk::SCWeaponConstructor,weaponBase)&&
                       cfgspawn::ObjectIs(r_,actor,weaponBase)&&cfgspawn::ObjectSlot(r_,context.objectArray,actor,actorSlot)&&actorSlot.HasAssignedSerial()&&
                       cfgobserver::Same(r_,actor+0x20,level)&&r_.Read(actor+cfgprobe::ObjectClass,type)&&
                       cfgobserver::Same(r_,cell,actor)&&cfgjason::SameSlot(r_,context.objectArray,actorSlot))J21LeanWeaponHint(type);
                }
                if(actor&&cfgspawn::ObjectIs(r_,actor,bundleCounselorClass)&&!DonorRosterObserve(actor,now))return false;
                if(r_.reads>=r_.cap)return false;
                ++shared.actor;++shared.cells;--quota;
            }
            cfgkillerstart::Header current{};
            if(!r_.Read(level+cfgradarworld::disk::LevelActors,current)||!(current==actors)||
               !cfgjason::OwnedComponentSlot(r_,context.objectArray,levelSlot))return false;
        }
        cfgkillerstart::Header current{};
        if(!r_.Read(plan.world+cfgradarworld::disk::WorldLevels,current)||!(current==levels))return false;
        if(shared.level>=end||shared.roster.count>=64){
            shared.censusComplete=true;shared.fallback=false;shared.maintenanceAt=now+250;
            if(!shared.roster.NextRosterRefreshAt)shared.roster.NextRosterRefreshAt=now+60000;
        }
        return true;
    }
    cfgdonorframe::Source DonorRosterSource(const cfgjasontick::Owner& owner){
        cfgdonorframe::Source source;source.owner=owner;source.revision=bundleDonorRoster.revision;
        source.count=static_cast<unsigned>(bundleDonorRoster.roster.count);
        for(unsigned i=0;i<source.count&&i<64;++i)source.targets[i]=bundleDonorRoster.pawns[i];
        return source;
    }
    bool DonorExitGuardOwns(const cfgjasontick::Owner& owner,std::uint64_t now){
        const auto& guard=bundleEscapeGuard;
        if(!guard.target||guard.followFallback||!guard.guard.Guarding(now)||now<bundleSchedule.meleeUntil)return false;
        const auto index=bundleDonorRoster.roster.DonorFindJasonAITargetIndex(guard.target);
        if(index<0)return false;
        cfgspawn::Vec3 here{},there{};
        if(!DonorNativePosition(owner.actor.pawn,here)||
           !DonorNativePosition(bundleDonorRoster.pawns[static_cast<unsigned>(index)],there))return true;
        const float x=here.x-there.x,y=here.y-there.y,z=here.z-there.z;
        // Existing guard relinquishes motion inside400cm. Do not let its
        // ten-second hysteresis suppress the donor's close-range combat.
        return x*x+y*y+z*z>400.f*400.f;
    }
    bool DonorPublishPrimaryTarget(std::uintptr_t target){
        if(!target){bundleReport.target=0;return true;}
        if(target==bundleHumanTarget.object){bundleTargetIndex=0;bundleReport.target=target;return true;}
        for(unsigned i=0;i<bundleReport.bots&&i<cfgbundle::MaximumBots;++i)
            if(bundlePawnSlots[i].object==target){bundleTargetIndex=i+1;bundleReport.target=target;return true;}
        cfgnativepursuit::List list{};
        if(!r_.Read(spawnAssets.gameState+cfgescape::disk::PlayerCharacterList,list)||list.count<1||list.count>16||
           list.capacity<list.count||list.capacity>32)return false;
        for(unsigned i=0;i<static_cast<unsigned>(list.count);++i){std::uintptr_t pawn=0;
            if(!r_.Read(list.data+8*i,pawn))return false;if(pawn!=target)continue;
            cfgnativepursuit::Proof proof;
            if(!cfgnativepursuit::Observe(r_,context.objectArray,plan.module,spawnAssets.gameState,bundleCounselorClass,
                spawnAssets.resources.classes[cfgspawn::BaseAI],bundleJason.pawn.object,i,target,proof))return false;
            bundleNativeTarget=proof;bundleTargetIndex=BundleNativeTargetIndex;bundleReport.target=target;return true;
        }
        return false;
    }
#endif
