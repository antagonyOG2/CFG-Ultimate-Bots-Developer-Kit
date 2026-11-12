// Included inside the observer namespace AFTER GcRegistryAccess and Reader.
// This is the existing startup coordinator's object-lifetime provider, not AI.
cfgretain::ResidentRootBank<32> startupRootBank;
class NativeStartupRootSession {
    cfgmenu::promotion::Context owner_{};
    cfgretain::CallbackRootLease<decltype(startupRootBank)> lease_;
    bool callback_=false,admitted_=false,finished_=false;
    cfgmenu::startupretain::Role role_=cfgmenu::startupretain::Role::Intro;
    std::uintptr_t pc_=0,function_=0;
    DWORD thread_=0;
    void Publish(unsigned failure=0)noexcept{
        auto& output=frontendRetentionReport;
        InterlockedIncrement(reinterpret_cast<volatile LONG*>(&output.generation));
        output.startupRoots=startupRootBank.Owners();output.startupClaims=lease_.Claims();
        output.startupPoisoned=startupRootBank.Poisoned()?1u:0u;
        if(failure){++output.startupFailures;output.startupLastFailure=failure;}
        output.startupRegistryReads=gcRegistryReport.reads;
        MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&output.generation));
    }
    template<class R> bool Context(R& reader)noexcept{
        if(!callback_||finished_||GetCurrentThreadId()!=thread_||thread_!=observerOwnerThread||
           owner_.module!=plan.module||!cfgprimary::LookupVetoClearForObservation(reader,owner_.module))return false;
        // Context is the INDEPENDENT engine/window proof. Never query the
        // bank/lease here: Grow deliberately marks its bank uncertain while
        // registering a new resident owner. Reentering Healthy from its own
        // registry window makes legitimate registration reject itself.
        // Bank/lease health remains mandatory at Window/Current/Required entry
        // and within the bank's publication operations, not inside this proof.
        cfgprobe::lifetime::ArrayHeader registry;
        // Full typed Level/class/owner proof is renewed once before every
        // window. Inside the non-yielding window retain exact positive owners
        // and live engine/world links, never reinterpret the old frontend plan.
        return cfgprobe::lifetime::ReadArrayHeader(reader,owner_.module+cfgprobe::lifetime::ObjectArrayRva,registry)&&
            cfgprobe::lifetime::RevalidateBatch(reader,owner_.module+cfgprobe::lifetime::ObjectArrayRva,registry,owner_.identities)&&
            cfgmenu::offlineowner::Same(reader,owner_.engine+cfgprobe::EngineGameViewport,owner_.viewport)&&
            cfgmenu::offlineowner::Same(reader,owner_.viewport+cfgprobe::ViewportWorld,owner_.world)&&
            cfgmenu::offlineowner::Same(reader,owner_.world+cfgprobe::WorldGameMode,owner_.mode)&&
            cfgmenu::offlineowner::Same(reader,owner_.world+cfgprobe::WorldNetDriver,std::uintptr_t{0});
    }
    GcRegistryAccess::NativeContextProof Proof()noexcept{
        return {this,[](Reader& r,void* p){return static_cast<NativeStartupRootSession*>(p)->Context(r);},
            [](Reader& r,void* p,std::uintptr_t f,std::uintptr_t pc){auto& self=*static_cast<NativeStartupRootSession*>(p);
                return self.callback_&&self.function_==f&&self.pc_==pc&&self.Context(r);}};
    }
    bool OwnerCurrent()noexcept{
        Reader reader;reader.cap=8192;
        cfgprobe::lifetime::ArrayHeader registry;
        if(!Context(reader)||!cfgprobe::lifetime::ReadArrayHeader(reader,owner_.module+cfgprobe::lifetime::ObjectArrayRva,registry))return false;
        owner_.registry=registry;
        return cfgmenu::offlineowner::Current(reader,owner_,thread_,nullptr,cfgmenu::offlineowner::Purpose::DiagnosticReadOnly);
    }
    template<class Work>bool Window(Work&& work)noexcept{
        // Startup session can retain multiple complete Blueprint field chains.
        // Keep a separate explicit bounded registry budget, never use the
        // already-spent startup/native-call reader for publication completion.
        unsigned attempts=0;
        const bool result=cfgretain::PublicationWindow(
            [&](){return lease_.Current()&&OwnerCurrent();},
            [&](bool& acquired,bool& contended)noexcept{
                GcRegistryAccess access(Proof(),131072);bool okay=false;
                __try{acquired=access.Begin(function_,pc_);if(acquired)okay=work(access);}
                __finally{access.End();}
                frontendRetentionReport.startupWindowStage=gcRegistryReport.stage;
                frontendRetentionReport.startupWindowGate=gcRegistryReport.gateFailure;
                frontendRetentionReport.startupWindowContended=access.LockContended()?1u:0u;
                contended=access.LockContended();return okay;
            },[](){return GetTickCount64();},[](){SwitchToThread();},attempts,nullptr,cfgretain::StartupPublicationLimits);
        frontendRetentionReport.startupWindowAttempts+=attempts;
        if(attempts>1)frontendRetentionReport.startupLockRetries+=attempts-1;
        if(!result)frontendRetentionReport.startupWindowRefusals++;
        return result;
    }
    bool Values(Reader& reader,const std::array<std::uintptr_t,32>& values,unsigned count)noexcept{
        cfgprobe::lifetime::ArrayHeader registry;cfgprobe::lifetime::SlotObservation slot;
        if(count>values.size()||!cfgprobe::lifetime::ReadArrayHeader(reader,owner_.module+cfgprobe::lifetime::ObjectArrayRva,registry))return false;
        for(unsigned i=0;i<count;++i)if(values[i]&&!cfgspawn::ObjectSlot(reader,registry,values[i],slot))return false;
        return true;
    }
    bool Required()noexcept{
        for(unsigned i=0;i<lease_.Claims();i+=32){
            const auto count=(std::min)(32u,lease_.Claims()-i);
            if(!Window([&](GcRegistryAccess& a)noexcept{return lease_.RequiredRange(a,i,count);}))return false;
        }
        return lease_.Current()&&OwnerCurrent();
    }
    bool Retain(const std::array<std::uintptr_t,32>& inputs,unsigned count)noexcept{
        if(count>inputs.size())return false;Reader reader;reader.cap=2048;
        const bool okay=Window([&](GcRegistryAccess& a)noexcept{return Values(reader,inputs,count)&&lease_.Retain(a,inputs);});
        Publish(okay?0u:3u);return okay;
    }
