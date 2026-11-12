#include "../Core/J22FeatureFlags.hpp"
#include "../Core/J21WorkPolicy.hpp"
// Included after GcRegistryAccess and the primary callback identifiers.
// Only the existing completed ReceiveTick service creates this capability.
#if defined(CFG_FRONTEND_BUNDLE)
// J1a's returned, typed native SCKillerAIController::Tick has a distinct,
// callback-local witness. It is never represented as a ReceiveTick UFunction.
// Only the native Tick's lexical provider may publish this pointer; nested
// callbacks are barred before service. Registry synchronization is unchanged.
thread_local const GcRegistryAccess::NativeContextProof* bundleNativeJasonTickProof=nullptr;
#endif
cfgretain::ResidentRootBank<32> bundleRootBank;
using BundleRootLease=cfgretain::CallbackRootLease<decltype(bundleRootBank)>;
BundleRootLease* bundleRootLease=nullptr; // native owner-thread only
// Frozen for one lexical lease. PE and native Tick cannot substitute witnesses
// into one another's publication/completion, including nested callbacks.
struct BundleRootWitness {
    BundleRootLease* lease=nullptr;
    std::uintptr_t actor=0,function=0,pc=0;
#ifdef CFG_FRONTEND_BUNDLE
    GcRegistryAccess::NativeContextProof native{};
#endif
} bundleRootWitness;
BundleRootLease::DeferredRetirement bundleDeferredRetirement; // bounded resident bookkeeping, never call authority
struct BundleRootDiagnostics {
    unsigned windowStage=0,windowGate=0,referencerFailure=0,strandedClaims=0;
    std::uint64_t attempts=0,retries=0,refusals=0,emptyRetirements=0;
    std::uint64_t deferredRetirements=0,drainedRetirements=0;
    std::uint64_t zeroOutputCompletions=0,zeroOutputContendedCompletions=0,zeroOutputProbeUnknown=0;
    unsigned failureTraces=0;
    cfgretain::PoisonEvidence firstPoison{};unsigned poisonWindowStage=0,poisonWindowGate=0;
    bool lastContention=false,lastUntouched=true,poisonContention=false,poisonUntouched=true,firstPoisonTracePending=false;
    // H2.91 timing only (QPC microseconds): whole window, contention sleeps, registry Begin, caller operation.
    std::uint64_t windows=0,windowUs=0,windowMaxUs=0,sleeps=0,sleepUs=0,beginUs=0,operationUs=0,endUs=0,provenSkips=0;
} bundleRootDiagnostics;
inline std::uint64_t BundleRootMicros()noexcept{LARGE_INTEGER t{},f{};QueryPerformanceCounter(&t);QueryPerformanceFrequency(&f);
    return f.QuadPart>0?static_cast<std::uint64_t>(t.QuadPart*1000000/f.QuadPart):0;}
void ObserveFirstBundlePoison(void*,const cfgretain::PoisonEvidence& evidence)noexcept{
    auto& d=bundleRootDiagnostics;if(d.firstPoison.site)return;
    d.firstPoison=evidence;d.poisonWindowStage=gcRegistryReport.stage;d.poisonWindowGate=gcRegistryReport.gateFailure;
    if(evidence.site==static_cast<unsigned>(cfgretain::PoisonSite::GrowRegistration)&&bundleRootLease)
        d.firstPoison.claims=bundleRootLease->Claims();
    d.poisonContention=d.lastContention;d.poisonUntouched=d.lastUntouched;
    // May run while a registry window/owner gate is held. Record only; file
    // tracing waits until the callback lease and all its windows have ended.
    d.firstPoisonTracePending=true;
}
void FlushFirstBundlePoisonTrace()noexcept{
    auto& d=bundleRootDiagnostics;if(bundleRootLease||!d.firstPoisonTracePending)return;
    d.firstPoisonTracePending=false;
#ifdef CFG_FRONTEND_BUNDLE
    cfgclosedtrace::Mark("root-first-poison",d.firstPoison.pc,d.firstPoison.site);
    cfgclosedtrace::Mark("root-first-poison-claims",d.firstPoison.claims,d.firstPoison.flags);
    cfgclosedtrace::Mark("root-first-poison-window",d.poisonWindowStage,
        (static_cast<std::uint64_t>(d.poisonWindowGate)<<32)|(d.poisonContention?1u:0u)|(d.poisonUntouched?2u:0u));
#endif
}
void PublishBundleRootState(unsigned failure=0,bool candidate=false,bool entered=false)noexcept;

