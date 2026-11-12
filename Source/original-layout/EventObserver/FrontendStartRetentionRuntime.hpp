// Retention for ONE proved borrowed native settings Start. Neither a HUD lease
// nor a native gameplay Intro lease may authorize this original FFrame forward.
class FrontendStartRootProvider {
    std::unique_ptr<cfgmenu::startboundary::Evidence> evidence_;
    cfgretain::CallbackRootLease<decltype(frontendRootBank)> lease_;
    bool admitted_=false,returned_=false;
    void Publish()noexcept{
        auto& output=frontendRetentionReport;
        InterlockedIncrement(reinterpret_cast<volatile LONG*>(&output.generation));
        output.startClaims=lease_.Claims();output.startLivePoison=frontendRootBank.Poisoned()?1u:0u;
        output.poisoned=output.startLivePoison;output.owners=frontendRootBank.Owners();
        MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&output.generation));
    }
    bool Context(Reader& reader)noexcept{
        return evidence_&&cfgprimary::LookupVetoClearForObservation(reader,evidence_->binding.context.module)&&
            (returned_?cfgmenu::startboundary::ReturnedInvocation(reader,*evidence_,GetCurrentThreadId()):
                cfgmenu::startboundary::Current(reader,*evidence_,GetCurrentThreadId()));
    }
    GcRegistryAccess::NativeContextProof Proof()noexcept{
        return {this,[](Reader& r,void* p){return static_cast<FrontendStartRootProvider*>(p)->Context(r);},
            [](Reader& r,void* p,std::uintptr_t f,std::uintptr_t pc){auto& self=*static_cast<FrontendStartRootProvider*>(p);
                return self.evidence_&&f==self.evidence_->node&&pc==self.evidence_->returnPc&&self.Context(r);}};
    }
    template<class Work>bool Window(Work&& work)noexcept{
        unsigned attempts=0;
        const bool okay=cfgretain::PublicationWindow([&](){return evidence_&&lease_.Current();},
            [&](bool& acquired,bool& contended)noexcept{GcRegistryAccess access(Proof(),32768);bool completed=false;
                __try{acquired=access.Begin(evidence_->node,evidence_->returnPc);if(acquired)completed=work(access);}
                __finally{access.End();}contended=access.LockContended();return completed;
            },[](){return GetTickCount64();},[](){Sleep(1);},attempts);
        if(attempts>1)frontendRetentionReport.startLockRetries+=attempts-1;return okay;
    }
    bool Required()noexcept{
        for(unsigned i=0;i<lease_.Claims();i+=16){const auto count=(std::min)(16u,lease_.Claims()-i);
            if(!Window([&](GcRegistryAccess& a)noexcept{return lease_.RequiredRange(a,i,count);}))return false;}
        Reader reader;reader.cap=2048;return admitted_&&Context(reader);
    }
