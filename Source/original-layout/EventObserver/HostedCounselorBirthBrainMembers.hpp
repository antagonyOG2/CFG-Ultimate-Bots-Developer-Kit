// Inside BundleAccess. F3 owns this exact native birth; no adopted Jason is
// required. Use its returned-callback lease and explicit inputs, never Invoke's
// universal Jason/manager roots. Ambiguous entry is never retried.
    bool HostedCounselorRetryTuple(std::uintptr_t expectedWorld,std::uintptr_t expectedLocalController,std::uintptr_t expectedLocalPawn,
        std::uintptr_t expectedBornPawn,
        const cfgprobe::lifetime::SlotObservation& expectedController,const cfgprobe::lifetime::SlotObservation& expectedState,
        const cfgprobe::lifetime::SlotObservation& expectedPawnSlot,bool& ready){
        ready=false;auto& f=hostedCounselor;
        if(plan.controller!=expectedLocalController||plan.pawn!=expectedLocalPawn||
           !cfghostedfill::MatchBrainRetryBirth(expectedWorld,expectedController.object,expectedState.object,expectedBornPawn,
                plan.world,f.born.controller.object,f.born.state.object,f.pawn)||!HostedWorld())return false;
        cfgprobe::lifetime::SlotObservation controller{},state{},pawn{};std::uintptr_t stateClass=0,pawnClass=0,aiClass=0;
        std::uint32_t flags=0;std::uint8_t controllerRole=0,stateRole=0,pawnRole=0;std::int32_t rosterCount=0;bool contains=false;
        cfghosted::listen35::LocalPlayers list{};unsigned ownCount=0;
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,f.born.controller.object,controller)||
           !(controller==expectedController)||!controller.HasAssignedSerial()||
           !cfgspawn::ObjectSlot(r_,context.objectArray,f.born.state.object,state)||
           state.object!=expectedState.object||state.index!=expectedState.index||
           (expectedState.HasAssignedSerial()&&!(state==expectedState))||
           !cfgspawn::ObjectSlot(r_,context.objectArray,f.pawn,pawn)||
           pawn.object!=expectedPawnSlot.object||pawn.index!=expectedPawnSlot.index||
           (expectedPawnSlot.HasAssignedSerial()&&!(pawn==expectedPawnSlot))||
           !cfghosted::native::Class(r_,plan.module,cfgescape::disk::SCPlayerStateReflection,cfgescape::disk::SCPlayerStateConstructor,stateClass)||
           !cfghosted::native::Class(r_,plan.module,cfgprobe::NativeClasses[cfgprobe::Counselor].reflectionCacheRva,
                cfgprobe::NativeClasses[cfgprobe::Counselor].constructorCacheRva,pawnClass)||
           !cfghosted::native::Class(r_,plan.module,cfgcompat::cfg35::ai::Classes[2].reflection,cfgcompat::cfg35::ai::Classes[2].constructor,aiClass)||
           !cfgspawn::ObjectIs(r_,f.born.state.object,stateClass)||!cfgspawn::ObjectIs(r_,f.pawn,pawnClass)||
           !cfgspawn::ObjectIs(r_,f.born.controller.object,aiClass)||!HostedInWorld(f.born.state.object)||
           !HostedInWorld(f.born.controller.object)||!HostedInWorld(f.pawn)||
           !r_.Read(f.born.state.object+cfghosted::birth35::BotFlags,flags)||!(flags&cfghosted::birth35::BotMask)||
           !r_.Read(f.born.controller.object+cfghosted::disk35::ActorRole,controllerRole)||controllerRole!=cfghosted::disk::AuthorityRole||
           !r_.Read(f.born.state.object+cfghosted::disk35::ActorRole,stateRole)||stateRole!=cfghosted::disk::AuthorityRole||
           !r_.Read(f.pawn+cfghosted::disk35::ActorRole,pawnRole)||pawnRole!=cfghosted::disk::AuthorityRole||
           !HostedReadRoster(f.born.state.object,rosterCount,contains)||!contains||
           !r_.Read(hostedTx.gameState+cfghosted::birth35::PlayerArray,list)||list.count!=rosterCount||list.count<1||list.count>32||
           list.capacity<list.count||list.capacity>64||!cfgprobe::ObjectPointer(list.data)||
           !cfgobserver::Same(r_,f.born.controller.object+cfghosted::adoption::ControllerState,f.born.state.object)||
           !cfgobserver::Same(r_,f.born.controller.object+cfgprobe::ControllerPawn,f.pawn)||
           !cfgobserver::Same(r_,f.pawn+cfgescape::disk::Controller,f.born.controller.object)||
           !cfgobserver::Same(r_,f.pawn+cfgescape::disk::PlayerState,f.born.state.object)||
           !cfgobserver::Same(r_,f.born.state.object+0x88,f.born.controller.object))return false;
        for(std::int32_t i=0;i<list.count;++i){std::uintptr_t entry=0;
            if(!r_.Read(list.data+8ULL*static_cast<unsigned>(i),entry))return false;
            if(entry==f.born.state.object)++ownCount;}
        if(ownCount!=1)return false;
        cfgprobe::lifetime::SlotObservation controllerAgain{},stateAgain{},pawnAgain{};
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,f.born.controller.object,controllerAgain)||!(controllerAgain==controller)||
           !cfgspawn::ObjectSlot(r_,context.objectArray,f.born.state.object,stateAgain)||!(stateAgain==state)||
           !cfgspawn::ObjectSlot(r_,context.objectArray,f.pawn,pawnAgain)||!(pawnAgain==pawn)||!HostedWorld())return false;
        if(!state.HasAssignedSerial()||!pawn.HasAssignedSerial())return true; // stable birth tuple; wait for promotion
        if(!f.born.state.HasAssignedSerial()){
            if(!cfghostedfill::PromoteUnassignedSlot(f.born.state.object,f.born.state.index,f.born.state.serial,
                state.object,state.index,state.serial,true))return false;
            f.born.state=state;f.born.stateSerialAssigned=true;
        }
        if(!f.pawnSlot.HasAssignedSerial()){
            if(!cfghostedfill::PromoteUnassignedSlot(f.pawnSlot.object,f.pawnSlot.index,f.pawnSlot.serial,
                pawn.object,pawn.index,pawn.serial,true))return false;
            f.pawnSlot=pawn;
        }
        ready=true;return true;
    }
    bool HostedCounselorBirthBrain(){
        auto& f=hostedCounselor;
        auto trace=std::find_if(hostedBots.begin(),hostedBots.end(),[&](const auto& d){return d.pawn==f.pawn&&d.controller==f.born.controller.object;});
        if(trace==hostedBots.end())return false;
        auto& d=*trace;++d.samples;
        const auto refuse=[&](unsigned why){d.last=why;if(!d.first)d.first=why;d.reads=r_.reads;return false;};
        namespace b=cfghostedbrain::disk;
        if(d.startAttempts)return false;
        (void)HostedPromoteCounselorState(); // a later registration serial may be observed at this owned callback
        const auto bornController=f.born.controller,bornState=f.born.state;
        const auto bornPawn=f.pawn,expectedWorld=plan.world;
        cfgprobe::lifetime::SlotObservation pawnSlot{},treeSlot{};
        std::uintptr_t treeClass=0;
        const auto observeOwnedTree=[&](cfgprobe::lifetime::SlotObservation& observed)->bool{
            std::uintptr_t tree=0,freshClass=0,value=0;
            cfgprobe::lifetime::SlotObservation first{},second{};
            if(!r_.Read(plan.mode+cfghosted::birth35::CounselorBehaviorTree,tree)||!tree||
               !cfghosted::native::Class(r_,plan.module,cfgcompat::cfg35::ai::Classes[4].reflection,
                    cfgcompat::cfg35::ai::Classes[4].constructor,freshClass)||!cfgspawn::ObjectIs(r_,tree,freshClass)||
               !cfgspawn::ReadSpawnAssetSlot(r_,context.objectArray,tree,5,first)||
               !r_.Read(plan.module+cfgcompat::cfg35::ai::Classes[4].reflection, value)||value!=freshClass||
               !r_.Read(plan.module+cfgcompat::cfg35::ai::Classes[4].constructor,value)||value!=freshClass||
               !r_.Read(plan.mode+cfghosted::birth35::CounselorBehaviorTree,value)||value!=tree||
               !cfgspawn::ObjectIs(r_,tree,freshClass)||!cfgspawn::ReadSpawnAssetSlot(r_,context.objectArray,tree,5,second)||
               !(first==second))return false;
            if((treeClass&&treeClass!=freshClass)||(observed.object&&
               (observed.object!=second.object||observed.index!=second.index||
                (observed.serial!=second.serial&&!cfghostedfill::PromoteUnassignedSlot(observed.object,observed.index,
                    observed.serial,second.object,second.index,second.serial,true)))))return false;
            treeClass=freshClass;
            observed=second;return true;
        };
        const auto bornCurrent=[&](){return f.pawn==bornPawn&&f.born.controller.object==bornController.object&&
           f.born.state.object==bornState.object&&plan.world==expectedWorld&&HostedWorld()&&
           cfgjason::SameSlot(r_,context.objectArray,bornController)&&cfgjason::SameSlot(r_,context.objectArray,bornState)&&
           cfgobserver::Same(r_,bornController.object+cfgprobe::ControllerPawn,bornPawn)&&
           cfgobserver::Same(r_,bornPawn+cfgescape::disk::Controller,bornController.object)&&
           cfgobserver::Same(r_,bornPawn+cfgescape::disk::PlayerState,bornState.object)&&
           cfgobserver::Same(r_,bornController.object+cfgescape::disk::PlayerState,bornState.object)&&
           cfgobserver::Same(r_,bornState.object+0x88,bornController.object)&&HostedInWorld(bornPawn);};
        if(!bornCurrent()||!cfgspawn::ObjectSlot(r_,context.objectArray,bornPawn,pawnSlot)||!pawnSlot.HasAssignedSerial()||
           !cfgjason::SameSlot(r_,context.objectArray,f.born.controller)||
           !cfgjason::SameSlot(r_,context.objectArray,f.born.state)||!HostedInWorld(f.pawn)||
           !cfgobserver::Same(r_,f.born.controller.object+cfgprobe::ControllerPawn,f.pawn)||
           !cfgobserver::Same(r_,f.pawn+cfgescape::disk::Controller,f.born.controller.object)||
           !cfgobserver::Same(r_,f.pawn+cfgescape::disk::PlayerState,f.born.state.object))return refuse(60);
        std::uintptr_t ai=0,tree=0,owner=0,function=0,reflection=0,constructor=0,brain=0;
        cfgprobe::lifetime::SlotObservation functionSlot{},ownerSlot{};
        if(!cfghosted::native::Class(r_,plan.module,cfgcompat::cfg35::ai::Classes[2].reflection,cfgcompat::cfg35::ai::Classes[2].constructor,ai)||
           !cfgspawn::ObjectIs(r_,f.born.controller.object,ai)||!observeOwnedTree(treeSlot))return refuse(61);
        tree=treeSlot.object;
        d.tree=tree;
        if(!r_.Read(f.born.controller.object+0x418,brain))return refuse(62);
        if(brain){std::uintptr_t brainClass=0;cfgprobe::lifetime::SlotObservation slot{};
            if(!cfghosted::native::Class(r_,plan.module,cfgcompat::cfg35::ai::Classes[5].reflection,cfgcompat::cfg35::ai::Classes[5].constructor,brainClass)||
               !cfgspawn::ObjectIs(r_,brain,brainClass)||!cfgspawn::ObjectSlot(r_,context.objectArray,brain,slot)||!slot.HasAssignedSerial())return refuse(72);
            d.brain=brain;return true;} // stock birth may already have started it
        const auto& binding=b::RunBehaviorTree;
        if(!r_.Read(plan.module+binding.ownerReflection,reflection)||!r_.Read(plan.module+binding.ownerConstructor,constructor)||
           !(owner=cfgprobe::MatchingClassCache(reflection,constructor))||!cfgbundle::Code(r_,plan.module,binding.factory,binding.code)||
           !r_.Read(plan.module+binding.cache,function)||!cfgspawn::ObjectSlot(r_,context.objectArray,owner,ownerSlot))return refuse(63);
        if(!function){
            if(!HostedPrepare(std::array<std::uintptr_t,10>{plan.world,plan.mode,hostedTx.gameState,plan.controller,plan.pawn,
                f.born.controller.object,f.pawn,f.born.state.object,tree,owner},1))return refuse(64);
            ++calls_;
            if(!cfgmenu::population::detail::GetClass(plan.module+binding.factory,function)){
                exception_=true;BundleRetentionFailed();return refuse(65);}
            if(!RetainBundleReturnedObject(function)||!HostedWorld())return refuse(66);
        }
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,function,functionSlot))return refuse(67);
        const auto bindingCurrent=[&](){
            // The pinned cache/owner links identify native metadata in this retained
            // window. No weak identity is cached for a serial-zero class or function.
            cfgprobe::lifetime::SlotObservation currentOwner{},currentFunction{};
            std::uint16_t bytes=0;
            return cfgspawn::ObjectSlot(r_,context.objectArray,owner,currentOwner)&&currentOwner==ownerSlot&&
                cfgspawn::ObjectSlot(r_,context.objectArray,function,currentFunction)&&currentFunction==functionSlot&&
                r_.Read(function+cfgreflection::arraydisk::FunctionParameterBytes,bytes)&&
                cfgbundle::NativeFrame(bytes,binding.nativeBytes,binding.storageBytes)&&
                cfgobserver::Same(r_,function+0x20,owner)&&cfgobserver::Same(r_,plan.module+binding.cache,function)&&
                cfgobserver::Same(r_,function+0xB0,plan.module+b::RunBehaviorTreeExecutor)&&
                cfgbundle::Code(r_,plan.module,b::RunBehaviorTreeExecutor,b::RunBehaviorTreeCode)&&
                cfgspawn::ObjectIs(r_,bornController.object,owner)&&
                cfgobserver::Same(r_,plan.module+binding.ownerReflection,owner)&&
                cfgobserver::Same(r_,plan.module+binding.ownerConstructor,owner);
        };
        if(!bindingCurrent())return refuse(67);
        struct Packet{std::uintptr_t tree;std::uint8_t returned=0;std::uint8_t padding[7]{};} packet{tree};
        static_assert(sizeof(Packet)==16);
        std::uintptr_t freshAi=0;
        if(!bornCurrent()||!cfgjason::SameSlot(r_,context.objectArray,pawnSlot)||
           !cfghosted::native::Class(r_,plan.module,cfgcompat::cfg35::ai::Classes[2].reflection,cfgcompat::cfg35::ai::Classes[2].constructor,freshAi)||
           freshAi!=ai||!cfgspawn::ObjectIs(r_,bornController.object,freshAi)||
           !observeOwnedTree(treeSlot)||
           !HostedPrepare(std::array<std::uintptr_t,11>{plan.world,plan.mode,hostedTx.gameState,plan.controller,plan.pawn,
               f.born.controller.object,f.pawn,f.born.state.object,tree,function,owner},0))return refuse(68);
        if(!bornCurrent()||!observeOwnedTree(treeSlot)||treeSlot.object!=tree||!bindingCurrent()){
            // HostedPrepare has entered a zero-output window. Close it even
            // when a final identity check refuses before native dispatch.
            (void)FinishBundleNative();
            return refuse(68);
        }
        ++d.startAttempts;++calls_;
        __try{reinterpret_cast<Dispatch>(plan.module+cfgspawn::GenericDispatcherRva)(reinterpret_cast<void*>(f.born.controller.object),reinterpret_cast<void*>(function),&packet);}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;BundleRetentionFailed();return refuse(69);}
        if(!FinishBundleNative())return refuse(70);
        d.startReturned=packet.returned;d.running=2;d.paused=2;
        if(!packet.returned)return refuse(71);
        d.last=0;d.reads=r_.reads;return true;
    }
