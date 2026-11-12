// Included INSIDE the observer namespace after NativeChallengeFrontend's
// Runtime storage. J32 is a narrowly scoped Challenge intro-only provider;
// never reuses the Offline Bots bundle or the HUD/menu root provider.
cfgretain::ResidentRootBank<32> frontendChallengeRootBank;
class ChallengeIntroRootProvider {
 using Request=cfgmenu::challengeintro::DispatchRequest;
 using RootBank=decltype(frontendChallengeRootBank);
 Request request_{};cfgmenu::challengeintro::Runtime& intro_;
 std::uintptr_t callback_=0,function_=0,pc_=0;std::uint32_t thread_=0;
 cfgretain::CallbackRootLease<RootBank> lease_;
 bool admitted_=false,enteredNative_=false,called_=false;
 cfgprobe::lifetime::SlotObservation retainedFunction_{};
 bool FunctionCurrent(Reader& reader)noexcept{
  if(!retainedFunction_.object)return true;
  cfgprobe::lifetime::ArrayHeader registry{};cfgprobe::lifetime::SlotObservation slot{};
  std::uint32_t internal=0,flags=0;
  return cfgprobe::lifetime::ReadArrayHeader(reader,request_.module+cfgprobe::lifetime::ObjectArrayRva,registry)&&
   cfgprobe::lifetime::ReadSlot(reader,registry,request_.function,slot)&&slot==retainedFunction_&&
   reader.Read(registry.data+24ULL*static_cast<unsigned>(slot.index)+8,internal)&&!(internal&0x30000000u)&&
   reader.Read(request_.function+8,flags)&&!(flags&0x30u)&&
   cfgobserver::Same(reader,request_.module+cfgoutro::disk::SkipCache,request_.function);
 }
 unsigned& proofStage_;
 static ChallengeIntroRootProvider& Self(void* p)noexcept{return *static_cast<ChallengeIntroRootProvider*>(p);}
 bool Context(Reader& reader)noexcept {
  if(GetCurrentThreadId()!=thread_||!cfgprimary::LookupVetoClearForObservation(reader,request_.module)||!FunctionCurrent(reader))return false;
  if(!enteredNative_)return intro_.CurrentRequest(reader,request_,callback_,thread_,proofStage_);
  // Once the native skip returns the intro phase may have transitioned. Only
  // read-only CURRENT world/owner/registry checks are permitted; no second call.
  std::uintptr_t netDriver=1,mode=0,state=0;std::uint32_t currentThread=0;cfgprobe::lifetime::ArrayHeader registry{};
  cfgprobe::lifetime::SlotObservation worldSlot{},levelSlot{},settingsSlot{};
  return reader.Read(request_.module+cfgprobe::dispatch::ThreadIdRva,currentThread)&&currentThread==thread_&&
   reader.Read(request_.world+cfgprobe::WorldGameMode,mode)&&mode==request_.mode&&
   reader.Read(request_.world+cfgprobe::WorldGameState,state)&&
   reader.Read(request_.world+cfgprobe::WorldNetDriver,netDriver)&&!netDriver&&
   cfgprobe::lifetime::ReadArrayHeader(reader,request_.module+cfgprobe::lifetime::ObjectArrayRva,registry)&&
   cfgprobe::lifetime::ReadSlot(reader,registry,request_.world,worldSlot)&&worldSlot.serial==request_.worldSerial&&
   cfgprobe::lifetime::ReadSlot(reader,registry,request_.level,levelSlot)&&levelSlot.serial==request_.levelSerial&&
   (!request_.strongChildren||levelSlot.index==request_.levelIndex)&&
   cfgprobe::lifetime::ReadSlot(reader,registry,request_.settings,settingsSlot)&&settingsSlot.serial==request_.settingsSerial&&
   (!request_.strongChildren||settingsSlot.index==request_.settingsIndex)&&
   (!request_.strongChildren||cfgmenu::challengeintro::ChildrenFlagsClear(reader,registry.data,levelSlot.index,settingsSlot.index))&&
   cfgobserver::Same(reader,request_.world+cfgoutro::disk::PersistentLevel,request_.level)&&
   cfgobserver::Same(reader,request_.level+cfgoutro::disk::WorldSettings,request_.settings)&&
   cfgobserver::Same(reader,request_.level+cfgoutro::disk::OwningWorld,request_.world)&&
   cfgobserver::Same(reader,request_.module+cfgoutro::disk::SkipCache,request_.function)&&
   cfgprimary::LookupVetoClearForObservation(reader,request_.module);
 }
 // No unassigned child can be promoted to a GC root. The world and
 // independent native inputs remain rooted for the call; its exact strong
 // children are reproved at every pre/post window instead.
 bool Callback(Reader& reader,std::uintptr_t f,std::uintptr_t pc)noexcept{
  return f==function_&&pc==pc_&&callback_&&function_&&pc_&&
   cfgtick::Observe(reader,request_.module,callback_,f,pc);
 }
 GcRegistryAccess::NativeContextProof Proof()noexcept{
  return {this,[](Reader& r,void* p){return Self(p).Context(r);},
   [](Reader& r,void* p,std::uintptr_t f,std::uintptr_t pc){return Self(p).Callback(r,f,pc);}};
 }
 template<class Work>bool Window(Work&& work)noexcept{
  Reader r;r.cap=8192;
  if(!admitted_||!lease_.Current()||!Context(r))return false;
  // Same bounded registry budget as the existing protected startup path;
  // first-owner registration/code pins repeat the full current-owner proof.
  GcRegistryAccess registry(Proof(),CFG_J214_INTRO_END_TO_END?131072:8192);bool okay=false;
  __try{if(registry.Begin(function_,pc_))okay=work(registry);}
  __finally{registry.End();}
  return okay;
 }
 bool Inputs(std::uintptr_t object,std::uintptr_t function,void* packet)noexcept{
  if(enteredNative_||called_||object!=request_.settings||function!=request_.function||!packet)return false;
  Reader r;r.cap=8192;
   return Window([&](GcRegistryAccess& access)noexcept{
    cfgprobe::lifetime::ArrayHeader registry{};cfgprobe::lifetime::SlotObservation slot{};
    if(!cfgprobe::lifetime::ReadArrayHeader(r,request_.module+cfgprobe::lifetime::ObjectArrayRva,registry))return false;
    // World/mode/callback remain positive. The exact, freshly metadata/code-
    // proved cached UFunction may have serial0: use the existing FeatureFunction
    // REGISTERED ROOT protocol, never a fabricated engine weak-identity lease.
    // Serial-0 level/settings are NOT roots; retain their positive World only.
    const auto retained=[&](const auto& values){
     for(const auto value:values){
      if(!cfgspawn::ObjectSlot(r,registry,value,slot))return false;
      if(CFG_J214_INTRO_END_TO_END&&value==request_.function){
       cfgprobe::lifetime::SlotObservation again;std::uint32_t internal=0,flags=0;
       if(!r.Read(registry.data+24ULL*static_cast<unsigned>(slot.index)+8,internal)||(internal&0x30000000u)||
          !r.Read(value+8,flags)||(flags&0x30u)||!cfgspawn::ObjectSlot(r,registry,value,again)||!(again==slot)||
          !intro_.CurrentRequest(r,request_,callback_,thread_,proofStage_))return false;
       retainedFunction_=slot;
      }else if(!slot.HasAssignedSerial())return false;
     }
     return lease_.Retain(access,values)&&lease_.PrepareCall(access,0);
    };
    if(request_.levelSerial>0&&request_.settingsSerial>0)
     return retained(std::array<std::uintptr_t,6>{{request_.world,request_.mode,request_.level,request_.settings,request_.function,callback_}});
    if(request_.levelSerial>0)
     return retained(std::array<std::uintptr_t,5>{{request_.world,request_.mode,request_.level,request_.function,callback_}});
    if(request_.settingsSerial>0)
     return retained(std::array<std::uintptr_t,5>{{request_.world,request_.mode,request_.settings,request_.function,callback_}});
    return retained(std::array<std::uintptr_t,4>{{request_.world,request_.mode,request_.function,callback_}});
   });
 }
 bool Outputs(std::uintptr_t function,void* packet)noexcept{
  if(!enteredNative_||function!=request_.function||!packet)return false;
  return Window([&](GcRegistryAccess& access)noexcept{
   const std::array<std::uintptr_t,0> noOutputs{};
   return lease_.CompleteCall(access,noOutputs,0);
  });
 }
 bool EnterNative()noexcept{
  if(enteredNative_||called_||!admitted_)return false;
  Reader r;r.cap=8192;
  if(!Context(r))return false;
  GcRegistryAccess unlocked(Proof());
  if(!lease_.EnterNative(unlocked))return false;
  called_=enteredNative_=true;return true;
 }
 void CancelPending()noexcept{
  if(lease_.EmptyIdle())return;
  if(!Window([&](GcRegistryAccess& a)noexcept{return lease_.CancelCall(a);}))lease_.Uncertain();
 }
 public:
 ChallengeIntroRootProvider(cfgmenu::challengeintro::Runtime& runtime,const Request& request,
     std::uintptr_t callback,std::uintptr_t function,std::uintptr_t pc,std::uint32_t thread,unsigned& proofStage):
     request_(request),intro_(runtime),callback_(callback),function_(function),pc_(pc),thread_(thread),
     lease_(frontendChallengeRootBank,request.module),proofStage_(proofStage){
  Reader reader;reader.cap=8192;
  if(!CFG_J32_CHALLENGE_NATIVE_SCOPE||cfgmenu::rows::NativeRetentionScope::Occupied()||
     GetCurrentThreadId()!=thread||!request.Valid()||!function||!pc||
     !intro_.CurrentRequest(reader,request,callback,thread,proofStage)||!Callback(reader,function,pc))return;
  admitted_=lease_.Current();
 }
 ~ChallengeIntroRootProvider(){
  if(admitted_&&!lease_.EmptyIdle()){
   if(!Window([&](GcRegistryAccess& a)noexcept{return lease_.Close(a);}))lease_.Uncertain();
  }
 }
 bool Admitted()const noexcept{return admitted_;}
 cfgmenu::rows::NativeRetentionHooks Hooks()noexcept{
  return {this,
   [](void* p,std::uintptr_t module){auto& s=Self(p);Reader r;r.cap=8192;return module==s.request_.module&&s.lease_.Current()&&s.Context(r);},
   [](void* p,std::uintptr_t,std::uintptr_t object,std::uintptr_t fn,void* packet){return Self(p).Inputs(object,fn,packet);},
   [](void* p,std::uintptr_t,std::uintptr_t fn,void* packet){return Self(p).Outputs(fn,packet);},
   [](void*,std::uintptr_t,std::uintptr_t,std::uintptr_t){return false;},
   [](void*,std::uintptr_t,std::uintptr_t){return false;},
   [](void* p){Self(p).lease_.Uncertain();},
   [](void* p){Self(p).CancelPending();},
   [](void* p){return Self(p).EnterNative();},
   nullptr};
 }
};
