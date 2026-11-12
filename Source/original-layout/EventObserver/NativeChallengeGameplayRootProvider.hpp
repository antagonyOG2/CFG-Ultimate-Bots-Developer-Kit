// Included inside the observer namespace. Uses the SAME named resident bank as
// intro skip; no new hidden World holder. Host supplies exact current native
// metadata, actor's strong World/Level membership and allowed call packets.
template<class Host>class ChallengeGameplayRootProvider {
 Host& host_;std::uintptr_t module_=0,actor_=0,function_=0,pc_=0;
 cfgretain::CallbackRootLease<decltype(frontendChallengeRootBank)> lease_;
 bool admitted_=false,entered_=false;unsigned admitClause_=0; // H2.78: 0 admitted,1 occupied,2 thread,3 lookup veto,4 ProveCurrent,5 tick,6 lease
 static ChallengeGameplayRootProvider& Self(void* p){return *static_cast<ChallengeGameplayRootProvider*>(p);}
 bool Current(Reader& r){return GetCurrentThreadId()==host_.Thread()&&
  cfgprimary::LookupVetoClearForObservation(r,module_)&&host_.ProveCurrent(r);}
 GcRegistryAccess::NativeContextProof Proof(){return {this,
  [](Reader& r,void* p){return Self(p).Current(r);},
  [](Reader& r,void* p,std::uintptr_t fn,std::uintptr_t pc){auto& s=Self(p);return fn==s.function_&&pc==s.pc_&&
   cfgtick::Observe(r,s.module_,s.actor_,fn,pc);}};}
 template<class Work>bool Window(Work&& work)noexcept{
  Reader r;r.cap=65536;if(!admitted_||!lease_.Current()||!Current(r))return false;
  GcRegistryAccess registry(Proof(),131072);bool okay=false;
  __try{if(registry.Begin(function_,pc_))okay=work(registry);}
  __finally{registry.End();}return okay;
 }
 bool Retain(std::uintptr_t target,std::uintptr_t function,void* packet,bool factory){
  Reader r;r.cap=65536;std::array<std::uintptr_t,8> values{};
  return Window([&](GcRegistryAccess& access)noexcept{
   if(!host_.Inputs(r,target,function,packet,factory,values))return false;
   // Native assets use the existing FeatureFunction registered-root protocol,
   // not an engine weak-serial lease. Unassigned World children are NOT roots;
   // their exact strong membership remains part of Host::Current instead.
   return lease_.Retain(access,values)&&lease_.PrepareCall(access,factory?1u:0u);
  });
 }
 bool Outputs(std::uintptr_t value,bool factory){
  Reader r;r.cap=65536;
  return Window([&](GcRegistryAccess& access)noexcept{
   if(!entered_||!host_.Outputs(r,value,factory))return false;
   if(factory){if(!lease_.CompleteCall(access,std::array<std::uintptr_t,1>{{value}}))return false;}
   else if(!lease_.CompleteCall(access,std::array<std::uintptr_t,0>{},0))return false;
   entered_=false;return true;
  });
 }
 void Cancel()noexcept{
  if(lease_.EmptyIdle())return;
  if(!Window([&](GcRegistryAccess& a)noexcept{return lease_.CancelCall(a);}))lease_.Uncertain();
 }
public:
 ChallengeGameplayRootProvider(Host& host,std::uintptr_t module,std::uintptr_t actor,std::uintptr_t function,std::uintptr_t pc):
  host_(host),module_(module),actor_(actor),function_(function),pc_(pc),lease_(frontendChallengeRootBank,module){
  Reader r;r.cap=65536;
  admitClause_=cfgmenu::rows::NativeRetentionScope::Occupied()?1u:GetCurrentThreadId()!=host_.Thread()?2u:!cfgprimary::LookupVetoClearForObservation(r,module_)?3u:!host_.ProveCurrent(r)?4u:!cfgtick::Observe(r,module,actor,function,pc)?5u:!lease_.Current()?6u:0u;admitted_=admitClause_==0;
 }
 ~ChallengeGameplayRootProvider(){if(admitted_&&!lease_.EmptyIdle()&&
   !Window([&](GcRegistryAccess& a)noexcept{return lease_.Close(a);}))lease_.Uncertain();}
 unsigned AdmitClause()const{return admitClause_;}
 bool Admitted()const{return admitted_;}
 cfgmenu::rows::NativeRetentionHooks Hooks(){return {this,
  [](void* p,std::uintptr_t module){auto& s=Self(p);Reader r;r.cap=65536;return module==s.module_&&s.lease_.Current()&&s.Current(r);},
  [](void* p,std::uintptr_t,std::uintptr_t object,std::uintptr_t fn,void* packet){return Self(p).Retain(object,fn,packet,false);},
  [](void* p,std::uintptr_t,std::uintptr_t,void*){return Self(p).Outputs(0,false);},
  [](void* p,std::uintptr_t,std::uintptr_t address,std::uintptr_t owner){return Self(p).Retain(owner,address,nullptr,true);},
  [](void* p,std::uintptr_t,std::uintptr_t value){return Self(p).Outputs(value,true);},
  [](void* p){Self(p).lease_.Uncertain();},[](void* p){Self(p).Cancel();},
  [](void* p){auto& s=Self(p);Reader r;r.cap=65536;if(s.entered_||!s.Current(r))return false;
   GcRegistryAccess unlocked(s.Proof(),131072);if(!s.lease_.EnterNative(unlocked))return false;s.entered_=true;return true;},nullptr};}
};