public:
    FrontendStartRootProvider(const cfgmenu::startbinding::Binding& binding,const cfgmenu::startroute::OriginalFrame& frame,
        std::uintptr_t pc,std::uintptr_t wrapper,std::uintptr_t graph):lease_(frontendRootBank,binding.context.module){
        ++frontendRetentionReport.startCandidates;frontendRetentionReport.startPc=pc;
        frontendRetentionReport.startExpectedPc=binding.context.module+cfgmenu::constructdisk::Return;
        frontendRetentionReport.startFailure=1;
        if(cfgmenu::startupretain::Scope::Occupied()||cfgmenu::rows::NativeRetentionScope::Occupied())return;
        auto evidence=std::make_unique<cfgmenu::startboundary::Evidence>();Reader reader;reader.cap=16384;unsigned stage=0;
        if(!cfgmenu::startboundary::Observe(reader,binding,frame,pc,GetCurrentThreadId(),wrapper,graph,*evidence,&stage)){
            frontendRetentionReport.startFailure=10+stage;return;}
        evidence_=std::move(evidence);
        frontendRetentionReport.startFailure=20;
        for(unsigned first=0;first<evidence_->count;first+=16){std::array<std::uintptr_t,16> values{};
            if(!Window([&](GcRegistryAccess& a)noexcept{
                Reader slots;slots.cap=1024;cfgprobe::lifetime::ArrayHeader registry;
                if(!cfgprobe::lifetime::ReadArrayHeader(slots,binding.context.module+cfgprobe::lifetime::ObjectArrayRva,registry))return false;
                for(unsigned i=first;i<first+16&&i<evidence_->count;++i){cfgprobe::lifetime::SlotObservation current;
                    if(!cfgspawn::ObjectSlot(slots,registry,evidence_->roots[i].object,current)||!(current==evidence_->roots[i]))return false;
                    values[i-first]=current.object;}
                return lease_.Retain(a,values);}))return;
        }
        admitted_=true;++frontendRetentionReport.startAdmitted;frontendRetentionReport.startFailure=0;Publish();
    }
    ~FrontendStartRootProvider(){
        if(!lease_.EmptyIdle()){
            if(Window([&](GcRegistryAccess& a)noexcept{return lease_.Close(a);})){++frontendRetentionReport.startRetirements;frontendRetentionReport.startCompletionStage=3;}
            else {lease_.Uncertain();frontendRetentionReport.startCompletionStage=4;}
        }
        Publish();
    }
    bool Admitted()const noexcept{return admitted_;}
    bool LookupCurrent(Reader& reader)noexcept{return admitted_&&!returned_&&Context(reader)&&Required();}
    cfgmenu::startupretain::Hooks Hooks()noexcept{
        using namespace cfgmenu::startupretain;
        return {this,[](void* p,std::uintptr_t m){auto& self=*static_cast<FrontendStartRootProvider*>(p);
                return self.evidence_&&!self.returned_&&m==self.evidence_->binding.context.module&&self.Required();},
            // Only pre-acquired source dependencies are allowed. Start has no
            // general factory/output/root-acquisition authority.
            [](void*,const std::array<std::uintptr_t,32>&,unsigned){return false;},
            [](void* p,Call call,const std::array<std::uintptr_t,32>& values,unsigned count,unsigned outputs){auto& self=*static_cast<FrontendStartRootProvider*>(p);
                if(self.returned_||call!=Call::OriginalStart||outputs||count>values.size()||!self.Required())return false;
                for(unsigned i=0;i<count;++i)if(values[i]){bool found=false;
                    for(unsigned j=0;j<self.evidence_->count;++j)found|=values[i]==self.evidence_->roots[j].object;
                    if(!found)return false;}
                return self.Window([&](GcRegistryAccess& a)noexcept{return self.lease_.PrepareCall(a,0);});},
            [](void* p){auto& self=*static_cast<FrontendStartRootProvider*>(p);GcRegistryAccess access(self.Proof());
                const bool okay=self.Required()&&self.lease_.EnterNative(access);if(okay)++frontendRetentionReport.startCalls;return okay;},
            [](void* p,const std::array<std::uintptr_t,32>& values,unsigned count){auto& self=*static_cast<FrontendStartRootProvider*>(p);
                // Scope invokes after ONLY when the exact executor returned
                // normally. Switch proof here, never on a guessed world change.
                self.returned_=true;++frontendRetentionReport.startExecutorReturns;frontendRetentionReport.startCompletionStage=1;
                const bool okay=!count&&self.Window([&](GcRegistryAccess& a)noexcept{return self.lease_.CompleteCall(a,values,0);})&&self.Required();
                if(okay){++frontendRetentionReport.startReturns;frontendRetentionReport.startCompletionStage=2;}
                self.Publish();return okay;},
            [](void* p){auto& self=*static_cast<FrontendStartRootProvider*>(p);
                if(!self.Window([&](GcRegistryAccess& a)noexcept{return self.lease_.CancelCall(a);}))self.lease_.Uncertain();},
            [](void* p){auto& self=*static_cast<FrontendStartRootProvider*>(p);self.lease_.Uncertain();self.Publish();}};
    }
};
