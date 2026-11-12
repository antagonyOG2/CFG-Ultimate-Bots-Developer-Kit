#pragma once
#include "NativeChallengeEscapeAccess.hpp"
#include "NativeChallengeEscapeDispatch.hpp"

namespace cfgchallengeescape {
namespace runtimedetail {
template<class L,class R>auto Lease(L& l,R& r,const scope::Witness& w,accessdetail::Priority<2>)->decltype(l(r,w),bool{}){return l(r,w);}
template<class L,class R>auto Lease(L& l,R& r,const scope::Witness&,accessdetail::Priority<1>)->decltype(l(r),bool{}){return l(r);}
template<class L,class R>auto Lease(L& l,R&,const scope::Witness&,accessdetail::Priority<0>)->decltype(l(),bool{}){return l();}
template<class E>auto Busy(const E& e,int)->decltype(e.Busy(),bool{}){return e.Busy();}
template<class E>bool Busy(const E&,long){return false;}
template<class R>auto SharedNativeCallback(R& r,int)->decltype(r.IsActiveCallback(),bool{}){
 return r.IsActiveCallback();
}
template<class R>bool SharedNativeCallback(R&,long){return false;}
struct BusyGuard {bool& flag;explicit BusyGuard(bool& f):flag(f){flag=true;}~BusyGuard(){flag=false;}};
struct Physical {unsigned& depth;explicit Physical(unsigned& d):depth(d){++depth;}~Physical(){--depth;}};
template<class E>struct CensusReserveEngine {
 E& engine;unsigned censusReads,volumeReads;
 auto Thread()const{return engine.Thread();}unsigned Calls()const{return engine.Calls();}bool Busy()const{return engine.Busy();}
 bool Invoke(std::uintptr_t object,std::uintptr_t function,void* packet){return engine.Invoke(object,function,packet);}
 bool Factory(std::uintptr_t address,std::uintptr_t& out){return engine.Factory(address,out);}
 bool Preflight(unsigned calls,unsigned reads){
  const auto census=static_cast<std::uint64_t>(calls)*censusReads;
  if(reads<volumeReads)return false;
  // The access allowance already reserves32k per volume for geometry/native
  // postchecks. Replace that tranche with the greater full-census cost instead
  // of counting the same volume/census work twice. Fixed actor/schema reserve
  // remains intact, and large worlds can only increase the required allowance.
  const auto reserve=static_cast<std::uint64_t>(reads-volumeReads)+(std::max)(static_cast<std::uint64_t>(volumeReads),census);
  if(reserve>dispatch::ReadLimit)return false;
  return engine.Preflight(calls,static_cast<unsigned>(reserve));
 }
};
}
// Callback orchestrator only. It has no installer, native hook handlers, UI
// submission path, process discovery, cap resets or independent reader.
class Runtime {
 struct Level {std::uintptr_t object=0,cell=0;actors::List actors{};actors::LevelLinkProof type{};};
 struct Cell {std::uintptr_t at=0,object=0,type=0,level=0;cfgprobe::lifetime::SlotObservation slot{};bool actionCandidate=false;};
 struct Candidate {boundary::Identity id{};boundary::Kind kind=boundary::Kind::Unknown;};
 struct Census {
  std::uint64_t generation=0;std::uintptr_t world=0;actors::List levels{};
  std::array<Level,128> entries{};unsigned level=0,actor=0,total=0;
  bool complete=false,usable=true;
  std::vector<Cell> cells;std::vector<boundary::Identity> volumes;std::vector<Candidate> candidates;
  std::array<std::uintptr_t,4> classes{};
 };
 scope::Runtime authority_;boundary::Policy policy_;Cache cache_;
 std::array<resources::Evidence,12> apis_{};
 std::shared_ptr<Census> census_;scope::Witness witness_{};
 std::uintptr_t module_=0,viewport_=0;std::uint32_t thread_=0;std::uint64_t generation_=0,epoch_=0;
 unsigned warm_=0,schedule_=0;bool busy_=false;
 inline static constexpr std::array<std::uintptr_t,4> ClassCaches{{disk::Actor,disk::SCCounselorCharacter,disk::SCDriveableVehicle,disk::SCEscapeVolume}};
 void ClearProofs(){policy_.Shutdown();cache_.Clear();apis_={};census_.reset();witness_={};warm_=schedule_=0;}
 template<class R>bool Lifecycle(R& r){
  if(!witness_.generation)return true;
  std::uintptr_t world=0,mode=0,state=0;
  if(!r.Read(viewport_+cfgprobe::ViewportWorld,world))return false;
  if(world!=witness_.World()){Cancel();return false;}
  if(!r.Read(witness_.World()+cfgprobe::WorldGameMode,mode)||
   !r.Read(witness_.World()+cfgprobe::WorldGameState,state))return false;
  if(world!=witness_.World()||mode!=witness_.Mode()||state!=witness_.State()){Cancel();return false;}
  cfgnames::Words phase{};if(!r.Read(mode+0x3F8,phase))return false;
  if(!(phase==witness_.phase)){cfgnames::Observation<R> names(r,module_);cfgnames::Text label;
   if(!names.Decode(phase,label))return false;
   if(label.View()!=u"InProgress"){Cancel();return false;}
  }
  return true;
 }
 template<class R>bool Bind(R& r,std::uint32_t thread,std::uint64_t now,scope::Witness& w){
  if(thread!=thread_||!authority_.Armed()||!Lifecycle(r)||!authority_.Observe(r,thread,now,w)){
   if(!authority_.Armed())ClearProofs();return false;
  }
  witness_=w;policy_.Configure(BoundaryScope(w));return true;
 }
 template<class R>bool CellCurrent(R& r,const scope::Witness& w,const Cell& cell){
  if(!actors::Same(r,cell.at,cell.object))return false;if(!cell.object)return true;
  // Fresh contiguous observations, never cached across a native dispatch.
  // These cover the same index/object/serial/flags/class/outer fields as the
  // former eight individual reads; names/cluster/padding confer no authority.
  struct ObjectHeader {std::uint32_t flags;std::int32_t index;std::uintptr_t type;cfgnames::Words name;std::uintptr_t outer;};
  struct Item {std::uintptr_t object;std::uint32_t flags,cluster;std::int32_t serial;std::uint32_t padding;};
  static_assert(sizeof(ObjectHeader)==32&&offsetof(ObjectHeader,type)==8&&offsetof(ObjectHeader,outer)==24);
  static_assert(sizeof(Item)==24&&offsetof(Item,serial)==16);
  ObjectHeader object{};Item item{};
  return w.registry.Sane()&&r.Read(cell.object+8,object)&&object.index==cell.slot.index&&
   object.index>=0&&object.index<w.registry.count&&object.type==cell.type&&object.outer==cell.level&&!(object.flags&0x30u)&&
   r.Read(w.registry.data+24ULL*static_cast<unsigned>(object.index),item)&&item.object==cell.object&&
   item.serial==cell.slot.serial&&item.serial>=0&&(!cell.actionCandidate||item.serial>0)&&!(item.flags&0x30000000u);
 }
 template<class R>bool ClassesCurrent(R& r,const Census& c){
  for(unsigned i=0;i<ClassCaches.size();++i)if(!actors::Same(r,module_+ClassCaches[i],c.classes[i])||
   !actors::Same(r,module_+ClassCaches[i]+8,c.classes[i]))return false;return true;
 }
 template<class R>bool CensusCurrent(R& r,const scope::Witness& w,const std::shared_ptr<Census>& held,bool complete){
  if(!held||held!=census_||held->generation!=generation_||held->world!=w.World()||!held->usable||
   (complete&&!held->complete)||!ClassesCurrent(r,*held)||!actors::Same(r,w.World()+cfgradarworld::disk::WorldLevels,held->levels))return false;
  for(int i=0;i<held->levels.count;++i){const auto& level=held->entries[i];
   if(!actors::Same(r,level.cell,level.object)||!actors::LevelLinkCurrent(r,module_,w,level.type)||
    !actors::Same(r,level.object+cfgkill::disk::OwningWorld,w.World())||
    !actors::Same(r,level.object+cfgradarworld::disk::LevelActors,level.actors))return false;
  }
  // Completeness is a claim about ALL visited cells, including unrelated
  // actors. A new volume in an unchanged TArray header must revoke recovery.
  for(const auto& cell:held->cells)if(!CellCurrent(r,w,cell))return false;
  return held==census_&&held->generation==generation_;
 }
 template<class R>bool BeginCensus(R& r,const scope::Witness& w){
  auto c=std::make_shared<Census>();c->generation=generation_;c->world=w.World();
  censusFailure=1;censusRejectedObject=0;
  for(unsigned i=0;i<ClassCaches.size();++i){std::uintptr_t ctor=0;
   if(!r.Read(module_+ClassCaches[i],c->classes[i])||!r.Read(module_+ClassCaches[i]+8,ctor)||
    !cfgprobe::MatchingClassCache(c->classes[i],ctor))return false;}
  censusFailure=2;if(!r.Read(w.World()+cfgradarworld::disk::WorldLevels,c->levels)||!c->levels.Sane(128)||!c->levels.count)return false;
  for(int i=0;i<c->levels.count;++i){auto& level=c->entries[i];level.cell=c->levels.data+8ULL*static_cast<unsigned>(i);
   censusFailure=3;if(!r.Read(level.cell,level.object))return false;censusRejectedObject=level.object;
   censusFailure=4;if(!actors::ObserveLevelLink(r,module_,w,level.object,level.type))return false;
   censusFailure=5;if(!actors::Same(r,level.object+cfgkill::disk::OwningWorld,w.World()))return false;
   censusFailure=6;if(!r.Read(level.object+cfgradarworld::disk::LevelActors,level.actors)||!level.actors.Sane(16000)||
    static_cast<unsigned>(level.actors.count)>16000-c->total)return false;
   censusFailure=7;
   for(int j=0;j<i;++j)if(c->entries[j].object==level.object)return false;
   c->total+=static_cast<unsigned>(level.actors.count);
  }
  c->cells.reserve(c->total);census_=c;
  censusFailure=8;if(!CensusCurrent(r,w,c,false)){census_.reset();return false;}censusFailure=0;return true;
 }
 template<class R,class Current>void Tombstone(R& r,const scope::Witness& w,Current&& current,std::uintptr_t actor){
  // This read-only ownership witness does not depend on a usable root/movement
  // component. Revalidate all cached identity/controller/member links; on any
  // change reacquire the minimal proof so a human takeover still tombstones AI.
  auto held=Cache::Find(cache_.ownership,actor);
  if(held&&actors::CurrentCounselorOwnership(r,module_,w,current,*held)){
   ++counters.ownershipHits;
   if(held->HumanOrUnknown())policy_.InvalidateHuman(BoundaryScope(w),Identity(held->member.Slot()));
   return;
  }
  actors::CounselorOwnershipProof ownership;
  ++counters.ownershipMisses;
  if(!actors::ObserveCounselorOwnership(r,module_,w,current,actor,ownership))return;
  const bool human=ownership.HumanOrUnknown();const auto id=Identity(ownership.member.Slot());
  (void)Cache::Put(cache_.ownership,actor,std::move(ownership));
  if(human)policy_.InvalidateHuman(BoundaryScope(w),id);
 }
 template<class R,class Current>void CarTombstones(R& r,const scope::Witness& w,Current&& current,std::uintptr_t car){
  actors::NativeProof type;actors::Membership member;actors::List seats;
  if(!actors::ObserveNative(r,module_,w,car,disk::SCDriveableVehicle,disk::SCDriveableVehicleCtor,type)||
   !actors::ObserveMembership(r,module_,w,car,member)||!r.Read(car+cfgvehicleinteraction::disk::Seats,seats)||!seats.Sane(16))return;
  for(int i=0;i<seats.count;++i){std::uintptr_t seat=0,pawn=0;actors::NativeProof seatType;
   if(!r.Read(seats.data+8ULL*static_cast<unsigned>(i),seat)||
    !actors::ObserveNative(r,module_,w,seat,disk::SCVehicleSeatComponent,disk::SCVehicleSeatComponentCtor,seatType)||
    !actors::Same(r,seat+0x20,car)||!actors::Same(r,seat+cfgvehicleinteraction::disk::ParentVehicle,car)||
    !r.Read(seat+cfgvehicleinteraction::disk::CounselorInSeat,pawn))return;
   if(pawn)Tombstone(r,w,current,pawn);
  }
 }
 template<class R,class Current>bool AdvanceCensus(R& r,const scope::Witness& w,Current&& current){
  auto c=census_;censusFailure=9;if(!c||!CensusCurrent(r,w,c,false))return false;censusFailure=0;if(c->complete)return true;
  unsigned spent=0;
  while(c->level<static_cast<unsigned>(c->levels.count)&&spent<ScanCellsPerService){
   const auto& level=c->entries[c->level];
   if(c->actor>=static_cast<unsigned>(level.actors.count)){++c->level;c->actor=0;continue;}
   Cell cell;cell.level=level.object;cell.at=level.actors.data+8ULL*c->actor++;++spent;++counters.cells;
   censusFailure=10;if(!r.Read(cell.at,cell.object))return false;censusRejectedObject=cell.object;
   if(!cell.object){c->cells.push_back(cell);continue;}
   censusFailure=11;
   for(const auto& old:c->cells)if(old.object==cell.object)return false;
   censusFailure=12;if(!cfgprobe::lifetime::ReadSlot(r,w.registry,cell.object,cell.slot)||!r.Read(cell.object+cfgprobe::ObjectClass,cell.type))return false;
   cfgprobe::Ancestry ancestry;censusFailure=13;if(!cfgprobe::CaptureAncestry(r,cell.object,ancestry)||!ancestry.Contains(c->classes[0]))return false;
   std::uint8_t role=0;std::uint32_t flags=0;
   const bool volume=ancestry.Contains(c->classes[3]),pawn=ancestry.Contains(c->classes[1]),car=ancestry.Contains(c->classes[2]);
   cell.actionCandidate=(volume||pawn||car)&&cell.slot.HasAssignedSerial();
   censusFailure=14;if(!r.Read(cell.object+0x110,role)||(cell.actionCandidate&&role!=3)||!r.Read(cell.object+8,flags)||(flags&0x30u)||
    !actors::Same(r,cell.object+0x20,level.object)||!cfgprobe::StableAncestry(r,cell.object,ancestry))return false;
   censusFailure=15;if(!CellCurrent(r,w,cell))return false;
   c->cells.push_back(cell);
   if(volume&&!cell.actionCandidate){c->usable=false;continue;}
   if(!cell.actionCandidate)continue;
   if(volume){
    actors::VolumeProof proof;censusFailure=16;if(!actors::ObserveVolume(r,module_,w,current,cell.object,proof))return false;
    if(c->volumes.size()==boundary::Policy::VolumeCapacity)c->usable=false;
    else {c->volumes.push_back(Identity(proof.volume.slot));Cache::Put(cache_.volumes,cell.object,std::move(proof));}
   }else if(pawn){
    Tombstone(r,w,current,cell.object);c->candidates.push_back({Identity(cell.slot),boundary::Kind::Pawn});
   }else if(car)c->candidates.push_back({Identity(cell.slot),boundary::Kind::Car});
  }
  ++counters.censusSteps;
  while(c->level<static_cast<unsigned>(c->levels.count)&&c->actor>=static_cast<unsigned>(c->entries[c->level].actors.count)){++c->level;c->actor=0;}
  if(c->level==static_cast<unsigned>(c->levels.count)){c->complete=true;policy_.SetVolumes(c->volumes.data(),c->volumes.size(),c->usable&&!c->volumes.empty());}
  censusFailure=17;const bool complete=c==census_&&current()&&CensusCurrent(r,w,c,false);
  if(complete){censusFailure=0;censusRejectedObject=0;}return complete;
 }
 template<class A>struct PhysicalHost {
  A& access;unsigned& physical;
  bool ScopeCurrent(const boundary::Scope& s){return access.ScopeCurrent(s);}
  bool Current(const boundary::Scope& s,boundary::Identity id,boundary::Snapshot& out){return access.Current(s,id,out);}
  template<class F>bool PhysicalCall(F&& f){runtimedetail::Physical guard(physical);return f();}
  bool Geometry(const boundary::Scope& s,boundary::Identity id,boundary::Sphere& out){return PhysicalCall([&]{return access.Geometry(s,id,out);});}
  bool Overlap(const boundary::Scope& s,boundary::Identity volume,boundary::Identity actor,bool& out){return PhysicalCall([&]{return access.Overlap(s,volume,actor,out);});}
  bool ReadTransform(const boundary::Scope& s,const boundary::Snapshot& p,boundary::Transform& out){return PhysicalCall([&]{return access.ReadTransform(s,p,out);});}
  bool PreflightRecovery(const boundary::Scope& s,const boundary::Snapshot& p,unsigned calls,std::size_t volumes){return PhysicalCall([&]{return access.PreflightRecovery(s,p,calls,volumes);});}
  bool Teleport(const boundary::Scope& s,const boundary::Snapshot& p,const boundary::Transform& t){return PhysicalCall([&]{return access.Teleport(s,p,t);});}
  bool StopController(const boundary::Scope& s,const boundary::Snapshot& p){return PhysicalCall([&]{return access.StopController(s,p);});}
  bool StopMovement(const boundary::Scope& s,const boundary::Snapshot& p){return PhysicalCall([&]{return access.StopMovement(s,p);});}
  bool Throttle(const boundary::Scope& s,const boundary::Snapshot& p,double v){return PhysicalCall([&]{return access.Throttle(s,p,v);});}
  bool Brake(const boundary::Scope& s,const boundary::Snapshot& p,double v){return PhysicalCall([&]{return access.Brake(s,p,v);});}
  bool Linear(const boundary::Scope& s,const boundary::Snapshot& p,boundary::Point v){return PhysicalCall([&]{return access.Linear(s,p,v);});}
  bool Angular(const boundary::Scope& s,const boundary::Snapshot& p,boundary::Point v){return PhysicalCall([&]{return access.Angular(s,p,v);});}
 };
public:
 struct GateDiagnostics {
  std::array<unsigned,5> attempts{},blocked{},forwarded{},budget{},lastStage{},lastRemaining{};
  std::array<unsigned,5> lastCurrent{},lastEligibility{};
  std::array<std::uintptr_t,5> lastActor{};
 } gates;
 void RecordGate(boundary::Seam seam,std::uintptr_t actor,unsigned stage,unsigned remaining,bool blocked,
  unsigned current=0,unsigned eligibility=0){
  const auto i=static_cast<unsigned>(seam);if(i>=gates.attempts.size())return;
  ++gates.attempts[i];if(blocked)++gates.blocked[i];else{
   ++gates.forwarded[i];if(!remaining)++gates.budget[i];
   gates.lastStage[i]=stage;gates.lastRemaining[i]=remaining;gates.lastActor[i]=actor;
   gates.lastCurrent[i]=current;gates.lastEligibility[i]=eligibility;
  }
 }
 static constexpr unsigned ScanCellsPerService=32;
 struct Counters {unsigned services=0,gates=0,nestedRefusals=0,warmSteps=0,censusSteps=0,cells=0,pumps=0,blocks=0,recoveries=0,ownershipHits=0,ownershipMisses=0;} counters;
 resources::Diagnostic diagnostic{};
 unsigned recoveryStage=0,recoveryRemaining=0,recoveryReserve=0;
 unsigned censusFailure=0,censusResets=0;std::uintptr_t censusRejectedObject=0;
 bool Armed()const{return authority_.Armed();}
 std::uint64_t Generation()const{return generation_;}
 unsigned WarmProgress()const{return warm_;}
 bool CensusComplete()const{return census_&&census_->complete&&census_->usable&&!census_->volumes.empty();}
 unsigned VolumeCount()const{return census_?static_cast<unsigned>(census_->volumes.size()):0;}
 bool CurrentWitness(scope::Witness& out)const{out={};if(!Armed()||!witness_.generation)return false;out=witness_;return true;}
 template<class R>bool ScopeCurrent(R& r,std::uint32_t thread,const scope::Witness& expected){
  return expected.generation==generation_&&authority_.Current(r,thread,expected);
 }
 bool Arm(const cfgmenu::active::Context& source,std::uint64_t generation,bool enabled,std::uint64_t now){
  if(!generation||generation<=generation_)return false;scope::Runtime candidate;
  if(!candidate.Arm(source,generation,enabled,now))return false;
  ++epoch_;ClearProofs();gates={};recoveryStage=recoveryRemaining=recoveryReserve=0;
  authority_=std::move(candidate);module_=source.module;viewport_=source.viewport;thread_=source.thread;generation_=generation;return true;
 }
 void Cancel(){++epoch_;authority_.Retire();ClearProofs();}
 template<class R,class Engine,class Lease>bool Service(R& r,Engine& engine,std::uint32_t thread,std::uint64_t now,Lease&& installed,bool physicalAllowed=true){
  if(busy_||runtimedetail::Busy(engine,0)){++counters.nestedRefusals;return false;}if(!Armed())return false;
  runtimedetail::BusyGuard guard(busy_);++counters.services;scope::Witness w;if(!Bind(r,thread,now,w))return false;
  const auto epoch=epoch_;const auto base=[&]{return epoch==epoch_&&runtimedetail::Lease(installed,r,w,accessdetail::Priority<2>{})&&authority_.Current(r,thread,w);};
  if(!base())return false;
  if(census_&&!CensusCurrent(r,w,census_,false)){++censusResets;censusFailure=18;census_.reset();policy_.SetVolumes(nullptr,0,false);}
  if(!census_)BeginCensus(r,w);
  if(census_&&!AdvanceCensus(r,w,base)){++censusResets;census_.reset();policy_.SetVolumes(nullptr,0,false);}
  // The five suppression hooks remain usable with no recovery APIs. The
  // current census and all block/forward gate proofs are read-only. No native
  // call (including a cold factory) can occur while physical work is disabled.
  if(!physicalAllowed){auto held=census_;
   if(held&&held->complete&&held->usable&&!held->candidates.empty()){
    const auto candidate=held->candidates[schedule_++%held->candidates.size()];
    if(candidate.kind==boundary::Kind::Pawn)Tombstone(r,w,base,candidate.id.address);
    else CarTombstones(r,w,base,candidate.id.address);
   }return base();}
  if(warm_<apis_.size()){resources::CallbackBudget budget;resources::Evidence fresh;
   const auto result=resources::Ensure(r,engine,module_,static_cast<resources::Api>(warm_),budget,base,fresh,diagnostic);++counters.warmSteps;
   if(result==resources::Result::Ready){apis_[warm_]=fresh;++warm_;}
   if(result!=resources::Result::Ready)return false;
  }
  auto held=census_;if(warm_!=apis_.size()||!held||!held->complete||!held->usable||held->candidates.empty())return base();
  const auto candidate=held->candidates[schedule_++%held->candidates.size()];
  if(candidate.kind==boundary::Kind::Pawn)Tombstone(r,w,base,candidate.id.address);else CarTombstones(r,w,base,candidate.id.address);
  // Census completeness is reused only in one uninterrupted read-only interval.
  // Every native dispatch increments Calls BEFORE possible reentry; both
  // mutation/query postchecks then repeat the full census. Never cache across
  // callbacks, dispatches, generations or a failed proof.
  unsigned physical=0,lastCensusCall=UINT_MAX;bool censusValid=false;
  auto lease=[&]{if(!base())return false;if(!physical)return true;
   if(!censusValid||lastCensusCall!=engine.Calls()){
    censusValid=CensusCurrent(r,w,held,true);lastCensusCall=engine.Calls();
   }return censusValid;
  };
  const auto censusReads=3u*static_cast<unsigned>(held->cells.size())+96u*static_cast<unsigned>(held->levels.count)+16u;
  runtimedetail::CensusReserveEngine<Engine> reservedEngine{engine,censusReads,32768u*static_cast<unsigned>(held->volumes.size())};
  Access<R,decltype(reservedEngine),decltype(lease)> access(r,reservedEngine,authority_,w,module_,thread,lease,cache_,policy_,apis_);
  PhysicalHost<decltype(access)> host{access,physical};
  if(candidate.kind==boundary::Kind::Pawn)policy_.TrackPawn(host,candidate.id);else policy_.TrackCar(host,candidate.id);
  const auto* before=candidate.kind==boundary::Kind::Pawn?policy_.Pawn(candidate.id):policy_.Car(candidate.id);
  const bool previouslyRecovered=before&&before->hasRecovery;const auto previousTime=before?before->recoveredAt:0;
  policy_.PumpOne(host,candidate.id,now);++counters.pumps;
  const auto* after=candidate.kind==boundary::Kind::Pawn?policy_.Pawn(candidate.id):policy_.Car(candidate.id);
  if(after&&after->hasRecovery&&after->recoveredAt==now&&(!previouslyRecovered||previousTime!=now))++counters.recoveries;
  diagnostic=access.diagnostic;
  if(access.recoveryStage){recoveryStage=access.recoveryStage;recoveryRemaining=access.recoveryRemaining;recoveryReserve=access.recoveryReserve;}
  return base();
 }
 template<class R,class Engine,class Lease,class Validate>boundary::Decision Gate(R& r,Engine& engine,std::uint32_t thread,
  std::uint64_t now,Lease&& installed,boundary::Identity actor,boundary::Seam seam,Validate&& validate){
  boundary::Decision decision;unsigned currentStage=0,eligibilityStage=0;
  unsigned failureStage=0;
  const auto finish=[&](boundary::Decision result,unsigned stage){
   RecordGate(seam,actor.address,result.blockNative?0:(failureStage?failureStage:stage),
    accessdetail::Remaining(r,accessdetail::Priority<4>{}),result.blockNative,currentStage,eligibilityStage);
   return result;
  };
  const auto proof=[&](const scope::Witness& w){
   if(!runtimedetail::Lease(installed,r,w,accessdetail::Priority<2>{})){failureStage=3;return false;}
   if(!validate(r,w)){failureStage=4;return false;}
   if(!authority_.Current(r,thread,w)){failureStage=5;return false;}return true;
  };
  if(busy_||runtimedetail::Busy(engine,0)){
   // Borrow the OUTER callback's reader for read-only nested admission. No
   // bind/reset, factories, census work or native recovery can occur here.
   // Eligible escape commitment must still block during recovery callbacks.
   ++counters.nestedRefusals;const auto w=witness_;const auto epoch=epoch_;
   if(!runtimedetail::SharedNativeCallback(r,0)||!Armed()||!actor.Positive()||!w.generation||thread!=thread_)return finish(decision,1);
   unsigned physical=0;auto base=[&]{return !physical&&epoch==epoch_&&
    proof(w);};
   if(!base())return finish(decision,1);
   Tombstone(r,w,base,actor.address);CarTombstones(r,w,base,actor.address);
   Access<R,Engine,decltype(base)> access(r,engine,authority_,w,module_,thread,base,cache_,policy_,apis_,true);
   PhysicalHost<decltype(access)> host{access,physical};decision=policy_.Gate(host,actor,seam,now,false);
   currentStage=access.currentStage;eligibilityStage=access.eligibilityStage;
   if(!base())return finish({},1);if(decision.blockNative)++counters.blocks;return finish(decision,6);
  }
  if(!Armed()||!actor.Positive())return finish(decision,1);runtimedetail::BusyGuard guard(busy_);++counters.gates;
  scope::Witness w;if(!Bind(r,thread,now,w))return finish(decision,2);
  const auto epoch=epoch_;const auto base=[&]{return epoch==epoch_&&proof(w);};
  if(!base())return finish(decision,1);
  Tombstone(r,w,base,actor.address);CarTombstones(r,w,base,actor.address);
  auto held=census_;unsigned physical=0;auto lease=[&]{return base()&&(!physical||CensusCurrent(r,w,held,true));};
  Access<R,Engine,decltype(lease)> access(r,engine,authority_,w,module_,thread,lease,cache_,policy_,apis_,true);
  // Native accounting admission is strictly read-only. Optional physical work
  // runs through Service/PumpOne, so it cannot spend the final blocking proof's
  // allowance or make an admitted escape forward on geometry budget exhaustion.
  PhysicalHost<decltype(access)> host{access,physical};decision=policy_.Gate(host,actor,seam,now,false);diagnostic=access.diagnostic;
  currentStage=access.currentStage;eligibilityStage=access.eligibilityStage;
  if(!base())return finish({},1);
  if(decision.blockNative)++counters.blocks;if(decision.recovered)++counters.recoveries;return finish(decision,6);
 }
};
}
