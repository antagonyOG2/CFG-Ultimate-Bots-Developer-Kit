#pragma once
#include "../RuntimeProbe/MenuScriptObservation.hpp"
#include "../RuntimeProbe/ChallengeEscapeRecovery.generated.hpp"
#include <algorithm>
#include <cstddef>

namespace cfgchallengeescape::resources {
enum class Api : unsigned {
 ActorLocation,ActorRotation,OverlappingActor,Teleport,StopController,
 StopMovement,ComponentLocation,SphereRadius,LinearVelocity,AngularVelocity,
 Throttle,Brake
};
enum class Result {Ready,Deferred,Refused};
enum class Failure {None,Scope,Owner,Cache,Code,Function,Parameter,Changed,Packet,Target,Call};
// Create exactly once at entry to an eligible callback, and share across ALL
// resource requests in that callback. A registration spends that whole callback;
// the caller repeats its ownership proof on the next callback before any action.
// This helper never opens/reset phases or changes NativeDispatch4's call budget.
struct CallbackBudget {bool factorySpent=false,blocked=false;};
struct Diagnostic {Failure failure=Failure::None;unsigned parameter=0,factories=0,factoryReturns=0;};
struct Vector {float x=0,y=0,z=0;};
struct Rotation {float pitch=0,yaw=0,roll=0;};
struct OverlapPacket {std::uintptr_t other=0;std::uint8_t returned=0;};
struct TeleportPacket {Vector location{};Rotation rotation{};std::uint8_t returned=0;};
struct VelocityPacket {Vector velocity{};std::uint8_t add=0;std::uint8_t padding[3]{};cfgnames::Words bone{};};
static_assert(sizeof(Vector)==12&&sizeof(Rotation)==12);
static_assert(sizeof(OverlapPacket)==16&&offsetof(OverlapPacket,returned)==8);
static_assert(sizeof(TeleportPacket)==28&&offsetof(TeleportPacket,rotation)==12&&offsetof(TeleportPacket,returned)==24);
static_assert(sizeof(VelocityPacket)==24&&offsetof(VelocityPacket,add)==12&&offsetof(VelocityPacket,bone)==16);
// Empty native functions still receive owned non-null dispatcher storage.
template<Api A>struct Packet {static constexpr unsigned index=static_cast<unsigned>(A);
 static_assert(index<disk::Apis.size());
 alignas(8) std::array<unsigned char,disk::Apis[index].storage?disk::Apis[index].storage:16> bytes{};
};
struct Evidence {
 std::uintptr_t owner=0,function=0,children=0;
 cfgreflection::Field field{};
 std::array<cfgreflection::Property,3> parameters{};
 std::array<std::array<unsigned char,4>,3> boolLayouts{};
};
inline bool SameField(const cfgreflection::Field& a,const cfgreflection::Field& b){
 return a.object==b.object&&a.owner==b.owner&&a.type==b.type&&a.next==b.next&&a.name==b.name&&
  a.label.View()==b.label.View()&&a.kind.View()==b.kind.View();
}
inline bool Same(const Evidence& a,const Evidence& b,unsigned count){
 if(a.owner!=b.owner||a.function!=b.function||a.children!=b.children||!SameField(a.field,b.field))return false;
 for(unsigned i=0;i<count;++i){const auto& x=a.parameters[i];const auto& y=b.parameters[i];
  if(!SameField(x.field,y.field)||x.dimension!=y.dimension||x.elementBytes!=y.elementBytes||x.offset!=y.offset||
     x.flags!=y.flags||a.boolLayouts[i]!=b.boolLayouts[i])return false;
 }
 return true;
}
template<class R>bool Owner(R& r,std::uintptr_t module,const disk::ApiBinding& b,std::uintptr_t& owner){
 std::uintptr_t ctor=0;owner=0;
 return r.Read(module+b.owner,owner)&&r.Read(module+b.ctor,ctor)&&cfgprobe::MatchingClassCache(owner,ctor);
}
template<class R>bool Code(R& r,std::uintptr_t module,const disk::ApiBinding& b){
 // Every generated factory is exactly 47 bytes, every thunk at most 326 bytes.
 std::array<unsigned char,47> factory{};std::array<unsigned char,326> executor{};
 return b.codeBytes&&b.codeBytes<=executor.size()&&
  r.ReadBytes(module+b.factory,factory.data(),factory.size())&&std::equal(factory.begin(),factory.end(),b.factoryCode)&&
  r.ReadBytes(module+b.executor,executor.data(),b.codeBytes)&&std::equal(executor.begin(),executor.begin()+b.codeBytes,b.code);
}
// Reads the whole owned parameter chain, rejecting locals, duplicates, extras,
// cycles and partial frames. Match by name; UHT declaration order is not a
// runtime linked-list guarantee. Only Parm/Return role bits come from UHT.
// Native construction adds other CPF bits: sample them, then demand stability.
template<class R>bool Observe(R& r,std::uintptr_t module,const disk::ApiBinding& b,Evidence& out,Diagnostic& d){
 out={};Evidence e;d.failure=Failure::Owner;
 if(!Owner(r,module,b,e.owner))return false;
 d.failure=Failure::Cache;if(!r.Read(module+b.cache,e.function)||!cfgprobe::ObjectPointer(e.function))return false;
 d.failure=Failure::Code;if(!Code(r,module,b))return false;
 cfgreflection::Observation<R> reflection(r,module);std::uint32_t flags=0;std::uint8_t count=0;
 std::uint16_t bytes=0;int storage=0;std::uintptr_t cleanup=0,executor=0;cfgmenu::capture::ScriptHeader script{};
 d.failure=Failure::Function;
 if(!reflection.ReadField(e.function,e.owner,e.field)||e.field.kind.View()!=u"Function"||e.field.label.View()!=b.name||
    !r.Read(e.function+0x88,flags)||flags!=b.flags||!(flags&0x400)||
    !r.Read(e.function+0x8C,count)||count!=b.count||!r.Read(e.function+0x8E,bytes)||bytes!=b.logical||
    !r.Read(e.function+cfgreflection::disk::StorageBytes,storage)||storage!=static_cast<int>(b.storage)||
    !r.Read(e.function+0x68,cleanup)||cleanup||!r.Read(e.function+cfgmenu::scriptdisk::Script,script)||
    script.data||script.count||script.capacity||!r.Read(e.function+0xB0,executor)||executor!=module+b.executor||
    !r.Read(e.function+cfgreflection::disk::Children,e.children)||b.count>e.parameters.size())return false;
 std::uintptr_t next=e.children;std::array<bool,3> seen{};
 for(unsigned i=0;i<b.count;++i){d.failure=Failure::Parameter;d.parameter=i;
  cfgreflection::Field f;cfgreflection::Property p;
  if(!reflection.ReadField(next,e.function,f)||!cfgmenu::capture::RawProperty(r,f,p))return false;
  unsigned match=b.count;for(unsigned j=0;j<b.count;++j)if(f.label.View()==b.parameters[j].name){match=j;break;}
  if(match==b.count||seen[match])return false;const auto& expected=b.parameters[match];
  constexpr std::uint64_t roles=0x80ULL|0x400ULL;
  if(f.kind.View()!=expected.kind||p.dimension!=1||p.elementBytes!=static_cast<int>(expected.bytes)||
     p.offset!=static_cast<int>(expected.offset)||(p.flags&roles)!=(expected.flags&roles)||
     !(p.flags&0x80)||((p.flags&0x400)!=0)!=(expected.name==u"ReturnValue")||
     expected.offset>b.logical||expected.bytes>b.logical-expected.offset)return false;
  if(expected.kind==u"BoolProperty"){
   // Native byte Boolean: never marshal a packed member Boolean as a byte.
   if(!r.Read(f.object+0x70,e.boolLayouts[match])||e.boolLayouts[match]!=std::array<unsigned char,4>{{1,0,1,0xFF}})return false;
  }
  e.parameters[match]=p;seen[match]=true;next=f.next;
 }
 if(next){d.failure=Failure::Parameter;return false;}
 std::uintptr_t owner=0,function=0;d.failure=Failure::Owner;
 if(!Owner(r,module,b,owner)||owner!=e.owner)return false;
 d.failure=Failure::Changed;if(!r.Read(module+b.cache,function)||function!=e.function)return false;
 out=e;d.failure=Failure::None;return true;
}
// No class-constructor calls: a cold/mismatched owner defers registration until
// stock native class construction has supplied the matching two cache words.
template<class R,class Engine,class Current>Result Ensure(R& r,Engine& engine,std::uintptr_t module,Api api,
 CallbackBudget& budget,Current&& current,Evidence& out,Diagnostic& d){
 out={};const auto index=static_cast<unsigned>(api);
 if(index>=disk::Apis.size()||budget.blocked||!current()){budget.blocked=true;d.failure=Failure::Scope;return Result::Refused;}
 const auto& b=disk::Apis[index];std::uintptr_t owner=0,function=0;
 if(!Owner(r,module,b,owner)){budget.blocked=true;d.failure=Failure::Owner;return Result::Deferred;}
 if(!r.Read(module+b.cache,function)){d.failure=Failure::Cache;return Result::Refused;}
 if(budget.factorySpent)return Result::Deferred;
 if(!function){
  d.failure=Failure::Code;if(!Code(r,module,b))return Result::Refused;
  std::uintptr_t fresh=0;
  if(!Owner(r,module,b,fresh)||fresh!=owner){budget.blocked=true;d.failure=Failure::Owner;return Result::Deferred;}
  if(!current()){budget.blocked=true;d.failure=Failure::Scope;return Result::Refused;}
  if(!Owner(r,module,b,fresh)||fresh!=owner){budget.blocked=true;d.failure=Failure::Owner;return Result::Deferred;}
  if(!r.Read(module+b.cache,fresh)||fresh){d.failure=Failure::Changed;return Result::Deferred;}
  budget.factorySpent=true;++d.factories;
  const bool returned=engine.Factory(module+b.factory,function);
  if(returned&&cfgprobe::ObjectPointer(function)&&r.Read(module+b.cache,fresh)&&fresh==function)++d.factoryReturns;
  // Even successful registration is only preparation. Never act here.
  d.failure=Failure::None;return Result::Deferred;
 }
 Evidence first,second;
 if(!Observe(r,module,b,first,d)){if(d.failure==Failure::Owner)budget.blocked=true;return Result::Refused;}
 if(!current()){budget.blocked=true;d.failure=Failure::Scope;return Result::Refused;}
 if(!Observe(r,module,b,second,d)){if(d.failure==Failure::Owner)budget.blocked=true;return Result::Refused;}
 if(!Same(first,second,b.count)){d.failure=Failure::Changed;return Result::Refused;}
 out=second;return Result::Ready;
}
// Re-observe at dispatch, including all CPF words, property identities and
// links. A prior Ensure result is not a durable lease or gameplay authority.
template<class R,class Engine,class Current>bool Invoke(R& r,Engine& engine,std::uintptr_t module,Api api,
 CallbackBudget& budget,Current&& current,std::uintptr_t target,const Evidence& expected,void* packet,
 unsigned packetBytes,Diagnostic& d){
 const auto index=static_cast<unsigned>(api);d.failure=Failure::Packet;
 if(index>=disk::Apis.size()||!packet||packetBytes<disk::Apis[index].storage)return false;
 if(budget.blocked||budget.factorySpent||!current()){budget.blocked=true;d.failure=Failure::Scope;return false;}
 const auto& b=disk::Apis[index];Evidence fresh;
 if(!Observe(r,module,b,fresh,d)){if(d.failure==Failure::Owner)budget.blocked=true;return false;}
 if(!Same(expected,fresh,b.count)){d.failure=Failure::Changed;return false;}
 cfgprobe::Ancestry ancestry;d.failure=Failure::Target;
 if(!cfgprobe::CaptureAncestry(r,target,ancestry)||!ancestry.Contains(fresh.owner)||
    !cfgprobe::StableAncestry(r,target,ancestry))return false;
 if(!current()){budget.blocked=true;d.failure=Failure::Scope;return false;}
 std::uintptr_t owner=0,function=0;
 if(!Owner(r,module,b,owner)||owner!=fresh.owner){budget.blocked=true;d.failure=Failure::Owner;return false;}
 if(!r.Read(module+b.cache,function)||function!=fresh.function||!cfgprobe::StableAncestry(r,target,ancestry)){d.failure=Failure::Changed;return false;}
 d.failure=Failure::Call;if(!engine.Invoke(target,fresh.function,packet))return false;
 d.failure=Failure::None;return true;
}
}
