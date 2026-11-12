#define CFG_RETAINED_GAMEPLAY_BUNDLE
#include "ObjectiveHudRuntime.hpp"
// Compile the real production capability gate before the historical fixture
// macro. This suite never calls the game or grants runtime authority to it.
#define CFG_OBJECTIVE_HUD_FIXTURE_ONLY
#include "ObjectiveHudTests.cpp"
struct Proof {bool retiring=true;unsigned cost=1,boundedCost=1;};
template<class R>bool ChargeProof(R& r,void* p){const auto& q=*static_cast<Proof*>(p);std::uint8_t s=1;
 const unsigned cost=std::is_same_v<R,BytesReader>?q.cost:q.boundedCost;
 for(unsigned n=0;n<cost;++n)if(!r.Read(MenuFixture::Base+cfgprimary::SerializationFlag,s)||s)return false;return q.retiring;}
int main(){
 unsigned failures=0;
 const auto check=[&](bool value,const char* label){if(!value){++failures;std::printf("FAIL %s\n",label);}};
 ObjectiveFixture f;Host host{f};cfgobjective::Runtime runtime;bool current=true;
 using Native=cfgprimary::ScopedLookupEligibility<BytesReader>;
 using Bounded=cfgprimary::ScopedLookupEligibility<cfgobjective::Budget<BytesReader>>;
 {
  Native native(MenuFixture::Base,&current,[](BytesReader&,void* p){return *static_cast<bool*>(p);},true);
  check(native.Entered(),"native reader admitted");
  Step(runtime,f,host);
  check(!runtime.Status().scanned&&!runtime.Status().peakReads,"captured zero-read bounded-view refusal reproduced");
  {
   Bounded bounded(MenuFixture::Base,&current,[](cfgobjective::Budget<BytesReader>& r,void* p){
    std::uint8_t proof=1;return *static_cast<bool*>(p)&&r.Read(MenuFixture::Base+cfgprimary::SerializationFlag,proof)&&!proof;
   },true);
   check(bounded.Entered(),"independent bounded proof admitted");
   for(unsigned i=0;i<20&&!runtime.Status().changed;++i)Step(runtime,f,host,2+i);
   check(runtime.Status().scanned&&runtime.Status().changed==1&&host.writes==1,"real objective helper progresses with bounded capability");
   f.reader.reads=160;cfgobjective::Budget<BytesReader> view(f.reader);
   check(cfgprimary::LookupSafe(view,MenuFixture::Base),"current proof admits view");
   current=false;check(!cfgprimary::LookupSafe(view,MenuFixture::Base),"changed owner proof veto");current=true;
   f.r.Set(MenuFixture::Base+cfgprimary::GarbageCollectionState,std::uint32_t{1});
   check(!cfgprimary::LookupSafe(view,MenuFixture::Base),"GC veto");
   f.r.Set(MenuFixture::Base+cfgprimary::GarbageCollectionState,std::uint32_t{});
   f.r.Set(MenuFixture::Base+cfgprimary::SerializationFlag,std::uint8_t{1});
   check(!cfgprimary::LookupSafe(view,MenuFixture::Base),"serialization veto");
   f.r.Set(MenuFixture::Base+cfgprimary::SerializationFlag,std::uint8_t{});
   view.reads=cfgobjective::PhaseReads;
   check(!cfgprimary::LookupSafe(view,MenuFixture::Base)&&view.exhausted,"proof cannot bypass bounded read cap");
  }
  f.reader.reads=160;cfgobjective::Budget<BytesReader> expired(f.reader);
  check(!cfgprimary::LookupSafe(expired,MenuFixture::Base),"bounded capability expires independently");
 }
 // Post-match retirement (live PID2472: 16 owned patches, Restored=0
 // RestoreRefused=0 Deferred=0, Offline Play menu epoch blocked forever by
 // PendingRestore). No gameplay scope exists after the match.
 {
  // Mirrors production BundleObjectiveLifecycle's Host: Current() is LookupSafe.
  struct RetireHost {ObjectiveFixture& f;Host& inner;
   bool Current(){return cfgprimary::LookupSafe(f.reader,MenuFixture::Base);}
   bool Exchange(std::uintptr_t at,std::uintptr_t expected,std::uintptr_t value){return inner.Exchange(at,expected,value);}
  } retire{f,host};
  check(runtime.PendingRestore(),"changed patch owes a restore");
  for(unsigned i=0;i<4;++i){f.reader.reads=0;runtime.RetireOne(f.reader,MenuFixture::Base,retire);}
  check(runtime.PendingRestore()&&!runtime.Status().restored&&!runtime.Status().restoreRefused&&!runtime.Status().deferred,
   "without any lookup scope retirement stays silently pending (live failure reproduced)");
  // Production MetadataContext charges its menu re-proof (veto + two
  // active::Sample walks, ~72 reads) to the SUPPLIED reader. Model that cost.
  Proof proof;
  Native native(MenuFixture::Base,&proof,&ChargeProof<BytesReader>,true);
  Bounded bounded(MenuFixture::Base,&proof,&ChargeProof<cfgobjective::Budget<BytesReader>>,native.Entered());
  check(native.Entered()&&bounded.Entered(),"retirement-only scopes admitted");
  // Exhaustion: a proof that cannot fit the bounded phase defers, never
  // refuses, writes or restores; the owed restore and epoch block remain.
  proof.boundedCost=cfgobjective::PhaseReads;const auto writes=host.writes,deferred=runtime.Status().deferred;
  for(unsigned i=0;i<4;++i){f.reader.reads=0;runtime.RetireOne(f.reader,MenuFixture::Base,retire);}
  check(runtime.PendingRestore()&&host.writes==writes&&!runtime.Status().restored&&!runtime.Status().restoreRefused&&
   runtime.Status().deferred>deferred&&runtime.Status().peakReads<=cfgobjective::PhaseReads,
   "supplied Budget exhaustion defers without write/refusal and stays within PhaseReads");
  // The outer phase reader's own cap also refuses before any write.
  proof.boundedCost=1;proof.cost=1536;
  for(unsigned i=0;i<4;++i){f.reader.reads=0;runtime.RetireOne(f.reader,MenuFixture::Base,retire);}
  check(runtime.PendingRestore()&&host.writes==writes&&!runtime.Status().restored&&!runtime.Status().restoreRefused,
   "supplied outer Reader exhaustion refuses before any write");
  // Bounded success: realistic proof cost charged to the supplied readers.
  proof.cost=proof.boundedCost=80;
  for(unsigned i=0;i<24&&runtime.PendingRestore();++i){f.reader.reads=0;runtime.RetireOne(f.reader,MenuFixture::Base,retire);}
  check(!runtime.PendingRestore()&&runtime.Status().restored==1&&!runtime.Status().restoreRefused&&!runtime.Status().indeterminate&&
   runtime.Status().peakReads<=cfgobjective::PhaseReads,
   "retirement-scoped capability restores the exact owned patch within the bounded budget, releasing the menu epoch");
 }
 check(peakPhaseReads<=1536,"outer read cap preserved");
 std::printf("Objective retained lookup: %u failures; actual helper, callback-scoped typed capability and vetoes; owned memory only.\n",failures);
 return failures?1:0;
}
