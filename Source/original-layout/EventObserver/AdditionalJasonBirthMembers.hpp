#ifdef CFG_H1_JASON
    // H2.56 hosted F1. The offline Pawn stage's SpawnActor of a killer pawn returns null online. Mirrors donor BeginHostedAdditionalJason
    // (OfflineSetup.cpp:10404-10469, commit 10694) and the working primary route (Takeover::RunPreborn): AI controller + PlayerState, bot
    // accounting, Jason class consumed into the new state, native RestartPlayer on the AI controller, crowd-follow disable. A native call
    // that may have run is never retried. Distinct refusal reasons: 1 prerequisites, 2 world/primary, 3 roster/bots range, 4 spawn not entered,
    // 5 spawn failed, 6 bot flag, 7 bot delta, 8 roster, 9 path, 10 class source, 11 class consume, 12 birth ready, 13 restart not entered,
    // 14 restart failed, 15 pawn invalid, 16 identity, 17 primary restore, 18 crowd path, 19 path slot, 20 AI class, 21 linked proof.
    bool HostedF1RestorePrimary(std::uintptr_t extraPawn,std::uintptr_t extraState){
        const auto& j=bundleAdditional.journal;cfgadditionaljournal::Pair pair{};
        if(!j.captured||!HostedWorld()||!r_.Read(spawnAssets.gameState+cfgprimary::CurrentKiller,pair.pawn)||
           !r_.Read(spawnAssets.gameState+cfgprimary::CurrentKillerState,pair.state))return false;
        if((pair.pawn!=j.primary.pawn&&(!extraPawn||pair.pawn!=extraPawn))||
           (pair.state!=j.primary.state&&pair.state&&(!extraState||pair.state!=extraState)))return false;
        if(pair.state!=j.primary.state&&!cfgadditionalwrite::Exchange(spawnAssets.gameState+cfgprimary::CurrentKillerState,pair.state,j.primary.state))return false;
        if(pair.pawn!=j.primary.pawn&&!cfgadditionalwrite::Exchange(spawnAssets.gameState+cfgprimary::CurrentKiller,pair.pawn,j.primary.pawn))return false;
        return AdditionalPrimary();
    }
    // H2.58 hosted F1 pawn: donor FindKillerPlayerStart + GetActorStartTransform + SpawnLifecycleActor(owner = mode, collision 2).
    // Extra reasons: 23 start unavailable/timeout, 24 start transform, 25 SpawnActor returned null (clean), 26 spawn fault, 27 pawn invalid, 29 possession postcondition.
    struct HostedStartHost{BundleAccess& a;bool Current(){return a.HostedWorld();}};
    // 0 waiting (read-only scan continues next turn), 1 start selected, 2 refused. Warms the pinned K2_GetActorRotation function once if not resident.
    unsigned HostedF1Scan(){
        auto& o=hostedF1Start;HostedStartHost host{*this};
        if(++hostedF1StartTurns>900){cfgclosedtrace::Mark("f1-h258-start-timeout",hostedF1StartTurns,static_cast<std::uint64_t>(o.CurrentState()));return 2;}
        if(!hostedAdoptionActive||!HostedWorld())return 2;
        if(o.CurrentState()==cfgkillerstart::State::Empty)(void)o.Begin(r_,host,plan.module,plan.world);
        for(unsigned i=0;i<8&&(o.CurrentState()==cfgkillerstart::State::Scanning||o.CurrentState()==cfgkillerstart::State::Verifying);++i)
            if(!o.Step(r_,host))break;
        if(o.CurrentState()==cfgkillerstart::State::Refused){
            cfgclosedtrace::Mark("f1-h258-start-refused",static_cast<std::uint64_t>(o.LastFailure()),hostedF1StartTurns);return 2;}
        if(o.CurrentState()!=cfgkillerstart::State::Ready)return 0;
        return 1;
    }
    // H2.58b: the start's world location comes straight from its typed root component, as the radar and car code read it. Live H2.58 refused
    // (reason 24) on the K2 location/rotation function proofs. Rotation stays zero: Jason's facing at birth does not affect the spawn.
    bool HostedF1StartTransform(cfgspawn::Vec3& at,cfgspawn::Vec3& rot){
        HostedStartHost host{*this};cfgkillerstart::Marker marker{};std::uintptr_t root=0,scene=0,sceneCtor=0;
        cfgprobe::lifetime::SlotObservation rootSlot{},fresh{};cfgspawn::Vec3 again{};at=rot={};unsigned clause=0;
        const auto fail=[&](unsigned value){clause=value;cfgclosedtrace::Mark("f1-h258b-transform-clause",root,clause);return false;};
        if(!HostedWorld()||!hostedF1Start.CurrentMarker(r_,host,marker))return fail(1);
        if(!r_.Read(marker.actor+cfgposition::disk::ActorRootComponent,root)||!cfgprobe::ObjectPointer(root))return fail(2);
        if(!r_.Read(plan.module+cfgposition::disk::SceneClassCache,scene)||!r_.Read(plan.module+cfgposition::disk::SceneConstructorCache,sceneCtor)||
           !cfgprobe::MatchingClassCache(scene,sceneCtor)||!cfgspawn::ObjectIs(r_,root,scene))return fail(3);
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,root,rootSlot)||!cfgobserver::Same(r_,root+0x20,marker.actor))return fail(4);
        if(!r_.Read(root+cfgposition::disk::SceneWorldTranslation,at)||!r_.Read(root+cfgposition::disk::SceneWorldTranslation,again)||
           at.x!=again.x||at.y!=again.y||at.z!=again.z||!cfgspawn::Finite(at))return fail(5);
        if(!hostedF1Start.CurrentMarker(r_,host,marker)||!cfgobserver::Same(r_,marker.actor+cfgposition::disk::ActorRootComponent,root)||
           !cfgspawn::ObjectSlot(r_,context.objectArray,root,fresh)||!(fresh==rootSlot))return fail(6);
        return true;
    }
    // how: 0 not entered, 1 SpawnActor returned null (window closed, nothing retained), 2 fault/uncertain, 3 returned and retained.
    // H2.59c: a killer's BeginPlay destroys any other killer it finds, so SpawnActor returned null online. Same scope as the offline AdditionalPawnSpawn:
    // only this exact new Jason may skip the primary in that scan. "constructed" names the actor even when the spawn returns null, so the primary can be restored.
    bool HostedSpawnPawn(std::uintptr_t type,const cfgspawn::Vec3& at,const cfgspawn::Vec3& rot,std::uintptr_t& pawn,unsigned& how,std::uintptr_t& constructed){
        pawn=0;how=0;constructed=0;
        auto& scope=bundleAdditionalConstructionScope;
        if(scope.active||!AdditionalConstructionReady())return false;
        scope={plan.module,plan.world,plan.mode,spawnAssets.gameState,type,bundleJason.pawn.object,plan.pawn,plan.threadId,true};
        scope.primaryState=bundleAdditional.journal.primary.state;scope.primaryController=bundleJason.killerController.object;
        scope.primarySlot=bundleJason.pawn;scope.primaryControllerSlot=bundleJason.killerController;scope.hosted=true;
        struct Exit {cfgadditionalconstruction::Scope& scope;std::uintptr_t& constructed;~Exit(){constructed=scope.actor;scope={};}}exit{scope,constructed};
        if(!type||!cfgnativejason::SpawnCode(r_,plan.module)||!HostedPrepare(HostedRoots(type),1))return false;
        cfgnativejason::Fault fault;++calls_;how=2;
        const bool returned=cfgnativejason::detail::Spawn(plan.module+cfgmenu::populationdisk::SpawnParametersConstructor,
            plan.module+cfgmenu::populationdisk::SpawnActor,plan.world,type,&at,&rot,pawn,fault,2,plan.mode);
        if(!returned||fault.code){BundleRetentionFailed();return false;}
        if(!pawn){(void)FinishBundleNative();how=1;return false;}
        if(!RetainBundleReturnedObject(pawn))return false;
        how=3;return HostedWorld()&&cfghosted::native::WorldOwned(r_,pawn,plan.world);
    }
    bool HostedF1BirthTx(){
        auto& s=bundleAdditional;const auto& t=s.selected;const auto ai=spawnAssets.resources.classes[cfgspawn::KillerAI];
        cfghostedtx::Birth birth{};std::uintptr_t pawn=0;std::int32_t beforeBots=-1,beforePlayers=-1,count=-1;
        bool accounted=false,entered=false,contains=false;
        const auto refuse=[&](unsigned reason,std::uintptr_t at){cfgclosedtrace::Mark("f1-h256-refused",at,reason);return false;};
        // Destroys only what this transaction created, then proves roster and NumBots are back. actors=false (primary not proven restored)
        // leaves them alive: CurrentKiller must never point at a destroyed pawn.
        const auto unwind=[&](bool actors){
            bool okay=actors;std::int32_t bots=-1;bool present=false;
            if(okay&&pawn)okay=HostedInWorld(pawn)&&HostedDestroy(pawn);
            if(okay&&birth.controller)okay=HostedInWorld(birth.controller)&&HostedDestroy(birth.controller);
            if(okay){okay=(!birth.state||(HostedReadRoster(birth.state,count,present)&&!present&&count==beforePlayers))&&HostedReadBots(bots);
                if(okay&&accounted&&bots==beforeBots+1&&HostedUnaccountBot(bots)){accounted=false;bots=beforeBots;}
                okay=okay&&bots==beforeBots;}
            if(!okay)cfgclosedtrace::Mark("f1-h256-cleanup-unverified",birth.controller,pawn);
            return okay;
        };
        // Before Restart: only our controller + PlayerState exist. A leaked one makes the stage Indeterminate, never the root bank.
        const auto clean=[&](unsigned reason,std::uintptr_t at){if(!unwind(true))s.allocationAttempted=true;return refuse(reason,at);};
        // After Restart was entered: uncertain. Restore the primary pair first, then remove our actors, then forget them.
        const auto uncertain=[&](unsigned reason,std::uintptr_t at){
            s.allocationAttempted=true;s.journal.uncertain=true;
            const bool restored=HostedF1RestorePrimary(pawn,birth.state);
            if(!restored)cfgclosedtrace::Mark("f1-h256-primary-restore-failed",pawn,birth.state);
            (void)unwind(restored);
            s.actor={};s.rawPawn=s.rawController=s.rawPath=0;
            return refuse(reason,at);
        };
        if(!hostedAdoptionActive||!hostedTx.gameState||hostedTx.gameState!=spawnAssets.gameState||!ai||!hostedTx.killerAIClass||
           !hostedTx.killerBase||!hostedTx.playerStateClass)return refuse(1,hostedTx.gameState);
        if(!hostedTx.restartFn){namespace b=cfghosted::birth35;std::uintptr_t vtable=0,restart=0; // same pinned vtable slot as HostedResolveClasses
            if(HostedWorld()&&r_.Read(plan.mode,vtable)&&vtable==plan.module+b::HuntVtable&&r_.Read(vtable+b::RestartSlot,restart)&&
               restart==plan.module+b::Restart&&cfgbundle::Code(r_,plan.module,b::Restart,b::RestartCode))hostedTx.restartFn=restart;
            if(!hostedTx.restartFn)return refuse(1,0);}
        if(!HostedContext(false)||!Jason()||!AdditionalInProgress()||!AdditionalPrimary()||!AdditionalLifecycle()||s.actor.pawn.object||
           calls_+6>cfgbundle::CallCap)return refuse(2,calls_);
        if(!HostedReadBots(beforeBots)||beforeBots>7||!HostedReadRoster(0,beforePlayers,contains)||beforePlayers<1||beforePlayers>=8)
            return refuse(3,static_cast<std::uintptr_t>(beforePlayers));
        // H2.59b: read the start before spawning our controller. The new actor changes the level actor array, which the start marker's
        // current-marker proof requires to be unchanged (live H2.59: reason 24, clause 1).
        cfgspawn::Vec3 at{},rot{};
        if(CFG_H261_F1_NEAR_PLAYER){at=s.position;if(!cfgspawn::Finite(at))return refuse(24,0);} // donor offline F1 ring near the player, already projected to the ground by the Project stage
        else if(CFG_H258_HOSTED_F1_SPAWN&&(hostedF1Start.CurrentState()!=cfgkillerstart::State::Ready||!HostedF1StartTransform(at,rot)))return refuse(24,0);
        const bool spawned=HostedSpawnAi(birth);
        hostedTx.birth=birth;
        if(!spawned||!birth.controller||!birth.state||!HostedInWorld(birth.controller)){
            if(!birth.attempted)return refuse(4,0);
            return clean(5,birth.controller);
        }
        if(!birth.botFlag)return clean(6,birth.controller);
        std::int32_t nowBots=-1;
        if(!HostedReadBots(nowBots))return clean(7,birth.controller);
        if(nowBots==beforeBots){if(!HostedAccountBot(beforeBots))return clean(7,birth.controller);accounted=true;}
        else if(nowBots==beforeBots+1)accounted=true;
        else return clean(7,static_cast<std::uintptr_t>(nowBots));
        if(!HostedReadRoster(birth.state,count,contains)||!contains||count!=beforePlayers+1)return clean(8,static_cast<std::uintptr_t>(count));
        if(!HostedPathReady(birth))return clean(9,birth.controller);
        // The new AI state receives the PRIMARY AI's Jason class (plain packet, read twice), then is read back.
        cfgmenu::promotion::Packet packet{},again{};const auto source=hostedAdoptionReceipt.controllerState;
        if(CFG_H261_HOSTED_RANDOM_JASON&&s.hostedRandom){packet.bytes=s.hostedRandomSoft; // random class chosen from the mode list (empty subpath proven at choice)
            if(!HostedWorld()||!cfgspawn::ObjectIs(r_,birth.state,hostedTx.playerStateClass)||!cfgending::EmptySubpath(packet.bytes))return clean(10,birth.state);}
        else if(!HostedWorld()||!source||!cfgspawn::ObjectIs(r_,birth.state,hostedTx.playerStateClass)||
           !r_.Read(source+cfgmenu::birthdisk::ActiveCharacterClass,packet)||!cfgending::EmptySubpath(packet.bytes)||
           !r_.Read(source+cfgmenu::birthdisk::ActiveCharacterClass,again)||!cfgending::SamePath(packet.bytes,again.bytes))return clean(10,source);
        const auto expected=packet.bytes;
        if(!HostedConsumeActive(birth.state,packet)||!r_.Read(birth.state+cfgmenu::birthdisk::ActiveCharacterClass,again)||
           !cfgending::SamePath(again.bytes,expected))return clean(11,birth.state);
        if(!HostedAiBirthReady(birth,0))return clean(12,birth.controller);
        if(CFG_H258_HOSTED_F1_SPAWN){
            // Donor BeginHostedAdditionalJason: SpawnActor(pawn class, owner = GameMode, AdjustIfPossibleButAlwaysSpawn) at a native start, restore, Possess.
            unsigned how=0;std::uintptr_t constructed=0;
            (void)HostedSpawnPawn(t.pawnClass,at,rot,pawn,how,constructed);
            if(how==0)return clean(13,birth.controller);
            if(how==1){ // SpawnActor returned null: a constructed-then-destroyed killer may have taken the killer pair, which ended the match live
                if(!HostedF1RestorePrimary(constructed,birth.state))cfgclosedtrace::Mark("f1-h259c-null-restore-failed",constructed,birth.state);
                return clean(25,t.pawnClass);}
            s.allocationAttempted=true;s.rawController=birth.controller;
            if(pawn==hostedTx.snap.jason||pawn==bundleJason.pawn.object||pawn==plan.pawn){pawn=0;return uncertain(27,0);} // never destroy or adopt the primary
            if(pawn){s.rawPawn=pawn;s.journal.Returned(how==3,pawn);}
            if(how!=3)return uncertain(26,pawn);
            if(!cfgspawn::ObjectIs(r_,pawn,hostedTx.killerBase)||!HostedInWorld(pawn)||!cfgobserver::Same(r_,pawn+0x3A0,std::uintptr_t{}))return uncertain(27,pawn);
            if(!HostedF1RestorePrimary(pawn,birth.state))return uncertain(17,pawn); // pair read AFTER the spawn; a new CurrentKiller is attributed to this pawn/state
            if(!HostedWorld()||!cfgspawn::ObjectSlot(r_,context.objectArray,pawn,s.actor.pawn)||!s.actor.pawn.HasAssignedSerial()||
               !cfgspawn::ObjectSlot(r_,context.objectArray,birth.controller,s.actor.killerController)||
               !s.actor.killerController.HasAssignedSerial())return uncertain(16,pawn);
            bool possessEntered=false; // includes the crowd setter (HostedAiPathAfterPossess)
            if(!HostedPossessAi(birth,pawn,possessEntered))return uncertain(18,birth.controller);
            if(!HostedF1RestorePrimary(pawn,birth.state))return uncertain(17,pawn);
            std::int32_t after=-1;
            if(!HostedReadBots(after)||after!=beforeBots+1||!cfgobserver::Same(r_,pawn+0x3A0,birth.controller)||
               !cfgobserver::Same(r_,birth.controller+0x370,pawn))return uncertain(29,static_cast<std::uintptr_t>(after));
        }else{
            const bool restarted=HostedRestart(plan.mode,birth.controller,entered);
            if(!entered)return clean(13,birth.controller);
            s.allocationAttempted=true;
            if(HostedWorld())(void)r_.Read(birth.controller+cfghosted::adoption::ControllerPawn,pawn);
            if(pawn==hostedTx.snap.jason||pawn==bundleJason.pawn.object||pawn==plan.pawn)pawn=0; // never ours
            s.rawController=birth.controller;
            if(pawn){s.rawPawn=pawn;s.journal.Returned(restarted,pawn);}
            if(!restarted)return uncertain(14,pawn);
            if(!pawn||!cfgspawn::ObjectIs(r_,pawn,hostedTx.killerBase)||!HostedInWorld(pawn)||!HostedAiBirthReady(birth,pawn))return uncertain(15,pawn);
            if(!HostedWorld()||!cfgspawn::ObjectSlot(r_,context.objectArray,pawn,s.actor.pawn)||!s.actor.pawn.HasAssignedSerial()||
               !cfgspawn::ObjectSlot(r_,context.objectArray,birth.controller,s.actor.killerController)||
               !s.actor.killerController.HasAssignedSerial())return uncertain(16,pawn);
            if(!HostedF1RestorePrimary(pawn,birth.state))return uncertain(17,pawn);
            if(!HostedAiPathAfterPossess(birth))return uncertain(18,birth.controller);
        }
        std::uintptr_t path=0,aiClass=0;
        if(!HostedWorld()||!r_.Read(birth.controller+cfgjason::disk::PathFollowing,path)||path!=hostedTx.aiPath||
           !cfgspawn::ObjectSlot(r_,context.objectArray,path,s.actor.path))return uncertain(19,path);
        s.rawPath=path;
        {cfghosted::adoption::Receipt traveled{};traveled.controller=s.actor.killerController;traveled.controllerState=birth.state; // reason 22: donor waits for travel readiness; the primary sends this same notification
         if(!HostedNotifyAiTraveled(traveled))return uncertain(22,birth.controller);}
        // Offline writes the pawn's AIControllerClass = KillerAI in its ControllerClass stage; LinkedKiller requires it. Same compare-write.
        if(!AdditionalPawn()||!r_.Read(pawn+cfgspawn::PawnAIClass,aiClass)||(aiClass!=ai&&aiClass!=t.defaultAI)||
           (aiClass!=ai&&!cfgobjective::CompareWrite(pawn+cfgspawn::PawnAIClass,t.defaultAI,ai)))return uncertain(20,aiClass);
        if(!AdditionalControllerState(s.actor.killerController,s.journal.killerState)||!AdditionalLinked()||!AdditionalPrimary()||
           !cfghosted::native::WorldOwned(r_,pawn,plan.world)||!cfghosted::native::WorldOwned(r_,birth.controller,plan.world))return uncertain(21,pawn);
        s.policy.stage=cfgadditional::Stage::Possess; // AdditionalPhase advances to Navigation: RegisterJason / RetireGeneric / Active follow unchanged
        cfgclosedtrace::Mark("f1-h256-born",pawn,birth.controller);
        return true;
    }
    bool HostedF1Birth(){
        if(CFG_H258_HOSTED_F1_SPAWN&&!CFG_H261_F1_NEAR_PLAYER&&hostedF1Start.CurrentState()!=cfgkillerstart::State::Ready){
            hostedF1StartWait=false;
            {const unsigned scan=HostedF1Scan();
                if(scan==2){cfgclosedtrace::Mark("f1-h256-refused",hostedF1StartTurns,23);return false;} // nothing created yet: clean refusal
                if(scan==0){hostedF1StartWait=true;return true;}}
        }
        const auto saved=hostedTx;const bool okay=HostedF1BirthTx();hostedTx=saved;return okay; // HostedSpawnAi overwrites hostedTx.birth
    }
