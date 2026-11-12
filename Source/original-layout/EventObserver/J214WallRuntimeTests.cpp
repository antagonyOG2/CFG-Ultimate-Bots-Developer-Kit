#define CFG_J36_CHALLENGE_WALLS 1
#define CFG_J214_INTRO_RETAINED_TEST_MAIN IntroProviderBaselineChecks
#include "J214IntroRetainedTests.cpp"
#include "NativeChallengeBoundaryRuntime.hpp"
#include "NativeChallengeEdgeWallsRuntime.hpp"
#include "NativeChallengeSettingsSpecs.hpp"
#include "FrontendIntegrationReport.hpp"
struct WallFixture:ChallengeFixture {
 static constexpr std::uintptr_t ActorType=0x2300000,VolumeType=0x2310000,FunctionType=0x2320000,
  BoundsFunction=0x2330000,SetFunction=0x2340000,LevelList=0x2350000,ActorList=0x2360000,FirstWall=0x2400000;
 static std::uintptr_t Wall(unsigned n){return FirstWall+0x10000ULL*n;}
 void Slot(std::uintptr_t object,unsigned n,int serial){r.Set(object+12,static_cast<int>(n));r.Set(object+8,std::uint32_t{});
  r.Set(Registry+24ULL*n,object);r.Set(Registry+24ULL*n+8,std::uint32_t{});r.Set(Registry+24ULL*n+12,std::uint32_t{});r.Set(Registry+24ULL*n+16,serial);}
 WallFixture(){using namespace cfgprobe;namespace d=cfgj36walls::disk;
  r.Set(Module+cfgchallengeescape::disk::SCGameState_SPChallenges,SCState);r.Set(Module+cfgchallengeescape::disk::SCGameState_SPChallengesCtor,SCState);
  r.Set(Module+cfgchallengeescape::disk::PlayerController,ControllerType);r.Set(Module+cfgchallengeescape::disk::PlayerControllerCtor,ControllerType);
  Add(ControllerType,"NetConnection","ObjectProperty",0x578,1,8);r.Set(Controller+0x578,std::uintptr_t{});
  Slot(Dossier,7,0);Slot(ActorType,8,0);Slot(VolumeType,9,0);Slot(BoundsFunction,10,0);Slot(SetFunction,11,0);
  r.Set(Module+d::ActorReflection,ActorType);r.Set(Module+d::ActorConstructor,ActorType);
  r.Set(Module+d::BlockingVolumeReflection,VolumeType);r.Set(Module+d::BlockingVolumeConstructor,VolumeType);
  r.Set(ActorType+ClassSuper,std::uintptr_t{});r.Set(VolumeType+ClassSuper,ActorType);
  r.Set(VolumeType+0xF8,VolumeType+0x8000);r.Set(VolumeType+0x8000+ObjectClass,VolumeType);
  r.Set(ActorType+0xF8,ActorType+0x8000);r.Set(ActorType+0x8000+ObjectClass,ActorType);
  Label(FunctionType,"Function");Label(ActorType,"Actor");Label(VolumeType,"BlockingVolume");
  for(unsigned n=0;n<2;++n){const auto function=n?SetFunction:BoundsFunction;
   Label(function,n?"SetActorEnableCollision":"GetActorBounds");r.Set(function+ObjectClass,FunctionType);r.Set(function+0x20,ActorType);
   r.Set(function+cfgreflection::disk::Next,std::uintptr_t{});r.Set(function+0x88,std::uint32_t{0x400});
   r.Set(function+0x8C,std::uint8_t(n?1:3));r.Set(function+0x8E,std::uint16_t(n?1:28));r.Set(function+0xB0,Module+(n?d::SetExecutor:cfgcabin::disk::Executor));
  }
  std::string param;for(const auto ch:d::SetParameterName)if(ch)param.push_back(static_cast<char>(ch));
  Add(SetFunction,param.c_str(),"BoolProperty",0,1,1);r.Set(heads[SetFunction]+cfgreflection::disk::PropertyFlags,std::uint64_t{0x80});
  r.Set(Module+cfgcabin::disk::GetActorBounds.cache,BoundsFunction);r.Set(Module+d::SetCache,SetFunction);
  r.Set(Module+cfgcabin::disk::GetActorBounds.factory,cfgcabin::disk::GetActorBounds.code);r.Set(Module+cfgcabin::disk::Executor,cfgcabin::disk::ExecutorCode);
  r.Set(Module+cfgcabin::disk::Native,cfgcabin::disk::NativeCode);r.Set(Module+d::SetFactory,d::SetFactoryCode);
  r.Set(Module+d::SetExecutor,d::SetExecutorCode);r.Set(Module+d::Native,d::NativeCode);
  r.Set(Module+d::CollisionPropertySetter,d::CollisionPropertySetterCode);r.Set(Module+cfgspawn::GenericDispatcherRva,cfgspawn::GenericDispatcherPrefix);
  r.Set(World+cfgradarworld::disk::WorldLevels,cfgchallengeescape::actors::List{LevelList,1,1});r.Set(LevelList,Level);
  r.Set(Level+8,std::uint32_t{});r.Set(Level+cfgkill::disk::OwningWorld,World);r.Set(Level+cfgradarworld::disk::LevelActors,cfgchallengeescape::actors::List{ActorList,4,4});
  for(unsigned n=0;n<4;++n){Slot(Wall(n),12+n,0);r.Set(ActorList+8ULL*n,Wall(n));r.Set(Wall(n)+ObjectClass,VolumeType);
   r.Set(Wall(n)+0x20,Level);r.Set(Wall(n)+0x110,std::uint8_t{3});r.Set(Wall(n)+d::CollisionReadbackOffset,std::uint8_t{2});
   const auto name="BlockingVolume_"+std::to_string(n);Label(Wall(n),name.c_str());}
  r.Set(Module+lifetime::ObjectArrayRva+28,int{16});for(unsigned n=0;n<8;++n){r.Set(Registry+24ULL*n+8,std::uint32_t{});r.Set(Registry+24ULL*n+12,std::uint32_t{});}
  r.Set(Names+cfgnames::disk::ElementCount,std::array<int,2>{{static_cast<int>(id),1}});
 }
};
namespace {
DWORD observerOwnerThread=0;
#include "NativeChallengeGameplayRootProvider.hpp"
unsigned boundsCalls=0,setCalls=0;bool failReadback=false;
struct WallPlatform:OwnedPlatform {
 static void Invoke(std::uintptr_t address,std::uintptr_t target,std::uintptr_t function,void* packet){
  if(address!=Fixture::Module+cfgspawn::GenericDispatcherRva||target<WallFixture::FirstWall||target>WallFixture::Wall(3))RaiseException(EXCEPTION_ACCESS_VIOLATION,0,0,nullptr);
  if(function==WallFixture::BoundsFunction){++boundsCalls;auto* out=static_cast<cfgcabin::Packet*>(packet);const unsigned n=static_cast<unsigned>((target-WallFixture::FirstWall)/0x10000);
   const std::array<cfgj36walls::Bounds,4> boxes{{{{-5000,0,0},{100,5000,1000}},{{5000,0,0},{100,5000,1000}},{{0,-5000,0},{5000,100,1000}},{{0,5000,0},{5000,100,1000}}}};
   const auto& b=boxes[n];out->origin={b.center.x,b.center.y,b.center.z};out->extent={b.extent.x,b.extent.y,b.extent.z};return;}
  if(function==WallFixture::SetFunction){++setCalls;if(!failReadback)active->Set(target+cfgj36walls::disk::CollisionReadbackOffset,std::uint8_t(*static_cast<std::uint8_t*>(packet)?2:0));return;}
  RaiseException(EXCEPTION_ACCESS_VIOLATION,0,0,nullptr);
 }
};
using WallEngine=cfgmenu::rows::NativeDispatchBase<WallPlatform>;
#include "NativeChallengeWallNativeHost.hpp"
}
#ifndef CFG_J214_WALL_TEST_MAIN
#define CFG_J214_WALL_TEST_MAIN main
#endif
int CFG_J214_WALL_TEST_MAIN(){static_assert(sizeof(cfgmenu::FrontendReport)<=cfgmenu::FrontendReportReadCap,"bounded complete wall diagnostic report");unsigned checks=0,failures=0;auto check=[&](bool good,const char* text){++checks;if(!good){++failures;std::printf("FAIL %s\n",text);}};
 {const auto rows=cfgmenu::challengemenu::Specs();check(rows.size()==9,"candidate Challenges adds one distinct wall row");
  check(rows[cfgmenu::challengemenu::choice::EdgeWalls].selected==0&&!rows[cfgmenu::challengemenu::choice::EdgeWalls].preserveNativeValue&&
    rows[cfgmenu::challengemenu::choice::EdgeWalls].Valid(),"wall row defaults OFF and never inherits native selection");}
 {WallFixture f;ChallengeReader memory;static_cast<StageReader&>(memory)=f.r;memory.cap=100000;active=&memory;
  auto menu=f.Menu();cfgj36walls::Runtime off;Reader r;
  check(off.Arm(menu,false,0)&&!off.Observe(r,42,100)&&r.reads==0,"OFF performs no gameplay reads or native work");}
 for(unsigned fault=0;fault<5;++fault){
  WallFixture f;ChallengeReader memory;static_cast<StageReader&>(memory)=f.r;memory.cap=10000000;active=&memory;
  observerOwnerThread=GetCurrentThreadId();memory.Set(f.Module+cfgprobe::dispatch::ThreadIdRva,observerOwnerThread);
  auto menu=f.Menu();menu.thread=observerOwnerThread;cfgj36walls::Runtime runtime;Reader r;r.cap=65536;
  check(runtime.Arm(menu,true,0)&&runtime.Observe(r,observerOwnerThread,100),"Challenges typed runtime binds exact mission owners");r.reads=0;
  check(runtime.Scan(r)&&runtime.Scanned()&&runtime.report.count==4,"bounded actual Level actor census finds typed BlockingVolumes");
  for(unsigned n=0;n<4;++n){
   ChallengeWallNativeHostBase<WallEngine> host{runtime,n,f.Mode};
   if(!fault&&n==0)std::printf("Gate runtime=%u member=%u cap=%u count=%u phase=%d\n",runtime.Current(memory,observerOwnerThread)?1u:0u,cfgj36walls::Member(memory,f.Module,runtime.Witness(),runtime.Actor(n))?1u:0u,memory.cap,memory.count,runtime.Witness().phase.index);
   ChallengeGameplayRootProvider<decltype(host)> provider(host,f.Module,f.Mode,0x810000,0x820000);
   cfgmenu::rows::NativeRetentionScope scope(f.Module,provider.Hooks(),provider.Admitted());WallEngine engine(f.Module,observerOwnerThread,true);host.engine=&engine;
   const bool read=scope.Entered()&&host.ReadBounds(runtime.report.records[n].bounds);
   if(!read)std::printf("Wall%u admitted=%u current=%u fn=%u failure=%u calls=%u nativebounds=%u scopefail=%u\n",n,scope.Entered()?1u:0u,host.ProveCurrent(memory)?1u:0u,host.Function(memory,f.BoundsFunction)?1u:0u,static_cast<unsigned>(engine.LastFailure()),engine.Calls(),boundsCalls,cfgmenu::rows::NativeRetentionScope::OutputFailure());
   check(read,"actual metadata/root provider/dispatcher reads volume bounds");
  }
  cfgj36walls::Classify(runtime.report);check(runtime.report.chosen==4,"native geometry reaches perimeter classifier");
  auto& record=runtime.report.records[0];boundsCalls=setCalls=0;failReadback=fault==4;
  if(fault==1)memory.Set(f.Wall(0)+0x20,f.World); // breaks exact strong actor membership
  if(fault==2)memory.Set(f.Module+cfgj36walls::disk::SetExecutor,std::uint8_t{});
  if(fault==3)memory.Set(f.Wall(0)+cfgj36walls::disk::CollisionReadbackOffset,std::uint8_t{});
  {
   ChallengeWallNativeHostBase<WallEngine> host{runtime,0,f.Mode};
   ChallengeGameplayRootProvider<decltype(host)> provider(host,f.Module,f.Mode,0x810000,0x820000);
   cfgmenu::rows::NativeRetentionScope scope(f.Module,provider.Hooks(),provider.Admitted());WallEngine engine(f.Module,observerOwnerThread,true);host.engine=&engine;
   const bool applied=scope.Entered()&&cfgj36walls::Apply(record,runtime.report,host);
   check(applied==(fault==0),"actual retained setter only accepts exact current actor/frame/code/readback");
   check(setCalls==((fault==0||fault==4)?1u:0u),"no setter for foreign member, changed code or originally-off actor");
   if(!fault)check(cfgj36walls::Restore(record,runtime.report,host)&&setCalls==2&&record.state==2,"actual retained setter restores and reads original flag");
   if(fault==4)check(record.state==3&&!cfgj36walls::Apply(record,runtime.report,host)&&setCalls==1,"native returned but readback refused never replays");
  }
  check(frontendChallengeRootBank.Healthy()&&!windowHeld,"actual callback root claims and registry windows close");
#if CFG_H279_WALL_LIST_REFRESH
  if(!fault){const auto before=runtime.Actor(1);const std::uintptr_t other=0x2500000,moved=f.ActorList+0x10000;using List=cfgchallengeescape::actors::List;const auto level=f.Level+cfgradarworld::disk::LevelActors;
   memory.Set(f.ActorList+32,other);memory.Set(level,List{f.ActorList,5,5}); // growth in place
   check(!cfgj36walls::Member(memory,f.Module,runtime.Witness(),before)&&runtime.Current(memory,observerOwnerThread)&&runtime.ProveMember(memory,1)&&
    runtime.Actor(1).list.count==5&&runtime.Actor(1).actorIndex==1,"H2.79 grown level actor list: exact header proof fails, refreshed membership holds");
   for(unsigned n=0;n<5;++n)memory.Set(moved+8ULL*n,n<4?f.Wall(3-n):other);memory.Set(level,List{moved,5,5}); // relocation + reorder
   check(runtime.Current(memory,observerOwnerThread)&&runtime.ProveMember(memory,1)&&runtime.Actor(1).list.data==moved&&runtime.Actor(1).actorIndex==2,"H2.79 relocated level actor list re-resolves the actor cell");
   memory.Set(moved+40,f.Wall(1));memory.Set(level,List{moved,6,6}); // duplicate actor entry is ambiguous
   check(runtime.Current(memory,observerOwnerThread)&&!runtime.ProveMember(memory,1),"H2.79 duplicate actor entry is refused");
   memory.Set(moved+8ULL*2,other);memory.Set(moved+40,other); // actor no longer in its level list
   check(runtime.Current(memory,observerOwnerThread)&&!runtime.ProveMember(memory,1),"H2.79 actor missing from level list is refused");
  }
#endif
 }
 std::printf("Frontend diagnostic bytes=%zu; wall decision record bytes=%zu\n",sizeof(cfgmenu::FrontendReport),sizeof(cfgj36walls::Record));
 active=nullptr;std::printf("J214 actual wall census/metadata/root lease/dispatcher: %u checks %u failures; native registry+Invoke simulated.\n",checks,failures);return failures?1:0;
}
