#define H29_PRODUCTION_EXTRA 1
#include "H28ProductionTests.cpp"
unsigned H29ProductionExtra(){
 unsigned checks=0,failures=0;const auto check=[&](bool ok){++checks;if(!ok){++failures;std::printf("H29 failure %u\n",checks);}};
 const auto saved=plan;const auto oldH1=hostedH1;const auto oldThread=observerOwnerThread;
 const auto oldBinding=cfgprimary::CurrentHostedBinding();const auto oldRoutine=hostedRoutineStarted;
 observerOwnerThread=GetCurrentThreadId();plan.threadId=observerOwnerThread;
 auto* arena=static_cast<unsigned char*>(VirtualAlloc(nullptr,0x3400000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));check(arena!=nullptr);
 if(arena){
  const auto base=reinterpret_cast<std::uintptr_t>(arena),world=base+0x1000,engine=base+0x2000,viewport=base+0x3000;
  plan.module=base;plan.world=world;plan.engine=engine;plan.viewport=viewport;
  *reinterpret_cast<std::uintptr_t*>(base+cfgprobe::EngineGlobalCandidate)=engine;
  *reinterpret_cast<std::uintptr_t*>(engine+cfgprobe::EngineGameViewport)=viewport;
  *reinterpret_cast<std::uintptr_t*>(viewport+cfgprobe::ViewportWorld)=world;
  hostedH1={};hostedH1.manualArm=hostedH1.armed=true;hostedRoutineStarted=true;
  const auto before=hostedOfflineRetirements;check(RetireHostedBeforeOffline(world));
  check(!hostedH1.armed&&!hostedH1.manualArm&&!hostedRoutineStarted&&!cfgprimary::CurrentHostedBinding().active);
  check(plan.world==world&&plan.module==base&&hostedOfflineRetirements==before+1);
  plan.pawn=base+0x5000;plan.controller=base+0x6000;
  check(!HostedH1EnterPlan(plan.pawn));check(RetireHostedBeforeOffline(world)&&hostedOfflineRetirements==before+1);
  cfgspawn::Identity identity{world,plan.pawn,41,42};cfgspawn::SpawnRequest request;request.identity=identity;
  check(spawnRuntime.Configure(identity,true,0,plan.pawn,0)&&spawnRuntime.Submit(request)&&spawnRuntime.Pending());
  check(cfgbundle::OwnedCallback(plan.pawn,plan.pawn,0,BundleGameplayStarted(),plan.controller,false)&&!HostedH1EnterPlan(plan.pawn));
  spawnRuntime.ResetEpoch();
  hostedH1.manualArm=true;*reinterpret_cast<std::uintptr_t*>(world+cfgprobe::WorldNetDriver)=base+0x4000;
  check(!RetireHostedBeforeOffline(world)&&hostedH1.manualArm); // hosted world is never retired as offline
  *reinterpret_cast<std::uintptr_t*>(world+cfgprobe::WorldNetDriver)=0;
  check(RetireHostedBeforeOffline(world)&&!hostedH1.armed);
  VirtualFree(arena,0,MEM_RELEASE);
 }
#ifdef H210_PRODUCTION_EXTRA
 {cfgbundle::HelperStartup helpers;check(helpers.Active());check(helpers.AdoptNative(0,0)&&!helpers.Active());
  check(cfgoptionalwork::RadioDue(false,helpers.Active(),0,1000));
  check(!cfgoptionalwork::RadioDue(true,helpers.Active(),0,1000));}
#endif
 hostedH1=oldH1;plan=saved;observerOwnerThread=oldThread;cfgprimary::CurrentHostedBinding()=oldBinding;hostedRoutineStarted=oldRoutine;
 std::printf("H29 actual hosted->offline one-shot retirement / armed isolation: %u checks, %u failures; own memory only.\n",checks,failures);return failures;
}
