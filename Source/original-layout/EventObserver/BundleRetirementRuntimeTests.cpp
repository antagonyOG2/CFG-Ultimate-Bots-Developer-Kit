#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define main ExistingLeaseFixtureMain
#include "CallbackRootLeaseTests.cpp"
#undef main
#include "NativePublicationWindow.hpp"
#define CFG_FRONTEND_BUNDLE
namespace {
struct Reader {unsigned cap=0;};
namespace cfgobserver {struct Plan {
    std::uintptr_t module=0x100000,world=1,mode=2,controller=3,pawn=4;
    std::array<int,4> objects{};int objectArray=0;DWORD threadId=GetCurrentThreadId();
};}
cfgobserver::Plan plan;
std::uintptr_t primaryRootTickActor=4,primaryRootTickFunction=5,primaryRootTickReturn=6;
Registry nativeRegistry;bool fixtureAdmission=true,tick=true,begin=true;unsigned operations=0,traces=0;
unsigned poisonTraces=0,traceWritesInsideWindow=0;
namespace cfgprimary {
template<class R>struct ScopedLookupEligibility {static bool Current(R&,std::uintptr_t){return fixtureAdmission;}};
template<class R>bool LookupVetoClearForObservation(R&,std::uintptr_t){return fixtureAdmission;}
template<class R>bool RetainedOfflineWorld(R&,const cfgobserver::Plan& p,cfgobserver::Plan& out){out=p;return fixtureAdmission;}
enum class RetentionContext {ScopedWorld};
template<class R>bool RefreshRetentionContext(R&,cfgobserver::Plan& p,cfgobserver::Plan& out,RetentionContext){out=p;return fixtureAdmission;}
}
namespace cfgprobe::lifetime {struct SlotObservation {};}
namespace cfgspawn {template<class R>bool ObjectSlot(R&,int,std::uintptr_t,cfgprobe::lifetime::SlotObservation&){return true;}}
namespace cfgtick {template<class R>bool Observe(R&,std::uintptr_t,std::uintptr_t,std::uintptr_t,std::uintptr_t){return tick;}}
struct {unsigned stage=1,gateFailure=0,tickFailure=0,tickReads=0;}gcRegistryReport;
unsigned gcRegistryReferencerFailure=0;
namespace cfgclosedtrace {void Mark(const char* name,std::uintptr_t,std::uint64_t){++traces;
    if(nativeRegistry.held)++traceWritesInsideWindow;
    if(std::strncmp(name,"root-first-poison",17)==0)++poisonTraces;}}
class GcRegistryAccess {
public:
    struct NativeContextProof {void* context;bool(*current)(Reader&,void*);bool(*completed)(Reader&,void*,std::uintptr_t,std::uintptr_t);};
    GcRegistryAccess(NativeContextProof,unsigned){}explicit GcRegistryAccess(std::uintptr_t){}
    bool Begin(std::uintptr_t,std::uintptr_t){nativeRegistry.held=begin&&tick;return nativeRegistry.held;}
    void End(){nativeRegistry.held=false;}
    bool LockContended()const{return false;}
    bool ObserveHashContention(bool& out){out=!begin;return true;}
    bool ProvenSlotWindow(){return nativeRegistry.held;}
    bool NativeEntryUnlocked(){return !nativeRegistry.held;}
    bool Snapshot(cfggcretain::ReferencerList& list,std::array<std::uintptr_t,cfggcprobe::MaximumOwners>& entries){return nativeRegistry.Snapshot(list,entries);}
    bool RegisterResidentOwner(cfgretain::SynchronizedRootOwner& owner,cfggcprobe::Report& report){return cfgretain::RegisterResidentRootOwnerLocked(*this,owner,report);}
    bool Register(std::uintptr_t address){return nativeRegistry.Register(address);}
};
void PublishBundleRootState(unsigned,bool,bool)noexcept{}
#include "BundleCallbackRootsRuntime.hpp"
}
int main(){
    {BundleCallbackRootScope scope(plan.module);Reader reader;CHECK(scope.Entered());
     CHECK(RetainBundleCallbackValues(reader,std::array<std::uintptr_t,1>{0x20000}));
     CHECK(bundleRootLease->Claims()==1);tick=false;
    }
    CHECK(bundleRootBank.Healthy()&&bundleDeferredRetirement.Claims()==1&&!bundleRootLease);
    {BundleCallbackRootScope scope(plan.module);CHECK(scope.Entered());
     CHECK(!DrainBundleDeferredRetirement()&&bundleDeferredRetirement.Claims()==1&&bundleRootBank.Healthy());}
    tick=true;
    {BundleCallbackRootScope scope(plan.module);CHECK(scope.Entered());
     CHECK(DrainBundleDeferredRetirement()&&!bundleDeferredRetirement.Claims()&&bundleRootBank.Healthy());}
    // A refused pre-work context likewise keeps native collector slots intact.
    {BundleCallbackRootScope scope(plan.module);Reader reader;
     CHECK(RetainBundleCallbackValues(reader,std::array<std::uintptr_t,1>{0x30000}));fixtureAdmission=false;}
    CHECK(bundleRootBank.Healthy()&&bundleDeferredRetirement.Claims()==1);
    fixtureAdmission=true;
    {BundleCallbackRootScope scope(plan.module);CHECK(DrainBundleDeferredRetirement());
     bool untouched=true;CHECK(!WithBundleRootWindow([&](GcRegistryAccess&)noexcept{++operations;return false;},nullptr,&untouched)&&!untouched&&operations==1);}
    CHECK(bundleRootBank.Healthy()&&!bundleDeferredRetirement.Claims());
    {BundleCallbackRootScope callback(plan.module);Reader reader;
     const auto localBefore=bundleRootDiagnostics.zeroOutputCompletions;
     CHECK(RetainBundleCallbackValues(reader,std::array<std::uintptr_t,1>{0x61000}));
     CHECK(PrepareBundleCallbackCall(1)&&EnterBundleCallbackCall());
     // An allocating call returning zero objects still must cancel/commit its
     // reserved output slots under the registry window, not the local shortcut.
     CHECK(CompleteBundleCallbackCall(reader,std::array<std::uintptr_t,0>{}));
     CHECK(bundleRootBank.Healthy()&&bundleRootDiagnostics.zeroOutputCompletions==localBefore);}
    // Fifteen minutes at60Hz, alternating native and PE callbacks. Menu return
    // and match2 happen halfway. Exercise real lease/witness/retirement code,
    // no engine calls. Forced completion contention must not poison zero-output
    // calls; claims remain rooted until a later fresh uncontended callback.
    unsigned nativeProofTag=1;
    GcRegistryAccess::NativeContextProof nativeProof{&nativeProofTag,
        [](Reader&,void* p){return tick&&*static_cast<unsigned*>(p)==1;},
        [](Reader&,void* p,std::uintptr_t fn,std::uintptr_t pc){return tick&&*static_cast<unsigned*>(p)==1&&fn==15&&pc==16;}};
    for(unsigned frame=0;frame<15*60*60;++frame){
        if(frame==15*60*30){
            {BundleCallbackRootScope finalReturn(plan.module);CHECK(DrainBundleDeferredRetirement());}
            nativeRegistry.held=true;unsigned releasedAtMenu=0;
            CHECK(bundleRootBank.RetireMapReferences(nativeRegistry,[](auto){return cfgretain::MapReference::OldMap;},releasedAtMenu));
            nativeRegistry.held=false;++plan.world;}
        for(unsigned kind=0;kind<2;++kind){
            primaryRootTickActor=kind?4:14;primaryRootTickFunction=kind?5:15;primaryRootTickReturn=kind?6:16;
            bundleNativeJasonTickProof=kind?nullptr:&nativeProof;
            {BundleCallbackRootScope callback(plan.module);Reader reader;CHECK(callback.Entered());
             CHECK(DrainBundleDeferredRetirement());
             CHECK(RetainBundleCallbackValues(reader,std::array<std::uintptr_t,1>{0x70000+kind*8}));
             {BundleCallbackRootScope nested(plan.module);CHECK(!nested.Entered());}
             CHECK(bundleRootWitness.lease==bundleRootLease);
             CHECK(PrepareBundleCallbackCall(0)&&EnterBundleCallbackCall());
             begin=false;CHECK(CompleteBundleCallbackCall(reader,std::array<std::uintptr_t,0>{}));
             CHECK(bundleRootBank.Healthy()&&!bundleRootLease->NativePending());
            }
            CHECK(bundleDeferredRetirement.Claims()==1&&bundleRootBank.Healthy()&&!bundleRootWitness.lease);
            begin=true;
            if(failures){std::printf("Alternation failed frame=%u kind=%u\n",frame,kind);return 1;}
        }
    }
    bundleNativeJasonTickProof=nullptr;primaryRootTickActor=4;primaryRootTickFunction=5;primaryRootTickReturn=6;
    {BundleCallbackRootScope callback(plan.module);CHECK(DrainBundleDeferredRetirement());}
    CHECK(bundleRootDiagnostics.zeroOutputContendedCompletions==15*60*60*2);
    CHECK(bundleRootBank.Healthy()&&!bundleDeferredRetirement.Claims());
    std::puts("J1a.1 fifteen-minute60Hz native/PE alternation+menu+match2 PASS: zero poison; zero-output contention defers only guarded claim retirement.");
    // Existing B1 behavior: a post-call window refusal poisons even when the
    // native call returned. Record its FIRST cause; do not weaken live authority.
    {BundleCallbackRootScope scope(plan.module);Reader reader;
     CHECK(RetainBundleCallbackValues(reader,std::array<std::uintptr_t,1>{0x40000}));
     CHECK(PrepareBundleCallbackCall(0)&&EnterBundleCallbackCall());tick=false;
     CHECK(!CompleteBundleCallbackCall(reader,std::array<std::uintptr_t,0>{}));
     CHECK(bundleRootDiagnostics.firstPoison.site==static_cast<unsigned>(cfgretain::PoisonSite::CompleteWindow)&&
        bundleRootDiagnostics.firstPoison.claims==1&&bundleRootDiagnostics.firstPoison.flags==3&&bundleRootDiagnostics.poisonUntouched);}
    CHECK(bundleRootBank.Poisoned()&&!bundleDeferredRetirement.Claims());
    CHECK(bundleRootDiagnostics.firstPoison.site==static_cast<unsigned>(cfgretain::PoisonSite::CompleteWindow)); // destructor cannot overwrite it
    tick=true;nativeRegistry.held=true;unsigned released=0;
    CHECK(bundleRootBank.RecoverMapReferencesAtMenu(nativeRegistry,[](auto){return cfgretain::MapReference::OldMap;},released)&&released==1);
    nativeRegistry.held=false;bundleRootDiagnostics.firstPoison={};
    {BundleCallbackRootScope scope(plan.module);Reader reader;CHECK(scope.Entered());
     CHECK(RetainBundleCallbackValues(reader,std::array<std::uintptr_t,1>{0x50000}));}
    CHECK(bundleRootBank.Healthy()&&!bundleRootLease&&!bundleRootDiagnostics.firstPoison.site);
    // First-cause observer is invoked INSIDE the proved registry window. It
    // records only; even an explicit flush cannot write until the lease ends.
    const auto beforePoisonTraces=poisonTraces;
    {BundleCallbackRootScope scope(plan.module);Reader reader;
     CHECK(RetainBundleCallbackValues(reader,std::array<std::uintptr_t,1>{0x60000}));
     CHECK(WithBundleRootWindow([&](GcRegistryAccess&)noexcept{
        CHECK(nativeRegistry.held);
        bundleRootBank.MarkUncertain({static_cast<unsigned>(cfgretain::PoisonSite::CloseRelease),1,0,0x1234});
        FlushFirstBundlePoisonTrace();
        CHECK(bundleRootDiagnostics.firstPoisonTracePending&&poisonTraces==beforePoisonTraces);
        return true;}));
     CHECK(!nativeRegistry.held&&poisonTraces==beforePoisonTraces);
    }
    CHECK(!bundleRootLease&&!bundleRootDiagnostics.firstPoisonTracePending&&poisonTraces==beforePoisonTraces+3);
    FlushFirstBundlePoisonTrace();
    CHECK(poisonTraces==beforePoisonTraces+3&&traceWritesInsideWindow==0);
    std::printf("Actual root-window/scope/drain members:%u failures; pre-work tick/context refusal retains idle roots until fresh admission; entered native uncertainty still quarantines; owned registry only.\n",failures);
    return failures?1:0;
}
