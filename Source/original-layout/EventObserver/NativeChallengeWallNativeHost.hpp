// Included inside observer namespace after the retained provider.
template<class Engine=cfgmenu::rows::NativeDispatch>struct ChallengeWallNativeHostBase {
 cfgj36walls::Runtime& runtime;unsigned index=0;std::uintptr_t callback=0;bool setter=false,desired=false;unsigned why=0; /* H2.76 diagnostic: last Prepare/Set refusal step */
 Engine* engine=nullptr;
 std::uintptr_t Module()const{return runtime.Source().module;}DWORD Thread()const{return observerOwnerThread;}
 template<class R>bool ProveCurrent(R& r){return runtime.Current(r,Thread())&&runtime.ProveMember(r,index)
#if CFG_J215_CHALLENGE_WALL_OWNERSHIP
  &&(!setter||runtime.GeometryCurrent(r,index))
#endif
  ;}
 template<class R>bool Function(R& r,std::uintptr_t function){
  namespace d=cfgj36walls::disk;const auto module=Module();std::uintptr_t owner=0,ctor=0;
  const auto cache=setter?d::SetCache:cfgcabin::disk::GetActorBounds.cache;
  const auto executor=setter?d::SetExecutor:cfgcabin::disk::Executor;
  if(!ProveCurrent(r)||!r.Read(module+d::ActorReflection,owner)||!r.Read(module+d::ActorConstructor,ctor)||
     !cfgprobe::MatchingClassCache(owner,ctor)||!cfgmenu::offlineowner::Same(r,module+cache,function))return false;
  cfgreflection::Observation<R> reflection(r,module);cfgreflection::Field field;
  std::uint32_t flags=0;std::uint8_t parms=0;std::uint16_t bytes=0;
  if(!reflection.ReadField(function,owner,field)||field.kind.View()!=u"Function"||
     field.label.View()!=(setter?u"SetActorEnableCollision":u"GetActorBounds")||
     !r.Read(function+0x88,flags)||!(flags&0x400u)||!r.Read(function+0x8C,parms)||parms!=(setter?1:3)||
     !r.Read(function+0x8E,bytes)||bytes!=(setter?1:28)||!cfgmenu::offlineowner::Same(r,function+0xB0,module+executor))return false;
  cfgprobe::lifetime::SlotObservation slot;
  if(!cfgj36walls::SlotClear(r,runtime.Witness().registry,owner,slot)||!cfgj36walls::SlotClear(r,runtime.Witness().registry,function,slot))return false;
  if(setter){
   std::uintptr_t child=0;cfgreflection::Field argument;cfgreflection::Property property;
   if(!r.Read(function+cfgreflection::disk::Children,child)||!reflection.ReadField(child,function,argument)||
      argument.label.View()!=d::SetParameterName||argument.kind.View()!=u"BoolProperty"||
      !cfgmenu::capture::RawProperty(r,argument,property)||property.offset!=0||property.dimension!=1||property.elementBytes!=1||
      !(property.flags&0x80u)||(property.flags&0x500u)||argument.next)return false;
   return cfgj36walls::Pin(r,module,d::SetFactory,d::SetFactoryCode)&&cfgj36walls::Pin(r,module,d::SetExecutor,d::SetExecutorCode)&&
    cfgj36walls::Pin(r,module,d::Native,d::NativeCode);
  }
  return cfgj36walls::Pin(r,module,cfgcabin::disk::GetActorBounds.factory,cfgcabin::disk::GetActorBounds.code)&&
   cfgj36walls::Pin(r,module,cfgcabin::disk::Executor,cfgcabin::disk::ExecutorCode)&&cfgj36walls::Pin(r,module,cfgcabin::disk::Native,cfgcabin::disk::NativeCode);
 }
 bool Inputs(Reader& r,std::uintptr_t target,std::uintptr_t function,void* packet,bool factory,std::array<std::uintptr_t,8>& values){
  namespace d=cfgj36walls::disk;std::uintptr_t owner=0,ctor=0;auto& w=runtime.Witness();const auto& actor=runtime.Actor(index);cfgprobe::lifetime::SlotObservation level;
  if(!ProveCurrent(r)||!r.Read(Module()+d::ActorReflection,owner)||!r.Read(Module()+d::ActorConstructor,ctor)||!cfgprobe::MatchingClassCache(owner,ctor))return false;
  values={{w.World(),w.Mode(),w.State(),callback,owner,0,actor.slot.serial>0?actor.actor:0,0}};
  if(!cfgj36walls::SlotClear(r,w.registry,actor.level,level))return false;if(level.serial>0)values[7]=actor.level;
  if(factory){const auto address=setter?d::SetFactory:cfgcabin::disk::GetActorBounds.factory;
   return target==owner&&function==Module()+address&&
    (setter?cfgj36walls::Pin(r,Module(),address,d::SetFactoryCode):cfgj36walls::Pin(r,Module(),address,cfgcabin::disk::GetActorBounds.code));}
  if(target!=actor.actor||!packet||!Function(r,function))return false;
  if(setter){std::uint8_t collision=0;
   if(*static_cast<std::uint8_t*>(packet)!=(desired?1:0)||!r.Read(actor.actor+d::CollisionReadbackOffset,collision)||
      ((collision&d::CollisionReadbackMask)!=0)==desired)return false;}
  values[5]=function;return true;
 }
 bool Outputs(Reader& r,std::uintptr_t value,bool factory){return ProveCurrent(r)&&(!factory||Function(r,value));}
 bool Prepare(std::uintptr_t& function){
  Reader r;r.cap=65536;namespace d=cfgj36walls::disk;std::uintptr_t owner=0;
  const auto factory=setter?d::SetFactory:cfgcabin::disk::GetActorBounds.factory,cache=setter?d::SetCache:cfgcabin::disk::GetActorBounds.cache;
  if(!ProveCurrent(r)||!r.Read(Module()+cache,function)){why=1;return false;}
  if(!function){
   why=2;if(!r.Read(Module()+d::ActorReflection,owner)||!engine||!engine->BeginPhase()){why=3;return false;}
   (void)engine->FactoryOwned(Module()+factory,owner,function);return false; // registration then fresh callback
  }const bool proved=Function(r,function);if(!proved)why=4;return proved;
 }
 bool ReadBounds(cfgj36walls::Bounds& output){
  setter=false;std::uintptr_t function=0;cfgcabin::Packet packet{};
  if(!Prepare(function)||!engine->BeginPhase()||!engine->Invoke(runtime.Actor(index).actor,function,&packet))return false;
  output={{packet.origin.x,packet.origin.y,packet.origin.z},{packet.extent.x,packet.extent.y,packet.extent.z}};return cfgj36walls::Finite(output);
 }
 bool WorldCurrent(){Reader r;r.cap=4096;return runtime.WorldCurrent(r);}
 bool Current(const cfgj36walls::Record& record){Reader r;r.cap=65536;return record.actor==runtime.Actor(index).actor&&ProveCurrent(r);}
 bool ReadCollision(const cfgj36walls::Record&,bool& result){
  Reader r;r.cap=65536;std::uint8_t bits=0;namespace d=cfgj36walls::disk;
  if(!ProveCurrent(r)||!cfgj36walls::Pin(r,Module(),d::CollisionPropertySetter,d::CollisionPropertySetterCode)||
     !r.Read(runtime.Actor(index).actor+d::CollisionReadbackOffset,bits)||!ProveCurrent(r))return false;
  result=(bits&d::CollisionReadbackMask)!=0;return true;
 }
 cfgj36walls::Write Set(const cfgj36walls::Record&,bool value){
  setter=true;desired=value;std::uintptr_t function=0;
  if(!Prepare(function))return frontendChallengeRootBank.Healthy()?cfgj36walls::Write::NotEntered:cfgj36walls::Write::Uncertain;
  if(!engine->BeginPhase()){why=5;return cfgj36walls::Write::NotEntered;}
  std::uint8_t packet=value?1u:0u;const bool returned=engine->Invoke(runtime.Actor(index).actor,function,&packet);
  if(returned)return cfgj36walls::Write::Returned;
  why=6;return engine->Calls()||!frontendChallengeRootBank.Healthy()?cfgj36walls::Write::Uncertain:cfgj36walls::Write::NotEntered;
 }
};
using ChallengeWallNativeHost=ChallengeWallNativeHostBase<>;
