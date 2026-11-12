#ifndef CFG_MULTI_MATCH_ACCESS_ONLY
#define CFG_WORLDROOT_PRODUCTION_MAIN PriorWorldRootProductionMain
#include "WorldRootRetirementProductionTests.cpp"
#endif
#include <map>
#include <cstdio>
#include "../Core/HostedOfflineEpoch.hpp"
namespace MultiMatchMarker {
struct Reader {
    std::map<std::uintptr_t,unsigned char> memory;unsigned reads=0,retiredReads=0;
    std::uintptr_t retiredBegin=0,retiredEnd=0;
    template<class T> void Set(std::uintptr_t at,const T& value){const auto* p=reinterpret_cast<const unsigned char*>(&value);for(unsigned i=0;i<sizeof(T);++i)memory[at+i]=p[i];}
    template<class T> bool Read(std::uintptr_t at,T& value){++reads;
        if(retiredBegin&&at<retiredEnd&&at+sizeof(T)>retiredBegin){++retiredReads;return false;}
        auto* p=reinterpret_cast<unsigned char*>(&value);for(unsigned i=0;i<sizeof(T);++i){auto it=memory.find(at+i);if(it==memory.end())return false;p[i]=it->second;}return true;}
};
struct Host {bool current=true;bool Current(){return current;}};
struct Fixture {
    static constexpr std::uintptr_t Module=0x100000,World=0x200000,Levels=0x210000,Level=0x220000,Actors=0x230000,
        Actor=0x240000,Type=0x250000,LevelType=0x260000,Table=0x270000,Other=0x280000;
    Reader reader;Host host;cfgkillerstart::Observation observation;
    Fixture(){using namespace cfgkillerstart;using namespace cfgprobe;
        reader.Set(Module+disk::ClassCache,Type);reader.Set(Module+disk::ConstructorCache,Type);
        reader.Set(Module+cfgradarworld::disk::LevelClassCache,LevelType);reader.Set(Module+cfgradarworld::disk::LevelConstructorCache,LevelType);
        reader.Set(World+0x110,Header{Levels,1,1});reader.Set(Levels,Level);reader.Set(Level+0x20,World);
        reader.Set(Level+cfgkill::disk::OwningWorld,World);
        reader.Set(Level+ObjectClass,LevelType);reader.Set(LevelType+ClassSuper,std::uintptr_t{});
        reader.Set(Level+0xA0,Header{Actors,33,33});
        for(unsigned i=0;i<33;++i)reader.Set(Actors+8*i,i==20?Actor:std::uintptr_t{});
        reader.Set(Actor+ObjectClass,Type);reader.Set(Type+ClassSuper,std::uintptr_t{});reader.Set(Type+0xF8,Other);
        reader.Set(Actor+0x20,Level);reader.Set(Actor+0x0C,int{0});
        reader.Set(Module+lifetime::ObjectArrayRva+0x10,Table);reader.Set(Module+lifetime::ObjectArrayRva+0x1C,int{2});
        reader.Set(Table,Actor);reader.Set(Table+0x10,int{42});
    }
    bool Begin(){return observation.Begin(reader,host,Module,World);}
    bool Finish(){for(unsigned i=0;i<15&&observation.CurrentState()!=cfgkillerstart::State::Ready;++i)if(!observation.Step(reader,host))return false;return observation.CurrentState()==cfgkillerstart::State::Ready;}
};

}
unsigned lifecycleChecks=0,lifecycleFailures=0;
void LifecycleCheck(bool okay,const char* label){++lifecycleChecks;if(!okay){++lifecycleFailures;std::printf("FAIL %s\n",label);}}
#define FIXTURE_CHECK(x) LifecycleCheck((x),#x)
struct MultiSpawnAccess {
    bool owner=true,resolve=false,uncertainWrite=false,budgetOkay=true; unsigned step=0,failAt=0,calls=0,writes=0,conversions=0,index=0xFFFFFFFF;
    unsigned lookupCount=1,nulls=0,spawnCalls=0;std::uint64_t now=1000;
    bool spawnCallOkay=true,nativeException=false;unsigned moveResult=2;
    std::uint64_t Now()const{return now;}
    bool Next() { return ++step!=failAt; }
    bool OwnerThread() const { return owner; }
    bool PrepareOwnerContext() { return Next(); }
    bool ContextMatches() { return Next(); }
    bool ResourcesMatch() { return Next(); }
    bool ResolveFunctions() { if (!Next()) return false; calls+=4; return true; }
    bool ResolvePawnClass(std::uintptr_t& value) {
        if (!Next()) return false;
        ++calls; conversions=lookupCount; calls+=lookupCount;
        if (!resolve) return false;
        value=0x40000000; index=lookupCount-1; return true;
    }
    bool Location(cfgspawn::Vec3& v) { if (!Next()) return false; ++calls; v={100,200,300}; return true; }
    bool StartTransform(cfgspawn::Vec3& v,cfgspawn::Vec3& rotation){
        if(!Location(v))return false;rotation={0,75,0};return true;
    }
    bool Spawn(cfgspawn::SpawnAIParameters& p) {
        if (!Next()) return false;
        FIXTURE_CHECK(p.pawnClass==0x40000000 && p.behaviorTree==0 && p.noCollisionFail==0);
        FIXTURE_CHECK(p.location.x==100&&p.location.y==200&&p.location.z==300&&p.rotation.y==75);
        ++calls;++spawnCalls;if(!spawnCallOkay)return false;
        p.result=spawnCalls<=nulls?0:0x90000000;return true;
    }
    bool VerifyInitial(std::uintptr_t pawn,std::uintptr_t& controller) {
        if (!Next() || pawn!=0x90000000) return false; controller=0x90010000; return true;
    }
    bool Unpossess(std::uintptr_t c) { if (!Next() || c!=0x90010000) return false; ++calls; return true; }
    bool VerifyUnpossessed() { return Next(); }
    bool WriteInstanceClass() { if (!Next()) return false; ++writes; return !uncertainWrite; }
    bool DefaultController() { if (!Next()) return false; ++calls; return true; }
    bool VerifyKiller(std::uintptr_t& c,std::uintptr_t& p) { if (!Next()) return false; c=0x90020000; p=0x90030000; return true; }
    bool ConfigureNavigation() { if (!Next()) return false; ++calls; return true; }
    void NavigationReport(cfgspawn::SpawnReport& outcome) const { outcome.crowdKind=3; outcome.crowdStateBefore=0; outcome.crowdStateAfter=2; }
    bool Move(cfgspawn::MoveParameters& p) { if (!Next()) return false; FIXTURE_CHECK(p.goal==0x20000000); ++calls; p.result=static_cast<std::uint8_t>(moveResult); return true; }
    bool VerifyAfterMove() { return Next(); }
    unsigned Reads() const { return step; } unsigned Calls() const { return calls; } unsigned Writes() const { return writes; }
    unsigned ClassConversions() const { return conversions; } unsigned CatalogueIndex() const { return index; }
    bool CompletionBudget() { return Next() && budgetOkay; }
    bool BudgetRefused() const { return !budgetOkay; }
    bool PrimaryKiller(std::uintptr_t& value) const { value=calls>=11?0x90000000:0; return true; }
    unsigned Error() const { return 0; } bool NativeException() const { return nativeException; }
    void Publish(std::uint32_t& value,std::uint32_t next) { value=next; }
};

