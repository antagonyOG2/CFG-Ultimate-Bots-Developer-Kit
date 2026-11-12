    bool AdditionalWorld(){
#ifdef CFG_H1_JASON
        if(HostedRoutineActive()){
            return HostedContext(false)&&
                cfgjason::SameSlot(r_,context.objectArray,bundleJason.pawn)&&
                cfgjason::SameSlot(r_,context.objectArray,bundleJason.killerController)&&
                cfgobserver::Same(r_,bundleJason.pawn.object+0x3A0,bundleJason.killerController.object)&&
                cfgobserver::Same(r_,bundleJason.killerController.object+0x370,bundleJason.pawn.object);
        }
#endif
        return GetCurrentThreadId()==plan.threadId&&originalActorDispatcher&&cfgprimary::LookupSafe(r_,plan.module)&&
            cfgprimary::RetainedOfflineWorld(r_,plan,context)&&
            cfgobserver::Same(r_,plan.world+cfgprobe::WorldGameState,spawnAssets.gameState)&&
            cfgjason::SameSlot(r_,context.objectArray,spawnAssets.primary.stateSlot)&&
            cfgjason::SameSlot(r_,context.objectArray,bundleJason.pawn)&&
            cfgjason::SameSlot(r_,context.objectArray,bundleJason.killerController)&&
            cfgobserver::Same(r_,bundleJason.pawn.object+0x3A0,bundleJason.killerController.object)&&
            cfgobserver::Same(r_,bundleJason.killerController.object+0x370,bundleJason.pawn.object);
    }
#ifdef CFG_H1_JASON
    // Donor FrozenJasonBridge.cpp:24539 (CanRegisterHostedLobbyJason) and
    // OfflineSetup.cpp:10412 preflight the hosted F1 transaction before birth.
    bool HostedF1CanRegisterJason(){
        if(!HostedRoutineActive())return true;
        return HostedContext(false)&&Jason()&&AdditionalInProgress()&&
            !bundleAdditional.journal.captured&&!bundleAdditional.actor.pawn.object&&
            bundleAdditional.policy.stage==cfgadditional::Stage::Select;
    }
    // Donor FrozenJasonBridge.cpp:24509 (RegisterHostedLobbyJason), called
    // by OfflineSetup.cpp:10696 after the native transaction commits. This
    // existing J2.19 F1 actor is admitted only after its linked identity proof.
    bool HostedF1RegisterJason(){
        if(!HostedRoutineActive())return true;
        auto& extra=bundleAdditional;
        cfgprobe::lifetime::SlotObservation pawn{},controller{};
        if(!HostedContext(false)||!AdditionalLinked()||!AdditionalPrimary()||
           !cfgspawn::ObjectSlot(r_,context.objectArray,extra.actor.pawn.object,pawn)||
           !(pawn==extra.actor.pawn)||!pawn.HasAssignedSerial()||
           !cfgspawn::ObjectSlot(r_,context.objectArray,extra.actor.killerController.object,controller)||
           !(controller==extra.actor.killerController)||!controller.HasAssignedSerial()||
           !cfghosted::native::WorldOwned(r_,extra.actor.pawn.object,plan.world)||
           !cfghosted::native::WorldOwned(r_,extra.actor.killerController.object,plan.world))return false;
        // rc1 sets these two native-controller flags after verifying the
        // reciprocal SCKillerCharacter/SCKillerAIController links. Preserve
        // unrelated bits and use same-value/compare-exchange writes only.
        const auto setBits=[&](std::uintptr_t address,std::uint8_t mask){
            std::uint8_t value=0;
            if(!r_.Read(address,value))return false;
            const auto desired=static_cast<std::uint8_t>(value|mask);
            if(value!=desired&&static_cast<std::uint8_t>(_InterlockedCompareExchange8(
                reinterpret_cast<volatile CHAR*>(address),static_cast<CHAR>(desired),static_cast<CHAR>(value)))!=value)return false;
            std::uint8_t verify=0;return r_.Read(address,verify)&&(verify&mask)==mask;
        };
        const bool registered=NativeCallAllowed()&&setBits(extra.actor.killerController.object+0x34,0x02)&&
            setBits(extra.actor.killerController.object+0x408,0x10)&&
            HostedContext(false)&&AdditionalLinked()&&AdditionalPrimary();
        if(registered)hostedExtraPresentationPending=true;
        return registered;
    }
