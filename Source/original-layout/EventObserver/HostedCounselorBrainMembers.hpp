// Included in BundleAccess and the H210 failure replay fixture.
    bool HostedCounselorBrainPhase(){
        if(!HostedContext(false)||!bundleReport.bots)return false;
        const unsigned index=hostedBotCursor++%bundleReport.bots;
        const auto pawn=bundleReport.roster[index].pawn,controller=bundleReport.roster[index].controller;
        auto it=std::find_if(hostedBots.begin(),hostedBots.end(),[&](const auto& d){return d.pawn==pawn&&d.controller==controller;});
        if(it==hostedBots.end())return false;auto& d=*it;++d.samples;
        const auto refuse=[&](unsigned reason){d.last=reason;if(!d.first)d.first=reason;d.reads=r_.reads;return false;};
        cfgprobe::lifetime::SlotObservation brain{};std::uintptr_t type=0,rawBrain=0;
        if(!cfgjason::SameSlot(r_,context.objectArray,bundleControllerSlots[index]))return refuse(10);
        if(!cfgobserver::Same(r_,controller+cfgprobe::ControllerPawn,pawn))return refuse(11);
        if(!cfgobserver::Same(r_,pawn+cfgescape::disk::Controller,controller))return refuse(12);
        if(!cfgspawn::ObjectIs(r_,controller,spawnAssets.resources.classes[cfgspawn::CounselorAI]))return refuse(13);
        if(!r_.Read(controller+cfgprobe::ObjectClass,type))return refuse(14);
        if(!r_.Read(controller+0x418,rawBrain))return refuse(15);
        if(rawBrain&&!cfgconvergencetravel::Brain(r_,context.objectArray,bundleControllerSlots[index],
            spawnAssets.resources.classes[cfgspawn::CounselorAI],spawnAssets.resources.classes[cfgspawn::Brain],brain))return refuse(2);
        if(!rawBrain){d.running=0;d.paused=0;}
        d.controllerClass=type;d.brain=brain.object;d.tree=spawnAssets.primary.counselorTree;
        namespace b=cfghostedbrain::disk;
        if(d.running==0&&d.paused==0&&!d.startAttempts){
            d.last=4;
            if(!d.tree)return refuse(40);
            if(!Tree())return refuse(41);
            struct Packet{std::uintptr_t tree;std::uint8_t returned=0;std::uint8_t padding[7]{};} packet{d.tree};
            static_assert(sizeof(Packet)==16);
            std::uintptr_t function=0,owner=0;
            if(!FeatureFunction(b::RunBehaviorTree,function,owner))return refuse(42);
            if(!cfgspawn::ObjectIs(r_,controller,owner))return refuse(43);
            if(!cfgobserver::Same(r_,function+0xB0,plan.module+b::RunBehaviorTreeExecutor))return refuse(44);
            if(!cfgbundle::Code(r_,plan.module,b::RunBehaviorTreeExecutor,b::RunBehaviorTreeCode))return refuse(45);
            if(!NativeCallAllowed())return refuse(46);
            // Mark before entry. Exceptions/ambiguous returns never replay StartTree.
            ++d.startAttempts;
            if(!Invoke(controller,function,&packet,false))return refuse(50+bundleInvocationStage_);
            d.startReturned=packet.returned;d.running=2;d.paused=2;
            if(!packet.returned)return refuse(5); // first actual native failure survives absent-brain samples
            d.last=0;d.reads=r_.reads;return true;
        }
        if(!brain.object)return refuse(6);
        std::uint8_t running=2;d.last=2;
        if(!CarCall(b::IsRunning,b::IsRunningExecutor,b::IsRunningCode,brain.object,&running)||running>1)return refuse(7);
        d.running=running;
        if(!running&&!d.startAttempts){std::uint8_t paused=2;d.last=3;
            if(!CarCall(b::IsPaused,b::IsPausedExecutor,b::IsPausedCode,brain.object,&paused)||paused>1)return refuse(8);d.paused=paused;}
        // Observation only; a failed location sample never blocks native startup.
        cfgspawn::Vec3 point{};
        if(DonorNativePosition(bundlePawnSlots[index],point)){
            if(d.samples>1&&(std::fabs(point.x-d.x)>2||std::fabs(point.y-d.y)>2||std::fabs(point.z-d.z)>2))++d.moves;
            d.x=point.x;d.y=point.y;d.z=point.z;
        }
        d.last=0;d.reads=r_.reads;return true;
    }