template<class Operation>bool WithBundleRootWindow(Operation&& operation,bool* preWorkContention=nullptr,bool* untouched=nullptr)noexcept{
    if(preWorkContention)*preWorkContention=false;
    // True ONLY while no caller operation has begun. Refused admission cannot
    // have released claims; opaque idle retirement may wait for a fresh tick.
    if(untouched)*untouched=true;
    bundleRootDiagnostics.lastUntouched=true;bundleRootDiagnostics.lastContention=false;
    if(!bundleRootLease||bundleRootWitness.lease!=bundleRootLease||!bundleRootLease->Current()||
       GetCurrentThreadId()!=plan.threadId||!primaryRootTickActor||
       primaryRootTickActor!=bundleRootWitness.actor||primaryRootTickFunction!=bundleRootWitness.function||
       primaryRootTickReturn!=bundleRootWitness.pc)return false;
#ifdef CFG_FRONTEND_BUNDLE
    // As in native startup, the locked window proof must be independent of
    // bank health: Grow marks that bank uncertain while registering an owner.
    // Ordinary lookup/call authority still requires the healthy lease OUTSIDE
    // that operation; never recursively query it from its own publication proof.
    struct WindowContext {cfgobserver::Plan source;std::uintptr_t actor,function,pc;}
        context{plan,bundleRootWitness.actor,bundleRootWitness.function,bundleRootWitness.pc};
    const auto current=[](Reader& reader,void* value){const auto& saved=*static_cast<WindowContext*>(value);cfgobserver::Plan fresh;
        return GetCurrentThreadId()==saved.source.threadId&&plan.module==saved.source.module&&plan.world==saved.source.world&&
            plan.mode==saved.source.mode&&plan.controller==saved.source.controller&&plan.pawn==saved.source.pawn&&plan.objects==saved.source.objects&&
            cfgprimary::LookupVetoClearForObservation(reader,saved.source.module)&&cfgprimary::RetainedWorld(reader,saved.source,fresh);};
    GcRegistryAccess::NativeContextProof proof{&context,current,
        [](Reader& reader,void* value,std::uintptr_t function,std::uintptr_t pc){const auto& saved=*static_cast<WindowContext*>(value);
            return function==saved.function&&pc==saved.pc&&cfgtick::Observe(reader,saved.source.module,saved.actor,function,pc);}};
    if(bundleRootWitness.native.context)proof=bundleRootWitness.native;
    unsigned attempts=0;const auto windowStart=BundleRootMicros();
    const bool result=cfgretain::PublicationWindow(
        [&](){Reader admission;admission.cap=512;
            return bundleRootLease&&bundleRootLease->Current()&&GetCurrentThreadId()==context.source.threadId&&
                cfgprimary::ScopedLookupEligibility<Reader>::Current(admission,context.source.module);},
        [&](bool& acquired,bool& contended)noexcept{
            GcRegistryAccess registry(proof,8192);bool okay=false;std::uint64_t mark=BundleRootMicros();
            __try{acquired=registry.Begin(context.function,context.pc);
                {const auto t=BundleRootMicros();bundleRootDiagnostics.beginUs+=t-mark;mark=t;}
                bundleRootDiagnostics.windowStage=gcRegistryReport.stage;bundleRootDiagnostics.windowGate=gcRegistryReport.gateFailure;
                if(acquired){if(untouched)*untouched=false;bundleRootDiagnostics.lastUntouched=false;okay=operation(registry);
                    const auto t=BundleRootMicros();bundleRootDiagnostics.operationUs+=t-mark;mark=t;}}
            __finally{registry.End();bundleRootDiagnostics.endUs+=BundleRootMicros()-mark;}
            bundleRootDiagnostics.windowStage=gcRegistryReport.stage;
            bundleRootDiagnostics.windowGate=gcRegistryReport.gateFailure;
            bundleRootDiagnostics.referencerFailure=static_cast<unsigned>(gcRegistryReferencerFailure);
            contended=registry.LockContended();bundleRootDiagnostics.lastContention=contended;return okay;
        },[](){return GetTickCount64();},[](){const auto s=BundleRootMicros();Sleep(1);++bundleRootDiagnostics.sleeps;bundleRootDiagnostics.sleepUs+=BundleRootMicros()-s;},attempts,preWorkContention);
    {auto& d=bundleRootDiagnostics;const auto us=BundleRootMicros()-windowStart;++d.windows;d.windowUs+=us;if(us>d.windowMaxUs)d.windowMaxUs=us;}
    bundleRootDiagnostics.attempts+=attempts;
    if(attempts>1)bundleRootDiagnostics.retries+=attempts-1;
    if(!result){++bundleRootDiagnostics.refusals;
        if(bundleRootDiagnostics.failureTraces++<8){
            cfgclosedtrace::Mark("root-window-refused",context.actor,(static_cast<std::uint64_t>(gcRegistryReport.stage)<<32)|gcRegistryReport.gateFailure);
            cfgclosedtrace::Mark("root-window-tick",context.function,(static_cast<std::uint64_t>(gcRegistryReport.tickFailure)<<32)|gcRegistryReport.tickReads);
            cfgclosedtrace::Mark("root-window-native",context.pc,(static_cast<std::uint64_t>(bundleRootLease->NativePending())<<32)|bundleRootLease->Claims());
        }}
    PublishBundleRootState(result?0u:5u);return result;
#else
    // Separate bounded registry-validation budget: up to 32 owner groups.
    // Do not charge repeated engine/list window proofs to feature AI reads.
    GcRegistryAccess registry(primaryRootTickActor,8192);bool result=false;
    __try{
        if(registry.Begin(primaryRootTickFunction,primaryRootTickReturn)){if(untouched)*untouched=false;result=operation(registry);}
    }__finally{registry.End();}
    return result;
#endif
}
template<std::size_t N>bool ValidateBundleRootValues(Reader& reader,const std::array<std::uintptr_t,N>& values)noexcept{
    cfgobserver::Plan current{};cfgprobe::lifetime::SlotObservation observed{};
    if(!cfgprimary::RefreshRetentionContext(reader,plan,current,cfgprimary::RetentionContext::ScopedWorld))return false;
    // Caller supplies typed/current acquisitions (or fresh native results).
    // Do not require serial allocation for static UFunction/class metadata.
    // Cached dynamic identities still require their original positive-serial
    // comparisons at feature acquisition; this is not a raw-cache resolver.
    for(const auto value:values)if(value&&!cfgspawn::ObjectSlot(reader,current.objectArray,value,observed))return false;
    return true;
}
template<std::size_t N>bool RetainBundleCallbackValues(Reader& reader,const std::array<std::uintptr_t,N>& values)noexcept{
    (void)reader;
#if CFG_H292_ROOT_PROVEN_SKIP
    // H2.92: every value is already a claim of THIS callback's lease, resolved by a locked window after the most recent native
    // entry, and nothing has run since that could let the collector or game code touch it: the full window adds no new fact.
    // Any new value, any native entry, any other lease or thread still takes the full locked window below.
    // H2.96 (audit 2): the legacy non-lean ConvergenceAxeGrant calls native code without lease entry; never skip in that build.
    if(cfgj21::LeanEnabled&&bundleRootLease&&bundleRootWitness.lease==bundleRootLease&&GetCurrentThreadId()==plan.threadId&&
       primaryRootTickActor&&primaryRootTickActor==bundleRootWitness.actor&&bundleRootLease->ProvenSinceLastNative(values)){
        ++bundleRootDiagnostics.provenSkips;return true;}
#endif
    Reader publication;publication.cap=1024;
    return WithBundleRootWindow([&](GcRegistryAccess& registry)noexcept{
        return ValidateBundleRootValues(publication,values)&&bundleRootLease->Retain(registry,values);
    });
}
bool PrepareBundleCallbackCall(unsigned maximumOutputs)noexcept{
    return WithBundleRootWindow([&](GcRegistryAccess& registry)noexcept{return bundleRootLease->PrepareCall(registry,maximumOutputs);});
}
bool EnterBundleCallbackCall()noexcept{
    if(!bundleRootLease||GetCurrentThreadId()!=plan.threadId)return false;
    GcRegistryAccess unlocked(primaryRootTickActor);
    return bundleRootLease->EnterNative(unlocked);
}
bool CancelBundleCallbackCall()noexcept{
    return WithBundleRootWindow([&](GcRegistryAccess& registry)noexcept{return bundleRootLease->CancelCall(registry);});
}
template<std::size_t N>bool CompleteBundleCallbackCall(Reader& reader,const std::array<std::uintptr_t,N>& outputs,unsigned count=N)noexcept{
    (void)reader;
#ifdef CFG_FRONTEND_BUNDLE
    if constexpr(N==0){
      if(!count&&bundleRootLease&&bundleRootLease->ZeroOutputCallEntered()){
        bundleRootDiagnostics.lastUntouched=true;bundleRootDiagnostics.lastContention=false;
        // The native call returned normally and cannot return UObject outputs.
        // There is no collector publication to lock. Reprove THIS callback,
        // then clear only its local call flags; locked retirement still owns
        // all input claims. Output-producing calls retain the locked path.
        Reader witness;witness.cap=8192;
        const bool bound=bundleRootLease&&bundleRootWitness.lease==bundleRootLease&&
            GetCurrentThreadId()==plan.threadId&&primaryRootTickActor==bundleRootWitness.actor&&
            primaryRootTickFunction==bundleRootWitness.function&&primaryRootTickReturn==bundleRootWitness.pc;
        const bool callback=bound&&(bundleRootWitness.native.context?
            bundleRootWitness.native.current&&bundleRootWitness.native.completed&&
            bundleRootWitness.native.current(witness,bundleRootWitness.native.context)&&
            bundleRootWitness.native.completed(witness,bundleRootWitness.native.context,bundleRootWitness.function,bundleRootWitness.pc):
            cfgprimary::LookupVetoClearForObservation(witness,plan.module)&&
            cfgprimary::ScopedLookupEligibility<Reader>::Current(witness,plan.module)&&
            cfgtick::Observe(witness,plan.module,bundleRootWitness.actor,bundleRootWitness.function,bundleRootWitness.pc));
        if(!count&&callback&&bundleRootLease->CompleteReturnedWithoutOutputs()){
            ++bundleRootDiagnostics.zeroOutputCompletions;
            GcRegistryAccess diagnostic(bundleRootWitness.native.context?bundleRootWitness.native:
                GcRegistryAccess::NativeContextProof{},8192);
            bool contended=false;
            if(!diagnostic.ObserveHashContention(contended))++bundleRootDiagnostics.zeroOutputProbeUnknown;
            else if(contended)++bundleRootDiagnostics.zeroOutputContendedCompletions;
            return true;
        }
        if(bundleRootLease)bundleRootLease->Uncertain(cfgretain::PoisonSite::CompleteWindow,CFG_ROOT_POISON_PC);
        return false;
      }
    }
#endif
    {
    Reader publication;publication.cap=1024;
    const bool result=WithBundleRootWindow([&](GcRegistryAccess& registry)noexcept{
        return ValidateBundleRootValues(publication,outputs)&&bundleRootLease->CompleteCall(registry,outputs,count);
    });
    if(!result&&bundleRootLease)bundleRootLease->Uncertain(cfgretain::PoisonSite::CompleteWindow,CFG_ROOT_POISON_PC);
    return result;
    }
}
bool DrainBundleDeferredRetirement()noexcept{
    if(!bundleDeferredRetirement.Claims())return true;
    bool contention=false,untouched=false;
    const bool retired=WithBundleRootWindow([&](GcRegistryAccess& registry)noexcept{
        return bundleDeferredRetirement.Retire(registry,bundleRootBank,plan.module);
    },&contention,&untouched);
    if(retired){++bundleRootDiagnostics.drainedRetirements;PublishBundleRootState();return true;}
    // Still-held native roots survive a pre-work lock OR callback-proof refusal.
    // No retirement operation ran. The new callback skips ALL gameplay and
    // acquisition until this single record drains under fresh full admission.
    if(!untouched){bundleRootBank.MarkUncertain({static_cast<unsigned>(cfgretain::PoisonSite::DeferredRelease),bundleDeferredRetirement.Claims(),0,CFG_ROOT_POISON_PC});
        bundleRootDiagnostics.strandedClaims=(std::max)(bundleRootDiagnostics.strandedClaims,bundleDeferredRetirement.Claims());}
    PublishBundleRootState(untouched?0u:6u);return false;
}
class BundleCallbackRootScope {
    BundleRootLease lease_;bool entered_=false;
public:
    explicit BundleCallbackRootScope(std::uintptr_t module)noexcept:lease_(bundleRootBank,module){
        bundleRootBank.ObservePoison(&ObserveFirstBundlePoison,nullptr);
        if(GetCurrentThreadId()==plan.threadId&&!bundleRootLease&&!bundleRootWitness.lease&&lease_.Current()){
            bundleRootLease=&lease_;entered_=true;
            bundleRootWitness.lease=&lease_;bundleRootWitness.actor=primaryRootTickActor;
            bundleRootWitness.function=primaryRootTickFunction;bundleRootWitness.pc=primaryRootTickReturn;
#ifdef CFG_FRONTEND_BUNDLE
            if(bundleNativeJasonTickProof)bundleRootWitness.native=*bundleNativeJasonTickProof;
#endif
        }
    }
    ~BundleCallbackRootScope(){
        if(entered_){
            if(lease_.CloseEmpty()){
                ++bundleRootDiagnostics.emptyRetirements;PublishBundleRootState();
            }else{
                bool contention=false,untouched=false;
                if(WithBundleRootWindow([&](GcRegistryAccess& registry)noexcept{return lease_.Close(registry);},&contention,&untouched))PublishBundleRootState();
                else if(untouched&&lease_.TakeIdleClaims(bundleDeferredRetirement)){
                    ++bundleRootDiagnostics.deferredRetirements;PublishBundleRootState();
                }else{
                    bundleRootDiagnostics.strandedClaims=(std::max)(bundleRootDiagnostics.strandedClaims,lease_.Claims());
                    lease_.Uncertain(cfgretain::PoisonSite::ScopeClose,CFG_ROOT_POISON_PC);PublishBundleRootState(6);
                }
            }
            bundleRootLease=nullptr;
            bundleRootWitness={};
            FlushFirstBundlePoisonTrace(); // every registry.End() has completed
        }
    }
    bool Entered()const noexcept{return entered_;}
};
