// Included in observer namespace after native frontend globals and thunks.
using FrontendHumanJasonMenu=cfgmenu::humanjason::Menu<Reader,cfgmenu::rows::NativeDispatch>;
std::unique_ptr<FrontendHumanJasonMenu> frontendHumanJason;
float FrontendHumanJasonDifficultyDamage(std::uintptr_t victim,float incoming,std::uintptr_t instigator,std::uintptr_t causer)noexcept{
 if(GetCurrentThreadId()!=observerOwnerThread||frontendCounselorClaim||!frontendHumanJason)return incoming;
 Reader r;r.cap=1536;
 try{return frontendHumanJason->gameplay.DifficultyDamage(r,observerOwnerThread,victim,incoming,instigator,causer);}
 catch(...){return incoming;}
}
std::array<std::unique_ptr<FrontendHumanJasonMenu>,cfgmenu::FrontendEpochCapacity> frontendHumanJasonCached;
std::uint64_t frontendHumanJasonNext=0;
std::uintptr_t frontendHumanJasonRetiringWidget=0;
bool FrontendHumanJasonOwns(std::uintptr_t address)noexcept{
 return (frontendHumanJason&&frontendHumanJason->Owns(address))||frontendHumanJasonRetiringWidget==address;
}
cfgmenu::humanjason::Top frontendHumanJasonTop{};
bool frontendHumanJasonTopNative=false;
cfgmenu::challengepage::Resolver frontendChallengePage;
cfgmenu::humanjason::Top frontendChallengeTop{};
bool frontendChallengeTopIsLobby=false;
std::uint64_t frontendChallengeReturnNext=0;
#include "NativeChallengeEscapeFrontend.hpp"
#include "NativeChallengeBoundaryFrontend.hpp"
#include "NativeChallengeFrontend.hpp"
void FrontendChallengePageOpportunity(const cfgmenu::humanjason::Top& top)noexcept{
 try{
 if(CFG_J22_CHALLENGES_RETENTION){auto* scope=cfgmenu::startupretain::Scope::Current(frontendSource.module);
  if(!scope||!scope->Allows(cfgmenu::startupretain::Call::FrontendChallengePageClass))return;}
 FrontendChallengeSettingsOpportunity(top);
 if(!(top==frontendChallengeTop)){
  frontendChallengeTop=top;frontendChallengeTopIsLobby=false;
  Reader hint;hint.cap=256;cfgnames::Observation<Reader> names(hint,frontendSource.module);cfgnames::Text label;
  frontendChallengeTopIsLobby=names.Object(top.type,label)&&label.View()==u"SP_LobbyWidget_C";
 }
 if(!frontendChallengeTopIsLobby)return;
 auto& r=frontendChallengeReader;r.reads=0;r.error=0;r.cap=32768;cfgmenu::active::Context current;
 if(!cfgmenu::active::Observe(r,frontendSource.module,current)||current.widget!=top.widget||current.hud!=top.hud||
    current.type!=top.type||current.world!=top.world||current.thread!=observerOwnerThread)return;
 std::uintptr_t native=0,ctor=0,modeType=0;
 if(!r.Read(current.module+cfgmenu::execdisk::FrontendClass,native)||!r.Read(current.module+cfgmenu::execdisk::FrontendConstructor,ctor)||
    !cfgprobe::MatchingClassCache(native,ctor)||!r.Read(current.mode+cfgprobe::ObjectClass,modeType)||
    !cfgmenu::maps::Subclass(r,modeType,native))return;
 if(!frontendChallengeEngine){frontendChallengeEngine=std::make_unique<cfgmenu::rows::NativeDispatch>(current.module,current.thread,true);
  frontendChallengeText=std::make_unique<cfgmenu::reflection::RetainedMenuText>();}
 if(frontendChallengePage.stage<4){frontendChallengePage.Service(r,*frontendChallengeEngine,current);}
 else if(frontendChallengeRoute&&frontendChallengeRoute->Published()){
  if(frontendChallengeRoute->state==5||frontendChallengeRoute->state==7)(void)frontendChallengeRoute->ResumeReturn();
  if(frontendChallengeRoute->state==8){frontendBoundaryRuntime.Cancel();frontendEscapeRuntime.Cancel();(void)frontendChallengeGameplay.CancelPending();frontendChallengeIntro.Cancel();}
  else if((frontendChallengeRoute->state==6||!(frontendChallengeRoute->Source()==current))&&frontendChallengeRoute->state!=2&&frontendChallengeRoute->state!=3){
   if(!frontendChallengeRoute->Rebind(current)&&(current.type!=frontendChallengeRoute->Source().type||
      !cfgmenu::reentry::Infrastructure(frontendChallengeRoute->Source(),current))){
    for(auto& old:frontendChallengeOldRoutes)if(!old){old=std::move(frontendChallengeRoute);frontendChallengeInherited=old.get();frontendChallengePrepareAttempted=false;break;}
   }
  }
 }else if(!frontendChallengePrepareAttempted){
  frontendChallengePrepareAttempted=true;frontendChallengeRoute=std::make_unique<FrontendChallengeRoute>(r,*frontendChallengeEngine);
  HMODULE resident=nullptr;
  if(GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,reinterpret_cast<LPCWSTR>(&ChallengeLobbyStart),&resident)){
   const std::array<std::uintptr_t,5> thunks{{reinterpret_cast<std::uintptr_t>(&ChallengeLobbyStart),reinterpret_cast<std::uintptr_t>(&ChallengeLobbyGraph),
     reinterpret_cast<std::uintptr_t>(&ChallengePageStart),reinterpret_cast<std::uintptr_t>(&ChallengePageBack),reinterpret_cast<std::uintptr_t>(&ChallengePageGraph)}};
   (void)frontendChallengePage.Reacquire(r,current,[&](std::uintptr_t type){return frontendChallengeRoute->Prepare(current,type,thunks,frontendChallengeInherited)&&frontendChallengeRoute->Publish(resident);});
  }
 }
 frontendReport.challengePageStage=frontendChallengePage.stage;frontendReport.challengePageFailure=frontendChallengePage.failure;
 frontendReport.challengePageAttempts=frontendChallengePage.attempts;frontendReport.challengePageSchemaMask=frontendChallengePage.schemaMask;
 frontendReport.challengePageClass=frontendChallengePage.pageClass;
 frontendReport.challengePagePeakReads=(std::max)(frontendReport.challengePagePeakReads,r.reads);
 SyncChallengeReport();
 }catch(...){frontendChallengePage.stage=5;frontendChallengePage.failure=7;
  frontendReport.challengePageStage=5;frontendReport.challengePageFailure=7;}
}
void FrontendChallengeReturnOpportunity(bool nested,bool completed)noexcept{
 if(CFG_J22_CHALLENGES_RETENTION){auto* scope=cfgmenu::startupretain::Scope::Current(plan.module);
  if(!scope||!scope->Allows(cfgmenu::startupretain::Call::FrontendChallengePageClass))return;}
 // A completed owner-thread callback supplies scheduling only. Returning from
 // a Challenge replaces the World/HUD/SP_Lobby class; the old OfflinePlay HUD
 // event cannot be its lifetime owner. Never adopt gameplay/online/error tops.
 if(nested||!completed||GetCurrentThreadId()!=observerOwnerThread||!frontendChallengeRoute||
    !frontendChallengeRoute->Published()||frontendChallengeRoute->Busy())return;
 // A recreated HUD never emits the old OfflinePlay event. Service the actual
 // routed settings independently, before the generic OfflinePlay epoch gate.
 // Each paced phase still proves PageCurrent; no callback receiver is adopted.
 if(frontendChallengeRoute->state==2||frontendChallengeRoute->state==3){
  try{FrontendChallengeSettingsAfter(nested,completed);}catch(...){frontendChallengeRoute->choicesReady=false;frontendChallengeRoute->failure=16;SyncChallengeReport();}
  return;
 }
 const auto now=GetTickCount64();if(now<frontendChallengeReturnNext||now>UINT64_MAX-250)return;
 frontendChallengeReturnNext=now+250;
 try{
  Reader r;r.cap=32768;cfgmenu::active::Context actual;
  if(!cfgmenu::challengereturn::Observe(r,frontendChallengeRoute->Source(),observerOwnerThread,actual))return;
  // Use only the NEW full positive current context to provide the routing hint.
  // PageOpportunity independently reobserves it before rebind/republication.
  cfgmenu::humanjason::Top top;top.world=actual.world;top.hud=actual.hud;top.stack=actual.stack;
  top.widget=actual.widget;top.type=actual.type;top.mode=actual.mode;top.array=actual.array;
  FrontendChallengePageOpportunity(top);
 }catch(...){/* No failed rebind may authorize native Start or old-world calls. */}
}
void SyncHumanJasonReport()noexcept{
 if(!frontendHumanJason)return;
 const auto& r=frontendHumanJason->gameplay.report;
 static unsigned rage=0,damage=0;
 if(r.rageState!=rage){rage=r.rageState;cfgclosedtrace::Mark("human-jason-difficulty-rage",rage,r.failure);}
 if(r.damageScaled!=damage){damage=r.damageScaled;if(damage<=8)cfgclosedtrace::Mark("human-jason-difficulty-scaled-hit",damage);}
 frontendReport.humanJasonMenuStage=frontendHumanJason->Stage();frontendReport.humanJasonMenuFailure=frontendHumanJason->Failure();
 frontendReport.humanJasonBound=r.bound;frontendReport.humanJasonRetired=r.retired;frontendReport.humanJasonOptionFailure=r.failure;
 frontendReport.humanJasonSkillState=r.skillState;frontendReport.humanJasonDurationState=r.durationState;
 frontendReport.humanJasonKnivesWrites=r.knivesWrites;frontendReport.humanJasonMovementWrites=r.movementWrites;frontendReport.humanJasonRechargeWrites=r.rechargeWrites;
}
struct FrontendHumanJasonWriter {Reader& r;
 template<class T>bool Write(std::uintptr_t address,const T& expected,const T& desired){return cfgmenu::humanjason::Write(r,address,expected,desired);}
};
void FrontendHumanJasonIntroSettings(void* mode)noexcept{
 if(frontendCounselorClaim||!frontendHumanJason||GetCurrentThreadId()!=observerOwnerThread)return;
 Reader r;r.cap=1536;FrontendHumanJasonWriter writer{r};
 try{frontendHumanJason->gameplay.BeforeIntro(r,reinterpret_cast<std::uintptr_t>(mode),observerOwnerThread,writer);}
 catch(...){frontendHumanJason->gameplay.report.skillState=4;}
 frontendReport.humanJasonGamePeakReads=(std::max)(frontendReport.humanJasonGamePeakReads,r.reads);SyncHumanJasonReport();
}
bool FrontendHumanJasonStart(cfgmenu::startroute::SlotKind kind,void* receiver,void* frame,void* result){
 if(frontendHumanJasonRetiringWidget==reinterpret_cast<std::uintptr_t>(receiver))return true;
 if(!frontendHumanJason||GetCurrentThreadId()!=observerOwnerThread||!frontendHumanJason->Owns(reinterpret_cast<std::uintptr_t>(receiver)))return false;
 if(!FrontendPreviousWorldAllowsLoad())return true;
 frontendReader.reads=0;frontendReader.error=0;frontendReader.cap=32768;
 if(!RetireChallengeAttempt(frontendReader,frontendHumanJason->Context()))return true;
 frontendHumanJason->Start(kind,GetTickCount64(),[&](){FrontendOriginal(receiver,frame,result);});
 frontendReport.humanJasonMenuPeakReads=(std::max)(frontendReport.humanJasonMenuPeakReads,frontendReader.reads);SyncHumanJasonReport();return true;
}
bool FrontendHumanJasonAfter(void* object,void* function,bool nested,bool completed,std::uintptr_t returnPc){
 using namespace cfgmenu;
 if(nested||!completed||GetCurrentThreadId()!=observerOwnerThread)return false;
 const auto now=GetTickCount64(),receiver=reinterpret_cast<std::uintptr_t>(object);
 FrontendChallengeGameplayAfter(object,now,reinterpret_cast<std::uintptr_t>(function),returnPc);
 struct ReportGuard{~ReportGuard(){SyncHumanJasonReport();}} reportGuard;
 if(frontendHumanJason&&frontendHumanJason->gameplay.Armed()){
  Reader r;r.cap=1536;
  if(frontendHumanJason->gameplay.Callback(r,receiver,now)){
   FrontendHumanJasonWriter writer{r};frontendHumanJason->gameplay.Step(r,receiver,observerOwnerThread,now,writer);
   frontendReport.humanJasonGamePeakReads=(std::max)(frontendReport.humanJasonGamePeakReads,r.reads);
  }
 }
 if(!frontendEngine||!frontendText||!frontendSettingsClass||
    reinterpret_cast<std::uintptr_t>(function)!=frontendBinding.event)return false;
 Reader hint;hint.cap=32;humanjason::Top top;
 if(!humanjason::ObserveTop(hint,frontendSource,receiver,top)){frontendHumanJasonTop={};frontendHumanJasonTopNative=false;return false;}
 const bool relevant=top.type==frontendSettingsClass||(frontendEntryResources&&top.type==frontendEntryResources->map);
 if(!relevant){FrontendChallengePageOpportunity(top);frontendHumanJasonTop={};frontendHumanJasonTopNative=false;frontendHumanJasonRetiringWidget=0;return false;}
 const bool same=top==frontendHumanJasonTop;
 // Route ownership does not depend on whether the next native UI phase is due.
 if(now<frontendHumanJasonNext)return !frontendCounselorClaim;
 if(!frontendCounselorClaim&&same&&frontendHumanJasonTopNative&&frontendHumanJason&&frontendHumanJason->Ready())return true;
 frontendHumanJasonNext=now+25;frontendReader.reads=0;frontendReader.error=0;frontendReader.cap=32768;
 active::Context current;
 if(!active::Observe(frontendReader,frontendSource.module,current)||receiver!=current.hud)return !frontendCounselorClaim;
 if(current.type!=frontendSettingsClass){
  frontendHumanJasonRetiringWidget=0;
  frontendHumanJasonTop=top;frontendHumanJasonTopNative=!frontendCounselorClaim&&humanjason::NativeRoute(frontendReader,current);
  return frontendHumanJasonTopNative;
 }
 if(frontendCounselorClaim){
  const auto retire=[&](std::unique_ptr<FrontendHumanJasonMenu>& owned){
   if(!owned||!reentry::SameWidget(owned->Context(),current))return false;
   hud::Binding transport;if(!hud::Observe(frontendReader,current,true,transport))return true;
   if(owned->RetireRowsForMode(transport,now))owned.reset();return true;
  };
  if(retire(frontendHumanJason))return true;
  for(auto& cached:frontendHumanJasonCached)if(retire(cached))return true;
  return false;
 }
 if(!same||!frontendHumanJasonTopNative){
  frontendHumanJasonTop=top;frontendHumanJasonTopNative=humanjason::NativeRoute(frontendReader,current);
 }
 if(!frontendHumanJasonTopNative)return false;
 const auto retireCounselor=[&](std::unique_ptr<FrontendLaunch>& owned){
  if(!owned||!reentry::SameWidget(owned->Context(),current))return false;
  frontendHumanJasonRetiringWidget=current.widget;
  hud::Binding transport;if(!hud::Observe(frontendReader,current,true,transport))return true;
  if(owned->RetireRowsForMode(transport,now)){
   frontendSharedStartBinding=owned->StartBinding();owned.reset();frontendHumanJasonRetiringWidget=0;
  }return true;
 };
 if(retireCounselor(frontendLaunch))return true;
 for(auto& cached:frontendCachedLaunches)if(retireCounselor(cached))return true;
 // Do not let the counselor coordinator adopt ordinary native Jason settings.
 if(frontendHumanJason&&reentry::SameWidget(frontendHumanJason->Context(),current)){
  if(!(frontendHumanJason->Context()==current)||frontendHumanJason->Stage()==4){
   hud::Binding transport;if(!hud::Observe(frontendReader,current,true,transport)||!frontendHumanJason->Resume(current,transport))return true;
  }
 }else{
  if(frontendHumanJason){
   unsigned slot=0;while(slot<frontendHumanJasonCached.size()&&frontendHumanJasonCached[slot])++slot;
   if(slot==frontendHumanJasonCached.size())return true;
   if(frontendHumanJason->StartBinding().start.field.object)frontendSharedStartBinding=frontendHumanJason->StartBinding();
   frontendHumanJasonCached[slot]=std::move(frontendHumanJason);
  }
  for(auto& cached:frontendHumanJasonCached)if(cached&&reentry::SameWidget(cached->Context(),current)){frontendHumanJason=std::move(cached);break;}
  hud::Binding transport;if(!hud::Observe(frontendReader,current,true,transport))return true;
  if(frontendHumanJason){if(!frontendHumanJason->Resume(current,transport))return true;}
  else{
   HMODULE resident=nullptr;
   // Do not publish the difficulty row in an isolated startup probe or when
   // the actual single TakeDamage hook has not been installed successfully.
   if(!bundleDifficultyHookEnabled)return true;
   if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,reinterpret_cast<LPCWSTR>(&FrontendStart),&resident))return true;
   frontendHumanJason=std::make_unique<FrontendHumanJasonMenu>(frontendReader,*frontendEngine,*frontendText);
   if(!frontendHumanJason->Open(current,transport,resident,reinterpret_cast<std::uintptr_t>(&FrontendStart),reinterpret_cast<std::uintptr_t>(&FrontendGraph),
        frontendSharedStartBinding?&*frontendSharedStartBinding:nullptr))return true;
  }
 }
 frontendHumanJason->After(receiver,reinterpret_cast<std::uintptr_t>(function),observerOwnerThread,false,true,now);
 frontendReport.humanJasonMenuPeakReads=(std::max)(frontendReport.humanJasonMenuPeakReads,frontendReader.reads);
 if(frontendHumanJason->Published())frontendSharedStartBinding=frontendHumanJason->StartBinding();return true;
}
