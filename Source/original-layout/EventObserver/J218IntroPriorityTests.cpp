#define CFG_J214_INTRO_RETAINED_TEST_MAIN ExistingRetainedIntroFixture
#include "J214IntroRetainedTests.cpp"
int main(){using namespace cfgmenu::challengeintro;unsigned checks=0,failures=0;
 const auto check=[&](bool okay){++checks;if(!okay){++failures;std::printf("FAIL check=%u\n",checks);}};
 IntroFixture f;ChallengeReader memory;static_cast<StageReader&>(memory)=f.r;memory.cap=1000000;active=&memory;
 const auto thread=GetCurrentThreadId();memory.Set(f.Module+cfgprobe::dispatch::ThreadIdRva,thread);
 memory.Set(f.SkipFunction+cfgprobe::lifetime::ObjectIndexOffset,int{9});memory.Set(f.Registry+24*9,f.SkipFunction);
 memory.Set(f.Registry+24*9+16,int{109});memory.Set(f.Registry+24*9+8,std::uint32_t{});memory.Set(f.SkipFunction+8,std::uint32_t{});
 memory.Set(f.Module+cfgprobe::lifetime::ObjectArrayRva+28,int{10});memory.Set(f.Module+cfgspawn::GenericDispatcherRva,cfgspawn::GenericDispatcherPrefix);
 memory.Set(f.Registry+24*4+16,int{0});memory.Set(f.Registry+24*7+16,int{0});
 auto menu=f.Menu();menu.thread=thread;Runtime runtime;runtime.Arm(menu,true);IntroEngine producer{memory};
 unsigned wallsNative=0;const auto wallOpportunity=[&]{if(!WallsWaitForIntro(runtime.Armed(),runtime.dispatch))++wallsNative;};
 wallOpportunity();check(!wallsNative);
 runtime.Step(memory,producer,f.Mode,thread,100,true,false);check(runtime.dispatch.Pending());wallOpportunity();check(!wallsNative);
 const auto request=runtime.dispatch.request;unsigned stage=0;callbackComplete=false;
 {ChallengeIntroRootProvider provider(runtime,request,f.Mode,0x810000,0x820000,thread,stage);
  check(!provider.Admitted()&&DeferUnadmittedIntro(runtime.dispatch,101,true,provider.Admitted()));
  ScopeEvidence evidence;evidence.flag=evidence.posted=evidence.completed=evidence.gameThread=evidence.offline=true;
  evidence.challengeMode=evidence.challengeState=evidence.routeOwner=evidence.worldCurrent=evidence.levelSettingsCurrent=evidence.gcClear=true;
  check(evidence.Refusal()==ScopeRefusal::MissingCallbackProof&&static_cast<unsigned>(evidence.Refusal())==10);
 }
 check(runtime.dispatch.Pending()&&!runtime.dispatch.claimed);wallOpportunity();check(!wallsNative);
 callbackComplete=true;transition=true;nativeCalls=0;
 {ChallengeIntroRootProvider provider(runtime,request,f.Mode,0x810000,0x820000,thread,stage);
  cfgmenu::rows::NativeRetentionScope scope(request.module,provider.Hooks(),provider.Admitted());RetainedEngine engine{f.Module,thread,true};
  Consumer host{runtime,engine,thread};ScopeDiagnostics diagnostics;check(provider.Admitted()&&scope.Entered());
  check(Consume(runtime.dispatch,host,102,diagnostics)&&nativeCalls==1&&runtime.dispatch.claimed==1);
 }
 wallOpportunity();check(wallsNative==1);check(!DeferUnadmittedIntro(runtime.dispatch,103,true,false));
 DispatchReceipt failed;failed.stage=DispatchStage::Refused;check(!WallsWaitForIntro(true,failed));check(!WallsWaitForIntro(false,DispatchReceipt{}));
 check(frontendChallengeRootBank.Healthy()&&!windowHeld);active=nullptr;
 std::printf("J218 walls ON + Start intro priority, actual retained claim before wall work, Reason10 and no replay: %u checks, %u failures\n",checks,failures);return failures?1:0;
}
