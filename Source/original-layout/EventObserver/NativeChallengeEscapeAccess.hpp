#pragma once
#include "NativeChallengeEscapeActors.hpp"
#include "NativeChallengeEscapeResources.hpp"
#include "NativeChallengeEscapeGeometry.hpp"
#include "../Core/ChallengeEscapeBoundaryPolicy.hpp"
#include <memory>
#include <limits>
#include <vector>
namespace cfgchallengeescape {
inline boundary::Identity Identity(const cfgprobe::lifetime::SlotObservation& s){return {s.object,s.index,s.serial};}
inline boundary::Scope BoundaryScope(const scope::Witness& w){
 return {w.generation,Identity(w.owners[0]),Identity(w.owners[1]),Identity(w.owners[4]),true,true,true,false,true};
}
inline boundary::Snapshot PawnSnapshot(const actors::CounselorProof& p){
 boundary::Snapshot s;s.actor=Identity(p.counselor.slot);s.controller=Identity(p.controllerType.slot);
 s.movement=Identity(p.movement.slot);s.root=Identity(p.root.slot);s.kind=boundary::Kind::Pawn;
 s.authority=true;s.living=!p.dead&&!(p.dying&1);s.seated=p.seat!=0;s.grabbed=p.grabbed!=0;
 s.escaping=p.spline!=0||p.escaped!=0||p.leaving!=0;
 s.aiPossessed=p.controllerKind==actors::ControllerKind::AI;
 s.unpossessed=p.controllerKind==actors::ControllerKind::Unpossessed;
 // A known AI cannot retain historical eligibility across an unfamiliar
 // non-null controller. Conservatively invalidate, never promote it to AI.
 s.human=p.controllerKind==actors::ControllerKind::Human||
   (p.controller!=0&&p.controllerKind==actors::ControllerKind::Unknown);
 return s;
}
struct Cache {
 std::array<std::shared_ptr<const actors::CounselorProof>,24> pawns{};
 std::array<std::shared_ptr<const actors::CounselorOwnershipProof>,24> ownership{};
 std::array<std::shared_ptr<const actors::CarProof>,24> cars{};
 std::array<std::shared_ptr<const actors::VolumeProof>,48> volumes{};
 std::array<std::shared_ptr<const geometry::Entry>,48> geometries{};
 void Clear(){for(auto& p:pawns)p.reset();for(auto& p:ownership)p.reset();for(auto& c:cars)c.reset();for(auto& v:volumes)v.reset();geometries={};}
 template<class T,std::size_t N>static std::shared_ptr<const T> Find(const std::array<std::shared_ptr<const T>,N>& list,std::uintptr_t actor){
  for(const auto& p:list)if(p&&p->member.actor.slot.object==actor)return p;return {};
 }
 template<class T,std::size_t N>static bool Store(std::array<std::shared_ptr<const T>,N>& list,std::uintptr_t actor,const std::shared_ptr<const T>& value){
  for(auto& p:list)if(!p||p->member.actor.slot.object==actor){p=value;return true;}return false;
 }
 template<class T,std::size_t N>static std::shared_ptr<const T> Put(std::array<std::shared_ptr<const T>,N>& list,std::uintptr_t actor,T&& value){
  auto proof=std::make_shared<const T>(std::move(value));return Store(list,actor,proof)?proof:std::shared_ptr<const T>{};
 }
};
namespace accessdetail {
template<unsigned N>struct Priority:Priority<N-1>{};template<>struct Priority<0>{};
template<class R>auto Remaining(const R& r,Priority<4>)->decltype(r.Remaining(),unsigned{}){return r.Remaining();}
template<class R>auto Remaining(const R& r,Priority<3>)->decltype(r.cap,r.count,unsigned{}){return r.count<r.cap?r.cap-r.count:0;}
template<class R>auto Remaining(const R& r,Priority<2>)->decltype(r.cap,r.reads,unsigned{}){return r.reads<r.cap?r.cap-r.reads:0;}
template<class R>auto Remaining(const R& r,Priority<1>)->decltype(r.limit,r.reads,unsigned{}){return r.reads<r.limit?r.limit-r.reads:0;}
inline unsigned Remaining(...){return 0;} // Unknown finite budget is not a reservation.
template<class E>auto Preflight(E& e,unsigned calls,unsigned reads,int)->decltype(e.Preflight(calls,reads),bool{}){return e.Preflight(calls,reads);}
template<class E>bool Preflight(E&,unsigned,unsigned,long){return false;}
inline bool SameSnapshot(const boundary::Snapshot& a,const boundary::Snapshot& b){
 return a.actor==b.actor&&a.kind==b.kind&&a.controller==b.controller&&a.root==b.root&&a.movement==b.movement&&
  a.authority==b.authority&&a.living==b.living&&a.human==b.human&&a.aiPossessed==b.aiPossessed&&
  a.unpossessed==b.unpossessed&&a.seated==b.seated&&a.grabbed==b.grabbed&&a.escaping==b.escaping&&
  a.seatsComplete==b.seatsComplete&&a.occupantsVerified==b.occupantsVerified&&a.occupantCount==b.occupantCount&&a.occupants==b.occupants;
}
}
// One actual callback owns this adapter and its reader/budget. Persistent cache
// stores positive identities and metadata, never call authority. The callback
// repeats the common native scope and whole source proof at each action edge.
template<class R,class Engine,class Lease>class Access {
 R& r_;Engine& engine_;scope::Runtime& authority_;const scope::Witness witness_;
 std::uintptr_t module_;std::uint32_t thread_;Lease& lease_;Cache& cache_;boundary::Policy& policy_;
 std::array<resources::Evidence,12>& apis_;resources::CallbackBudget budget_{};
 // Callback-local overflow proofs have no persistent AI history. Shared const
 // ownership keeps a proof alive through synchronous native reentry, even when
 // that reentry clears/replaces every persistent cache cell.
 std::vector<std::shared_ptr<const actors::CounselorProof>> pawnSpill_;
 std::vector<std::shared_ptr<const actors::CarProof>> carSpill_;
 std::vector<std::shared_ptr<const actors::VolumeProof>> volumeSpill_;
 // A relaxed transition witness can suppress native accounting but can never
 // obtain a movement reservation. Production Gate passes this flag explicitly.
 unsigned callDepth_=0;bool admissionOnly_=false;
 struct Reservation {bool active=false;boundary::Scope scope{};boundary::Snapshot snapshot{};
  std::array<resources::Api,9> actions{};unsigned count=0,next=0;} reservation_;
 template<class T>static std::shared_ptr<const T> SpillFind(const std::vector<std::shared_ptr<const T>>& list,std::uintptr_t actor){
  for(const auto& p:list)if(p->member.actor.slot.object==actor)return p;return {};
 }
 template<class T,std::size_t N>std::shared_ptr<const T> Remember(std::array<std::shared_ptr<const T>,N>& cache,
  std::vector<std::shared_ptr<const T>>& spill,std::uintptr_t actor,std::shared_ptr<const T> p){
  if(Cache::Store(cache,actor,p))return p;
  for(auto& saved:spill)if(saved->member.actor.slot.object==actor){saved=p;return p;}
  // A bounded callback can keep more objects than its retained cache. Beyond
  // this scratch bound the local returned owner still preserves call lifetime;
  // a later observation must prove the object afresh rather than retain it.
  if(spill.size()<64)spill.push_back(p);return p;
 }
 bool Scope(){return lease_()&&authority_.Current(r_,thread_,witness_);}
 template<class Current>bool Call(resources::Api api,std::uintptr_t target,void* packet,unsigned bytes,Current&& current){
  const auto i=static_cast<unsigned>(api);if(i>=apis_.size()||!apis_[i].function||callDepth_)return false;
  struct Depth {unsigned& value;explicit Depth(unsigned& v):value(v){++value;}~Depth(){--value;}} depth(callDepth_);
  return resources::Invoke(r_,engine_,module_,api,budget_,current,target,apis_[i],packet,bytes,diagnostic);
 }
 std::shared_ptr<const actors::CounselorProof> Pawn(std::uintptr_t actor){
  const auto current=[&]{return Scope();};auto p=Cache::Find(cache_.pawns,actor);if(!p)p=SpillFind(pawnSpill_,actor);
  if(p&&actors::CurrentCounselor(r_,module_,witness_,current,*p))return p;
  auto fresh=std::make_shared<actors::CounselorProof>();
  if(!actors::ObserveCounselor(r_,module_,witness_,current,actor,*fresh))return {};
  return Remember(cache_.pawns,pawnSpill_,actor,std::shared_ptr<const actors::CounselorProof>(fresh));
 }
 bool Eligible(const actors::CounselorProof& p){
  Remember(cache_.pawns,pawnSpill_,p.Actor(),std::make_shared<const actors::CounselorProof>(p));
  const auto id=Identity(p.counselor.slot);
  const auto* known=policy_.Pawn(id);
  if(known&&known->humanInvalidated){eligibilityStage=1;return false;}
  if(p.controllerKind==actors::ControllerKind::AI&&known&&known->knownAi){eligibilityStage=2;return true;}
  // TrackPawn also observes Human/Unknown before refusing it, permanently
  // invalidating the exact old AI incarnation in the policy's history.
  if(policy_.TrackPawn(*this,id)){eligibilityStage=3;return true;}
  known=policy_.Pawn(id);
  const bool allowed=p.controllerKind==actors::ControllerKind::Unpossessed&&known&&known->knownAi&&!known->humanInvalidated;
  eligibilityStage=allowed?4:(p.controllerKind==actors::ControllerKind::Unpossessed?5:6);return allowed;
 }
 std::shared_ptr<const actors::CarProof> Car(std::uintptr_t actor){
  unsigned refusedEligibility=0;
  const auto current=[&]{return Scope();};const auto eligible=[&](const actors::CounselorProof& p){
   const bool allowed=Eligible(p);if(!allowed&&!refusedEligibility)refusedEligibility=eligibilityStage;return allowed;
  };
  auto p=Cache::Find(cache_.cars,actor);if(!p)p=SpillFind(carSpill_,actor);
  if(p&&actors::CurrentCar(r_,module_,witness_,current,*p,eligible,!admissionOnly_))return p;
  auto fresh=std::make_shared<actors::CarProof>();
  if(!actors::ObserveCar(r_,module_,witness_,current,actor,eligible,*fresh,!admissionOnly_)){
   if(refusedEligibility)eligibilityStage=refusedEligibility;return {};
  }
  return Remember(cache_.cars,carSpill_,actor,std::shared_ptr<const actors::CarProof>(fresh));
 }
 std::shared_ptr<const actors::VolumeProof> Volume(std::uintptr_t actor){
  const auto current=[&]{return Scope();};auto p=Cache::Find(cache_.volumes,actor);if(!p)p=SpillFind(volumeSpill_,actor);
  if(p&&actors::CurrentVolume(r_,module_,witness_,current,*p))return p;
  auto fresh=std::make_shared<actors::VolumeProof>();
  if(!actors::ObserveVolume(r_,module_,witness_,current,actor,*fresh))return {};
  return Remember(cache_.volumes,volumeSpill_,actor,std::shared_ptr<const actors::VolumeProof>(fresh));
 }
 bool SnapshotCurrent(const boundary::Scope& scope,const boundary::Snapshot& expected){
  boundary::Snapshot actual;return Current(scope,expected.actor,actual)&&accessdetail::SameSnapshot(actual,expected);
 }
 template<class Current>bool Mutation(const boundary::Scope& scope,const boundary::Snapshot& s,resources::Api api,
  std::uintptr_t target,void* packet,unsigned bytes,Current&& current){
  if(callDepth_||!reservation_.active||!(reservation_.scope==scope)||!accessdetail::SameSnapshot(reservation_.snapshot,s)||
     reservation_.next>=reservation_.count||reservation_.actions[reservation_.next]!=api)return false;
  ++reservation_.next; // consume BEFORE the native callback can reenter.
  const bool success=Call(api,target,packet,bytes,current);
  if(!success)reservation_.active=false;return success;
 }
 bool Velocity(const boundary::Scope& scope,const boundary::Snapshot& s,boundary::Point value,resources::Api api){
  if(s.kind!=boundary::Kind::Car||!boundary::Finite(value)||value.x||value.y||value.z)return false;
  resources::VelocityPacket packet{};const auto current=[&]{return SnapshotCurrent(scope,s);};
  return Mutation(scope,s,api,s.root.address,&packet,sizeof(packet),current);
 }
 bool Input(const boundary::Scope& scope,const boundary::Snapshot& s,double value,resources::Api api){
  if(s.kind!=boundary::Kind::Car||!std::isfinite(value)||value<0||value>1)return false;
  float packet=static_cast<float>(value);const auto current=[&]{return SnapshotCurrent(scope,s);};
  return Mutation(scope,s,api,s.movement.address,&packet,sizeof(packet),current);
 }
public:
 resources::Diagnostic diagnostic{};
 unsigned recoveryStage=0,recoveryRemaining=0,recoveryReserve=0;
 unsigned currentStage=0,eligibilityStage=0;
 Access(R& r,Engine& engine,scope::Runtime& authority,const scope::Witness& witness,std::uintptr_t module,std::uint32_t thread,
  Lease& lease,Cache& cache,boundary::Policy& policy,std::array<resources::Evidence,12>& apis,bool admissionOnly=false):
  r_(r),engine_(engine),authority_(authority),witness_(witness),module_(module),thread_(thread),lease_(lease),cache_(cache),policy_(policy),apis_(apis),admissionOnly_(admissionOnly){}
 bool ScopeCurrent(const boundary::Scope& s){return s==BoundaryScope(witness_)&&Scope();}
 bool Warm(){
  const auto current=[&]{return Scope();};
  for(unsigned i=0;i<apis_.size();++i){resources::Evidence fresh;
   const auto result=resources::Ensure(r_,engine_,module_,static_cast<resources::Api>(i),budget_,current,fresh,diagnostic);
   if(result!=resources::Result::Ready)return false;apis_[i]=fresh;
  }return true;
 }
 bool Current(const boundary::Scope& scope,boundary::Identity id,boundary::Snapshot& out){
  currentStage=1;
  out={};if(!id.Positive()||!ScopeCurrent(scope))return false;
  currentStage=2;
  std::uintptr_t type=0,native=0,ctor=0;cfgprobe::Ancestry ancestry;
  if(!r_.Read(id.address+cfgprobe::ObjectClass,type)||!cfgprobe::CaptureAncestry(r_,id.address,ancestry))return false;
  if(r_.Read(module_+disk::SCCounselorCharacter,native)&&r_.Read(module_+disk::SCCounselorCharacterCtor,ctor)&&
     cfgprobe::MatchingClassCache(native,ctor)&&ancestry.Contains(native)){
   currentStage=3;auto p=Pawn(id.address);if(!p||!(Identity(p->counselor.slot)==id)){currentStage=3;return false;}out=PawnSnapshot(*p);
  }else if(r_.Read(module_+disk::SCDriveableVehicle,native)&&r_.Read(module_+disk::SCDriveableVehicleCtor,ctor)&&
           cfgprobe::MatchingClassCache(native,ctor)&&ancestry.Contains(native)){
   currentStage=4;auto p=Car(id.address);if(!p||!(Identity(p->car.slot)==id)||p->occupied>out.occupants.size()){currentStage=4;return false;}
   out.actor=id;out.kind=boundary::Kind::Car;out.authority=out.living=out.seatsComplete=out.occupantsVerified=true;
   out.root=Identity(p->root.slot);out.movement=Identity(p->movement.slot);
   for(int i=0;i<p->seats.count;++i)if(p->occupants[i].occupant)out.occupants[out.occupantCount++]=Identity(p->occupants[i].counselor.counselor.slot);
  }else return false;
  currentStage=5;const bool valid=ScopeCurrent(scope);if(valid)currentStage=6;return valid;
 }
 bool VolumeCurrent(boundary::Identity id){auto p=Volume(id.address);return p&&Identity(p->volume.slot)==id&&Scope();}
 bool Geometry(const boundary::Scope& scope,boundary::Identity id,boundary::Sphere& out){
  out={};if(!ScopeCurrent(scope))return false;auto p=Volume(id.address);
  if(!p||!(Identity(p->volume.slot)==id))return false;
  const auto current=[&,p]{return ScopeCurrent(scope)&&actors::CurrentVolume(r_,module_,witness_,[&]{return Scope();},*p);};
  geometry::Inputs inputs,again;
  // Metadata and native bodies remain fresh even on a numeric-cache hit.
  for(const auto api:{resources::Api::ComponentLocation,resources::Api::SphereRadius}){
   const auto i=static_cast<unsigned>(api);resources::Evidence fresh;
   if(!apis_[i].function||!resources::Observe(r_,module_,disk::Apis[i],fresh,diagnostic)||
    !resources::Same(apis_[i],fresh,disk::Apis[i].count)||!p->sphere.ancestry.Contains(fresh.owner))return false;
  }
  if(!geometry::ReadInputs(r_,module_,p->sphere.slot.object,inputs))return false;
  for(const auto& held:cache_.geometries){if(!held)continue;const auto saved=*held;
   if(saved.valid&&saved.scope==scope&&saved.volume==id&&saved.sphere==Identity(p->sphere.slot)&&saved.inputs==inputs){
    if(!current()||!geometry::ReadInputs(r_,module_,p->sphere.slot.object,again)||!(again==inputs))return false;
    out=saved.value;return boundary::Finite(out);
   }
  }
  resources::Vector location{};float radius=0;
  if(!Call(resources::Api::ComponentLocation,p->sphere.slot.object,&location,sizeof(location),current)||
     !Call(resources::Api::SphereRadius,p->sphere.slot.object,&radius,sizeof(radius),current))return false;
  out={{location.x,location.y,location.z},radius};
  if(!boundary::Finite(out)||!current()||!geometry::ReadInputs(r_,module_,p->sphere.slot.object,again)||!(again==inputs))return false;
  for(auto& saved:cache_.geometries)if(!saved||saved->volume.address==id.address){
   saved=std::make_shared<const geometry::Entry>(geometry::Entry{true,scope,id,Identity(p->sphere.slot),inputs,out});break;
  }
  return true;
 }
 bool Overlap(const boundary::Scope& scope,boundary::Identity volume,boundary::Identity actor,bool& overlaps){
  overlaps=true;if(!ScopeCurrent(scope))return false;auto p=Volume(volume.address);
  if(!p||!(Identity(p->volume.slot)==volume))return false;
  boundary::Snapshot snapshot;if(!Current(scope,actor,snapshot))return false;
  const auto current=[&,p]{return SnapshotCurrent(scope,snapshot)&&actors::CurrentVolume(r_,module_,witness_,[&]{return Scope();},*p);};
  resources::OverlapPacket packet;packet.other=actor.address;
  if(!Call(resources::Api::OverlappingActor,volume.address,&packet,sizeof(packet),current)||packet.returned>1)return false;
  overlaps=packet.returned!=0;return current();
 }
 bool ReadTransform(const boundary::Scope& scope,const boundary::Snapshot& s,boundary::Transform& out){
  out={};resources::Vector location{};resources::Rotation rotation{};const auto current=[&]{return SnapshotCurrent(scope,s);};
  if(!Call(resources::Api::ActorLocation,s.actor.address,&location,sizeof(location),current)||
     !Call(resources::Api::ActorRotation,s.actor.address,&rotation,sizeof(rotation),current))return false;
  out={{location.x,location.y,location.z},{rotation.pitch,rotation.yaw,rotation.roll}};
  return boundary::Finite(out)&&current();
 }
 // Read-only admission for the ENTIRE remaining recovery, including transform
 // readback and two geometry getters plus overlap for EVERY known volume.
 // Engine must reserve these calls against synchronous reentry without resetting
 // or enlarging its dispatcher cap. Unknown reader/engine budgets fail closed.
 bool PreflightRecovery(const boundary::Scope& scope,const boundary::Snapshot& s,unsigned actionCalls,std::size_t volumeCount){
  recoveryStage=1;
  if(callDepth_||admissionOnly_)return false;reservation_={};
  if(!volumeCount||volumeCount>boundary::Policy::VolumeCapacity||
     (s.kind!=boundary::Kind::Pawn&&s.kind!=boundary::Kind::Car))return false;
  const bool car=s.kind==boundary::Kind::Car;
  const auto actions=car?boundary::Policy::CarActionLimit:boundary::Policy::FootActionLimit;
  if(actionCalls!=actions||!SnapshotCurrent(scope,s))return false;recoveryStage=2;
  // All remaining native schemas must already be ready. Missing/corrupt later
  // stop/input frames must not permit the first movement mutation to start.
  const std::array<resources::Api,9> required{{resources::Api::Teleport,
   car?resources::Api::Throttle:resources::Api::StopController,
   car?resources::Api::Brake:resources::Api::StopMovement,
   car?resources::Api::LinearVelocity:resources::Api::Teleport,
   car?resources::Api::AngularVelocity:resources::Api::Teleport,
   resources::Api::ActorLocation,resources::Api::ActorRotation,
   resources::Api::ComponentLocation,resources::Api::SphereRadius}};
  std::array<bool,12> checked{};
  const auto ready=[&](resources::Api api){const auto i=static_cast<unsigned>(api);if(checked[i])return true;
   checked[i]=true;resources::Evidence fresh;
   return apis_[i].function&&resources::Observe(r_,module_,disk::Apis[i],fresh,diagnostic)&&
    resources::Same(apis_[i],fresh,disk::Apis[i].count);
  };
  for(const auto api:required)if(!ready(api))return false;recoveryStage=3;
  if(!ready(resources::Api::OverlappingActor)||!ScopeCurrent(scope))return false;
  const auto reserveReads=(car?196608u:65536u)+32768u*static_cast<unsigned>(volumeCount);
  recoveryRemaining=accessdetail::Remaining(r_,accessdetail::Priority<4>{});recoveryReserve=reserveReads;recoveryStage=4;
  const auto reserveCalls=actions+2u+3u*static_cast<unsigned>(volumeCount);
  if(accessdetail::Remaining(r_,accessdetail::Priority<4>{})<reserveReads||
     !accessdetail::Preflight(engine_,reserveCalls,reserveReads,0))return false;
  reservation_.scope=scope;reservation_.snapshot=s;reservation_.count=actions;
  if(car)reservation_.actions={{resources::Api::Throttle,resources::Api::Brake,resources::Api::LinearVelocity,
   resources::Api::AngularVelocity,resources::Api::Teleport,resources::Api::Throttle,resources::Api::Brake,
   resources::Api::LinearVelocity,resources::Api::AngularVelocity}};
  else reservation_.actions={{resources::Api::Teleport,resources::Api::StopController,resources::Api::StopMovement}};
  reservation_.active=true;recoveryStage=5;return true;
 }
 bool Teleport(const boundary::Scope& scope,const boundary::Snapshot& s,const boundary::Transform& transform){
  if(!boundary::Finite(transform))return false;
  // Native FVector/FRotator use floats. Reject values that cannot be represented;
  // never silently turn a finite policy double into an infinite native packet.
  const double values[]{transform.position.x,transform.position.y,transform.position.z,
   transform.rotation.x,transform.rotation.y,transform.rotation.z};
  for(const double v:values)if(std::abs(v)>static_cast<double>((std::numeric_limits<float>::max)()))return false;
  resources::TeleportPacket packet{{static_cast<float>(values[0]),static_cast<float>(values[1]),static_cast<float>(values[2])},
    {static_cast<float>(values[3]),static_cast<float>(values[4]),static_cast<float>(values[5])},0};
  const auto current=[&]{return SnapshotCurrent(scope,s);};
  const bool moved=Mutation(scope,s,resources::Api::Teleport,s.actor.address,&packet,sizeof(packet),current)&&packet.returned==1&&current();
  if(!moved)reservation_.active=false;return moved;
 }
 bool StopController(const boundary::Scope& scope,const boundary::Snapshot& s){
  if(s.kind!=boundary::Kind::Pawn||!s.controller.Positive()||!s.aiPossessed)return false;
  resources::Packet<resources::Api::StopController> packet;const auto current=[&]{return SnapshotCurrent(scope,s);};
  return Mutation(scope,s,resources::Api::StopController,s.controller.address,packet.bytes.data(),static_cast<unsigned>(packet.bytes.size()),current);
 }
 bool StopMovement(const boundary::Scope& scope,const boundary::Snapshot& s){
  if(s.kind!=boundary::Kind::Pawn||!s.movement.Positive())return false;
  resources::Packet<resources::Api::StopMovement> packet;const auto current=[&]{return SnapshotCurrent(scope,s);};
  return Mutation(scope,s,resources::Api::StopMovement,s.movement.address,packet.bytes.data(),static_cast<unsigned>(packet.bytes.size()),current);
 }
 bool Throttle(const boundary::Scope& scope,const boundary::Snapshot& s,double value){return value==0&&Input(scope,s,value,resources::Api::Throttle);}
 bool Brake(const boundary::Scope& scope,const boundary::Snapshot& s,double value){return value==1&&Input(scope,s,value,resources::Api::Brake);}
 bool Linear(const boundary::Scope& scope,const boundary::Snapshot& s,boundary::Point value){return Velocity(scope,s,value,resources::Api::LinearVelocity);}
 bool Angular(const boundary::Scope& scope,const boundary::Snapshot& s,boundary::Point value){return Velocity(scope,s,value,resources::Api::AngularVelocity);}
};
}
