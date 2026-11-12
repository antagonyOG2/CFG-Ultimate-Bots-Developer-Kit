// Included inside the observer namespace after intro retained provider.
#include "NativeChallengeGameplayRootProvider.hpp"
cfgj36walls::Runtime frontendWallRuntime;
std::uint64_t frontendWallNext=0;unsigned frontendWallRetries=0;
#include "NativeChallengeWallNativeHost.hpp"
bool ArmChallengeWalls(const cfgmenu::active::Context& source,bool enabled,std::uint64_t now){
 frontendWallNext=0;frontendWallRetries=0;return frontendWallRuntime.Arm(source,enabled,now);
}
void FrontendWallsAfter(void* object,void* function,bool nested,bool completed,std::uintptr_t pc){
#if CFG_J36_CHALLENGE_WALLS
 if(nested||!completed||GetCurrentThreadId()!=observerOwnerThread||!frontendWallRuntime.Armed())return;
#if CFG_J218_FINISHING
 // Run before any wall observation, native dispatch or root publication.
 if(cfgmenu::challengeintro::WallsWaitForIntro(frontendChallengeIntro.Armed(),frontendChallengeIntro.dispatch))return;
#endif
 const auto now=GetTickCount64();if(now<frontendWallNext)return;frontendWallNext=now+50;
 Reader r;r.cap=65536;
 auto& run=frontendWallRuntime;auto& walls=run.report;
 if(!run.Observe(r,observerOwnerThread,now)){SyncChallengeReport();return;}
 const auto actor=reinterpret_cast<std::uintptr_t>(object),fn=reinterpret_cast<std::uintptr_t>(function);
 bool owner=false;for(const auto& slot:run.Witness().owners)owner|=slot.object==actor;
 if(!owner||!cfgtick::Observe(r,run.Source().module,actor,fn,pc))return;
 if(!run.Stopping()&&!run.Scanned()){
  if(!run.Scan(r)){walls.failure=4;run.Cancel();}
  SyncChallengeReport();return;
 }
 if(!run.Stopping()&&walls.stage==3){cfgj36walls::Classify(walls);run.ResetWork();walls.stage=4;}
 if(run.Stopping()&&walls.stage!=6){walls.stage=6;run.ResetWork();}
 if(walls.stage==5)return;
 if(run.Work()>=walls.count){
  run.ResetWork();
#if CFG_J218_FINISHING
  if(!run.Stopping()&&walls.stage==2&&run.SamplesPending()){SyncChallengeReport();return;}
#endif
  if(run.Stopping())run.Stopped();else walls.stage=walls.stage==2?3u:5u;
  SyncChallengeReport();return;
 }
 const auto index=run.Work();auto& record=walls.records[index];
#if CFG_J218_FINISHING
 if(!run.Stopping()&&walls.stage==2&&!run.NeedSample(index)){run.Advance();SyncChallengeReport();return;}
#endif
 if((run.Stopping()&&record.state!=1)||(!run.Stopping()&&walls.stage==4&&record.decision!=cfgj36walls::Decision::Edge)){
  run.Advance();SyncChallengeReport();return;}
 ChallengeWallNativeHost host{run,index,actor};
 ChallengeGameplayRootProvider<ChallengeWallNativeHost> provider(host,run.Source().module,actor,fn,pc);
 cfgmenu::rows::NativeRetentionScope scope(run.Source().module,provider.Hooks(),provider.Admitted());
#if CFG_J218_FINISHING && CFG_H276_CHALLENGE_WALLS
 if(provider.AdmitClause()<walls.gateAdmit.size())++walls.gateAdmit[provider.AdmitClause()];
 if(!scope.Entered()&&!run.Stopping()&&walls.stage==2){++walls.gateSampleScope;walls.gateSampleInput=cfgmenu::rows::NativeRetentionScope::InputFailure();walls.gateSampleOutput=cfgmenu::rows::NativeRetentionScope::OutputFailure();}
#endif
 cfgmenu::rows::NativeDispatch engine(run.Source().module,observerOwnerThread,true);host.engine=&engine;
 bool done=false;
 if(scope.Entered()){
  if(run.Stopping())done=cfgj36walls::Restore(record,walls,host)||record.state!=1;
  else if(walls.stage==2){
#if CFG_J218_FINISHING
   Reader geometry;geometry.cap=8192;done=run.SampleWall(geometry,index,host);
#else
   cfgj36walls::Bounds world;done=host.ReadBounds(world)&&run.RememberWorld(index,world);
#if CFG_J215_CHALLENGE_WALL_OWNERSHIP
   if(done){Reader geometry;geometry.cap=8192;(void)run.CaptureGeometry(geometry,index);}
#endif
#endif
  }
  else if(walls.stage==4){
   cfgj36walls::Bounds fresh;
   if(host.ReadBounds(fresh)){
    done=run.ApplyWorldSample(index,fresh,host);
#if CFG_J218_FINISHING && CFG_H276_CHALLENGE_WALLS
    walls.gateApply+=!done;
#endif
   }
#if CFG_J218_FINISHING && CFG_H276_CHALLENGE_WALLS
   else ++walls.gateBounds;
#endif
  }
 }
#if CFG_J218_FINISHING
 if(!scope.Entered()&&!run.Stopping()&&walls.stage==2)(void)run.SampleFailed(index);
#if CFG_H276_CHALLENGE_WALLS
 // H2.76: which gate refused during apply. scope never entered / ReadBounds refused / Apply refused (gateApplyStep), plus the last detail.
 if(!run.Stopping()&&walls.stage==4&&!done){
  if(!scope.Entered()){++walls.gateScope;walls.gateInput=cfgmenu::rows::NativeRetentionScope::InputFailure();walls.gateOutput=cfgmenu::rows::NativeRetentionScope::OutputFailure();}
  walls.gateWhy=host.why;walls.gateEngine=static_cast<unsigned>(engine.LastFailure());
 }
#endif
#endif
 if(CFG_J218_FINISHING&&!run.Stopping()&&walls.stage==2){frontendWallRetries=0;run.Advance();}
 else if(done){frontendWallRetries=0;run.Advance();}
 else if(++frontendWallRetries>=64){walls.failure=20+static_cast<unsigned>(engine.LastFailure());frontendWallRetries=0;
  if(run.Stopping()){walls.failure=40;record.state=3;++walls.uncertain;run.Advance();}
  else{
#if CFG_J218_FINISHING && CFG_H276_CHALLENGE_WALLS
   ++walls.gateExhausted;
#endif
   record.decision=cfgj36walls::Decision::Changed;run.Advance();}}
 if(!frontendChallengeRootBank.Healthy()){walls.failure=50;run.Cancel();}
 SyncChallengeReport();
#else
 (void)object;(void)function;(void)nested;(void)completed;(void)pc;
#endif
}
