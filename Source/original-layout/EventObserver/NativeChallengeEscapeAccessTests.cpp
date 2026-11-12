#define CFG_ESCAPE_ACTORS_FIXTURE_ONLY
#include "NativeChallengeEscapeActorsTests.cpp"
#undef CFG_ESCAPE_ACTORS_FIXTURE_ONLY
#include "NativeChallengeEscapeAccess.hpp"
#include <vector>
#include <functional>
namespace ce=cfgchallengeescape;
namespace b=ce::boundary;
namespace rs=ce::resources;
struct AccessFixture:ActorFixture {
 static constexpr std::uintptr_t Functions=0x2800000,FunctionKind=0x2900000,Parameters=0x2A00000,
  SceneType=0x2390000,BaseControllerType=0x23A0000;
 AccessFixture(){
  r.Set(Module+ce::geometrydisk::LocationBody,ce::geometrydisk::LocationCode);
  r.Set(Module+ce::geometrydisk::AbsMask,std::array<std::uint32_t,4>{{0x7FFFFFFF,0x7FFFFFFF,0x7FFFFFFF,0x7FFFFFFF}});
  Type(SceneType,d::SceneComponent,d::SceneComponentCtor);r.Set(PrimitiveType+cfgprobe::ClassSuper,SceneType);
  Type(BaseControllerType,d::Controller,d::ControllerCtor,ActorType);r.Set(AiType+cfgprobe::ClassSuper,BaseControllerType);
  Label(FunctionKind,"Function");
  for(unsigned index=0;index<d::Apis.size();++index){const auto& api=d::Apis[index];const auto fn=Functions+0x1000*index;
   std::uintptr_t owner=0;r.Read(Module+api.owner,owner);std::string name;for(const auto ch:api.name)name.push_back(static_cast<char>(ch));
   Label(fn,name.c_str());r.Set(fn+cfgprobe::ObjectClass,FunctionKind);r.Set(fn+0x20,owner);r.Set(fn+cfgreflection::disk::Next,std::uintptr_t{});
   r.Set(Module+api.cache,fn);r.Set(fn+0x88,api.flags);r.Set(fn+0x8C,static_cast<std::uint8_t>(api.count));
   r.Set(fn+0x8E,static_cast<std::uint16_t>(api.logical));r.Set(fn+cfgreflection::disk::StorageBytes,static_cast<int>(api.storage));
   r.Set(fn+0x68,std::uintptr_t{});r.Set(fn+cfgmenu::scriptdisk::Script,cfgmenu::capture::ScriptHeader{});r.Set(fn+0xB0,Module+api.executor);
   const auto head=Parameters+0x10000*index;r.Set(fn+cfgreflection::disk::Children,api.count?head:0);
   for(unsigned j=0;j<api.count;++j){const auto& p=api.parameters[j];const auto property=head+0x1000*j,kind=property+0x800;
    name.clear();for(const auto ch:p.name)name.push_back(static_cast<char>(ch));Label(property,name.c_str());
    name.clear();for(const auto ch:p.kind)name.push_back(static_cast<char>(ch));Label(kind,name.c_str());
    r.Set(property+cfgprobe::ObjectClass,kind);r.Set(property+0x20,fn);r.Set(property+cfgreflection::disk::Next,j+1<api.count?property+0x1000:0);
    r.Set(property+cfgreflection::disk::PropertyDimensions,int{1});r.Set(property+cfgreflection::disk::PropertyElementSize,static_cast<int>(p.bytes));
    r.Set(property+cfgreflection::disk::PropertyOffset,static_cast<int>(p.offset));r.Set(property+cfgreflection::disk::PropertyFlags,(p.flags&0x480)|0x0018001040000200ULL);
    r.Set(property+0x70,std::array<unsigned char,4>{{1,0,1,0xFF}});
   }
   for(unsigned j=0;j<47;++j)r.Set(Module+api.factory+j,api.factoryCode[j]);
   for(unsigned j=0;j<api.codeBytes;++j)r.Set(Module+api.executor+j,api.code[j]);
  }
  r.Set(Names+cfgnames::disk::ElementCount,std::array<int,2>{{static_cast<int>(id),1}});
 }
 void ThreeVolumes(){
  Object(0x20F1000,VolumeType,Level);Object(0x20F2000,SphereType,0x20F1000);r.Set(std::uintptr_t{0x20F1000}+0x368,std::uintptr_t{0x20F2000});
  Object(0x20F3000,VolumeType,Level);Object(0x20F4000,SphereType,0x20F3000);r.Set(std::uintptr_t{0x20F3000}+0x368,std::uintptr_t{0x20F4000});
  r.Set(ActorArray+8*7,std::uintptr_t{0x20F1000});r.Set(ActorArray+8*8,std::uintptr_t{0x20F3000});
  r.Set(Level+cfgradarworld::disk::LevelActors,act::List{ActorArray,9,9});
  r.Set(Module+cfgprobe::lifetime::ObjectArrayRva+28,static_cast<int>(nextSlot));
 }
 void FourOccupants(){
  CounselorObject(0x2100000,0x2101000,0x2102000,0x2103000);
  CounselorObject(0x2104000,0x2105000,0x2106000,0x2107000);
  Object(0x2108000,SeatType,Car);Object(0x2109000,SeatType,Car);
  r.Set(std::uintptr_t{0x2108000}+0x548,Car);r.Set(std::uintptr_t{0x2109000}+0x548,Car);
  r.Set(std::uintptr_t{0x2108000}+0x500,std::uintptr_t{0x2100000});r.Set(std::uintptr_t{0x2109000}+0x500,std::uintptr_t{0x2104000});
  r.Set(std::uintptr_t{0x2108000}+0x511,std::uint8_t{});r.Set(std::uintptr_t{0x2109000}+0x511,std::uint8_t{});
  r.Set(std::uintptr_t{0x2100000}+0x15C0,std::uintptr_t{0x2108000});r.Set(std::uintptr_t{0x2104000}+0x15C0,std::uintptr_t{0x2109000});
  r.Set(SeatArray+16,std::uintptr_t{0x2108000});r.Set(SeatArray+24,std::uintptr_t{0x2109000});r.Set(Car+0x4E8,act::List{SeatArray,4,4});
  const std::array<std::uintptr_t,4> extra{{0x2100000,0x2101000,0x2104000,0x2105000}};
  r.Set(ActorArray+8*9,extra);r.Set(Level+cfgradarworld::disk::LevelActors,act::List{ActorArray,13,13});
  r.Set(Module+cfgprobe::lifetime::ObjectArrayRva+28,static_cast<int>(nextSlot));
 }
 void ExtraCounselors(unsigned count){
  unsigned actors=7;
  for(unsigned i=0;i<count;++i){const std::uintptr_t base=0x3100000+0x10000*i;
   CounselorObject(base,base+0x1000,base+0x2000,base+0x3000);
   r.Set(ActorArray+8ULL*actors++,base);r.Set(ActorArray+8ULL*actors++,base+0x1000);
  }
  r.Set(Level+cfgradarworld::disk::LevelActors,act::List{ActorArray,static_cast<int>(actors),static_cast<int>(actors)});
  r.Set(Module+cfgprobe::lifetime::ObjectArrayRva+28,static_cast<int>(nextSlot));
 }
};
struct AccessEngine {
 ChallengeReader& r;unsigned calls=0,factories=0,limit=64,failApi=99;std::vector<unsigned> actions;
 unsigned preflights=0,reserveCalls=0,reserveReads=0;bool reserveAllowed=true;float sphereRadius=5;
 std::function<void(unsigned)> nested;
 std::map<std::uintptr_t,rs::Vector> positions;
 bool Factory(std::uintptr_t,std::uintptr_t&){++factories;return false;}
 bool Preflight(unsigned requiredCalls,unsigned requiredReads){++preflights;reserveCalls=requiredCalls;reserveReads=requiredReads;
  return reserveAllowed&&calls<=limit&&requiredCalls<=limit-calls&&r.count<=r.cap&&requiredReads<=r.cap-r.count;}
 bool Invoke(std::uintptr_t target,std::uintptr_t function,void* data){
  if(calls>=limit||!data)return false;++calls;
  const auto index=static_cast<unsigned>((function-AccessFixture::Functions)/0x1000);
  if(index>=d::Apis.size()||index==failApi)return false;
  if(nested){auto callback=std::move(nested);nested={};callback(index);}
  switch(static_cast<rs::Api>(index)){
  case rs::Api::ActorLocation:case rs::Api::ComponentLocation:*static_cast<rs::Vector*>(data)=positions[target];break;
  case rs::Api::ActorRotation:*static_cast<rs::Rotation*>(data)={};break;
  case rs::Api::SphereRadius:*static_cast<float*>(data)=sphereRadius;break;
  case rs::Api::OverlappingActor:{auto& packet=*static_cast<rs::OverlapPacket*>(data);const auto point=positions[packet.other],center=positions[AccessFixture::Sphere];
   const auto actualCenter=positions[target];
   const double x=point.x-actualCenter.x,y=point.y-actualCenter.y,z=point.z-actualCenter.z;
   (void)center;packet.returned=x*x+y*y+z*z<=25;break;}
  case rs::Api::Teleport:{auto& packet=*static_cast<rs::TeleportPacket*>(data);positions[target]=packet.location;packet.returned=1;actions.push_back(index);break;}
  default:actions.push_back(index);break;
  }
  return true;
 }
};
using OwnedAccess=ce::Access<ChallengeReader,AccessEngine,std::function<bool()>>;
struct AccessHarness {
 AccessFixture f;ChallengeReader r;scope::Runtime authority;scope::Witness w;AccessEngine engine{r};
 std::function<bool()> lease=[](){return true;};ce::Cache cache;b::Policy policy;std::array<rs::Evidence,12> apis{};
 std::unique_ptr<OwnedAccess> access;bool ready=false,car=false;b::Identity id{},volume{};
 explicit AccessHarness(bool vehicle=false,unsigned extra=0):car(vehicle){
  if(car)f.Occupied();if(extra)f.ExtraCounselors(extra);static_cast<StageReader&>(r)=f.r;r.cap=524288;
  authority.Arm(f.Menu(),1,true,0);if(!authority.Observe(r,42,100,w))return;
  policy.Configure(ce::BoundaryScope(w));access=std::make_unique<OwnedAccess>(r,engine,authority,w,f.Module,42,lease,cache,policy,apis);
  if(!access->Warm())return;
  cfgprobe::lifetime::SlotObservation slot;cfgprobe::lifetime::ReadSlot(r,w.registry,car?f.Car:f.Npc,slot);id=ce::Identity(slot);
  cfgprobe::lifetime::ReadSlot(r,w.registry,f.Volume,slot);volume=ce::Identity(slot);
  engine.positions[f.Volume]=engine.positions[f.Sphere]={50,0,0};r.Set(f.Sphere+0x190,std::array<float,4>{{50,0,0,0}});
  policy.SetVolumes(&volume,1,true);ready=true;r.count=0;
 }
 bool Learn(){if(!(car?policy.TrackCar(*access,id):policy.TrackPawn(*access,id)))return false;
  policy.PumpOne(*access,id,1000);const auto* record=car?policy.Car(id):policy.Pawn(id);
  r.count=0;engine.calls=0;engine.actions.clear();return record&&record->hasSafePoint;
 }
};
int main(){unsigned checks=0,failures=0,peak=0;
 const auto check=[&](bool ok,const char* label){++checks;if(!ok){++failures;std::printf("FAIL %s\n",label);}};
 for(unsigned change=0;change<11;++change){AccessHarness h;check(h.ready,"geometry dependency fixture");
  b::Sphere initial,again;check(h.access->Geometry(ce::BoundaryScope(h.w),h.volume,initial)&&initial.center.x==50,
   "native getter results establish geometry snapshot");
  const auto calls=h.engine.calls;
  check(h.access->Geometry(ce::BoundaryScope(h.w),h.volume,again)&&again.center.x==50&&h.engine.calls==calls,
   "unchanged getter inputs reuse numeric results without native calls");
  if(change==0)h.engine.positions[h.f.Sphere]={100,0,0};
  if(change==0)h.r.Set(h.f.Sphere+0x190,100.0f);
  if(change==1)h.r.Set(h.f.Sphere+0x1A0,2.0f);
  if(change==2){h.r.Set(h.f.Sphere+0x6F8,7.0f);h.engine.sphereRadius=7;}
  if(change==3)h.r.Set(h.f.Module+ce::geometrydisk::AbsMask,std::uint32_t{0x3FFFFFFF});
  if(change==4)h.r.Set(h.f.Module+ce::geometrydisk::LocationBody,std::uint8_t{0xCC});
  if(change==5)h.r.Set(h.f.Sphere+0x20,h.f.Car);
  if(change==6){int index=0;h.r.Read(h.f.Sphere+cfgprobe::lifetime::ObjectIndexOffset,index);
   h.r.Set(h.f.Registry+24ULL*static_cast<unsigned>(index)+16,int{99999});}
  if(change==7)h.r.Set(h.f.Module+d::Apis[6].executor,std::uint8_t{0xCC});
  if(change==8)h.r.bytes.erase(h.f.Sphere+0x6F8+1);
  if(change==9)h.r.Set(h.f.Sphere+0x6F8,(std::numeric_limits<float>::quiet_NaN)());
  if(change==10)h.r.Set(h.f.PrimitiveType+cfgprobe::ClassSuper,std::uintptr_t{});
  const bool okay=h.access->Geometry(ce::BoundaryScope(h.w),h.volume,again);
  const bool recapture=change<4||change==6;
  check(recapture?(okay&&again.center.x==(change==0?100:50)&&again.radius==(change==2?7:5)&&h.engine.calls==calls+2):(!okay&&h.engine.calls==calls),
   "changed position/scale/radius/mask/identity recaptures; changed native code or owner refuses stale geometry");
 }
 {AccessHarness h(true);OwnedAccess admission(h.r,h.engine,h.authority,h.w,h.f.Module,42,h.lease,h.cache,h.policy,h.apis,true);
  b::Snapshot snapshot;check(admission.Current(ce::BoundaryScope(h.w),h.id,snapshot)&&
   !admission.PreflightRecovery(ce::BoundaryScope(h.w),snapshot,b::Policy::CarActionLimit,1)&&h.engine.actions.empty(),
   "read-only admission adapter cannot reserve or start physical recovery");
 }
 {AccessHarness h;check(h.ready&&h.Learn(),"geometry change during recovery fixture");
  h.engine.positions[h.id.address]={50,0,0};
  h.engine.nested=[&](unsigned){h.engine.positions[h.f.Sphere]=h.engine.positions[h.f.Volume]={0,0,0};
   h.r.Set(h.f.Sphere+0x190,std::array<float,4>{{0,0,0,0}});};
  const auto result=h.policy.Gate(*h.access,h.id,b::Seam::Overlap,1250);
  check(result.blockNative&&!result.recovered&&!h.engine.actions.empty(),
   "a volume moved by native reentry invalidates cached geometry and cannot report successful recovery");
 }
 {AccessHarness h;b::Sphere sphere;
  h.engine.nested=[&](unsigned){h.r.Set(h.f.Sphere+0x1A0,2.0f);};
  check(!h.access->Geometry(ce::BoundaryScope(h.w),h.volume,sphere)&&!h.cache.geometries[0],
   "input changes during native geometry queries cannot publish a numeric snapshot");
  check(h.access->Geometry(ce::BoundaryScope(h.w),h.volume,sphere),"later stable native inputs can establish a new snapshot");
  const auto calls=h.engine.calls;h.authority.Retire();
  check(!h.access->Geometry(ce::BoundaryScope(h.w),h.volume,sphere)&&h.engine.calls==calls,
   "numeric cache cannot outlive current Challenges authority");
 }
 for(unsigned car=0;car<4;++car){AccessFixture f;if(car)f.Occupied();if(car>=2)f.ThreeVolumes();if(car==3)f.FourOccupants();ChallengeReader r;static_cast<StageReader&>(r)=f.r;r.cap=524288;
  scope::Runtime authority;scope::Witness w;authority.Arm(f.Menu(),1,true,0);check(authority.Observe(r,42,100,w),"real exact submitted scope binds");
  AccessEngine engine{r};engine.positions[f.Sphere]=engine.positions[f.Volume]={50,0,0};
  engine.positions[0x20F2000]=engine.positions[0x20F1000]={100,0,0};engine.positions[0x20F4000]=engine.positions[0x20F3000]={150,0,0};
  r.Set(f.Sphere+0x190,std::array<float,4>{{50,0,0,0}});
  if(car>=2){r.Set(std::uintptr_t{0x20F2000}+0x190,std::array<float,4>{{100,0,0,0}});r.Set(std::uintptr_t{0x20F4000}+0x190,std::array<float,4>{{150,0,0,0}});}
  const auto lease=[](){return true;};auto leaseCheck=lease;
  ce::Cache cache;b::Policy policy;policy.Configure(ce::BoundaryScope(w));std::array<rs::Evidence,12> apis{};
  ce::Access<ChallengeReader,AccessEngine,decltype(leaseCheck)> access(r,engine,authority,w,f.Module,42,leaseCheck,cache,policy,apis);
  check(access.Warm()&&!engine.calls&&!engine.factories,"all12 actual frame guards compose without native action or factory");
  cfgprobe::lifetime::SlotObservation actorSlot,volumeSlot;
  cfgprobe::lifetime::ReadSlot(r,w.registry,car?f.Car:f.Npc,actorSlot);cfgprobe::lifetime::ReadSlot(r,w.registry,f.Volume,volumeSlot);
  const auto id=ce::Identity(actorSlot),volumeId=ce::Identity(volumeSlot);
  std::array<b::Identity,3> volumes{{volumeId,{},{}}};
  if(car>=2){cfgprobe::lifetime::ReadSlot(r,w.registry,0x20F1000,volumeSlot);volumes[1]=ce::Identity(volumeSlot);
   cfgprobe::lifetime::ReadSlot(r,w.registry,0x20F3000,volumeSlot);volumes[2]=ce::Identity(volumeSlot);}
  policy.SetVolumes(volumes.data(),car>=2?3:1,true);
  r.count=0;check(car?policy.TrackCar(access,id):policy.TrackPawn(access,id),"policy consumes actual typed actor and all-seat adapter");
  policy.PumpOne(access,id,1000);peak=(std::max)(peak,r.count);
  const auto* record=car?policy.Car(id):policy.Pawn(id);check(record&&record->hasSafePoint,"real resource getters/query establish actual safe transform");
  r.count=0;engine.calls=0;engine.actions.clear();engine.positions[id.address]={50,0,0};
  const auto result=policy.Gate(access,id,car?b::Seam::VehicleStart:b::Seam::Overlap,1250);
  peak=(std::max)(peak,r.count);
  if(!result.recovered)std::printf("Access car%u reads%u calls%u resourceFailure%u block%u attempt%u\n",car,r.count,engine.calls,static_cast<unsigned>(access.diagnostic.failure),result.blockNative,result.recoveryAttempted);
  check(result.blockNative&&result.recoveryAttempted&&result.recovered,"composed native calls physically recover and independently block precommit");
  const std::vector<unsigned> foot{{3,4,5}},vehicle{{10,11,8,9,3,10,11,8,9}};
  check(engine.actions==(car?vehicle:foot),"native action order exact, car angular restop included, no damage/flag writes");
  check(engine.positions[id.address].x==0&&engine.calls<=64,"native returned actual safe transform within explicit callback call bound");
  check(engine.preflights==1&&engine.reserveCalls==(car?9u:3u)+2u+3u*(car>=2?3u:1u),"preflight covers every remaining mutation/readback/volume query before action");
  check(engine.calls<=engine.reserveCalls+2u*(car>=2?3u:1u)&&engine.calls>=b::Policy::CarActionLimit*(car!=0)+b::Policy::FootActionLimit*(car==0),
   "wholetransaction reservation still covers actual calls when unchanged geometry skips getters");
  r.count=0;authority.Retire();const auto oldCalls=engine.calls;
  check(!policy.Gate(access,id,b::Seam::Completion,1500).blockNative&&engine.calls==oldCalls,"retired Challenges scope cannot suppress or move departed actors");
 }
 for(unsigned car=0;car<2;++car)for(unsigned budget=0;budget<3;++budget){AccessHarness h(car!=0);check(h.ready&&h.Learn(),"wholetransaction budget refusal fixture owns safe point");
  h.engine.positions[h.id.address]={50,0,0};
  if(budget==0)h.engine.limit=4; // Explicitly preserves a legacy four-call dispatcher cap.
  if(budget==1)h.r.cap=car?200000u:60000u;
  if(budget==2)h.engine.reserveAllowed=false;
  const auto result=h.policy.Gate(*h.access,h.id,car?b::Seam::VehicleStart:b::Seam::Overlap,1250);
  check(result.blockNative&&result.recoveryAttempted&&!result.recovered&&h.engine.actions.empty()&&h.engine.positions[h.id.address].x==50,
   "insufficient whole native/read reservation blocks precommit but performs zero mutations");
  check(h.engine.calls<=h.engine.limit&&h.r.count<=h.r.cap,"preflight never enlarges or resets reader/dispatcher bound");
 }
 {AccessHarness h;check(h.ready,"strict direct mutation fixture");b::Snapshot snapshot;
  check(h.access->Current(ce::BoundaryScope(h.w),h.id,snapshot),"actual current snapshot for direct mutation guard");
  check(!h.access->Teleport(ce::BoundaryScope(h.w),snapshot,{})&&!h.access->StopController(ce::BoundaryScope(h.w),snapshot)&&
   !h.access->StopMovement(ce::BoundaryScope(h.w),snapshot)&&h.engine.actions.empty(),"no mutation without policy wholetransaction preflight");
  const auto calls=h.engine.calls;
  check(!h.access->PreflightRecovery(ce::BoundaryScope(h.w),snapshot,9,1)&&!h.access->PreflightRecovery(ce::BoundaryScope(h.w),snapshot,3,0)&&
   !h.access->PreflightRecovery(ce::BoundaryScope(h.w),snapshot,3,49)&&h.engine.calls==calls,"wrong action count and invalid volume roster cannot reserve");
  check(h.access->PreflightRecovery(ce::BoundaryScope(h.w),snapshot,3,1),"exact scope/snapshot owns explicit foot reservation");
  check(!h.access->StopController(ce::BoundaryScope(h.w),snapshot)&&h.engine.actions.empty(),"reserved actions cannot be skipped or reordered");
  check(h.access->Teleport(ce::BoundaryScope(h.w),snapshot,{})&&!h.access->Teleport(ce::BoundaryScope(h.w),snapshot,{})&&h.engine.actions.size()==1,
   "each reserved ordinal consumed before native call and cannot replay");
 }
 {AccessHarness h(true);check(h.ready,"car direct native depth fixture");b::Snapshot snapshot;
  check(h.access->Current(ce::BoundaryScope(h.w),h.id,snapshot)&&h.access->PreflightRecovery(ce::BoundaryScope(h.w),snapshot,9,1),"car reservation anchored before first action");
  bool nestedReserve=true,nestedBrake=true;
  h.engine.nested=[&](unsigned){nestedReserve=h.access->PreflightRecovery(ce::BoundaryScope(h.w),snapshot,9,1);
   nestedBrake=h.access->Brake(ce::BoundaryScope(h.w),snapshot,1);};
  check(h.access->Throttle(ce::BoundaryScope(h.w),snapshot,0)&&!nestedReserve&&!nestedBrake&&h.engine.actions==std::vector<unsigned>{10},
   "synchronous native reentry cannot reserve or consume next mutation ordinal");
  check(h.access->Brake(ce::BoundaryScope(h.w),snapshot,1)&&h.engine.actions==std::vector<unsigned>({10,11}),"outer transaction keeps its reservation across nested refusal");
 }
 for(unsigned car=0;car<2;++car){AccessHarness h(car!=0);check(h.ready,"full persistent caches fixture");
  for(unsigned i=0;i<h.cache.pawns.size();++i){auto p=std::make_shared<act::CounselorProof>();p->member.actor.slot.object=0x5000000+0x1000*i;h.cache.pawns[i]=p;}
  for(unsigned i=0;i<h.cache.cars.size();++i){auto p=std::make_shared<act::CarProof>();p->member.actor.slot.object=0x6000000+0x1000*i;h.cache.cars[i]=p;}
  for(unsigned i=0;i<h.cache.volumes.size();++i){auto p=std::make_shared<act::VolumeProof>();p->member.actor.slot.object=0x7000000+0x1000*i;h.cache.volumes[i]=p;}
  check(h.Learn(),"full actor/car/volume caches retain fresh proof through callback-local spills");
  check(!ce::Cache::Find(h.cache.pawns,h.f.Npc)&&!ce::Cache::Find(h.cache.cars,h.f.Car)&&!ce::Cache::Find(h.cache.volumes,h.f.Volume),
   "overflow proof does not evict retained cells or invent persistent storage");
  h.engine.positions[h.id.address]={50,0,0};
  const auto result=h.policy.Gate(*h.access,h.id,car?b::Seam::VehicleStart:b::Seam::Overlap,1250);
  check(result.blockNative&&result.recovered&&h.engine.positions[h.id.address].x==0,"callback-owned spill supports suppression and complete verified recovery despite full persistent caches");
 }
 {AccessHarness h(false,24);check(h.ready,"policy and cache capacity fixture");
  for(unsigned i=0;i<24;++i){cfgprobe::lifetime::SlotObservation slot;cfgprobe::lifetime::ReadSlot(h.r,h.w.registry,0x3100000+0x10000*i,slot);
   check(h.policy.TrackPawn(*h.access,ce::Identity(slot)),"fill exact positive AI history rows");h.r.count=0;}
  check(!ce::Cache::Find(h.cache.pawns,h.f.Npc)&&!h.policy.Pawn(h.id),"new verified actor has neither retained proof nor AI history slot");
  const auto result=h.policy.Gate(*h.access,h.id,b::Seam::Overlap,1250);
  check(result.blockNative&&!result.recoveryAttempted&&!h.policy.Pawn(h.id),"both capacity overflows still block fresh reciprocal AI without persisting invented history");
  h.r.Set(h.f.Npc+0x3A0,std::uintptr_t{});h.r.count=0;
  check(!h.policy.Gate(*h.access,h.id,b::Seam::Completion,1500).blockNative&&!h.policy.Pawn(h.id),"overflow scratch proof cannot fabricate historical eligibility after controller becomes null");
 }
 for(unsigned query=0;query<2;++query){AccessHarness h;check(h.ready,"nested cache mutation fixture");
  b::Sphere initial;check(h.access->Geometry(ce::BoundaryScope(h.w),h.volume,initial),"positive volume proof cached before nested replacement");
  std::weak_ptr<const act::VolumeProof> old=ce::Cache::Find(h.cache.volumes,h.f.Volume);bool aliveInNative=false;
  if(!query)h.cache.geometries={}; // Exercise a real native getter miss, not numeric-cache reuse.
  h.engine.nested=[&](unsigned){auto current=ce::Cache::Find(h.cache.volumes,h.f.Volume);auto replacement=std::make_shared<act::VolumeProof>(*current);
   current.reset();h.cache.Clear();h.cache.volumes[0]=replacement;aliveInNative=!old.expired();};
  bool okay=false;if(!query){b::Sphere sphere;okay=h.access->Geometry(ce::BoundaryScope(h.w),h.volume,sphere)&&sphere.radius==5;}
  else {bool overlap=true;okay=h.access->Overlap(ce::BoundaryScope(h.w),h.volume,h.id,overlap)&&!overlap;}
  check(okay&&aliveInNative&&old.expired(),"geometry/overlap own immutable proof across nested cache Clear and same-actor Put replacement");
 }
 for(unsigned query=0;query<2;++query){AccessHarness h;check(h.ready,"nested source mutation fixture");
  b::Sphere initial;h.access->Geometry(ce::BoundaryScope(h.w),h.volume,initial);
  std::weak_ptr<const act::VolumeProof> old=ce::Cache::Find(h.cache.volumes,h.f.Volume);bool alive=false;
  if(!query)h.cache.geometries={};
  h.engine.nested=[&](unsigned){h.cache.Clear();h.r.Set(h.f.Sphere+0x20,h.f.Car);alive=!old.expired();};
  const auto calls=h.engine.calls;bool okay=false;
  if(!query){b::Sphere sphere;okay=h.access->Geometry(ce::BoundaryScope(h.w),h.volume,sphere);}
  else {bool overlaps=true;okay=h.access->Overlap(ce::BoundaryScope(h.w),h.volume,h.id,overlaps);}
  check(!okay&&alive&&old.expired()&&h.engine.calls==calls+1,"nested cache Clear cannot free proof; changed physical source revokes native result after exactly one query");
 }
 {AccessHarness h;check(h.ready&&h.Learn(),"nested recovery cache retirement fixture");h.engine.positions[h.id.address]={50,0,0};
  h.engine.nested=[&](unsigned){h.cache.Clear();};
  const auto result=h.policy.Gate(*h.access,h.id,b::Seam::Overlap,1250);
  check(result.blockNative&&result.recovered,"cleared persistent pawn and volume caches cannot invalidate still-positive callback owned physical proof");
 }
 {AccessHarness h;check(h.ready&&h.Learn(),"native action failure fixture");h.engine.positions[h.id.address]={50,0,0};h.engine.failApi=static_cast<unsigned>(rs::Api::Teleport);
  const auto result=h.policy.Gate(*h.access,h.id,b::Seam::Overlap,1250);
  check(result.blockNative&&!result.recovered&&h.engine.actions.empty(),"failed native action consumes reservation and cannot start subsequent mutations");
 }
 for(unsigned car=0;car<2;++car){AccessHarness h(car!=0);check(h.ready&&h.Learn(),"later action API corruption fixture");h.engine.positions[h.id.address]={50,0,0};
  const auto api=car?rs::Api::AngularVelocity:rs::Api::StopMovement;
  h.r.Set(h.f.Module+d::Apis[static_cast<unsigned>(api)].executor,std::uint8_t{0xCC});
  const auto result=h.policy.Gate(*h.access,h.id,car?b::Seam::VehicleStart:b::Seam::Overlap,1250);
  check(result.blockNative&&!result.recovered&&h.engine.actions.empty()&&!h.engine.preflights,"wholetransaction native schema preflight refuses before first mutation when a later API is corrupt");
 }
 {ce::Cache cache;cache.ownership[0]=std::make_shared<const act::CounselorOwnershipProof>();
  cache.Clear();check(!cache.ownership[0],"clearing departed-world proofs also retires cached ownership evidence");}
 check(peak<524288,"composed source/native frames/all-seat recovery has hard bounded reader budget");
 std::printf("Challenge escape composed real Access: %u checks, %u failures, peak%u/524288; owned native simulator only.\n",checks,failures,peak);
 return failures?1:0;
}
