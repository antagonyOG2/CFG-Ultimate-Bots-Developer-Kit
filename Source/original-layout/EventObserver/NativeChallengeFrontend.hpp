// Included inside the observer namespace, with persistent storage. Challenge
// engine/text owners are independent of OfflineBots epoch moves/rebinding.
Reader frontendChallengeReader;
std::unique_ptr<cfgmenu::rows::NativeDispatch> frontendChallengeEngine;
std::unique_ptr<cfgmenu::reflection::RetainedMenuText> frontendChallengeText;
using FrontendChallengeRoute=cfgmenu::challengeroute::Route<Reader,cfgmenu::rows::NativeDispatch>;
using FrontendChallengeRows=cfgmenu::rows::NativeSetup<Reader,cfgmenu::rows::NativeDispatch>;
std::unique_ptr<FrontendChallengeRoute> frontendChallengeRoute;
std::array<std::unique_ptr<FrontendChallengeRoute>,cfgmenu::FrontendEpochCapacity> frontendChallengeOldRoutes;
FrontendChallengeRoute* frontendChallengeInherited=nullptr;
struct ChallengeView{std::uintptr_t widget=0;cfgprobe::lifetime::SlotObservation identity{};cfgmenu::active::Context context{};std::unique_ptr<FrontendChallengeRows> rows;bool ready=false;};
std::array<ChallengeView,cfgmenu::FrontendEpochCapacity> frontendChallengeViews;
ChallengeView* frontendChallengeView=nullptr;
cfgmenu::challengeoptions::Runtime frontendChallengeGameplay;
cfgmenu::challengeintro::Runtime frontendChallengeIntro;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
#include "NativeChallengeRetainedScope.hpp"
void SyncChallengeReport()noexcept;
#include "NativeChallengeEdgeWallsFrontend.hpp"
#endif
cfgmenu::challengeintro::ScopeDiagnostics frontendChallengeNativeScope{};
unsigned frontendChallengeNativeProofStage=0;
cfgmenu::challengerestart::Runtime frontendChallengeRestart;
cfgrbemote::Gamepad frontendChallengeStartPad;
cfgrbemote::Input frontendChallengeStartInput;
bool frontendChallengePrepareAttempted=false;
cfgmenu::challengeservice::Service frontendChallengeSettingsService;
bool ChallengeRowsCurrent()noexcept{
 return frontendChallengeSettingsService.current&&frontendChallengeRoute&&frontendChallengeView&&frontendChallengeView->rows&&
  cfgmenu::challengeservice::OwnsView(*frontendChallengeRoute,frontendChallengeView->context);
}
void SyncChallengeReport()noexcept{
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
 frontendReport.challengeWalls=frontendWallRuntime.report;
#endif
 frontendReport.challengeRestarts=frontendChallengeRestart.restarts;frontendReport.challengeRestartFailure=frontendChallengeRestart.failure;
 frontendReport.challengeBoundaryGeneration=frontendBoundaryRuntime.Generation();
 frontendReport.challengeBoundaryArmed=frontendBoundaryRuntime.Armed();frontendReport.challengeBoundaryReady=frontendBoundaryRuntime.Ready();
 frontendReport.challengeBoundaryCleanup=frontendBoundaryRuntime.CleanupPending();frontendReport.challengeBoundaryFailure=frontendBoundaryRuntime.failure;
 frontendReport.challengeBoundaryApplied=frontendBoundaryRuntime.Applied();frontendReport.challengeBoundaryRestored=frontendBoundaryRuntime.Restored();
 frontendReport.challengeBoundaryCells=frontendBoundaryRuntime.Cells();frontendReport.challengeBoundaryAttempts=frontendBoundaryRuntime.attempts;
 frontendReport.challengeBoundaryPeakReads=frontendBoundaryPeakReads;frontendReport.challengeBoundaryErrors=frontendBoundaryErrors+frontendBoundaryWriter.errors;
 frontendReport.challengeBoundaryProtectionPending=frontendBoundaryWriter.Pending();
 frontendReport.challengeBoundaryGuard=frontendBoundaryRuntime.guard;
 frontendReport.challengeBoundaryScopeFailure=frontendBoundaryRuntime.ScopeFailure();
 frontendReport.challengeBoundaryRefreshes=frontendBoundaryRuntime.refreshes;
 frontendReport.challengeBoundaryDeferrals=frontendBoundaryRuntime.deferrals;
 const auto& boundaryProof=frontendBoundaryRuntime.VerifyFailure();
 frontendReport.challengeBoundaryVerifyStage=boundaryProof.stage;frontendReport.challengeBoundaryVerifyRecord=boundaryProof.record;
 frontendReport.challengeBoundaryVerifyDifference=boundaryProof.difference;
 frontendReport.challengeBoundaryVerifyRetries=frontendBoundaryRuntime.VerifyRetries();
 frontendReport.challengeBoundaryVerifyRecoveries=frontendBoundaryRuntime.VerifyRecoveries();
 frontendReport.challengeEscapeLastHookError=frontendEscapeHooks.LastError();
 frontendReport.challengeEscapeArmFailures=frontendEscapeArmFailures;
 frontendReport.challengeBoundaryWallClasses=frontendBoundaryRuntime.WallReport();
 frontendReport.challengeEscapeGeneration=frontendEscapeGeneration;
 frontendReport.challengeEscapeArmed=frontendEscapeRuntime.Armed();frontendReport.challengeEscapeHookState=static_cast<unsigned>(frontendEscapeHooks.Status());
 frontendReport.challengeEscapeErrors=frontendEscapeErrors;frontendReport.challengeEscapeWarm=frontendEscapeRuntime.WarmProgress();
 frontendReport.challengeEscapeCensus=frontendEscapeRuntime.CensusComplete();frontendReport.challengeEscapeVolumes=frontendEscapeRuntime.VolumeCount();
 frontendReport.challengeEscapeBlocks=frontendEscapeRuntime.counters.blocks;frontendReport.challengeEscapeRecoveries=frontendEscapeRuntime.counters.recoveries;
 frontendReport.challengeEscapePeakReads=frontendEscapePeakReads;frontendReport.challengeEscapePeakCalls=frontendEscapePeakCalls;
 frontendReport.challengeEscapeCensusFailure=frontendEscapeRuntime.censusFailure;frontendReport.challengeEscapeCensusResets=frontendEscapeRuntime.censusResets;
 frontendReport.challengeEscapeCensusSteps=frontendEscapeRuntime.counters.censusSteps;frontendReport.challengeEscapeCells=frontendEscapeRuntime.counters.cells;
 frontendReport.challengeEscapeRejectedObject=frontendEscapeRuntime.censusRejectedObject;
 frontendReport.challengeEscapeGateAttempts=frontendEscapeRuntime.gates.attempts;
 frontendReport.challengeEscapeGateBlocks=frontendEscapeRuntime.gates.blocked;
 frontendReport.challengeEscapeGateForwards=frontendEscapeRuntime.gates.forwarded;
 frontendReport.challengeEscapeGateBudgets=frontendEscapeRuntime.gates.budget;
 frontendReport.challengeEscapeGateStage=frontendEscapeRuntime.gates.lastStage;
 frontendReport.challengeEscapeGateRemaining=frontendEscapeRuntime.gates.lastRemaining;
 frontendReport.challengeEscapeGateCurrent=frontendEscapeRuntime.gates.lastCurrent;
 frontendReport.challengeEscapeGateEligibility=frontendEscapeRuntime.gates.lastEligibility;
 frontendReport.challengeEscapeGateActor=frontendEscapeRuntime.gates.lastActor;
 frontendReport.challengeEscapeRecoveryStage=frontendEscapeRuntime.recoveryStage;
 frontendReport.challengeEscapeRecoveryRemaining=frontendEscapeRuntime.recoveryRemaining;
 frontendReport.challengeEscapeRecoveryReserve=frontendEscapeRuntime.recoveryReserve;
 frontendReport.challengeEscapeOwnershipHits=frontendEscapeRuntime.counters.ownershipHits;
 frontendReport.challengeEscapeOwnershipMisses=frontendEscapeRuntime.counters.ownershipMisses;
 if(frontendChallengeRoute){const auto& route=*frontendChallengeRoute;
  frontendReport.challengeRouteState=route.state;frontendReport.challengeRouteFailure=route.failure;
  frontendReport.challengeRouteSub=route.sub;
  frontendReport.challengeRouteOpens=route.opens;frontendReport.challengeRouteBacks=route.backs;frontendReport.challengeRouteStarts=route.starts;
  frontendReport.challengeRoutePublished=route.Published();
  frontendReport.challengePendingQueued=route.pendingQueued;frontendReport.challengePendingCoalesced=route.pendingCoalesced;
  frontendReport.challengePendingDropped=route.pendingDropped;frontendReport.challengePendingExpired=route.pendingExpired;frontendReport.challengePendingConsumed=route.pendingConsumed;}
 if(frontendChallengeRoute){const auto& route=*frontendChallengeRoute;
  frontendReport.challengeStartGuard=route.startGuard;frontendReport.challengeEventGuard=route.eventGuard;
  frontendReport.challengeEventRole=route.eventRole;frontendReport.challengePageRefreshFailure=route.pageRefreshFailure;
  frontendReport.challengePageRefreshes=route.PageRefreshes();}
 if(frontendChallengeRoute){frontendReport.challengePageGuard=frontendChallengeRoute->pageGuard;
  frontendReport.challengeReturnGuard=frontendChallengeRoute->returnGuard;frontendReport.challengeNativeBackFallbacks=frontendChallengeRoute->nativeBackFallbacks;}
 const bool rowsCurrent=ChallengeRowsCurrent();
 frontendReport.challengeRowsState=rowsCurrent?static_cast<unsigned>(frontendChallengeView->rows->State()):0;
 frontendReport.challengeRowsWarmStage=rowsCurrent?frontendChallengeView->rows->WarmStage():0;
 frontendReport.challengeRowsPrepareStage=rowsCurrent?frontendChallengeView->rows->PrepareStage():0;
 frontendReport.challengeSettingsServices=frontendChallengeSettingsService.services;
 frontendReport.challengeSettingsRefusals=frontendChallengeSettingsService.refusals;
 frontendReport.challengeRowsCurrent=rowsCurrent;
 frontendReport.challengeRowsWidget=rowsCurrent?frontendChallengeView->widget:0;
 frontendReport.challengeGameBound=frontendChallengeGameplay.bound;frontendReport.challengeGameRetired=frontendChallengeGameplay.retired;
 frontendReport.challengeGameFailure=frontendChallengeGameplay.failure;frontendReport.challengeGameWrites=frontendChallengeGameplay.writes;
 frontendReport.challengeIntroArmed=frontendChallengeIntro.Armed();frontendReport.challengeIntroEdges=frontendChallengeIntro.input.edges;
 frontendReport.challengeIntroAttempts=frontendChallengeIntro.input.attempts;frontendReport.challengeIntroReturns=frontendChallengeIntro.input.returns;
 frontendReport.challengeIntroFailure=frontendChallengeIntro.failure;frontendReport.challengeIntroWarmAttempts=frontendChallengeIntro.warmAttempts;
 frontendReport.challengeIntroWarmReturns=frontendChallengeIntro.warmReturns;frontendReport.challengeIntroPeakReads=frontendChallengeIntro.peakReads;
 frontendReport.challengeIntroLastRefusal=frontendChallengeIntro.lastRefusal;frontendReport.challengeIntroOwnerFailure=frontendChallengeIntro.ownerFailure;
 frontendReport.challengeIntroScopeFailure=frontendChallengeIntro.scopeFailure;
 frontendReport.challengeIntroValidateStage=frontendChallengeIntro.introValidateStage;
 frontendReport.challengeIntroDispatchStage=static_cast<unsigned>(frontendChallengeIntro.dispatch.stage);
 frontendReport.challengeIntroDispatchReason=static_cast<unsigned>(frontendChallengeIntro.dispatch.reason);
 frontendReport.challengeIntroDispatchPosted=frontendChallengeIntro.dispatch.posted;
 frontendReport.challengeIntroDispatchClaimed=frontendChallengeIntro.dispatch.claimed;
 frontendReport.challengeIntroDispatchReturned=frontendChallengeIntro.dispatch.returned;
 frontendReport.challengeIntroDispatchRefused=frontendChallengeIntro.dispatch.refused;
 frontendReport.challengeIntroNativeScopeEntered=frontendChallengeNativeScope.entered;
 frontendReport.challengeIntroNativeScopeRefused=frontendChallengeNativeScope.refused;
 frontendReport.challengeIntroNativeScopeReason=static_cast<unsigned>(frontendChallengeNativeScope.reason);
 frontendReport.challengeIntroNativeProofStage=frontendChallengeNativeProofStage;
 frontendReport.challengeIntroNativeConsumerServiced=frontendChallengeNativeScope.serviced;
 frontendReport.challengeIntroNativeConsumerDropped=frontendChallengeNativeScope.dropped;
 frontendReport.challengeRoutePeakReads=(std::max)(frontendReport.challengeRoutePeakReads,frontendChallengeReader.reads);
 frontendReport.challengeReacquireFailure=frontendChallengePage.reacquireFailure;
 frontendReport.challengeInspectStage=frontendChallengePage.inspectStage;
 frontendReport.challengeRouteWin32Error=frontendChallengeReader.error;
 frontendReport.challengeRouteBudgetHit=frontendChallengeReader.reads>=frontendChallengeReader.cap&&frontendChallengeReader.error==ERROR_INVALID_PARAMETER;
}
bool RetireChallengeAttempt(Reader& r,const cfgmenu::active::Context& current)noexcept{
 try{
  // A fresh native Menu proof authorizes discarding departed per-world actor
  // leases. It never restores or dereferences departed actors, nor frees rows
  // or installed callbacks. The next explicit Start supplies all new choices.
  if(frontendEscapeActiveReader||current.module!=plan.module||
     !cfgmenu::active::Current(r,current,observerOwnerThread)||
     !cfgmenu::controllergate::Menu(r,current.module,current.controller)||
     !frontendChallengeGameplay.Departed(current.identities[0]))return false;
  frontendChallengeRestart.Cancel();frontendChallengeIntro.Cancel();
  frontendEscapeRuntime.Cancel();frontendBoundaryRuntime.Cancel();
#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && CFG_J36_CHALLENGE_WALLS
  // Fresh menu proves departure. Observe checks the current viewport before
  // any old actor read; missed end callbacks become departed diagnostics.
  frontendWallRuntime.Cancel();
  (void)frontendWallRuntime.Observe(r,observerOwnerThread,GetTickCount64());
  if(frontendWallRuntime.Pending())return false;
#endif
  frontendChallengeStartInput={};SyncChallengeReport();return true;
 }catch(...){return false;}
}
bool CommitChallenge(const cfgmenu::active::Context& current){
 frontendChallengeRestart.Cancel(); // A refused new submission cannot replay old choices.
 if(!ChallengeRowsCurrent()||!frontendChallengeView->ready||
    frontendChallengeView->widget!=current.widget||!(frontendChallengeView->identity==current.identities[2]))return false;
 std::array<unsigned,cfgmenu::rows::Capacity> choices{};auto& view=*frontendChallengeView->rows;
 if(!view.Rows().Choices(view,1,choices))return false;
 if(!RetireChallengeAttempt(frontendChallengeReader,current))return false;
 namespace option=cfgmenu::challengemenu::choice;
#if CFG_J36_CHALLENGE_WALLS
 if(!ArmChallengeWalls(frontendChallengeRoute->Source(),choices[option::EdgeWalls]!=0,GetTickCount64()))return false;
#endif
 if(!ArmChallengeBoundary(frontendChallengeRoute->Source(),choices[option::Boundary]!=0,GetTickCount64()))return false;
 if(!ArmChallengeEscape(frontendChallengeRoute->Source(),choices[option::PreventEscapes]!=0,GetTickCount64())){
#if CFG_J29_CHALLENGE_ESCAPE_HOOKS
  ++frontendEscapeArmFailures;frontendEscapeRuntime.Cancel();
#else
  frontendBoundaryRuntime.Cancel();return false;
#endif
 }
 if(!frontendChallengeGameplay.Arm(frontendChallengeRoute->Source(),
    {choices[0]!=0,choices[1]!=0,choices[2]!=0,choices[3]!=0,choices[4]!=0},GetTickCount64())){frontendBoundaryRuntime.Cancel();frontendEscapeRuntime.Cancel();return false;}
 frontendChallengeIntro.Arm(frontendChallengeRoute->Source()); // First attempt: manual Start skip only.
 std::array<unsigned,option::Count> retained{};std::copy_n(choices.begin(),retained.size(),retained.begin());
 frontendChallengeRestart.Arm(frontendChallengeRoute->Source(),retained);return true;
}
bool FrontendChallengeContinuationInvokeAllowed(std::uintptr_t actor,std::uintptr_t function)noexcept{
 return frontendChallengeRoute&&frontendChallengeRoute->ContinuationInvocation(actor,function);
}
void ChallengeDispatch(cfgmenu::challengeroute::Role role,void* receiver,void* frame,void* result)noexcept{
 if(!frontendChallengeRoute){FrontendOriginal(receiver,frame,result);return;}
 if(!frontendChallengeRoute->Busy()){frontendChallengeReader.reads=0;frontendChallengeReader.error=0;frontendChallengeReader.cap=32768;}
 try{
 if(CFG_J22_CHALLENGES_RETENTION){
  if(!frontendChallengeRoute->QueueBorrowed(role,receiver,frame,GetTickCount64()))FrontendOriginal(receiver,frame,result);
  SyncChallengeReport();return;
 }
 frontendChallengeRoute->Dispatch(role,receiver,frame,[&](){FrontendOriginal(receiver,frame,result);},CommitChallenge,
    [&](const cfgmenu::active::Context& current){return frontendChallengePage.Reacquire(frontendChallengeReader,current,
        [&](std::uintptr_t type){HMODULE resident=nullptr;
         return GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
            reinterpret_cast<LPCWSTR>(&ChallengeDispatch),&resident)&&frontendChallengeRoute->RefreshPage(type,resident);});},
    [](){frontendChallengeRestart.Cancel();(void)frontendChallengeGameplay.CancelPending();frontendChallengeIntro.Cancel();frontendBoundaryRuntime.Cancel();frontendEscapeRuntime.Cancel();});}
 catch(...){frontendChallengeRoute->failure=14;frontendBoundaryRuntime.Cancel();frontendEscapeRuntime.Cancel();}
 if(frontendChallengeRoute->state==8){frontendChallengeRestart.Cancel();frontendBoundaryRuntime.Cancel();frontendEscapeRuntime.Cancel();(void)frontendChallengeGameplay.CancelPending();frontendChallengeIntro.Cancel();}
 SyncChallengeReport();
}
void __fastcall ChallengeLobbyStart(void* o,void* f,void* v){ChallengeDispatch(cfgmenu::challengeroute::Role::LobbyStart,o,f,v);}
void __fastcall ChallengeLobbyGraph(void* o,void* f,void* v){ChallengeDispatch(cfgmenu::challengeroute::Role::LobbyGraph,o,f,v);}
void __fastcall ChallengePageStart(void* o,void* f,void* v){ChallengeDispatch(cfgmenu::challengeroute::Role::PageStart,o,f,v);}
void __fastcall ChallengePageBack(void* o,void* f,void* v){ChallengeDispatch(cfgmenu::challengeroute::Role::PageBack,o,f,v);}
void __fastcall ChallengePageGraph(void* o,void* f,void* v){ChallengeDispatch(cfgmenu::challengeroute::Role::PageGraph,o,f,v);}
bool RefreshQueuedChallengePage(const cfgmenu::active::Context& current){
 return frontendChallengePage.Reacquire(frontendChallengeReader,current,[&](std::uintptr_t type){
  HMODULE resident=nullptr;return GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
   reinterpret_cast<LPCWSTR>(&ChallengeDispatch),&resident)&&frontendChallengeRoute->RefreshPage(type,resident);});
}
void FrontendChallengeQueuedAfter(){
 if(!CFG_J22_CHALLENGES_RETENTION||!frontendChallengeRoute||
    !cfgmenu::rows::NativeRetentionScope::Current(frontendSource.module))return;
 frontendChallengeReader.reads=0;frontendChallengeReader.error=0;frontendChallengeReader.cap=32768;
 (void)frontendChallengeRoute->ServicePending(GetTickCount64(),CommitChallenge,RefreshQueuedChallengePage,
  [](){frontendChallengeRestart.Cancel();(void)frontendChallengeGameplay.CancelPending();frontendChallengeIntro.Cancel();frontendBoundaryRuntime.Cancel();frontendEscapeRuntime.Cancel();});
 SyncChallengeReport();
}
std::array<cfgmenu::rows::Spec,cfgmenu::challengemenu::choice::Count> ChallengeSpecs(){
 return cfgmenu::challengemenu::Specs();
}
void FrontendChallengeGameplayAfter(void* object,std::uint64_t now,std::uintptr_t callbackFunction,std::uintptr_t callbackPc){
 {
  Reader replay;replay.cap=32768;cfgchallengeescape::scope::Witness fresh;
  if(frontendChallengeRestart.Observe(replay,reinterpret_cast<std::uintptr_t>(object),observerOwnerThread,now,fresh)){
   const auto& source=frontendChallengeRestart.Source();const auto& c=frontendChallengeRestart.choices;
   namespace option=cfgmenu::challengemenu::choice;
   frontendChallengeIntro.Cancel();frontendEscapeRuntime.Cancel();frontendBoundaryRuntime.Cancel();
   const bool ready=frontendChallengeGameplay.Departed(fresh.owners[0])&&
      ArmChallengeBoundary(source,c[option::Boundary]!=0,now);
#if CFG_J36_CHALLENGE_WALLS
  if(!ArmChallengeWalls(source,c[option::EdgeWalls]!=0,now)){frontendChallengeRestart.failure=3;return;}
#endif
   const bool escapeArmed=ready?ArmChallengeEscape(source,c[option::PreventEscapes]!=0,now):true;
#if CFG_J29_CHALLENGE_ESCAPE_HOOKS
   if(ready&&!escapeArmed){++frontendEscapeArmFailures;frontendEscapeRuntime.Cancel();}
#endif
   if(!ready||
#if !CFG_J29_CHALLENGE_ESCAPE_HOOKS
      !escapeArmed||
#endif
      !frontendChallengeGameplay.Arm(source,{c[0]!=0,c[1]!=0,c[2]!=0,c[3]!=0,c[4]!=0},now)){
    frontendChallengeRestart.failure=2;frontendChallengeRestart.Cancel();frontendEscapeRuntime.Cancel();frontendBoundaryRuntime.Cancel();
   }else frontendChallengeIntro.Arm(source,(!CFG_J22_CHALLENGES_RETENTION||CFG_J22_CHALLENGE_REPLAY)&&cfgmenu::challengemenu::ReplayChoice(c));
  }
 }
 if(frontendChallengeIntro.PollDue(now)&&frontendChallengeEngine){
  cfgrbemote::Input freshInput;
  if(frontendChallengeStartPad.Poll(now,freshInput,XINPUT_GAMEPAD_START))frontendChallengeStartInput=freshInput;
  Reader intro;intro.cap=8192;
  frontendChallengeIntro.Step(intro,*frontendChallengeEngine,reinterpret_cast<std::uintptr_t>(object),observerOwnerThread,now,
    frontendChallengeStartInput.foreground,frontendChallengeStartInput.down||
      (frontendChallengeStartInput.foreground&&(GetAsyncKeyState(VK_RETURN)&0x8000)!=0));
  frontendChallengeIntro.peakReads=(std::max)(frontendChallengeIntro.peakReads,intro.reads);
 }
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
 // Task008: the J31 handoff is consumed only in THIS completed gameplay
 // callback, never by the ordinary Offline Bots bundle (absent in Challenges).
 // A request is not permission to call. Require an independently admitted
 // callback/GC root provider in addition to every J30/owner reproof.
 if(CFG_J32_CHALLENGE_NATIVE_SCOPE&&CFG_J31_INTRO_SKIP_GAMEPLAY_DISPATCH&&frontendChallengeIntro.dispatch.Pending()){
  const auto request=frontendChallengeIntro.dispatch.request;
  const auto actor=reinterpret_cast<std::uintptr_t>(object);
  Reader preflight;preflight.cap=8192;unsigned proofStage=0;
  const bool sourceCurrent=frontendChallengeRoute&&frontendChallengeRoute->Published()&&
    frontendChallengeGameplay.Armed()&&frontendChallengeIntro.CurrentRequest(preflight,request,actor,observerOwnerThread,proofStage);
  frontendChallengeNativeProofStage=proofStage;
  ChallengeIntroRootProvider provider(frontendChallengeIntro,request,actor,callbackFunction,callbackPc,
      observerOwnerThread,frontendChallengeNativeProofStage);
  cfgmenu::rows::NativeRetentionScope scope(request.module,provider.Hooks(),sourceCurrent&&provider.Admitted());
#if CFG_J218_FINISHING
  if(cfgmenu::challengeintro::DeferUnadmittedIntro(frontendChallengeIntro.dispatch,now,sourceCurrent,provider.Admitted())){
   SyncChallengeReport();return;
  }
#endif
  cfgmenu::challengeintro::ScopeEvidence evidence{};
  evidence.flag=true;evidence.posted=true;evidence.completed=true;
  evidence.gameThread=GetCurrentThreadId()==observerOwnerThread&&observerOwnerThread==request.thread;
  evidence.offline=sourceCurrent;evidence.challengeMode=sourceCurrent;evidence.challengeState=sourceCurrent;
  evidence.routeOwner=sourceCurrent;evidence.worldCurrent=sourceCurrent;evidence.levelSettingsCurrent=sourceCurrent;
  evidence.gcClear=sourceCurrent;evidence.callbackProof=provider.Admitted();
  evidence.retainedInputs=provider.Admitted();evidence.exclusiveScope=scope.Entered();
  frontendChallengeNativeScope.Observe(evidence);
  struct Host {
   cfgmenu::challengeintro::Runtime& runtime;cfgmenu::rows::NativeDispatch& engine;
   std::uintptr_t actor=0,module=0;std::uint32_t thread=0;bool entered=false;
   bool CurrentIntro(const cfgmenu::challengeintro::DispatchRequest& request){
    Reader r;r.cap=8192;unsigned stage=0;
    return entered&&cfgmenu::rows::NativeRetentionScope::Current(module)&&
      runtime.CurrentRequest(r,request,actor,thread,stage);
   }
   bool InvokeRetainedIntro(const cfgmenu::challengeintro::DispatchRequest& request){
    if(!entered||!cfgmenu::rows::NativeRetentionScope::Current(module)||!engine.BeginPhase())return false;
    std::array<unsigned char,16> packet{};
    return engine.Invoke(request.settings,request.function,packet.data());
   }
  };
  if(frontendChallengeEngine){
   // A prior settings-stack transition can quarantine the persistent MENU
   // dispatcher. It must not contaminate this independently proved gameplay
   // request. This short-lived dispatcher still repeats all retention/GC,
   // exact image/code, thread, ownership and post-native proofs.
   cfgmenu::rows::NativeDispatch isolated(request.module,observerOwnerThread,true);
   auto& dispatch=CFG_J214_INTRO_END_TO_END?isolated:*frontendChallengeEngine;
   Host host{frontendChallengeIntro,dispatch,actor,request.module,observerOwnerThread,scope.Entered()};
   if(cfgmenu::challengeintro::Consume(frontendChallengeIntro.dispatch,host,now,frontendChallengeNativeScope))
    ++frontendChallengeIntro.input.returns;
   else if(CFG_J214_INTRO_END_TO_END&&frontendChallengeIntro.dispatch.reason==cfgmenu::challengeintro::DispatchReason::NativeRefused)
    frontendChallengeIntro.failure=100+static_cast<unsigned>(dispatch.LastFailure());
  }else{
   // No dispatch object can satisfy the native capability; consume once.
   struct Refuse {bool CurrentIntro(const cfgmenu::challengeintro::DispatchRequest&){return false;}
       bool InvokeRetainedIntro(const cfgmenu::challengeintro::DispatchRequest&){return false;}} refuse;
   (void)cfgmenu::challengeintro::Consume(frontendChallengeIntro.dispatch,refuse,now,frontendChallengeNativeScope);
  }
 }
#endif // CFG_RETAINED_GAMEPLAY_BUNDLE: no unretained native dispatch in legacy builds
 if(!frontendChallengeGameplay.Armed()){SyncChallengeReport();return;}
 Reader r;r.cap=8192;
 if(frontendChallengeGameplay.Callback(r,reinterpret_cast<std::uintptr_t>(object),now))frontendChallengeGameplay.Step(r,reinterpret_cast<std::uintptr_t>(object),observerOwnerThread,now);
 frontendReport.challengeGamePeakReads=(std::max)(frontendReport.challengeGamePeakReads,r.reads);SyncChallengeReport();
}
void FrontendChallengeSettingsCurrent(const cfgmenu::active::Context& current,std::uint64_t now){
 frontendChallengeRoute->choicesReady=false;
 if(!frontendChallengeView||!cfgmenu::reentry::SameWidget(frontendChallengeView->context,current)){
  frontendChallengeView=nullptr;
  for(auto& v:frontendChallengeViews)if(v.widget&&cfgmenu::reentry::SameWidget(v.context,current)){frontendChallengeView=&v;break;}
  if(!frontendChallengeView){for(auto& v:frontendChallengeViews)if(!v.widget){frontendChallengeView=&v;break;}}
  if(!frontendChallengeView){frontendChallengeRoute->failure=15;SyncChallengeReport();return;}
  auto& view=*frontendChallengeView;
  if(!view.widget){view.widget=current.widget;view.identity=current.identities[2];view.context=current;view.rows=std::make_unique<FrontendChallengeRows>(frontendChallengeReader,*frontendChallengeEngine,*frontendChallengeText);
   const auto specs=ChallengeSpecs();if(!view.rows->OpenForClass(current,1,specs.data(),static_cast<unsigned>(specs.size()),true)){frontendChallengeRoute->failure=16;SyncChallengeReport();return;}}
 }
 auto& view=*frontendChallengeView;
 if(!(view.context==current)){
  if(!view.rows->Rebind(current)){view.ready=false;frontendChallengeRoute->failure=17;SyncChallengeReport();return;}
  view.context=current;
 }
 if(view.rows->State()==cfgmenu::rows::ViewState::Failed||view.rows->State()==cfgmenu::rows::ViewState::Detached){SyncChallengeReport();return;}
 view.rows->Service(now);
 // Keep the native Difficulty widget as the first renamed Off/On modifier.
 // CFG's cooked page retains that widget as its initial/return focus target.
 // Hiding it while appending a replacement leaves an invisible focus owner.
 if(view.rows->State()==cfgmenu::rows::ViewState::Ready)view.ready=true;
 frontendChallengeRoute->choicesReady=view.rows->State()==cfgmenu::rows::ViewState::Ready&&view.ready;SyncChallengeReport();
}
void FrontendChallengeSettingsAfter(bool nested,bool completed){
 if(CFG_J22_CHALLENGES_RETENTION&&!cfgmenu::rows::NativeRetentionScope::Current(frontendSource.module))return;
 if(!frontendChallengeRoute)return;
 frontendChallengeReader.reads=0;frontendChallengeReader.error=0;frontendChallengeReader.cap=32768;
 (void)frontendChallengeSettingsService.After(*frontendChallengeRoute,GetCurrentThreadId(),nested,completed,GetTickCount64(),
  [](const cfgmenu::active::Context& current,std::uint64_t now){FrontendChallengeSettingsCurrent(current,now);});
 SyncChallengeReport();
}
void FrontendChallengeSettingsOpportunity(const cfgmenu::humanjason::Top& top){
 if(!frontendChallengeRoute||top.widget!=frontendChallengeRoute->Page()||top.type!=frontendChallengeRoute->PageType())return;
 FrontendChallengeSettingsAfter(false,true);
}
