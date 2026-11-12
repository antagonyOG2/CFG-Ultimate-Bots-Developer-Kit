// Included inside the observer namespace after Reader and native context helpers.
#include "GcOwnerDiagnosticRegistry.hpp"
#ifdef CFG_SYNCHRONIZED_ROOT_PROBE
cfgretain::SynchronizedRootOwner synchronizedProbeOwner;
cfggcretain::Owner& gcRegistryOwner=synchronizedProbeOwner.native;
cfgrootprobe::Report rootPublicationReport;
#else
cfggcretain::Owner gcRegistryOwner; // Pinned DLL storage; NEVER a stack owner.
#endif
cfggcprobe::Report gcRegistryReport;
cfggcretain::ReferencerWindowFailure gcRegistryReferencerFailure=cfggcretain::ReferencerWindowFailure::None;
enum class GcRegistryGateFailure : std::uint32_t { None, WrongThread, DispatchInactive,
    WrongDispatchThread, Serializing, Collecting, ContextChanged, TickMismatch };
class GcRegistryAccess {
#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
public:
    struct NativeContextProof {
        void* context=nullptr;
        bool (*current)(Reader&,void*)=nullptr;
        bool (*completed)(Reader&,void*,std::uintptr_t,std::uintptr_t)=nullptr;
    };
private:
    NativeContextProof frontendProof_{};
#endif
    Reader reader_;bool hashHeld_=false,listHeld_=false,lockContended_=false;
    std::uintptr_t referencer_=0,type_=0,retainedTickActor_=0;
    static bool Writable(std::uintptr_t address,std::size_t bytes) noexcept {
        MEMORY_BASIC_INFORMATION m{};
        if(!cfgprobe::lifetime::Extent(address,bytes)||
           VirtualQuery(reinterpret_cast<void*>(address),&m,sizeof(m))!=sizeof(m)||
           m.State!=MEM_COMMIT||(m.Protect&(PAGE_GUARD|PAGE_NOACCESS)))return false;
        const auto protection=m.Protect&0xFF;
        const auto base=reinterpret_cast<std::uintptr_t>(m.BaseAddress);
        return (protection==PAGE_READWRITE||protection==PAGE_WRITECOPY)&&address>=base&&
            address-base<=m.RegionSize&&bytes<=m.RegionSize-(address-base);
    }
    template<std::size_t N>bool Code(std::uintptr_t rva,const std::array<unsigned char,N>& expected)noexcept{
        std::array<unsigned char,N> actual{};
        return reader_.Read(plan.module+rva,actual)&&actual==expected;
    }
    bool Current() noexcept {
        std::uint8_t initialized=0;std::uint32_t thread=0;
        const auto fail=[&](GcRegistryGateFailure reason){
            gcRegistryReport.gateFailure=static_cast<std::uint32_t>(reason);return false;
        };
        if(GetCurrentThreadId()!=plan.threadId)return fail(GcRegistryGateFailure::WrongThread);
        if(!reader_.Read(plan.module+cfgcompat::cfg35::dispatch::ThreadFlagRva,initialized)||initialized!=1)
            return fail(GcRegistryGateFailure::DispatchInactive);
        if(!reader_.Read(plan.module+cfgcompat::cfg35::dispatch::ThreadIdRva,thread)||thread!=plan.threadId)
            return fail(GcRegistryGateFailure::WrongDispatchThread);
        std::uint8_t serializing=1;std::uint32_t collecting=1;
        if(!reader_.Read(plan.module+cfgprimary::SerializationFlag,serializing)||serializing)
            return fail(GcRegistryGateFailure::Serializing);
        if(!reader_.Read(plan.module+cfgprimary::GarbageCollectionState,collecting)||collecting)
            return fail(GcRegistryGateFailure::Collecting);
#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
        if(frontendProof_.context){
            if(!frontendProof_.current||!frontendProof_.completed||!frontendProof_.current(reader_,frontendProof_.context))
                return fail(GcRegistryGateFailure::ContextChanged);
            gcRegistryReport.contextFailure=0;gcRegistryReport.gateFailure=0;return true;
        }
#endif
        // Object allocation can grow or relocate GUObjectArray between our
        // plan snapshot and this completed native tick. Refresh only that
        // global header, then retain the exact world/mode/controller/pawn
        // pointers and positive serials. Do not adopt replacement owners.
        cfgobserver::Plan currentPlan{};
        const auto contextKind=retainedTickActor_?cfgprimary::RetentionContext::ScopedWorld:cfgprimary::RetentionContext::Human;
        if(!cfgprimary::RefreshRetentionContext(reader_,plan,currentPlan,contextKind)){
            const auto context=cfgobserver::DiagnoseContextFailure(reader_,plan);
            gcRegistryReport.contextFailure=static_cast<std::uint32_t>(context);
            return fail(GcRegistryGateFailure::ContextChanged);
        }
        gcRegistryReport.contextFailure=static_cast<std::uint32_t>(cfgobserver::ContextFailure::None);
        gcRegistryReport.gateFailure=static_cast<std::uint32_t>(GcRegistryGateFailure::None);return true;
    }
public:
    GcRegistryAccess()noexcept{reader_.cap=1024;}
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    explicit GcRegistryAccess(std::uintptr_t completedTickActor,unsigned readCap=1024)noexcept:
        retainedTickActor_(completedTickActor){reader_.cap=readCap<=8192?readCap:1024;}
#endif
#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
    explicit GcRegistryAccess(NativeContextProof proof,unsigned readCap=8192)noexcept:frontendProof_(proof){
        reader_.cap=readCap>=8192&&readCap<=131072?readCap:8192;
    }
#endif
    bool Begin(std::uintptr_t function,std::uintptr_t pc)noexcept{
        lockContended_=false;
        gcRegistryReferencerFailure=cfggcretain::ReferencerWindowFailure::None;
        namespace d=cfggcretain::disk;namespace p=cfgcompat::cfg35::gcRetention;
        gcRegistryReport.stage=1;
        if(!Current())return false;
        cfgtick::Failure tickFailure{};
        const auto tickReadsBefore=reader_.reads;
        const auto tickActor=retainedTickActor_?retainedTickActor_:plan.pawn;
        bool tickObserved=false;
#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
        if(frontendProof_.context)tickObserved=frontendProof_.completed(reader_,frontendProof_.context,function,pc);
        else
#endif
        tickObserved=cfgtick::Observe(reader_,plan.module,tickActor,function,pc,&tickFailure);
        gcRegistryReport.tickFailure=static_cast<std::uint32_t>(tickFailure);
        gcRegistryReport.tickReads=reader_.reads-tickReadsBefore;
        if(!tickObserved){
            gcRegistryReport.gateFailure=static_cast<std::uint32_t>(GcRegistryGateFailure::TickMismatch);return false;
        }
        gcRegistryReport.stage=2;
        if(!Code(d::RegisterRva,d::RegisterCode)||!Code(d::UnregisterRva,d::UnregisterCode)||
           !Code(d::HashLockGetterRva,d::HashLockGetterCode)||!Code(d::HashLockAcquireRva,d::HashLockAcquireCode)||
           !Code(d::HashLockReleaseRva,d::HashLockReleaseCode)||!Code(d::RegistryAddUniqueRva,d::RegistryAddUniqueCode)||
           !Code(d::RegistryRemoveRva,d::RegistryRemoveCode))return false;
        // Getter's initialized static epoch excludes zero/uninitialized and
        // -1/construction-in-progress. Never run its lazy constructor here.
        std::int32_t epoch=0;
        if(!reader_.Read(plan.module+p::HashLockInitGuardRva,epoch)||epoch>=-1||
           !Writable(plan.module+p::HashLockGlobalRva,sizeof(CRITICAL_SECTION)))return false;
        gcRegistryReport.stage=3;
        if(!TryEnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(plan.module+p::HashLockGlobalRva))){lockContended_=true;return false;}
        hashHeld_=true;
        // Required AFTER taking this recursive lock: do not enter from GC itself.
        cfggcretain::Observation observation;
        if(!Current()||!cfggcretain::ObserveReferencerWindow(reader_,plan.module,
            [&](const cfggcretain::Observation& endpoint)noexcept{
                referencer_=endpoint.object;type_=endpoint.type;
                gcRegistryReport.referencer=referencer_;gcRegistryReport.stage=4;
                if(!Writable(referencer_+p::OwnerListLock,sizeof(CRITICAL_SECTION)))return false;
                if(!TryEnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(referencer_+p::OwnerListLock))){lockContended_=true;return false;}
                listHeld_=true;return Current();
            },observation,gcRegistryReferencerFailure))return false;
        // Register/Unregister enter this SAME recursive list lock. Holding it
        // here makes the capacity proof atomic with AddUnique, even against a
        // foreign native registrant. Never wait for either lock.
        return Current();
    }
    bool Snapshot(cfggcretain::ReferencerList& out,std::array<std::uintptr_t,cfggcprobe::MaximumOwners>& entries)noexcept{
        namespace p=cfgcompat::cfg35::gcRetention;
        out={};entries={};std::uintptr_t value=0;
        if(!hashHeld_||!listHeld_||!Current()||
           !reader_.Read(plan.module+p::ReferencerGlobalRva,value)||value!=referencer_||
           !reader_.Read(plan.module+p::ReferencerClassCacheRva,value)||value!=type_||
           !reader_.Read(referencer_+cfgprobe::ObjectClass,value)||value!=type_||
           !reader_.Read(referencer_+p::OwnerArray,out)||out.count<0||
           out.count>cfggcprobe::MaximumOwners||out.capacity<out.count||out.capacity>65536||
           !Writable(referencer_+p::OwnerArray,sizeof(out))||
           !Writable(out.data,(static_cast<std::size_t>(out.count)+(out.capacity>out.count?1u:0u))*sizeof(std::uintptr_t)))return false;
        return !out.count||reader_.ReadBytes(out.data,entries.data(),out.count*sizeof(std::uintptr_t));
    }
    // Caller must ALSO serialize against its own exact known Submit callback.
    // This proves only that this access instance still holds the independently
    // acquired engine/list window. It is NOT a claim that GC workers joined.
    bool ProvenSlotWindow()noexcept{return hashHeld_&&listHeld_&&Current();}
    bool NativeEntryUnlocked()const noexcept{return !hashHeld_&&!listHeld_;}
    bool LockContended()const noexcept{return lockContended_;}
    // Diagnostic only: a single nonblocking hash-lock sample, no registry/list
    // window or slot publication. Unknown proof is not reported as no contention.
    bool ObserveHashContention(bool& contended)noexcept{
        namespace p=cfgcompat::cfg35::gcRetention;
        contended=false;
        if(!Current()||!Writable(plan.module+p::HashLockGlobalRva,sizeof(CRITICAL_SECTION)))return false;
        auto* lock=reinterpret_cast<CRITICAL_SECTION*>(plan.module+p::HashLockGlobalRva);
        contended=!TryEnterCriticalSection(lock);
        if(!contended)LeaveCriticalSection(lock);
        return true;
    }
