#define CFG_J215_CHALLENGE_WALL_OWNERSHIP 1
#define CFG_J214_WALL_TEST_MAIN LegacyWallRuntimeChecks
#include "J214WallRuntimeTests.cpp"
struct OwnedWallFixture:WallFixture {
 static constexpr std::uintptr_t WorldType=0x2510000,BrushType=0x2520000,FirstRoot=0x2600000,
  ForeignWorld=0x2700000,ForeignPackage=0x2710000,BodyType=0x2800000,FirstBody=0x2900000,Convex=0x2A00000;
 static std::uintptr_t Root(unsigned n){return FirstRoot+0x10000ULL*n;}
 OwnedWallFixture(){namespace d=cfgj215walls::disk;using namespace cfgprobe;
  r.Set(Module+lifetime::ObjectArrayRva+28,int{32});Slot(World,0,100);Slot(Level,4,104);Slot(WorldPackage,16,0);Slot(ForeignWorld,26,0);Slot(ForeignPackage,27,0);
  r.Set(Module+d::WorldReflection,WorldType);r.Set(Module+d::WorldConstructor,WorldType);r.Set(World+ObjectClass,WorldType);r.Set(WorldType+ClassSuper,std::uintptr_t{});
  r.Set(LevelType+ClassSuper,std::uintptr_t{});
  r.Set(Level+0x20,World);Label(Level,"PersistentLevel");r.Set(World+d::PersistentLevel,Level);
  r.Set(ForeignWorld+ObjectClass,WorldType);r.Set(ForeignWorld+0x20,ForeignPackage);Label(ForeignPackage,"/Game/Maps/CrystalLake/CrystalLake");
  r.Set(Module+d::BrushComponentReflection,BrushType);r.Set(Module+d::BrushComponentConstructor,BrushType);r.Set(BrushType+ClassSuper,std::uintptr_t{});
  r.Set(Module+d::BodySetupReflection,BodyType);r.Set(Module+d::BodySetupConstructor,BodyType);r.Set(BodyType+ClassSuper,std::uintptr_t{});
  r.Set(Module+d::BrushVtable+d::CalcBoundsSlot,Module+d::BrushCalcBounds);
  r.Set(Module+d::BrushNativeConstructor,d::BrushNativeConstructorCode);r.Set(Module+d::BrushCalcBounds,d::BrushCalcBoundsCode);
  r.Set(Module+d::AggregateAabb,d::AggregateAabbCode);r.Set(Module+d::ConvexAabb,d::ConvexAabbCode);
  for(unsigned i=0;i<4;++i){Slot(Root(i),20+i,0);r.Set(Root(i)+ObjectClass,BrushType);r.Set(Root(i)+0xE0,Wall(i));
   r.Set(Wall(i)+d::RootComponent,Root(i));r.Set(Wall(i)+d::BrushComponent,Root(i));r.Set(Root(i)+d::RelativeRotation,cfgj36walls::Vec{0,45,0});
   const auto body=FirstBody+0x10000ULL*i,data=Convex+0x10000ULL*i;Slot(body,28+i,0);
   r.Set(body+ObjectClass,BodyType);r.Set(Root(i),Module+d::BrushVtable);r.Set(Root(i)+d::BrushBodySetup,body);
   for(unsigned n=0;n<4;++n)r.Set(body+d::AggGeom+16ULL*n,cfgchallengeescape::actors::List{n==3?data:0,n==3?1:0,n==3?1:0});
   r.Set(data+d::ConvexBox,cfgj215walls::Box{{-100,-5000,-1500},{100,5000,1500},1,{}});
   r.Set(data+d::ConvexTransform,cfgj215walls::Transform{{0,0,0,1},{0,0,0,0},{1,1,1,0}});
  }
  r.Set(Names+cfgnames::disk::ElementCount,std::array<int,2>{{static_cast<int>(id),1}});
 }
};
struct DiagnoseWallReader {
 ChallengeReader& memory;std::uintptr_t missing=0;
 template<class T>bool Read(std::uintptr_t at,T& out){const bool okay=memory.Read(at,out);if(!okay&&!missing)missing=at;return okay;}
 bool ReadBytes(std::uintptr_t at,void* out,std::size_t size){const bool okay=memory.ReadBytes(at,out,size);if(!okay&&!missing)missing=at;return okay;}
};
#ifndef CFG_J215_WALL_TEST_MAIN
#define CFG_J215_WALL_TEST_MAIN main
#endif
int CFG_J215_WALL_TEST_MAIN(){unsigned checks=0,failures=0;auto check=[&](bool good,const char* label){++checks;if(!good){++failures;std::printf("FAIL %s\n",label);}};
 check(sizeof(cfgmenu::FrontendReport)<=cfgmenu::FrontendReportReadCap&&cfgmenu::FrontendReportVersion==96,"exact schema96 fits reviewed64KiB bound");
 for(unsigned fault=0;fault<10;++fault){OwnedWallFixture f;ChallengeReader memory;static_cast<StageReader&>(memory)=f.r;memory.cap=10000000;active=&memory;
  observerOwnerThread=GetCurrentThreadId();memory.Set(f.Module+cfgprobe::dispatch::ThreadIdRva,observerOwnerThread);
  if(fault==1)memory.Set(f.Level+0x20,f.ForeignWorld);
  auto menu=f.Menu();menu.thread=observerOwnerThread;cfgj36walls::Runtime runtime;Reader r;r.cap=65536;
  check(runtime.Arm(menu,true,0)&&runtime.Observe(r,observerOwnerThread,100),"actual mission scope binds");r.reads=0;
  check(runtime.Scan(r)&&runtime.Scanned()&&runtime.report.count==4,"actual strong Level/actor census completes");
  if(!runtime.report.ownerCount){DiagnoseWallReader diagnostic{memory};cfgj36walls::LevelOwner owner;
   const bool observed=runtime.ReadOwner(diagnostic,runtime.Actor(0),owner);
   std::printf("OwnershipDiagnostic fault=%u returned=%u missing=0x%llX level=0x%llX outer=0x%llX package=0x%llX\n",fault,observed?1u:0u,
    static_cast<unsigned long long>(diagnostic.missing),static_cast<unsigned long long>(owner.level),static_cast<unsigned long long>(owner.outerWorld),static_cast<unsigned long long>(owner.package));}
  for(unsigned n=0;n<4;++n){Reader geometry;geometry.cap=8192;check(runtime.CaptureGeometry(geometry,n),"actual root/brush ownership and relative rotation observed");
   check(runtime.report.records[n].geometry==3&&runtime.report.records[n].localExtent.x==100&&runtime.report.records[n].localExtent.y==5000,
    "actual current BrushBodySetup convex aggregate supplies local extents independently of rotated world AABB");
   runtime.report.records[n].bounds={{0,0,0},{2006.4f,1999.4f,1500}};}
  cfgj36walls::Classify(runtime.report);check(runtime.report.chosen==(fault==1?0u:4u),"actual package ownership distinguishes mission from original map");
  check(runtime.report.ownerCount==1&&runtime.report.owners[0].persistent==1,"owning ULevel and persistent status published");
  if(fault==2)memory.Set(f.Root(0)+cfgj215walls::disk::RelativeRotation,cfgj36walls::Vec{0,46,0});
  if(fault==3)memory.Set(f.Level+0x20,f.ForeignWorld);
  if(fault==4)memory.Set(f.Root(0)+0xE0,f.World);
  if(fault==5)memory.Set(f.Module+cfgj36walls::disk::SetExecutor,std::uint8_t{});
  if(fault==6)memory.Set(f.Wall(0)+cfgj36walls::disk::CollisionReadbackOffset,std::uint8_t{});
  if(fault==7)memory.Set(f.Root(0)+cfgj215walls::disk::BrushBodySetup,f.ForeignWorld);
  if(fault==8)memory.Set(f.Convex+cfgj215walls::disk::ConvexBox,cfgj215walls::Box{{-200,-5000,-1500},{100,5000,1500},1,{}});
  if(fault==9)memory.Set(f.Convex+cfgj215walls::disk::ConvexTransform,cfgj215walls::Transform{{0,0,0,1},{10,0,0,0},{1,1,1,0}});
  setCalls=0;failReadback=false;ChallengeWallNativeHostBase<WallEngine> host{runtime,0,f.Mode};
  ChallengeGameplayRootProvider<decltype(host)> provider(host,f.Module,f.Mode,0x810000,0x820000);
  cfgmenu::rows::NativeRetentionScope scope(f.Module,provider.Hooks(),provider.Admitted());WallEngine engine(f.Module,observerOwnerThread,true);host.engine=&engine;
  const bool applied=scope.Entered()&&cfgj36walls::Apply(runtime.report.records[0],runtime.report,host);
  check(applied==(fault==0),"real native setter requires unchanged mission/geometry/owner/code/collision");
  check(setCalls==(fault==0?1u:0u),"no native write on stale/foreign/unowned/originally-off input");
  if(!fault)check(cfgj36walls::Restore(runtime.report.records[0],runtime.report,host)&&setCalls==2,"owned mission collision restores through same retained provider");
 }
 active=nullptr;std::printf("J215 actual membership/ownership/geometry/root-provider/setter: %u checks, %u failures; frontend bytes=%zu; native platform simulated.\n",checks,failures,sizeof(cfgmenu::FrontendReport));return failures?1:0;
}
