#define CFG_J217_WALL_TEST_MAIN ExistingWorldProofFixture
#include "J217WallFlowTests.cpp"
int main(){unsigned checks=0,failures=0;const auto check=[&](bool okay){++checks;if(!okay){++failures;std::printf("FAIL check=%u\n",checks);}};
 OwnedWallFixture f;ChallengeReader memory;static_cast<StageReader&>(memory)=f.r;memory.cap=10000000;active=&memory;
 observerOwnerThread=GetCurrentThreadId();memory.Set(f.Module+cfgprobe::dispatch::ThreadIdRva,observerOwnerThread);
 auto menu=f.Menu();menu.thread=observerOwnerThread;cfgj36walls::Runtime run;Reader r;r.cap=65536;
 check(run.Arm(menu,true,0)&&run.Observe(r,observerOwnerThread,100));r.reads=0;check(run.Scan(r)&&run.Scanned());
 using Engine=cfgmenu::rows::NativeDispatchBase<J217WallPlatform>;
 for(unsigned pass=0;pass<2;++pass)for(unsigned index=0;index<4;++index){
  if(pass==0&&index>0){check(!run.SampleFailed(index));continue;}
  ChallengeWallNativeHostBase<Engine> host{run,index,f.Mode};ChallengeGameplayRootProvider<decltype(host)> provider(host,f.Module,f.Mode,0x810000,0x820000);
  cfgmenu::rows::NativeRetentionScope scope(f.Module,provider.Hooks(),provider.Admitted());Engine engine(f.Module,observerOwnerThread,true);host.engine=&engine;
  Reader geometry;geometry.cap=8192;check(scope.Entered()&&run.SampleWall(geometry,index,host));
 }
 check(!run.SamplesPending()&&run.report.sampled==4&&run.report.sampleFailures==3);cfgj36walls::Classify(run.report);check(run.report.chosen==4);
 setCalls=0;failReadback=false;
 for(unsigned index=0;index<4;++index){ChallengeWallNativeHostBase<Engine> host{run,index,f.Mode};
  ChallengeGameplayRootProvider<decltype(host)> provider(host,f.Module,f.Mode,0x810000,0x820000);cfgmenu::rows::NativeRetentionScope scope(f.Module,provider.Hooks(),provider.Admitted());
  Engine engine(f.Module,observerOwnerThread,true);host.engine=&engine;cfgj36walls::Bounds fresh;
  check(scope.Entered()&&host.ReadBounds(fresh)&&run.ApplyWorldSample(index,fresh,host)&&run.report.records[index].state==1);
  check(cfgj36walls::Restore(run.report.records[index],run.report,host));
 }
 check(run.report.applied==4&&run.report.restored==4&&setCalls==8);
 auto changed=run.Actor(0).worldSample;changed.center.x+=1;struct NoWrite {bool Current(const cfgj36walls::Record&){return false;}bool ReadCollision(const cfgj36walls::Record&,bool&){return false;}cfgj36walls::Write Set(const cfgj36walls::Record&,bool){return cfgj36walls::Write::NotEntered;}} noWrite;
 check(run.ApplyWorldSample(0,changed,noWrite)&&run.report.mismatchCount==1&&run.report.mismatches[0].fresh.center.x!=run.report.mismatches[0].stored.center.x);
 active=nullptr;std::printf("J218 every owned wall retries before classification, actual Apply/Restore, fresh/stored mismatch log: %u checks, %u failures\n",checks,failures);return failures?1:0;
}
