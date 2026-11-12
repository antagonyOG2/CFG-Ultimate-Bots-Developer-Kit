#define CFG_J215_WALL_TEST_MAIN ExistingJ215WallChecks
#include "J215WallRuntimeTests.cpp"
struct J217WallPlatform:WallPlatform {
 static void Invoke(std::uintptr_t address,std::uintptr_t target,std::uintptr_t function,void* packet){
  if(function==WallFixture::BoundsFunction){auto* p=static_cast<cfgcabin::Packet*>(packet);
   p->origin={-550.3f,-8928.7f,586.1f};p->extent={1574.9f,6052.1f,1500.f};++boundsCalls;return;}
  WallPlatform::Invoke(address,target,function,packet);
 }
};
#ifndef CFG_J217_WALL_TEST_MAIN
#define CFG_J217_WALL_TEST_MAIN main
#endif
int CFG_J217_WALL_TEST_MAIN(){unsigned checks=0,failures=0;auto check=[&](bool okay){++checks;if(!okay){++failures;std::printf("FAIL check=%u\n",checks);}};
 for(unsigned fault:{0u,3u,1u,2u}){OwnedWallFixture f;ChallengeReader memory;static_cast<StageReader&>(memory)=f.r;
  memory.cap=10000000;active=&memory;observerOwnerThread=GetCurrentThreadId();memory.Set(f.Module+cfgprobe::dispatch::ThreadIdRva,observerOwnerThread);
  auto menu=f.Menu();menu.thread=observerOwnerThread;cfgj36walls::Runtime run;Reader r;r.cap=65536;
  check(run.Arm(menu,true,0)&&run.Observe(r,observerOwnerThread,100));r.reads=0;check(run.Scan(r)&&run.Scanned());
  using Engine=cfgmenu::rows::NativeDispatchBase<J217WallPlatform>;
  ChallengeWallNativeHostBase<Engine> host{run,0,f.Mode};ChallengeGameplayRootProvider<decltype(host)> provider(host,f.Module,f.Mode,0x810000,0x820000);
  cfgmenu::rows::NativeRetentionScope scope(f.Module,provider.Hooks(),provider.Admitted());Engine engine(f.Module,observerOwnerThread,true);host.engine=&engine;
  cfgj36walls::Bounds world;check(scope.Entered()&&host.ReadBounds(world)&&run.RememberWorld(0,world));
  Reader geometry;geometry.cap=8192;check(run.CaptureGeometry(geometry,0));
  check(run.Actor(0).worldSample.extent.x!=run.Actor(0).local.extent.x);
  cfgj36walls::Classify(run.report);check(run.report.chosen==1&&run.report.records[0].decision==cfgj36walls::Decision::Edge);
  // Replay the live cross-kind pollution: stage4 must use its WORLD snapshot.
  run.report.records[0].bounds=run.Actor(0).local;
  cfgj36walls::Bounds fresh;check(host.ReadBounds(fresh));
  if(fault==1)fresh.extent.x+=1.f;
  if(fault==2)memory.Set(f.Root(0)+cfgj215walls::disk::RelativeRotation,cfgj36walls::Vec{0,46,0});
  if(fault==3)fresh.center.x+=.01f;
  setCalls=0;failReadback=false;run.ApplyWorldSample(0,fresh,host);
  const bool okay=fault==0||fault==3;check(run.report.applied==(okay?1u:0u)&&setCalls==(okay?1u:0u));
  if(okay)check(cfgj36walls::Restore(run.report.records[0],run.report,host)&&run.report.restored==1&&setCalls==2);
 }
 active=nullptr;std::printf("J217 rotated ownership wall stage2-world/local proof/stage4 Apply/Restore, drift refusal: %u checks, %u failures\n",checks,failures);return failures?1:0;
}
