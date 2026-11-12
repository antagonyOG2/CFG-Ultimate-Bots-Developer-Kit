// Compatibility provider for the EXISTING NativeDispatch/menu-row machinery.
// Two DISTINCT proofs: completed ReceiveDrawHUD for ordinary UI work, or the
// live engine UFunction invocation of a proved borrowed Construct for ONE
// ownership getter + original constructor forwarding/restoration. Neither
// grants a tick, arbitrary widget, factory, or startup/birth capability.
cfgretain::ResidentRootBank<32> frontendRootBank;
#if CFG_H21_LOBBY_ROWS
cfgretain::ResidentRootBank<32> hostedLobbyRootBank; // separate listen-host UI domain
#endif
cfgmenu::retention::Report frontendRetentionReport;
bool FrontendChallengeContinuationInvokeAllowed(std::uintptr_t,std::uintptr_t)noexcept;
class FrontendMenuRootProvider {
    cfgmenu::hud::Binding binding_;
    std::uintptr_t receiver_,function_,pc_;
    cfgretain::CallbackRootLease<decltype(frontendRootBank)> lease_;
    unsigned maximum_=0;bool admitted_=false;
    std::unique_ptr<cfgmenu::constructboundary::Evidence> borrowed_;
    bool originalForwarding_=false,getterSpent_=false;void* getterPacket_=nullptr;
    unsigned challengeTransition_=0;std::uintptr_t challengeType_=0,challengeTransitionFunction_=0;
    bool challengeStartEntered_=false,challengeTail_=false;
    void Record(unsigned failure=0)noexcept{
        auto& r=frontendRetentionReport;
        InterlockedIncrement(reinterpret_cast<volatile LONG*>(&r.generation));
        if(failure){++r.failures;r.lastFailure=failure;}
        r.owners=frontendRootBank.Owners();r.peakClaims=(std::max)(r.peakClaims,lease_.Claims());
        r.poisoned=frontendRootBank.Poisoned()?1u:0u;r.registryStage=gcRegistryReport.stage;
        r.registryReads=gcRegistryReport.reads;r.registryGate=gcRegistryReport.gateFailure;r.registryTick=gcRegistryReport.tickFailure;
        MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&r.generation));
    }
    static FrontendMenuRootProvider& Self(void* p)noexcept{return *static_cast<FrontendMenuRootProvider*>(p);}
    bool Context(Reader& reader)noexcept{
        if(challengeTail_)return admitted_&&!borrowed_&&binding_.context.module==plan.module&&
            cfgmenu::challengetransition::ReturnedStartCurrent(reader,binding_,receiver_,function_,pc_,GetCurrentThreadId());
        if(borrowed_)return GetCurrentThreadId()==borrowed_->source.thread&&
            cfgprimary::LookupVetoClearForObservation(reader,plan.module)&&
            cfgmenu::constructboundary::Current(reader,*borrowed_,GetCurrentThreadId());
        return admitted_&&GetCurrentThreadId()==binding_.context.thread&&binding_.context.module==plan.module&&
            cfgprimary::LookupVetoClearForObservation(reader,plan.module)&&
            cfgmenu::active::Current(reader,binding_.context,GetCurrentThreadId());
    }
    bool Completed(Reader& reader,std::uintptr_t function,std::uintptr_t pc)noexcept{
        if(challengeTail_)return function==function_&&pc==pc_&&Context(reader);
        if(borrowed_)return function==function_&&pc==pc_&&Context(reader);
        return Context(reader)&&function==function_&&pc==pc_&&
            cfgmenu::hud::CompletedNativeCallback(reader,binding_,receiver_,function,pc,GetCurrentThreadId(),false,true);
    }
    GcRegistryAccess::NativeContextProof Proof()noexcept{
        return {this,[](Reader& r,void* p){return Self(p).Context(r);},
            [](Reader& r,void* p,std::uintptr_t f,std::uintptr_t pc){return Self(p).Completed(r,f,pc);}};
    }
    bool Current()noexcept{
        if((!admitted_&&!borrowed_)||!lease_.Current())return false;
        Reader reader;reader.cap=1024;return Context(reader);
    }
    template<class Work>bool Window(Work&& work)noexcept{
        if(!Current())return false;GcRegistryAccess access(Proof());bool result=false;
        __try{if(access.Begin(function_,pc_))result=work(access);}
        __finally{access.End();}
        return result;
    }
    template<unsigned N>bool Values(Reader& reader,const std::array<std::uintptr_t,N>& values)noexcept{
        cfgprobe::lifetime::ArrayHeader registry{};cfgprobe::lifetime::SlotObservation slot{};
        if(!Context(reader)||!cfgprobe::lifetime::ReadArrayHeader(reader,plan.module+cfgprobe::lifetime::ObjectArrayRva,registry))return false;
        for(auto value:values)if(value&&!cfgspawn::ObjectSlot(reader,registry,value,slot))return false;
        return true;
    }
    bool Inputs(std::uintptr_t actor,std::uintptr_t function,void* packet)noexcept{
        if(challengeTail_)return false;
        challengeStartEntered_=false;
        challengeTransition_=0;challengeType_=0;challengeTransitionFunction_=0;
        if(borrowed_){
            if(originalForwarding_||actor!=borrowed_->receiver.widget||function!=borrowed_->getter||getterSpent_)return false;
            getterSpent_=true;getterPacket_=packet;
        }
        Reader metadata;metadata.cap=4096;cfgretainframe::RuntimeHost<Reader> host(metadata,plan.module);
        std::array<std::uintptr_t,32> values{};
        const std::array<std::uintptr_t,4> owners{binding_.context.world,binding_.context.hud,actor,function};
        if(!Window([&](GcRegistryAccess& access)noexcept{
            return Values(metadata,owners)&&lease_.Retain(access,owners);
        })||!cfgretainframe::Observe(host,function,reinterpret_cast<std::uintptr_t>(packet),values)||
           !cfgretainframe::MaximumOutputRoots(host,function,maximum_))return false;
        const bool prepared=Window([&](GcRegistryAccess& access)noexcept{
            return Values(metadata,values)&&lease_.Retain(access,values)&&lease_.PrepareCall(access,maximum_);
        });
        if(prepared&&CFG_J22_CHALLENGES_RETENTION&&!borrowed_){
            auto* scope=cfgmenu::startupretain::Scope::Current(plan.module);
            if(scope&&scope->Allows(cfgmenu::startupretain::Call::FrontendChallengePageClass)){
                challengeTransition_=cfgmenu::challengetransition::Classify(metadata,binding_.context,actor,function);
                if(!challengeTransition_&&FrontendChallengeContinuationInvokeAllowed(actor,function))challengeTransition_=3;
                if(challengeTransition_==1&&!metadata.Read(reinterpret_cast<std::uintptr_t>(packet),challengeType_))return false;
                if(challengeTransition_)challengeTransitionFunction_=function;
            }
        }
        return prepared;
    }
    bool CurrentForOutputs(std::uintptr_t function,void* packet)noexcept{
        if(!CFG_J25_CHALLENGE_PUSH_OUTPUTS||!challengeTransition_)return Current();
        if(borrowed_||challengeTail_||!admitted_||!lease_.Current()||!packet||function!=challengeTransitionFunction_||
           binding_.context.module!=plan.module||GetCurrentThreadId()!=binding_.context.thread)return false;
        Reader proof;proof.cap=8192;
        if(challengeTransition_==3)return challengeStartEntered_&&
            cfgmenu::challengetransition::ReturnedStartCurrent(proof,binding_,receiver_,function_,pc_,GetCurrentThreadId());
        if(challengeTransition_>2)return false;
        cfgmenu::active::Context fresh;std::uintptr_t returnedWidget=0;
        if(challengeTransition_==1&&(!proof.Read(reinterpret_cast<std::uintptr_t>(packet)+16,returnedWidget)||
           !cfgprobe::ObjectPointer(returnedWidget)))return false;
        return cfgprimary::LookupVetoClearForObservation(proof,plan.module)&&
            cfgmenu::challengetransition::Observe(proof,binding_.context,challengeTransition_,challengeType_,returnedWidget,fresh)&&
            cfgmenu::active::Current(proof,fresh,GetCurrentThreadId());
    }
    bool Outputs(std::uintptr_t function,void* packet)noexcept{
        if(challengeTail_)return false; // a returned Start has no second output/native transaction
        if(borrowed_&&(originalForwarding_||function!=borrowed_->getter||packet!=getterPacket_))return false;
        if(challengeTransition_==3){
            if(!challengeStartEntered_)return false;
            challengeTail_=true;Reader proof;proof.cap=8192;
            if(!Context(proof))return false;
            challengeTransition_=0;
        }else if(challengeTransition_){
            Reader proof;proof.cap=8192;cfgmenu::active::Context fresh;std::uintptr_t returnedWidget=0;
            if(challengeTransition_==1&&!proof.Read(reinterpret_cast<std::uintptr_t>(packet)+16,returnedWidget))return false;
            if(!cfgmenu::challengetransition::Observe(proof,binding_.context,challengeTransition_,challengeType_,returnedWidget,fresh))return false;
            // Read-only reobserve after a normal pinned Push/Pop return. The
            // source widget/HUD is still rooted; no new call or allocation.
            const auto old=binding_.context;binding_.context=fresh;
            if(!cfgmenu::hud::Current(proof,binding_,GetCurrentThreadId())){binding_.context=old;return false;}
            challengeTransition_=0;challengeType_=0;challengeTransitionFunction_=0;
        }
        Reader metadata;metadata.cap=4096;cfgretainframe::RuntimeHost<Reader> host(metadata,plan.module);
        std::array<std::uintptr_t,32> values{};
        if(!cfgretainframe::Observe(host,function,reinterpret_cast<std::uintptr_t>(packet),values,cfgretainframe::Phase::Outputs))return false;
        const bool okay=Window([&](GcRegistryAccess& access)noexcept{
            return Values(metadata,values)&&lease_.CompleteCall(access,values,maximum_);
        });
        if(okay)getterPacket_=nullptr;return okay;
    }
    bool FactoryInput(std::uintptr_t owner)noexcept{
        if(borrowed_||challengeTail_)return false; // no new native work in a returned Start tail
        Reader reader;reader.cap=1024;const std::array<std::uintptr_t,2> values{binding_.context.world,owner};
        return Window([&](GcRegistryAccess& access)noexcept{
            return Values(reader,values)&&lease_.Retain(access,values)&&lease_.PrepareCall(access,1);
        });
    }
    bool FactoryOutput(std::uintptr_t object)noexcept{
        Reader reader;reader.cap=1024;const std::array<std::uintptr_t,1> value{object};
        return Window([&](GcRegistryAccess& access)noexcept{
            return Values(reader,value)&&lease_.CompleteCall(access,value);
        });
    }
