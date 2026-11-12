// Included in BundleAccess. Reuses the accepted typed schema, travel policy
// and reviewed allocating Observe/Grant/Verify gateway; no object-array scan.
#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
    void J21LeanWeaponHint(std::uintptr_t type){
        if(!type)return;cfgnames::Text name;
        std::uintptr_t weaponBase=0;cfgprobe::lifetime::SlotObservation slot{};
        if(CFG_J21C_RESIDENT_WEAPONS&&(!NativeClass(cfgconvergence::disk::SCWeaponReflection,cfgconvergence::disk::SCWeaponConstructor,weaponBase)||
            !cfgspawn::ClassContains(r_,type,weaponBase)||!cfgspawn::ObjectSlot(r_,context.objectArray,type,slot)||!slot.HasAssignedSerial()))return;
        if(!J21ClassName(r_,type,name))return;
        if(CFG_J216_FINISHING&&(name.View()==u"CounselorTwoHandedAxe_C"||name.View()==u"CounselorMachete_C")&&
           !(bundleJ21Lean.world==plan.objects[0])){bundleJ21Lean={};bundleJ21Lean.world=plan.objects[0];}
        if(name.View()==u"CounselorTwoHandedAxe_C"){bundleJ21Lean.axe=type;bundleJ21Lean.axeClass=slot;}
        if(name.View()==u"CounselorMachete_C"){bundleJ21Lean.machete=type;bundleJ21Lean.macheteClass=slot;}
    }
    bool J21LeanStrong(std::uintptr_t pawn){
        // Preference only, never bot identity/admission. The donor's exact
        // strong-class classification; names are world/positive-slot cached.
        std::uintptr_t type=0;if(!r_.Read(pawn+cfgprobe::ObjectClass,type))return false;
        for(unsigned depth=0;type&&depth<8;++depth){cfgnames::Text name;if(!J21ClassName(r_,type,name))return false;
            char text[160]{};for(unsigned i=0;i<name.count;++i)text[i]=name.data[i]<128?static_cast<char>(name.data[i]):'?';
            for(const auto* token:{"Jock","Tough","Biker","Tommy","Hero","Rob","Julius","Boxer","Mark"})
                if(std::strstr(text,token))return true;
            if(!r_.Read(type+cfgprobe::ClassSuper,type))return false;
        }return false;
    }
    bool J21LeanCleanup(const cfgjasontick::Owner& owner,std::uint64_t now){
        if(!J21LeanOwedCleanup())return false;
        if(!(owner.actor.pawn==bundleJason.pawn)||!DonorNativeOwner(owner))return false;
        const auto before=bundleTravelResumes;
        const bool worked=TravelCleanup(now);
        bundleJ21Lean.counters.cleanup+=bundleTravelResumes-before;bundleReport.j21Lean=bundleJ21Lean.counters;
        return worked;
    }
    bool J21LeanTravelSlice(unsigned index,std::uint64_t now){
        const auto moves=bundleTravelMoves;
        bundleTravelAt=0;bundleTravelCursor=index;bundleTravelTurn=false;
        if(CFG_J23_BOT_NEAR_FOLLOW)J23ConvergenceTravel(now);else ConvergenceTravel(now);
        bundleJ21Lean.counters.moves+=bundleTravelMoves-moves;
        if(bundleConvergenceTravel[index].policy.owned&&!bundleConvergenceTravel[index].policy.submitted)return true;
        bundleJ21Lean.schedule.Finish(now);return true;
    }
    bool J21LeanConvergence(const cfgjasontick::Owner& owner,std::uint64_t now){
#ifdef CFG_H1_JASON
            HostedLaneEnter(cfgh1::Lane::Convergence);
#endif

        if(!J21LeanDue(owner,now))return false; // one timestamp/latch compare before reads
        J21Timing timing(cfgj21::Lean,true);auto& lean=bundleJ21Lean;auto& schedule=lean.schedule;
        schedule.Continue(now);
        if(!DonorNativeOwner(owner)||!bundleSchedule.active)return false;
        if(!(lean.world==plan.objects[0])){lean={};lean.world=plan.objects[0];lean.schedule.Continue(now);}
        ++lean.counters.passes;const auto callsBefore=Calls();
        struct ReportScope {BundleAccess& a;unsigned before;~ReportScope(){
            auto& state=bundleJ21Lean;state.counters.calls+=a.Calls()-before;
            state.counters.peakReads=(std::max)(state.counters.peakReads,static_cast<std::uint64_t>(a.Reads()));
            if(a.Reads()>=cfgbundle::ReadCap)++state.counters.readBudget;
            bundleReport.j21Lean=state.counters;bundleReport.j21LeanBot=state.schedule.current;
            bundleReport.j21LeanPhase=state.schedule.phase;bundleReport.j21LeanKey=state.schedule.key;
        }} reportScope{*this,callsBefore};
        J21LeanDrivingScope driving;
        if(bundleConvergencePrepared<ConvergenceFunctions.size()||
           bundleReport.convergenceKeys<cfgconvergence::KeyCount||bundleBlackboardClassIndex<4){
            ++lean.counters.warmSlices;DonorConvergenceWarm();return true;
        }
        if(!bundleReport.bots||bundleReport.bots>cfgconvergence::MaximumBots){schedule.Finish(now);return true;}
        if(schedule.current>=bundleReport.bots){schedule.current=schedule.cursor++%bundleReport.bots;schedule.phase=schedule.key=0;}
        const auto index=schedule.current;auto& bot=lean.bots[index];
        if(schedule.phase==4){
            std::uintptr_t current=0;cfgprobe::lifetime::SlotObservation bb{};
            if(!(bot.pawn==bundlePawnSlots[index])||!(bot.controller==bundleControllerSlots[index])||
               !r_.Read(bot.controller.object+cfgconvergence::disk::ControllerBlackboard,current)||
               !cfgspawn::ObjectSlot(r_,context.objectArray,current,bb)||!(bb==bot.blackboard)){
                schedule.phase=0;return true;
            }
            // The accepted travel gateway re-proves the helper/brain/controller;
            // do not re-scan all9 key descriptors for a movement-only slice.
            return J21LeanTravelSlice(index,now);
        }
        std::uintptr_t pawn=0,blackboard=0;cfgprobe::lifetime::SlotObservation slot{};
        if(BundleEmoteHeld(index)||!Blackboard(index,pawn,blackboard,slot)||pawn==plan.pawn||pawn==bundleHumanTarget.object||
           DonorConvergencePostGrab(pawn,now)){
            ++lean.counters.refused;schedule.Finish(now);return true;
        }
        if(!(bot.pawn==bundlePawnSlots[index])||!(bot.controller==bundleControllerSlots[index])||!(bot.blackboard==slot)){
            bot={};bot.pawn=bundlePawnSlots[index];bot.controller=bundleControllerSlots[index];bot.blackboard=slot;
            if(CFG_J23_BOT_NEAR_FOLLOW){bundleReport.j23Arrival[index]={};bundleReport.j23Arrival[index].pawn=pawn;
                bundleReport.j23Arrival[index].controller=bot.controller.object;}
            bundleBlackboardSchemas[index].Reset();schedule.phase=schedule.key=0;
        }
        auto& schema=bundleBlackboardSchemas[index];
        if(CFG_J21C_LEAN_SLICES&&!J21HasBudget(1800))return true;
        const auto state=schema.Step(r_,context.objectArray,plan.module,blackboard,slot,bundleConvergenceNames);
        if(state!=cfgconvergence::BlackboardSchema::State::Ready){
            if(r_.reads>=r_.cap)schema.Reset();
            else if(state==cfgconvergence::BlackboardSchema::State::Unavailable){++lean.counters.refused;schedule.Finish(now);}
            return true;
        }
        if(schedule.phase==0){schedule.phase=1;return true;}
        if(schedule.phase==5){
            // A normally returned setter is NEVER replayed while its readback
            // is owed. Separate callbacks keep a setter/getter pair from
            // stacking two allocating/native retention paths in one slice.
            const auto key=bot.pendingKey;if(key>=cfgconvergence::KeyCount){++lean.counters.refused;schedule.Finish(now);return true;}
            bool returned=false,confirmed=false;std::uintptr_t observed=0;bundleInvocationStage_=0;
            if(key<=cfgconvergence::Jason){cfgconvergence::ObjectPacket p{bundleConvergenceNames[key],0};
                returned=ConvergenceInvoke(1,blackboard,&p);observed=p.value;confirmed=returned&&p.value==bot.pendingValue;
            }else{cfgconvergence::BoolPacket p{bundleConvergenceNames[key]};p.value=2;
                returned=ConvergenceInvoke(2,blackboard,&p);observed=p.value;confirmed=returned&&p.value==bot.pendingValue;}
            if(CFG_J23_BOT_NEAR_FOLLOW&&returned){auto& d=bundleReport.j23Arrival[index];
                d.values[key]=observed;d.readAt[key]=now;d.knownKeys|=1u<<key;}
            if(!returned&&bundleInvocationStage_<5)return true; // clearly unentered GETTER only may retry
            if(confirmed){bot.cache.Confirm(key,bot.pendingValue,now);++lean.counters.publications;}
            else{bot.cache.Quarantine(key);++lean.counters.uncertain;}
            bot.pendingKey=cfgconvergence::KeyCount;bot.pendingValue=0;schedule.key=key+1;schedule.phase=3;return true;
        }
        if(schedule.phase==1){
            // Numeric flee threshold is not mapped. Use the native blackboard
            // flee decision as the gate; never invent a fear-unit constant.
            if(now>=bot.fearAt&&schema.Supports(cfgconvergence::Flee)){
                bot.fearAt=now+1000;cfgconvergence::BoolPacket packet{bundleConvergenceNames[cfgconvergence::Flee]};packet.value=2;
                cfgconvergencefear::Evidence fear;
                if(ConvergenceInvoke(2,blackboard,&packet)&&packet.value==1&&
                   cfgconvergencefear::Capture(r_,context.objectArray,plan.module,pawn,fear)&&fear.amount>0&&
                   cfgconvergencefear::Current(r_,context.objectArray,plan.module,fear)&&
                   CarSpeedWrite(fear.manager.object+cfghook::disk::FearValue,fear.amount,0.f))++lean.counters.fearWrites;
            }schedule.phase=2;return true;
        }
        if(schedule.phase==2){
            std::uintptr_t weapon=0,type=0;
            if(!r_.Read(pawn+cfgloadout::restricteddisk::EquippedItem,weapon)||(weapon&&!HeldWeapon(pawn,weapon))){
                ++lean.counters.refused;schedule.Finish(now);return true;}
            if(weapon){if(!r_.Read(weapon+cfgprobe::ObjectClass,type))return true;J21LeanWeaponHint(type);}
            J21LeanWeaponHint(bundleExpandedLoadout.report.type); // existing loaded Starting Weapon cache only
            if(CFG_J21C_RESIDENT_WEAPONS){
                if(lean.axe&&!cfgjason::SameSlot(r_,context.objectArray,lean.axeClass)){lean.axe=0;lean.axeClass={};}
                if(lean.machete&&!cfgjason::SameSlot(r_,context.objectArray,lean.macheteClass)){lean.machete=0;lean.macheteClass={};}
            }
            auto desired=CFG_J216_FINISHING?cfgpractice::ConvergenceWeapon(lean.axe,lean.machete,bundleReport.j215Practice):
                (J21LeanStrong(pawn)?lean.axe:lean.machete);
            if(!CFG_J216_FINISHING&&!desired)desired=lean.machete?lean.machete:lean.axe;
            if(!weapon||(desired&&type!=desired)){
                if(desired){bundleDonorConvergence.grantThisTurn=false;
                    #ifdef CFG_H1_JASON
                    HostedLaneEnter(cfgh1::Lane::Axes);
#endif
                    if(DonorConvergenceGrant(pawn,desired)){++lean.counters.grants;
                        if(CFG_J216_FINISHING){if(desired==lean.axe)++bundleReport.j215Practice.axeGrants;
                            else ++bundleReport.j215Practice.macheteFallbackGrants;}
                        // Re-observe in the next bounded slice. Never replay an
                        // ambiguous grant; the existing gateway quarantines it.
                        return true;}
                    if(CFG_H247_MOB){++bundleReport.h247Mob.grantRefused;++bundleReport.h247Mob.grantStage[(std::min)(bundleDonorConvergence.grantStage,7u)];}
                }else ++lean.counters.missingWeaponClass;
            }
            bot.weapon=weapon;schedule.phase=3;schedule.key=0;return true;
        }
        if(schedule.phase==3){
            // A selection is not permission to write: re-prove current held
            // weapon and typed schema in THIS callback, before each pair.
            std::uintptr_t weapon=0;if(!r_.Read(pawn+cfgloadout::restricteddisk::EquippedItem,weapon)||
                (weapon&&!HeldWeapon(pawn,weapon)))return true; // schema.Step already proved Current; no UE call intervened
            if(weapon!=bot.weapon){bot.weapon=weapon;schedule.key=0;}
            unsigned pairs=0;
            while(schedule.key<cfgconvergence::KeyCount){const auto key=schedule.key;
                if(pairs&&(pairs>=(CFG_J21C_LEAN_SLICES?1u:2u)||r_.reads+512>=r_.cap||!J21HasBudget(1000)))return true;
                if(CFG_J21C_LEAN_SLICES&&!J21HasBudget(1800))return true;
                if(pairs){std::uintptr_t freshPawn=0,freshBoard=0;cfgprobe::lifetime::SlotObservation freshSlot{};
                    if(!Blackboard(index,freshPawn,freshBoard,freshSlot)||freshPawn!=pawn||freshBoard!=blackboard||
                       !(freshSlot==slot)||!schema.Current(r_,context.objectArray))return true; // native pair invalidated earlier observations
                }
                if(bot.cache.quarantined&(1u<<key)){++schedule.key;continue;}
                if(!schema.Supports(key)||(key==cfgconvergence::Weapon&&!weapon)){++schedule.key;continue;}
                const bool armed=weapon!=0;
                if(CFG_H247_MOB&&!armed){
                    // Donor: an unarmed bot stays on its stock tree; Clear (flee=true) only if it was fighting.
                    const bool was=(bot.cache.confirmed&(1u<<cfgconvergence::Fight))&&bot.cache.value[cfgconvergence::Fight]!=0;
                    if(cfglean::MobDecide(false,false,false,true,was)==cfglean::MobStep::Hold){++bundleReport.h247Mob.mobHeld;schedule.Finish(now);return true;}
                    ++bundleReport.h247Mob.mobReleased;
                }
                const auto action=armed?cfgconvergence::Action::Publish:cfgconvergence::Action::Clear;
                const std::uintptr_t wanted=key==cfgconvergence::Weapon?weapon:key==cfgconvergence::Jason?bundleJason.pawn.object:
                    cfgconvergence::Desired(static_cast<cfgconvergence::Key>(key),action)?1u:0u;
                if(!bot.cache.Needs(key,wanted,now,slot.HasAssignedSerial())){++lean.counters.skipped;++schedule.key;continue;}
                if(!slot.HasAssignedSerial())++lean.counters.zeroSerialCache;
                bool set=false,confirmed=false;bundleInvocationStage_=0;
                if(key<=cfgconvergence::Jason){cfgconvergence::ObjectPacket p{bundleConvergenceNames[key],wanted};
                    if(!schema.AcceptsObject(r_,key,wanted))return true;
                    set=ConvergenceInvoke(3,blackboard,&p);const bool entered=bundleInvocationStage_>=5;
                    if(set&&CFG_J21C_LEAN_SLICES){bot.pendingKey=key;bot.pendingValue=wanted;schedule.phase=5;return true;}
                    if(set){p.value=0;const bool got=ConvergenceInvoke(1,blackboard,&p);confirmed=got&&p.value==wanted;
                        if(CFG_J23_BOT_NEAR_FOLLOW&&got){auto& d=bundleReport.j23Arrival[index];d.values[key]=p.value;d.readAt[key]=now;d.knownKeys|=1u<<key;}}
                    if(!confirmed&&entered){bot.cache.Quarantine(key);++lean.counters.uncertain;++schedule.key;return true;}
                }else{cfgconvergence::BoolPacket p{bundleConvergenceNames[key]};p.value=static_cast<std::uint8_t>(wanted);
                    set=ConvergenceInvoke(4,blackboard,&p);const bool entered=bundleInvocationStage_>=5;
                    if(set&&CFG_J21C_LEAN_SLICES){bot.pendingKey=key;bot.pendingValue=wanted;schedule.phase=5;return true;}
                    if(set){p.value=2;const bool got=ConvergenceInvoke(2,blackboard,&p);confirmed=got&&p.value==wanted;
                        if(CFG_J23_BOT_NEAR_FOLLOW&&got){auto& d=bundleReport.j23Arrival[index];d.values[key]=p.value;d.readAt[key]=now;d.knownKeys|=1u<<key;}}
                    if(!confirmed&&entered){bot.cache.Quarantine(key);++lean.counters.uncertain;++schedule.key;return true;}
                }
                if(confirmed){bot.cache.Confirm(key,wanted,now);++lean.counters.publications;++schedule.key;++pairs;}
                else return true; // bounded max2 pairs; entered ambiguity is never replayed
            }
            const unsigned required=schema.Mask()&~bot.cache.quarantined;
            bundleConvergence.active=true;
            bundleConvergence.bots[index].published=weapon&&(bot.cache.confirmed&required)==required&&bot.cache.quarantined==0;
            schedule.phase=4;return true;
        }
        ++lean.counters.refused;schedule.Finish(now);return true; // unknown phase: no native work
    }
#endif