public:
    explicit NativeStartupRootSession(const cfgmenu::promotion::Context& owner)noexcept:
        owner_(owner),lease_(startupRootBank,owner.module),thread_(owner.thread){}
    bool Begin(cfgmenu::startupretain::Role role,const cfgmenu::promotion::Context& current,std::uintptr_t pc)noexcept{
        if(callback_||finished_||cfgmenu::startupretain::Scope::Occupied()||
           current.module!=owner_.module||current.thread!=thread_||current.world!=owner_.world||current.mode!=owner_.mode||
           current.controller!=owner_.controller||current.playerState!=owner_.playerState||current.identities!=owner_.identities)return false;
        role_=role;pc_=pc;owner_.registry=current.registry;
        function_=owner_.module+(role==cfgmenu::startupretain::Role::Intro?cfgmenu::offlinelifecycledisk::OfflineBotsIntro958:
            cfgmenu::offlinelifecycledisk::OfflineBotsMatchStarted818);
        callback_=true;
        std::array<std::uintptr_t,32> values{};const auto inputs=cfgmenu::promotion::RetainedInputs(owner_);
        std::copy(inputs.begin(),inputs.end(),values.begin());
        admitted_=Retain(values,static_cast<unsigned>(inputs.size()))&&Required();
        if(!admitted_){callback_=false;Publish(1);return false;}
        Publish();return true;
    }
    void End()noexcept{callback_=false;admitted_=false;}
    bool Retirable()const noexcept{
        return cfgstartupreuse::Retirable(finished_,callback_,admitted_,startupRootBank.Healthy(),lease_);
    }
    bool Current()noexcept{return admitted_&&lease_.Current()&&OwnerCurrent();}
    template<class R> bool LookupCurrent(R& reader)noexcept{return admitted_&&Context(reader)&&Required();}
    bool Finish()noexcept{
        if(!admitted_||!Required())return false;
        const bool okay=Window([&](GcRegistryAccess& a)noexcept{return lease_.Close(a);});
        if(okay)finished_=true;else lease_.Uncertain();Publish(okay?0u:6u);return okay;
    }
    void Uncertain()noexcept{lease_.Uncertain();Publish(5);}
    cfgmenu::startupretain::Hooks Hooks()noexcept{
        using namespace cfgmenu::startupretain;
        return {this,[](void* p,std::uintptr_t m){auto& self=*static_cast<NativeStartupRootSession*>(p);return m==self.owner_.module&&self.Current();},
            [](void* p,const std::array<std::uintptr_t,32>& values,unsigned count){return static_cast<NativeStartupRootSession*>(p)->Retain(values,count);},
            [](void* p,Call call,const std::array<std::uintptr_t,32>& values,unsigned count,unsigned maximum){auto& self=*static_cast<NativeStartupRootSession*>(p);
                auto& diagnostic=frontendRetentionReport;diagnostic.startupPrepareCall=static_cast<unsigned>(call);
                diagnostic.startupPrepareStage=1;if(!self.Required()){self.Publish(2);return false;}
                diagnostic.startupPrepareStage=2;if(!self.Retain(values,count)){self.Publish(2);return false;}
                diagnostic.startupPrepareStage=3;
                const bool okay=self.Window([&](GcRegistryAccess& a)noexcept{return self.lease_.PrepareCall(a,maximum);});
                if(!okay)self.Publish(2);else diagnostic.startupPrepareStage=0;return okay;},
            [](void* p){auto& self=*static_cast<NativeStartupRootSession*>(p);GcRegistryAccess access(self.Proof());
                const bool okay=self.Current()&&self.lease_.EnterNative(access);
                if(okay)++frontendRetentionReport.startupCalls;else self.Publish(4);return okay;},
            [](void* p,const std::array<std::uintptr_t,32>& values,unsigned count){auto& self=*static_cast<NativeStartupRootSession*>(p);
                Reader reader;reader.cap=2048;
                auto& diagnostic=frontendRetentionReport;
                diagnostic.startupPrepareStage=4; // output publication after ONE native return
                bool okay=self.Window([&](GcRegistryAccess& a)noexcept{return self.Values(reader,values,count)&&self.lease_.CompleteCall(a,values,count);});
                if(okay){diagnostic.startupPrepareStage=5;okay=self.Required();}
                if(okay)diagnostic.startupPrepareStage=0;
                if(okay)++frontendRetentionReport.startupReturns;self.Publish(okay?0u:5u);return okay;},
            [](void* p){auto& self=*static_cast<NativeStartupRootSession*>(p);
                if(!self.Window([&](GcRegistryAccess& a)noexcept{return self.lease_.CancelCall(a);}))self.Uncertain();},
            [](void* p){static_cast<NativeStartupRootSession*>(p)->Uncertain();}};
    }
};
std::unique_ptr<NativeStartupRootSession> nativeStartupRootSession;