#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) || defined(CFG_SYNCHRONIZED_ROOT_PROBE)
    bool RegisterResidentOwner(cfgretain::SynchronizedRootOwner& owner,cfggcprobe::Report& outcome)noexcept{
        return cfgretain::RegisterResidentRootOwnerLocked(*this,owner,outcome);
    }
#endif
    bool Register(std::uintptr_t owner)noexcept{
        cfggcdiagnostic::registeredOwners.Track(owner); // mod-owned native referencer, not a UObject/root acquisition
        using Fn=void(__fastcall*)(void*,void*);
        reinterpret_cast<Fn>(plan.module+cfggcretain::disk::RegisterRva)(
            reinterpret_cast<void*>(referencer_),reinterpret_cast<void*>(owner));return true;
    }
    bool Unregister(std::uintptr_t owner)noexcept{
        using Fn=void(__fastcall*)(void*,void*);
        reinterpret_cast<Fn>(plan.module+cfggcretain::disk::UnregisterRva)(
            reinterpret_cast<void*>(referencer_),reinterpret_cast<void*>(owner));return true;
    }
    void End()noexcept{
        namespace p=cfgcompat::cfg35::gcRetention;
        if(listHeld_){listHeld_=false;LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(referencer_+p::OwnerListLock));}
        if(hashHeld_){hashHeld_=false;LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(plan.module+p::HashLockGlobalRva));}
        gcRegistryReport.reads=reader_.reads;gcRegistryReport.error=reader_.error;
    }
};
#ifdef CFG_GC_REGISTRY_PROBE
void PublishGcOwnerCounters()noexcept{
    InterlockedExchange(reinterpret_cast<volatile LONG*>(&gcRegistryReport.callbacks),
        static_cast<LONG>(gcRegistryOwner.callbacks.load(std::memory_order_relaxed)));
    InterlockedExchange(reinterpret_cast<volatile LONG*>(&gcRegistryReport.permanent),
        gcRegistryOwner.state.load(std::memory_order_acquire)==cfggcretain::State::Registered?1:0);
}
void ServiceGcRegistryProbe(void* object,void* function,bool nested,bool returned,std::uintptr_t pc)noexcept{
    // Foreign threads must not even read owner-thread mutable report storage.
    if(GetCurrentThreadId()!=plan.threadId)return;
#ifdef CFG_SYNCHRONIZED_ROOT_PROBE
    using Stage=cfgrootprobe::Stage;
    auto& output=rootPublicationReport;
    if(output.stage==static_cast<unsigned>(Stage::Observed)||output.stage==static_cast<unsigned>(Stage::Refused))return;
    if(nested||!returned||reinterpret_cast<std::uintptr_t>(object)!=plan.pawn||pc!=plan.module+cfgtick::disk::ReturnPc)return;
    if(synchronizedProbeOwner.unknownSubmit.load()){output.reason=1;output.stage=static_cast<unsigned>(Stage::Refused);return;}
    if(output.stage==static_cast<unsigned>(Stage::HumanRetained)&&
       gcRegistryOwner.completedCallbacks.load()<=output.callbacksAtHumanPublication)return;
    if(output.stage==static_cast<unsigned>(Stage::Swapped)){
        if(gcRegistryOwner.completedCallbacks.load()>output.callbacksAtSwap)output.stage=static_cast<unsigned>(Stage::Observed);
        return;
    }
    GcRegistryAccess access;
    __try{
        if(!access.Begin(reinterpret_cast<std::uintptr_t>(function),pc)){
            output.reason=2;output.stage=static_cast<unsigned>(Stage::Refused);return;
        }
        if(output.stage==static_cast<unsigned>(Stage::Waiting)){
            synchronizedProbeOwner.module=plan.module;output.module=plan.module;output.world=plan.world;output.human=plan.pawn;
            if(!access.RegisterResidentOwner(synchronizedProbeOwner,gcRegistryReport)){
                output.reason=3;output.stage=static_cast<unsigned>(Stage::Refused);return;
            }
            output.owner=reinterpret_cast<std::uintptr_t>(&gcRegistryOwner);output.registrations=1;
            if(!synchronizedProbeOwner.TryPublication(access,[&](cfggcretain::Owner& root)noexcept{
                root.slots[0]=root.expected[0]=plan.pawn;
                output.callbacksAtHumanPublication=root.completedCallbacks.load();
                output.submissionsAtHumanPublication=root.submissions.load();return true;})){
                output.reason=4;output.stage=static_cast<unsigned>(Stage::Refused);return;
            }
            gcRegistryReport.pawn=plan.pawn;gcRegistryReport.references=1;
            output.stage=static_cast<unsigned>(Stage::HumanRetained);
        }else{
            if(!synchronizedProbeOwner.TryPublication(access,[&](cfggcretain::Owner& root)noexcept{
                if(!root.completedCallbacks.load()||root.slots[0]!=plan.pawn||root.expected[0]!=plan.pawn)return false;
                output.callbacksAtSwap=root.completedCallbacks.load();output.submissionsAtSwap=root.submissions.load();
                root.slots[1]=root.expected[1]=plan.pawn;root.slots[0]=root.expected[0]=plan.world;return true;})){
                output.reason=5;output.stage=static_cast<unsigned>(Stage::Refused);return;
            }
            output.swaps=1;gcRegistryReport.references=2;output.stage=static_cast<unsigned>(Stage::Swapped);
        }
    }__finally{access.End();PublishGcOwnerCounters();}
    return;
#else
    if(InterlockedCompareExchange(reinterpret_cast<volatile LONG*>(&gcRegistryReport.published),0,0)){
        PublishGcOwnerCounters();return;
    }
    if(!cfggcprobe::Candidate(true,nested,returned,reinterpret_cast<std::uintptr_t>(object)==plan.pawn,
       pc==plan.module+cfgtick::disk::ReturnPc,gcRegistryReport.published!=0))return;
    GcRegistryAccess access;
    gcRegistryReport.result=cfggcprobe::Result::Refused;
    __try{
        if(access.Begin(reinterpret_cast<std::uintptr_t>(function),pc))
            cfggcprobe::RegisterPermanentPawnLocked(access,gcRegistryOwner,gcRegistryReport,plan.pawn);
    }__finally{
        access.End();MemoryBarrier();
        PublishGcOwnerCounters();
        InterlockedExchange(reinterpret_cast<volatile LONG*>(&gcRegistryReport.published),1);
    }
#endif
}
#endif