#endif
    bool AdditionalInProgress(){
        cfgnames::Words phase{};cfgnames::Text text;cfgnames::Observation<Reader> names(r_,plan.module);
        return r_.Read(plan.mode+cfgescape::disk::MatchState,phase)&&names.Decode(phase,text)&&text.View()==u"InProgress";
    }
    bool AdditionalPawn(){
        const auto& t=bundleAdditional.selected;const auto& e=bundleAdditional.actor;
        return e.pawn.object!=bundleJason.pawn.object&&
            cfgjason::NewPawn(r_,context.objectArray,e,plan,plan.pawn,t.pawnClass,t.defaults,spawnAssets.killerBase);
    }
    bool AdditionalLinked(){
        const auto& t=bundleAdditional.selected;const auto& e=bundleAdditional.actor;
        std::uintptr_t state=0;
        return e.pawn.object!=bundleJason.pawn.object&&e.killerController.object!=bundleJason.killerController.object&&
            cfgjason::LinkedKiller(r_,context.objectArray,e,plan,plan.pawn,t.pawnClass,t.defaults,
                spawnAssets.killerBase,spawnAssets.resources.classes[cfgspawn::KillerAI],spawnAssets.classes[3],plan.controller)&&
            AdditionalControllerState(e.killerController,state)&&state==bundleAdditional.journal.killerState&&
            cfgobserver::Same(r_,e.pawn.object+0x388,state);
    }
    bool AdditionalReadPair(cfgadditionaljournal::Pair& pair){
        return AdditionalWorld()&&r_.Read(spawnAssets.gameState+cfgprimary::CurrentKiller,pair.pawn)&&
            r_.Read(spawnAssets.gameState+cfgprimary::CurrentKillerState,pair.state);
    }
    bool AdditionalPrimary(){
        cfgadditionaljournal::Pair pair{};const auto& j=bundleAdditional.journal;
        if(!AdditionalReadPair(pair)){cfgclosedtrace::Mark("f1-primary-pair-read-refused",spawnAssets.gameState,Reads());
#ifdef CFG_H1_JASON
            cfgclosedtrace::Mark("f1-hosted-context-clause",plan.pawn,hostedContextClause_);
#endif
            return false;}
        if(!j.Primary(pair)){
            cfgclosedtrace::Mark("f1-primary-pawn-mismatch",pair.pawn,j.primary.pawn);
            cfgclosedtrace::Mark("f1-primary-state-observed",pair.state,j.primary.state);return false;
        }
        const bool owner=cfgobserver::Same(r_,bundleJason.killerController.object+0x388,j.primary.state);
        if(!owner)cfgclosedtrace::Mark("f1-primary-controller-state-refused",bundleJason.killerController.object,j.primary.state);
        return owner;
    }
    bool AdditionalLifecycle(){
        using namespace cfgadditionallifecycle::disk;
        std::uintptr_t table=0;std::int32_t added=-1,removed=-1;
        return AdditionalWorld()&&r_.Read(spawnAssets.gameState,table)&&
            cfgobserver::Same(r_,table+0x6F0,plan.module+Add)&&cfgobserver::Same(r_,table+0x6F8,plan.module+Remove)&&
            cfgbundle::Code(r_,plan.module,Add,AddCode)&&cfgbundle::Code(r_,plan.module,Remove,RemoveCode)&&
            cfgbundle::Code(r_,plan.module,DispatchEmpty,DispatchEmptyCode)&&
            cfgbundle::Code(r_,plan.module,AddedDelegate,AddedDelegateCode)&&
            cfgbundle::Code(r_,plan.module,RemovedDelegate,RemovedDelegateCode)&&
            r_.Read(spawnAssets.gameState+0x3E0,added)&&added==0&&
            r_.Read(spawnAssets.gameState+0x3F0,removed)&&removed==0;
    }
    bool AdditionalTemplate(){
        auto& s=bundleAdditional;cfgspawn::TemplateObservation fresh{};
#ifndef CFG_H251_HOSTED_F1
#define CFG_H251_HOSTED_F1 1
#endif
#if CFG_H251_HOSTED_F1
        // H2.51: hosted F1 with no captured catalogue selected the adopted Jason's own
        // resident class (not a catalogue entry); re-prove it is still exactly that class.
        std::uintptr_t ownClass=0;
        const bool entryCurrent=(CFG_H261_HOSTED_RANDOM_JASON&&s.hostedRandom)?(r_.Read(bundleJason.pawn.object+cfgprobe::ObjectClass,ownClass)&&ownClass&&ownClass!=s.selected.pawnClass):s.hostedOwnClass?
            (r_.Read(bundleJason.pawn.object+cfgprobe::ObjectClass,ownClass)&&ownClass==s.selected.pawnClass):
            (s.selectedIndex<spawnAssets.primary.entries.size()&&
             cfgprimary::SameEntry(r_,plan.mode,spawnAssets.primary,s.selectedIndex));
#else
        const bool entryCurrent=s.selectedIndex<spawnAssets.primary.entries.size()&&
            cfgprimary::SameEntry(r_,plan.mode,spawnAssets.primary,s.selectedIndex);
#endif
        const bool current=entryCurrent&&
            cfgprimary::ObserveResolvedTemplate(r_,context.objectArray,s.selected.pawnClass,spawnAssets.killerBase,
                spawnAssets.resources.classes[cfgspawn::BaseAI],fresh)&&
            cfgbundle::OwnedClassSlotMatches(s.selected.classSlot,fresh.classSlot)&&
            cfgbundle::OwnedClassSlotMatches(s.selected.defaultSlot,fresh.defaultSlot)&&fresh.defaults==s.selected.defaults;
        if(current)s.selected=fresh;return current;
    }
    bool AdditionalRestorePrimary(){
        const auto& j=bundleAdditional.journal;cfgadditionaljournal::Pair pair{};
        if(!AdditionalReadPair(pair))return false;
        if(j.Primary(pair))return AdditionalPrimary();
        if(!AdditionalPawn()||!j.Attributed(pair)||
           !cfgobserver::Same(r_,bundleJason.killerController.object+0x388,j.primary.state))return false;
        if(pair.state!=j.primary.state&&!cfgadditionalwrite::Exchange(spawnAssets.gameState+cfgprimary::CurrentKillerState,pair.state,j.primary.state))return false;
        if(pair.pawn!=j.primary.pawn&&!cfgadditionalwrite::Exchange(spawnAssets.gameState+cfgprimary::CurrentKiller,pair.pawn,j.primary.pawn))return false;
        return AdditionalPrimary();
    }
    bool AdditionalControllerState(const cfgprobe::lifetime::SlotObservation& controller,std::uintptr_t& output){
        if(!cfgjason::SameSlot(r_,context.objectArray,controller)||controller.object==plan.controller||
           controller.object==bundleJason.killerController.object)return false;
        std::uintptr_t state=0;cfgprobe::lifetime::SlotObservation slot{};
        if(!r_.Read(controller.object+0x388,state)||!state||state==bundleAdditional.journal.primary.state||
           !cfgspawn::ObjectSlot(r_,context.objectArray,state,slot)||
           !cfgspawn::ObjectIs(r_,state,spawnAssets.resources.classes[cfgspawn::PlayerState])||
           !cfgobserver::Same(r_,state+0x88,controller.object)||
           !cfgobserver::Same(r_,controller.object+0x388,state)||
           !cfgjason::OwnedComponentSlot(r_,context.objectArray,slot)||
           !cfgjason::SameSlot(r_,context.objectArray,controller))return false;
        // A native controller strongly owns this PlayerState even before a
        // weak serial is assigned. This grants observation, not write authority.
        output=state;return true;
    }
    bool AdditionalUnpossessBinding(){
        cfgprobe::lifetime::SlotObservation slot{};const auto fn=spawnAssets.functions[0];
        return cfgspawn::ObjectSlot(r_,context.objectArray,fn,slot)&&slot==spawnAssets.functionSlots[0]&&
            cfgobserver::Same(r_,plan.module+cfgjason::disk::Functions[0].cache,fn)&&
            cfgobserver::Same(r_,fn+0x20,spawnAssets.classes[0])&&
            cfgbundle::Code(r_,plan.module,cfgjason::disk::Functions[0].factory,cfgjason::disk::Functions[0].prefix);
    }