#endif
    bool AdditionalBirth(std::uint64_t){
        auto& s=bundleAdditional;using P=cfgadditional::Stage;
        if(!s.policy.BirthPending())return false;
        const unsigned gate=!Context()?1u:!Jason()?2u:!AdditionalInProgress()?3u:!AdditionalTemplate()?4u:0u;
        if(gate){
            cfgclosedtrace::Mark("f1-h265-birth-gate",s.hostedGateRetries,(static_cast<std::uint64_t>(gate)<<32)|Reads());
#if defined(CFG_H1_JASON)&&CFG_H265_F1_RETRY
            // Live H2.64: one failed gate before anything existed locked F1 for the match. Up to 4 s of the same stage, nothing has been made yet.
            if(HostedRoutineActive()&&!s.allocationAttempted&&s.hostedGateRetries<40){++s.hostedGateRetries;s.hostedGateWait=true;return true;}
#endif
            return false;
        }
        const auto& t=s.selected;const auto ai=spawnAssets.resources.classes[cfgspawn::KillerAI];
        cfgprobe::lifetime::SlotObservation slot{};
        switch(s.policy.stage){
        case P::Project:{
            std::uintptr_t human=0,function=0,owner=0,cdo=0;cfgspawn::Vec3 at{};
            if(!HelperContext()||!Known(0,human)||!Location(human,at)||
               !FeatureFunction(cfgvehicle::disk::Project,function,owner)||!r_.Read(owner+cfgspawn::ClassDefaultObject,cdo)||
               !cfgspawn::ObjectIs(r_,cdo,owner)||!cfgspawn::ObjectSlot(r_,context.objectArray,cdo,slot))return false;
            // Existing donor12-offset ring, shallow ground query and90cm lift.
            cfgvehicle::Projection packet;packet.world=plan.world;packet.point=s.placement.Candidate(at,0);packet.extent={220,220,110};
            cfgadditionaljournal::Pair pair{};
            if(!Invoke(cdo,function,&packet,false))return false;
            if(packet.result!=1||!s.placement.Projected(packet.point,packet.projected))return ++s.placement.candidate<12;
            if(!AdditionalReadPair(pair)||pair.pawn!=bundleJason.pawn.object||!s.journal.Capture(pair)||!AdditionalPrimary())return false;
            s.primaryState=pair.state;s.position=s.placement.location;return true;}
        case P::Pawn:{
#ifdef CFG_H1_JASON
            if(CFG_H256_HOSTED_F1_RESTART&&HostedRoutineActive())return HostedF1Birth();
#endif
            if(!AdditionalPrimary()||!AdditionalLifecycle()||!AdditionalConstructionReady()||
               !cfgspawn::Finite(s.position)||r_.reads>cfgbundle::ReadCap-900)return false;
            // H2.55: allocationAttempted is set at the real dispatch (AdditionalNativeSpawn); a refusal before it allocated nothing.
            const bool returned=AdditionalPawnSpawn(t.pawnClass,s.position,s.rawPawn);
            s.journal.Returned(returned,s.rawPawn);
            if(!s.rawPawn){const bool unchanged=AdditionalPrimary();
                cfgclosedtrace::Mark(unchanged?"f1-null-primary-preserved":"f1-null-registry-indeterminate",0,returned);return false;}
            // Allocation may append/relocate the UObject storage. Primary
            // Context is not valid until its singleton is restored; refresh
            // only the retained world/identities before journaling new slots.
            const bool fresh=AdditionalWorld();
            const bool captured=fresh&&cfgspawn::ObjectSlot(r_,context.objectArray,s.rawPawn,s.actor.pawn)&&AdditionalPawn();
            if(!fresh)cfgclosedtrace::Mark("f1-post-pawn-world-refused",s.rawPawn,Reads());
            else if(!captured)cfgclosedtrace::Mark("f1-post-pawn-identity-refused",s.rawPawn,Reads());
            std::uintptr_t controller=0;
            const bool detached=captured&&r_.Read(s.rawPawn+0x3A0,controller)&&!controller&&!s.actor.initialController.object;
            const bool restored=AdditionalRestorePrimary();
            if(captured&&!detached)cfgclosedtrace::Mark("f1-unexpected-birth-controller",controller,Reads());
            if(!restored)cfgclosedtrace::Mark("f1-post-pawn-restore-refused",s.rawPawn,Reads());
            return returned&&captured&&restored&&detached&&AdditionalInProgress();}
        case P::Detach:{
            if(!s.actor.initialController.object)return AdditionalPawn()&&AdditionalPrimary()&&
                cfgobserver::Same(r_,s.rawPawn+0x3A0,std::uintptr_t{});
            if(!AdditionalPawn()||!AdditionalPrimary()||!AdditionalUnpossessBinding()||
               !cfgjason::SameSlot(r_,context.objectArray,s.actor.initialController)||
               !cfgobserver::Same(r_,s.actor.initialController.object+0x370,s.rawPawn)||
               !cfgobserver::Same(r_,s.rawPawn+0x3A0,s.actor.initialController.object)||r_.reads>cfgbundle::ReadCap-450)return false;
            const bool returned=AdditionalInvoke(s.actor.initialController.object,spawnAssets.functions[0],nullptr);
            const bool restored=AdditionalRestorePrimary();
            return returned&&restored&&cfgjason::Detached(r_,context.objectArray,s.actor,plan,plan.pawn,t.pawnClass,t.defaults,
                spawnAssets.killerBase,t.defaultAI,plan.controller);}
        case P::ControllerClass:
            return AdditionalPawn()&&cfgobserver::Same(r_,s.actor.pawn.object+0x3A0,std::uintptr_t{})&&
                cfgobserver::Same(r_,s.actor.pawn.object+cfgspawn::PawnAIClass,t.defaultAI)&&
                cfgobjective::CompareWrite(s.actor.pawn.object+cfgspawn::PawnAIClass,t.defaultAI,ai);
        case P::Controller:{
            if(!AdditionalPawn()||!cfgobserver::Same(r_,s.actor.pawn.object+cfgspawn::PawnAIClass,ai)||
               !cfgobserver::Same(r_,plan.module+cfgnativejason::disk::KillerControllerReflection,ai)||
               !cfgobserver::Same(r_,plan.module+cfgnativejason::disk::KillerControllerClassCache,ai))return false;
            if(r_.reads>cfgbundle::ReadCap-600)return false;
            const bool returned=AdditionalNativeSpawn(ai,{},s.rawController);
            const bool fresh=AdditionalWorld();
            const bool captured=fresh&&s.rawController&&cfgspawn::ObjectSlot(r_,context.objectArray,s.rawController,s.actor.killerController)&&
                s.actor.killerController.HasAssignedSerial()&&s.rawController!=plan.controller&&
                s.rawController!=bundleJason.killerController.object&&cfgspawn::ObjectIs(r_,s.rawController,ai)&&
                cfgobserver::Same(r_,s.rawController+0x370,std::uintptr_t{})&&
                AdditionalControllerState(s.actor.killerController,s.journal.killerState);
            const bool restored=AdditionalRestorePrimary();
            if(!fresh||!captured)cfgclosedtrace::Mark("f1-post-controller-identity-refused",s.rawController,Reads());
#if CFG_H283_F1_KILLER_CLASS
            // Hosted order (HostedPrepareAi): the new AI PlayerState takes the Jason class before Possess. Non-fatal: the birth continues either way.
            if(returned&&captured&&restored&&AdditionalKillerClass(s.journal.killerState)&&!AdditionalRestorePrimary())
                cfgclosedtrace::Mark("h283-f1-class-restore-refused",s.journal.killerState,Reads());
#endif
            return returned&&captured&&restored;}
        case P::Possess:{
            cfgnativejason::Fault fault;
            if(!AdditionalPawn()||!cfgjason::SameSlot(r_,context.objectArray,s.actor.killerController)||
               !cfgobserver::Same(r_,s.rawPawn+0x3A0,std::uintptr_t{})||!cfgobserver::Same(r_,s.rawController+0x370,std::uintptr_t{})||
               !cfgnativejason::PossessCode(r_,plan.module)||calls_>=cfgbundle::CallCap||!AdditionalPrimary()||
               !AdditionalLifecycle()||r_.reads>cfgbundle::ReadCap-650)return false;
            ++calls_;const bool returned=AdditionalPossessCall(s.rawController,s.rawPawn,fault);
            if(!returned){s.journal.uncertain=true;cfgclosedtrace::Mark("f1-native-possess-fault",fault.address,fault.code);}
            const bool state=AdditionalWorld()&&AdditionalControllerState(s.actor.killerController,s.journal.killerState);
            const bool restored=AdditionalRestorePrimary();
            return returned&&state&&restored&&
                r_.Read(s.rawController+cfgjason::disk::PathFollowing,s.rawPath)&&
                cfgspawn::ObjectSlot(r_,context.objectArray,s.rawPath,s.actor.path)&&AdditionalLinked();}
        case P::Navigation:return AdditionalNavigation();
        case P::RetireGeneric:return AdditionalRetireGeneric();
        default:return false;
        }
    }
