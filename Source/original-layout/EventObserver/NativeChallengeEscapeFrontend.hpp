// Included inside the frontend's process-resident private namespace.
cfgchallengeescape::hooks::WindowsBackend frontendEscapeBackend;
cfgchallengeescape::hooks::Five<> frontendEscapeHooks(frontendEscapeBackend);
cfgchallengeescape::hooks::Originals frontendEscapeOriginals;
cfgchallengeescape::Runtime frontendEscapeRuntime;
std::uint64_t frontendEscapeGeneration=0,frontendEscapeNext=0;
using EscapeReader=cfgchallengeescape::dispatch::CallbackReader<cfgchallengeescape::dispatch::WindowsSource>;
struct EscapeDispatchLease {
 bool operator()(EscapeReader& r){cfgchallengeescape::scope::Witness w;
  // A read-only ChallengeLookupSafe veto is NOT native-call authority.
  // Without an independently entered retained ScopedLookupEligibility for
  // THIS reader/session, native factories/queries/mutations are disabled.
#if CFG_J29_CHALLENGE_ESCAPE_HOOKS
  if(!CFG_J29_CHALLENGE_ESCAPE_PHYSICAL)return false;
  if(!cfgprimary::LookupSafe(r,plan.module))return false;
#endif
  return frontendEscapeHooks.BudgetedSuppressionReady(r)&&frontendEscapeRuntime.CurrentWitness(w)&&
   frontendEscapeRuntime.ScopeCurrent(r,observerOwnerThread,w);
 }
};
using EscapeEngine=cfgchallengeescape::dispatch::NativeEngine<EscapeReader,EscapeDispatchLease>;
thread_local EscapeReader* frontendEscapeActiveReader=nullptr;
thread_local EscapeEngine* frontendEscapeActiveEngine=nullptr;
struct EscapeCallbackScope {
 EscapeCallbackScope(EscapeReader& r,EscapeEngine& e){frontendEscapeActiveReader=&r;frontendEscapeActiveEngine=&e;}
 ~EscapeCallbackScope(){frontendEscapeActiveReader=nullptr;frontendEscapeActiveEngine=nullptr;}
};
unsigned frontendEscapePeakReads=0,frontendEscapePeakCalls=0,frontendEscapeErrors=0,frontendEscapeArmFailures=0;
void SyncChallengeReport()noexcept;
void EscapeMetrics(const EscapeReader& r,const EscapeEngine& e){
 frontendEscapePeakReads=(std::max)(frontendEscapePeakReads,r.Reads());
 frontendEscapePeakCalls=(std::max)(frontendEscapePeakCalls,e.Calls());
 SyncChallengeReport();
}
bool EscapeGateBorrowed(EscapeReader& r,EscapeEngine& engine,cfgchallengeescape::boundary::Seam seam,
 const cfgchallengeescape::seams::Arguments& args){
 using namespace cfgchallengeescape;
 cfgprobe::lifetime::ArrayHeader registry;cfgprobe::lifetime::SlotObservation actor;
 if(!cfgprobe::lifetime::ReadArrayHeader(r,plan.module+cfgprobe::lifetime::ObjectArrayRva,registry)||
    !actors::Positive(r,registry,seams::ActorAddress(seam,args),actor)){
  frontendEscapeRuntime.RecordGate(seam,seams::ActorAddress(seam,args),7,r.Remaining(),false);EscapeMetrics(r,engine);return false;
 }
 const auto installed=[&](EscapeReader& reader,const scope::Witness&){return frontendEscapeHooks.BudgetedSuppressionReady(reader);};
 seams::Evidence evidence;
 const auto validate=[&](EscapeReader& reader,const scope::Witness& w){
  const auto current=[&]{return frontendEscapeRuntime.ScopeCurrent(reader,observerOwnerThread,w);};
  if(!evidence.valid)return seams::Validate(reader,plan.module,w,seam,args,current,nullptr,&evidence);
  return seams::CurrentEvidence(reader,plan.module,w,seam,args,current,evidence);
 };
 const auto result=frontendEscapeRuntime.Gate(r,engine,observerOwnerThread,GetTickCount64(),installed,Identity(actor),seam,validate);
 EscapeMetrics(r,engine);return result.blockNative;
}
bool EscapeGate(cfgchallengeescape::boundary::Seam seam,const cfgchallengeescape::seams::Arguments& args)noexcept{
 if(GetCurrentThreadId()!=observerOwnerThread||!frontendEscapeRuntime.Armed()||!frontendEscapeHooks.Enabled())return false;
 try{
  if(frontendEscapeActiveReader&&frontendEscapeActiveEngine)return EscapeGateBorrowed(*frontendEscapeActiveReader,*frontendEscapeActiveEngine,seam,args);
  cfgchallengeescape::dispatch::WindowsSource source;EscapeReader r(source);EscapeEngine engine(r,plan.module,observerOwnerThread,true,EscapeDispatchLease{});
  EscapeCallbackScope callback(r,engine);return EscapeGateBorrowed(r,engine,seam,args);
 }catch(...){++frontendEscapeErrors;frontendEscapeRuntime.Cancel();return false;}
}
void __fastcall EscapeOverlap(void* self,void* component,void* actor,void* other,int index,bool sweep,const void* hit){
 cfgchallengeescape::seams::Arguments args;args.self=reinterpret_cast<std::uintptr_t>(self);args.component=reinterpret_cast<std::uintptr_t>(component);
 args.actor=reinterpret_cast<std::uintptr_t>(actor);args.otherComponent=reinterpret_cast<std::uintptr_t>(other);args.bodyIndex=index;args.sweep=sweep;
 if(!EscapeGate(cfgchallengeescape::boundary::Seam::Overlap,args))frontendEscapeOriginals.overlap(self,component,actor,other,index,sweep,hit);
}
void __fastcall EscapeCharacter(void* self,void* path,float delay){
 cfgchallengeescape::seams::Arguments args;args.self=reinterpret_cast<std::uintptr_t>(self);args.exitPath=reinterpret_cast<std::uintptr_t>(path);args.delay=delay;
 if(!EscapeGate(cfgchallengeescape::boundary::Seam::CharacterStart,args))frontendEscapeOriginals.character(self,path,delay);
}
void __fastcall EscapePresentation(void* self,void* actor){
 cfgchallengeescape::seams::Arguments args;args.self=reinterpret_cast<std::uintptr_t>(self);args.actor=reinterpret_cast<std::uintptr_t>(actor);
 if(!EscapeGate(cfgchallengeescape::boundary::Seam::Presentation,args))frontendEscapeOriginals.presentation(self,actor);
}
void __fastcall EscapeCompletion(void* self,void* controller,void* actor){
 cfgchallengeescape::seams::Arguments args;args.self=reinterpret_cast<std::uintptr_t>(self);args.controller=reinterpret_cast<std::uintptr_t>(controller);args.actor=reinterpret_cast<std::uintptr_t>(actor);
 if(!EscapeGate(cfgchallengeescape::boundary::Seam::Completion,args))frontendEscapeOriginals.completion(self,controller,actor);
}
void __fastcall EscapeVehicle(void* self,void* volume,void* path,float delay){
 cfgchallengeescape::seams::Arguments args;args.self=reinterpret_cast<std::uintptr_t>(self);args.volume=reinterpret_cast<std::uintptr_t>(volume);args.exitPath=reinterpret_cast<std::uintptr_t>(path);args.delay=delay;
 if(!EscapeGate(cfgchallengeescape::boundary::Seam::VehicleStart,args))frontendEscapeOriginals.vehicle(self,volume,path,delay);
}
bool PrepareChallengeEscapeHooks(std::uintptr_t module,HMODULE resident)noexcept{
 const cfgchallengeescape::hooks::Request request{module,reinterpret_cast<std::uintptr_t>(resident),
  {&EscapeOverlap,&EscapeCharacter,&EscapePresentation,&EscapeCompletion,&EscapeVehicle},true,true,true,GetCurrentThreadId()};
 return frontendEscapeHooks.Prepare(request)&&frontendEscapeHooks.PublishOriginals(frontendEscapeOriginals)&&frontendEscapeHooks.Enable();
}
bool ArmChallengeEscape(const cfgmenu::active::Context& source,bool enabled,std::uint64_t now){
#if defined(CFG_J27_ESCAPE_OPTIONAL)&&CFG_J27_ESCAPE_OPTIONAL
 if(!frontendEscapeHooks.Enabled()){frontendEscapeRuntime.Cancel();return true;} // hooks absent: no escape guard, never refuse Start
#endif
 if(frontendEscapeGeneration==UINT64_MAX||!frontendEscapeHooks.Enabled()){
#if CFG_J29_CHALLENGE_ESCAPE_HOOKS
  return cfgj29escape::AllowStart(false,frontendEscapeArmFailures,[&]{frontendEscapeRuntime.Cancel();});
#else
  return false;
#endif
 }
 const auto generation=frontendEscapeGeneration+1;
 if(!frontendEscapeRuntime.Arm(source,generation,enabled,now)){
#if CFG_J29_CHALLENGE_ESCAPE_HOOKS
  return cfgj29escape::AllowStart(false,frontendEscapeArmFailures,[&]{frontendEscapeRuntime.Cancel();});
#else
  return false;
#endif
 }
 frontendEscapeGeneration=generation;frontendEscapeNext=0;return true;
}
void FrontendEscapeAfter(bool nested,bool completed)noexcept{
 if(nested||!completed||GetCurrentThreadId()!=observerOwnerThread||frontendEscapeActiveReader||!frontendEscapeRuntime.Armed())return;
 const auto now=GetTickCount64();if(now<frontendEscapeNext)return;frontendEscapeNext=now+25;
 try{
  cfgchallengeescape::dispatch::WindowsSource source;EscapeReader r(source);EscapeEngine engine(r,plan.module,observerOwnerThread,true,EscapeDispatchLease{});
  EscapeCallbackScope callback(r,engine);
  const auto installed=[&](EscapeReader& reader,const cfgchallengeescape::scope::Witness&){return frontendEscapeHooks.BudgetedSuppressionReady(reader);};
#if CFG_J29_CHALLENGE_ESCAPE_HOOKS
  frontendEscapeRuntime.Service(r,engine,observerOwnerThread,now,installed,CFG_J29_CHALLENGE_ESCAPE_PHYSICAL!=0);
#else
  frontendEscapeRuntime.Service(r,engine,observerOwnerThread,now,installed);
#endif
  EscapeMetrics(r,engine);
 }catch(...){++frontendEscapeErrors;frontendEscapeRuntime.Cancel();}
}