struct MultiRegistry {
 cfggcretain::ReferencerList list{0x10000,1,256};
 std::array<std::uintptr_t,cfggcprobe::MaximumOwners> entries{0x20000};bool held=true;
 bool ProvenSlotWindow()const{return held;}
 bool Snapshot(cfggcretain::ReferencerList& out,decltype(entries)& values){out=list;values=entries;return true;}
 bool Register(std::uintptr_t owner){entries[list.count++]=owner;cfggcdiagnostic::registeredOwners.Track(owner);return true;}
 bool Unregister(std::uintptr_t owner){for(int i=0;i<list.count;++i)if(entries[i]==owner){--list.count;
  for(int j=i;j<list.count;++j)entries[j]=entries[j+1];entries[list.count]=0;return true;}return false;}
 bool RegisterResidentOwner(cfgretain::SynchronizedRootOwner& owner,cfggcprobe::Report& outcome){return cfgretain::RegisterResidentRootOwnerLocked(*this,owner,outcome);}
};
struct MultiWarm {
 std::uintptr_t reflection=0,constructor=0;unsigned calls=0;
 bool Current(){return true;}
 bool Caches(std::uintptr_t& a,std::uintptr_t& b){a=reflection;b=constructor;return true;}
 bool InitializeClass(bool getter,std::uintptr_t& result){++calls;result=0x50000;if(getter)constructor=result;else reflection=result;return true;}
};
struct MultiPopulation {
 unsigned humans=0,bots=0;
 bool Human(){++humans;return true;}
 bool Population(unsigned n){bots=n;return true;}
};
#ifndef CFG_MULTI_MATCH_ACCESS_ONLY
#ifndef CFG_MULTI_MATCH_MAIN
#define CFG_MULTI_MATCH_MAIN main
#endif
int CFG_MULTI_MATCH_MAIN(){
 const int prior=PriorWorldRootProductionMain();MultiRegistry registry;
 const auto ready=static_cast<unsigned>(cfgspawn::SpawnState::MoveSubmitted);
 std::uintptr_t reusedOwner=0;unsigned births=0,moves=0;
 const auto archivedBefore=frontendOldEpochs.size();FrontendEpochStorage* firstArchived=nullptr;
 for(unsigned match=0;match<10;++match){
  // Actual production reset, roots, bootstrap, marker observation and native
  // spawn policy; only the engine boundary responses/registry are synthetic.
  FIXTURE_CHECK(BundleEpochRetirable());BundleResetEpoch();
  FIXTURE_CHECK(!bundleStarted&&!bundleCompletionReady&&!bundleDeathEnd.confirmedAt&&!bundleKill.policy.pamela);
  FIXTURE_CHECK(!bundleSweaterPolicy.lastUse&&!bundleEmotes.report.attempts);
  FIXTURE_CHECK(!bundleContextDeferrals&&!bundlePresentationPending&&primaryBootstrapRootsOpen);
  FIXTURE_CHECK(primaryStartBootstrap.state==cfgkillerstart::BootstrapState::Empty&&primaryStartObservation.CurrentState()==cfgkillerstart::State::Empty);
  MultiWarm warm;while(primaryStartBootstrap.state!=cfgkillerstart::BootstrapState::Ready&&warm.calls<3)primaryStartBootstrap.Step(warm);
  FIXTURE_CHECK(primaryStartBootstrap.state==cfgkillerstart::BootstrapState::Ready&&warm.calls==2);
  MultiMatchMarker::Fixture marker;
  FIXTURE_CHECK(primaryStartObservation.Begin(marker.reader,marker.host,marker.Module,marker.World));
  for(unsigned step=0;step<15&&primaryStartObservation.CurrentState()!=cfgkillerstart::State::Ready;++step)
   FIXTURE_CHECK(primaryStartObservation.Step(marker.reader,marker.host));
  cfgkillerstart::Marker found;FIXTURE_CHECK(primaryStartObservation.CurrentMarker(marker.reader,marker.host,found)&&found.actor==marker.Actor);
  MultiPopulation population;FIXTURE_CHECK(cfgpopulation::Birth(population,match==9?0u:2u)&&population.humans==1&&population.bots==(match==9?0u:2u));
  const cfgspawn::Identity human{0x10000000,0x20000000,7,8};cfgspawn::SpawnRequest request;request.identity=human;
  FIXTURE_CHECK(spawnRuntime.Configure(human,true,100,human.pawn,0)&&spawnRuntime.Submit(request));
  MultiSpawnAccess access;access.resolve=true;spawnRuntime.Service(access);
  FIXTURE_CHECK(spawnRuntime.report.state==ready&&access.spawnCalls==1&&access.calls==12);++births;++moves;
  cfggcprobe::Report registration;
  FIXTURE_CHECK(primaryRoots.AddLocked(registry,std::array<std::uintptr_t,1>{0x40000+match*0x10000},registration));
  if(!reusedOwner)reusedOwner=primaryRoots.Address(0);FIXTURE_CHECK(primaryRoots.Address(0)==reusedOwner&&primaryRoots.Count()==1);
  primaryBootstrapRootsOpen=false; // unchanged gameplay-scope transition
  const auto bankCycle=[&](auto& bank){
   using Bank=std::remove_reference_t<decltype(bank)>;
   typename Bank::Reservation reserved;std::array<typename Bank::Handle,2> handles{};
   const auto oldMap=std::uintptr_t{0x600000}+match*0x10000;
   FIXTURE_CHECK(bank.ReserveOutputs(registry,plan.module,2,reserved));
   FIXTURE_CHECK(bank.CommitFreshReturns(registry,reserved,std::array<std::uintptr_t,2>{oldMap,0xA00000},handles));
   unsigned released=0;FIXTURE_CHECK(bank.RetireMapReferences(registry,[&](auto value){return value==oldMap?cfgretain::MapReference::OldMap:cfgretain::MapReference::Unrelated;},released)&&released==1);
   std::uintptr_t discarded=0;FIXTURE_CHECK(!bank.ResolveRequired(registry,handles[0],discarded));
   FIXTURE_CHECK(bank.ReleaseAcquired(registry,handles[1])&&bank.Owners()==1&&bank.Healthy());
  };
  bankCycle(startupRootBank);bankCycle(frontendRootBank);bankCycle(bundleRootBank);WorldRootNames();
  FIXTURE_CHECK(worldRootRetirement.count<=5);
  cfggcprobe::Report removal;removal.result=cfggcprobe::Result::Passed;removal.stage=12;
  FIXTURE_CHECK(!CompleteWorldRootRetirement(registry,removal,true)&&!primaryBootstrapRootsOpen); // roots still live
  FIXTURE_CHECK(primaryRoots.ReleaseAllLocked(registry,removal)&&!primaryRoots.Count());
  FIXTURE_CHECK(!CompleteWorldRootRetirement(registry,removal,false)&&!primaryBootstrapRootsOpen); // failed after-inventory
  registry.held=false;FIXTURE_CHECK(!CompleteWorldRootRetirement(registry,removal,true)&&!primaryBootstrapRootsOpen);registry.held=true;
  FIXTURE_CHECK(CompleteWorldRootRetirement(registry,removal,true)&&primaryBootstrapRootsOpen);
  // Poisoned/bootstrap members may reset only after verified retirement. The
  // C++ epoch reset must never itself grant a reopened bootstrap.
  bundleContextDeferrals=64;bundleContextDeferralsTotal=64;bundlePresentationPending=true;
  bundleCompletionReady=true;bundleDeathEnd.confirmedAt=123;bundleSweaterPolicy.lastUse=123;
  ReserveFrontendEpochArchive();frontendOldEpochs.push_back(std::make_unique<FrontendEpochStorage>());
  if(!match)firstArchived=frontendOldEpochs.back().get();
  FIXTURE_CHECK(frontendOldEpochs.size()==archivedBefore+match+1&&frontendOldEpochs.capacity()>=frontendOldEpochs.size());
  FIXTURE_CHECK(frontendOldEpochs[archivedBefore].get()==firstArchived); // vector growth never moves published backing
 }
 FIXTURE_CHECK(births==10&&moves==10&&!primaryRoots.Count()&&!primaryRoots.Poisoned());
 std::array<std::uintptr_t,cfggcdiagnostic::Registry::Capacity> owners{};unsigned count=0;
 cfggcdiagnostic::registeredOwners.Snapshot(owners,count);FIXTURE_CHECK(count==4&&!cfggcdiagnostic::registeredOwners.overflow.load());
 primaryBootstrapRootsOpen=false;BundleResetEpoch();FIXTURE_CHECK(!primaryBootstrapRootsOpen); // only retirement reopens
 FIXTURE_CHECK(frontendChallengeOldRoutes.size()>=64&&frontendChallengeViews.size()>=64&&frontendHumanJasonCached.size()>=64);
 {using namespace cfghostedoffline; // H2.75 decision: leftover offline epoch + new hosted world, no lease/hosted routine
  LifecycleCheck(LeftoverEpochDecision(true,true,false,true,true,0x100,0x200,false,true)==Retire,"h275 leftover retire");
  LifecycleCheck(LeftoverEpochDecision(true,true,false,true,true,0x100,0x200,false,false)==Blocked,"h275 leftover blocked");
  LifecycleCheck(LeftoverEpochDecision(true,true,false,true,true,0x100,0x100,false,true)==Leave,"h275 live offline match same world");
  LifecycleCheck(LeftoverEpochDecision(true,false,false,true,true,0x100,0x200,false,true)==Leave,"h275 not a hosted world");
  LifecycleCheck(LeftoverEpochDecision(true,true,true,true,true,0x100,0x200,false,true)==Leave,"h275 hosted active");
  LifecycleCheck(LeftoverEpochDecision(true,true,false,true,true,0x100,0x200,true,true)==Leave,"h275 root lease");
  LifecycleCheck(LeftoverEpochDecision(true,true,false,false,true,0x100,0x200,false,true)==Leave&&
   LeftoverEpochDecision(true,true,false,true,false,0x100,0x200,false,true)==Leave,"h275 no leftover epoch");
  LifecycleCheck(LeftoverEpochDecision(false,true,false,true,true,0x100,0x200,false,true)==Leave,"h275 flag off");}
 std::printf("Actual multi-match lifecycle: %u checks,%u failures;10 epochs/warm/marker/MoveSubmitted/population/verified-root-release,reused owner; final zero-bot population accepted. No game/native calls.\n",lifecycleChecks,lifecycleFailures);
 return prior||lifecycleFailures?1:0;
}
#endif