public:
    // Completed CURRENT menu HUD only; no borrowed Construct/active native call.
    // Retirement owns a separate registry window and never dispatches gameplay.
    template<class Work>bool RegistryRetirementWindow(Work&& work)noexcept{
        if(borrowed_||challengeTail_||!admitted_||!lease_.EmptyIdle()||cfgmenu::rows::NativeRetentionScope::Occupied()||!Current())return false;
        GcRegistryAccess access(Proof(),131072);bool okay=false;
        __try{if(access.Begin(function_,pc_))okay=work(access);}
        __finally{access.End();}
        return okay;
    }
    FrontendMenuRootProvider(const cfgmenu::active::Context& actual,const cfgmenu::pickerprepare::Prepared& prepared,
        const cfgmenu::pickerprepare::Executors& expected,std::uintptr_t widget,std::uintptr_t frame,std::uintptr_t pc):
        receiver_(widget),function_(0),pc_(pc),lease_(frontendRootBank,actual.module){
        binding_.context=actual;auto& constructReport=frontendRetentionReport;++constructReport.constructCandidates;
        constructReport.constructReturnPc=pc;constructReport.constructExpectedPc=actual.module+cfgmenu::constructdisk::Return;
        // No independent nested admission and no borrowed HUD authority.
        if(cfgmenu::rows::NativeRetentionScope::Occupied()){constructReport.constructStage=1;return;}
        auto evidence=std::make_unique<cfgmenu::constructboundary::Evidence>();Reader proof;proof.cap=16384;unsigned stage=0;
        if(!cfgmenu::constructboundary::Observe(proof,actual,prepared,expected,widget,frame,pc,GetCurrentThreadId(),*evidence,&stage)){
            constructReport.constructStage=10+stage;return;}
        borrowed_=std::move(evidence);function_=borrowed_->node;
        // These are freshly proved metadata/source dependencies, not a list of
        // raw cached pointers. Recheck each exact slot while publication is
        // protected; roots survive original forwarding AND cast restoration.
        for(unsigned first=0;first<borrowed_->count;first+=32){std::array<std::uintptr_t,32> values{};
            Reader slots;slots.cap=1024;
            const bool okay=Window([&](GcRegistryAccess& access)noexcept{
                cfgprobe::lifetime::ArrayHeader registry;
                if(!cfgprobe::lifetime::ReadArrayHeader(slots,actual.module+cfgprobe::lifetime::ObjectArrayRva,registry))return false;
                for(unsigned i=first;i<first+32&&i<borrowed_->count;++i){cfgprobe::lifetime::SlotObservation fresh;
                    if(!cfgspawn::ObjectSlot(slots,registry,borrowed_->roots[i].object,fresh)||!(fresh==borrowed_->roots[i]))return false;
                    values[i-first]=fresh.object;}
                return lease_.Retain(access,values);
            });
            if(!okay){constructReport.constructStage=20;Record(9);return;}
        }
        admitted_=true;++constructReport.constructAdmitted;constructReport.constructStage=0;constructReport.constructRoots=borrowed_->count;Record();
    }
    FrontendMenuRootProvider(const cfgmenu::hud::Binding& binding,std::uintptr_t receiver,std::uintptr_t function,
        std::uintptr_t pc,bool nested,bool completed):binding_(binding),receiver_(receiver),function_(function),pc_(pc),
        lease_(frontendRootBank,binding.context.module){
        Reader proof;proof.cap=4096;
        // Native Push/Pop can replace the active widget on the SAME HUD.
        // Reobserve that actual stack owner before granting this callback;
        // do not transplant an old widget's ownership to a new instance.
        if(!nested&&completed&&GetCurrentThreadId()==binding.context.thread&&receiver==binding.context.hud&&
           function==binding.event&&pc==binding.context.module+cfgmenu::transportdisk::ReturnPc){
            cfgmenu::active::Context current;
            if(cfgmenu::active::Observe(proof,binding.context.module,current)&&current.hud==receiver&&
               current.world==binding.context.world&&current.thread==binding.context.thread)binding_.context=current;
        }
        admitted_=lease_.Current()&&cfgmenu::hud::CompletedNativeCallback(proof,binding_,receiver,function,pc,
            GetCurrentThreadId(),nested,completed);
        if(!nested&&completed&&GetCurrentThreadId()==binding.context.thread&&receiver==binding.context.hud&&function==binding.event){
            auto& r=frontendRetentionReport;++r.candidates;r.admitted+=admitted_?1u:0u;
            r.receiver=receiver;r.function=function;r.returnPc=pc;r.expectedReceiver=binding.context.hud;
            r.expectedFunction=binding.event;r.expectedPc=binding.context.module+cfgmenu::transportdisk::ReturnPc;Record(admitted_?0u:8u);
        }
    }
    // Metadata-only work (post-match objective script restore) inside the proved
    // completed menu ReceiveDrawHUD + native GC registry window. Reader-typed
    // lookup is granted ONLY between Begin/End and every use repeats Context().
    // Never the borrowed Construct capability; no native invocation or tick.
    // Every proof read is charged to the SUPPLIED reader (the caller's phase
    // Reader or its Budget view), never a hidden fresh reader, so the menu
    // re-proof counts against ReadCap/PhaseReads and the reported peak.
    template<class R>bool MetadataContext(R& reader)noexcept{
        return !borrowed_&&!challengeTail_&&admitted_&&GetCurrentThreadId()==binding_.context.thread&&binding_.context.module==plan.module&&
            cfgprimary::LookupVetoClearForObservation(reader,plan.module)&&
            cfgmenu::active::Current(reader,binding_.context,GetCurrentThreadId());
    }
    template<class BoundedReader,class Work>bool MetadataWindow(Reader& reader,Work&& work)noexcept{
        if(borrowed_||challengeTail_||!admitted_)return false;
        return Window([&](GcRegistryAccess&)noexcept{
            cfgprimary::ScopedLookupEligibility<Reader> lookup(plan.module,this,
                [](Reader& r,void* p){return Self(p).MetadataContext(r);},true);
            cfgprimary::ScopedLookupEligibility<BoundedReader> bounded(plan.module,this,
                [](BoundedReader& r,void* p){return Self(p).MetadataContext(r);},lookup.Entered());
            return lookup.Entered()&&bounded.Entered()&&MetadataContext(reader)&&work(reader);
        });
    }
    ~FrontendMenuRootProvider(){
        if((admitted_||borrowed_)&&!lease_.EmptyIdle()){
            if(borrowed_&&!Required()){lease_.Uncertain();Record(10);return;}
            Record();if(!Window([&](GcRegistryAccess& access)noexcept{return lease_.Close(access);})){
                lease_.Uncertain();Record(6);
            }else Record();
        }
    }
    bool Admitted()const noexcept{return admitted_;}
    bool CleanupOnly()const noexcept{return challengeTail_;}
    bool Required()noexcept{
        for(unsigned at=0;at<lease_.Claims();at+=32)if(!Window([&](GcRegistryAccess& access)noexcept{
            return lease_.RequiredRange(access,at,(std::min)(32u,lease_.Claims()-at));}))return false;
        return Current();
    }
    bool RetainConstructorState(const cfgmenu::pickerprepare::ConstructorState& state)noexcept{
        if(!borrowed_||!state.identity.HasAssignedSerial())return false;
        Reader reader;reader.cap=1024;
        const std::array<std::uintptr_t,4> values{state.state,state.playerState.field.object,state.playerState.field.type,state.playerState.field.owner};
        return Window([&](GcRegistryAccess& access)noexcept{
            cfgprobe::lifetime::SlotObservation slot;cfgprobe::lifetime::ArrayHeader registry;
            return cfgprobe::lifetime::ReadArrayHeader(reader,plan.module+cfgprobe::lifetime::ObjectArrayRva,registry)&&
                cfgprobe::lifetime::ReadSlot(reader,registry,state.state,slot)&&slot==state.identity&&
                Values(reader,values)&&lease_.Retain(access,values);
        });
    }
    bool BeginOriginal()noexcept{
        if(!borrowed_||originalForwarding_||!Required()||!Window([&](GcRegistryAccess& a)noexcept{return lease_.PrepareCall(a,0);})){
            frontendRetentionReport.constructStage=24;return false;}
        GcRegistryAccess unlocked(Proof());if(!lease_.EnterNative(unlocked)){
            lease_.Uncertain();frontendRetentionReport.constructStage=24;return false;}
        originalForwarding_=true;++frontendRetentionReport.constructForwards;return true;
    }
    bool EndOriginal(bool returned)noexcept{
        originalForwarding_=false;
        const std::array<std::uintptr_t,1> empty{};
        if(!returned||!Window([&](GcRegistryAccess& a)noexcept{return lease_.CompleteCall(a,empty,0);})||!Required()){
            lease_.Uncertain();Record(11);return false;}
        return true;
    }
    void Uncertain()noexcept{lease_.Uncertain();Record(12);}
    // Same completed menu callback/lease, but a distinct, path-allowlisted
    // startup capability. It cannot forward Start/Intro or load arbitrary assets.
    cfgmenu::startupretain::Hooks ChallengePageHooks()noexcept{
        using Call=cfgmenu::startupretain::Call;
        return {this,
            [](void* p,std::uintptr_t m){auto& s=Self(p);return !s.borrowed_&&!s.challengeTail_&&s.admitted_&&m==s.binding_.context.module&&s.Current();},
            [](void* p,const std::array<std::uintptr_t,32>& values,unsigned count){auto& s=Self(p);
                if(s.borrowed_||count>values.size())return false;Reader r;r.cap=4096;
                return s.Window([&](GcRegistryAccess& a)noexcept{return s.Values(r,values)&&s.lease_.Retain(a,values);});},
            [](void* p,Call call,const std::array<std::uintptr_t,32>& values,unsigned count,unsigned outputs){auto& s=Self(p);
                if(s.borrowed_||!s.admitted_||call!=Call::FrontendChallengePageClass||count>values.size()||outputs!=1||!s.Required())return false;
                const auto& c=s.binding_.context;const std::array<std::uintptr_t,6> owners{c.world,c.mode,c.controller,c.hud,c.widget,c.type};
                Reader r;r.cap=4096;
                const bool okay=s.Window([&](GcRegistryAccess& a)noexcept{
                    return s.Values(r,owners)&&s.Values(r,values)&&s.lease_.Retain(a,owners)&&s.lease_.Retain(a,values)&&s.lease_.PrepareCall(a,1);});
                ++frontendRetentionReport.inputs;s.Record(okay?0u:1u);return okay;},
            [](void* p){auto& s=Self(p);GcRegistryAccess unlocked(s.Proof());
                const bool okay=s.Current()&&s.lease_.EnterNative(unlocked);
                if(okay)++frontendRetentionReport.nativeEntries;s.Record(okay?0u:5u);return okay;},
            [](void* p,const std::array<std::uintptr_t,32>& values,unsigned count){auto& s=Self(p);Reader r;r.cap=4096;
                const bool okay=count==1&&s.Window([&](GcRegistryAccess& a)noexcept{
                    return s.Values(r,values)&&s.lease_.CompleteCall(a,values,1);})&&s.Required();
                if(okay)++frontendRetentionReport.outputs;s.Record(okay?0u:2u);return okay;},
            [](void* p){auto& s=Self(p);if(!s.lease_.EmptyIdle()&&
                !s.Window([&](GcRegistryAccess& a)noexcept{return s.lease_.CancelCall(a);}))s.lease_.Uncertain();},
            [](void* p){Self(p).Uncertain();}};
    }
    cfgmenu::rows::NativeRetentionHooks Hooks()noexcept{
        return {this,
            [](void* p,std::uintptr_t m){auto& s=Self(p);return m==s.binding_.context.module&&s.Current();},
            [](void* p,std::uintptr_t,std::uintptr_t a,std::uintptr_t f,void* packet){auto& s=Self(p);++frontendRetentionReport.inputs;
                const bool okay=s.Inputs(a,f,packet);s.Record(okay?0u:1u);return okay;},
            [](void* p,std::uintptr_t,std::uintptr_t f,void* packet){auto& s=Self(p);const bool okay=s.Outputs(f,packet);
                if(okay)++frontendRetentionReport.outputs;s.Record(okay?0u:2u);return okay;},
            [](void* p,std::uintptr_t,std::uintptr_t,std::uintptr_t owner){auto& s=Self(p);++frontendRetentionReport.factories;
                const bool okay=s.FactoryInput(owner);s.Record(okay?0u:3u);return okay;},
            [](void* p,std::uintptr_t,std::uintptr_t object){auto& s=Self(p);const bool okay=s.FactoryOutput(object);
                if(okay)++frontendRetentionReport.outputs;s.Record(okay?0u:4u);return okay;},
            [](void* p){Self(p).lease_.Uncertain();},
            [](void* p){auto& s=Self(p);if(!s.lease_.EmptyIdle()&&
                !s.Window([&](GcRegistryAccess& access)noexcept{return s.lease_.CancelCall(access);}))s.lease_.Uncertain();
                s.challengeTransition_=0;s.challengeType_=0;s.challengeTransitionFunction_=0;},
            [](void* p){auto& s=Self(p);GcRegistryAccess unlocked(s.Proof());const bool okay=s.Current()&&s.lease_.EnterNative(unlocked);
                if(okay&&s.challengeTransition_==3)s.challengeStartEntered_=true;
                if(okay)++frontendRetentionReport.nativeEntries;s.Record(okay?0u:5u);return okay;},
            [](void* p,std::uintptr_t m,std::uintptr_t f,void* packet){auto& s=Self(p);
                return m==s.binding_.context.module&&s.CurrentForOutputs(f,packet);}};
    }
};
