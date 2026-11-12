#pragma once
#include "../Core/ChallengeEdgeWallsPolicy.hpp"
#include "../RuntimeProbe/ChallengeEdgeWallsBindings.generated.hpp"
#include "NativeChallengeEscapeActors.hpp"
#include "CabinBoundsRuntime.hpp"
#if CFG_J215_CHALLENGE_WALL_OWNERSHIP
#include "../RuntimeProbe/ChallengeWallOwnershipBindings.generated.hpp"
#include "../Core/BrushLocalGeometry.hpp"
#endif
namespace cfgj36walls {
namespace actors=cfgchallengeescape::actors;namespace life=cfgprobe::lifetime;
struct Proof {
 std::uintptr_t level=0,type=0,actor=0;unsigned levelIndex=0,actorIndex=0;
 actors::List levels{},list{};life::SlotObservation slot{};cfgnames::Words name{};
#if CFG_J217_FINISHING
 Bounds worldSample{};bool worldSampleValid=false;
#if CFG_J218_FINISHING
 unsigned sampleAttempts=0;
#endif
#endif
#if CFG_J215_CHALLENGE_WALL_OWNERSHIP
 life::SlotObservation root{};
 Bounds local{};
#endif
};
// Separate named overload avoids C++20 abbreviated function parameters.
template<class R,std::size_t N>bool Pin(R& r,std::uintptr_t module,std::uintptr_t at,const std::array<unsigned char,N>& bytes){
 std::array<unsigned char,N> current{};return r.ReadBytes(module+at,current.data(),current.size())&&current==bytes;
}
template<class R>bool SlotClear(R& r,const life::ArrayHeader& registry,std::uintptr_t object,life::SlotObservation& slot){
 std::uint32_t flags=0,objectFlags=0;return life::ReadSlot(r,registry,object,slot)&&
  r.Read(registry.data+24ULL*static_cast<unsigned>(slot.index)+8,flags)&&!(flags&0x30000000u)&&
  r.Read(object+8,objectFlags)&&!(objectFlags&0x30u);
}
// Returns 0 when the proof holds, else the 1-based number of the first failing clause (H2.79 diagnostic).
template<class R>unsigned MemberClause(R& r,std::uintptr_t module,const cfgchallengeescape::scope::Witness& w,const Proof& p){
 life::SlotObservation slot;std::uintptr_t klass=0,ctor=0,actorClass=0,actorCtor=0,defaults=0;unsigned c=0;
 const auto ok=[&](bool good){++c;return good;};namespace o=cfgmenu::offlineowner;
 const bool good=ok(SlotClear(r,w.registry,p.actor,slot))&&ok(cfgchallengeboundary::BorrowSlot(p.slot,slot))&&
  ok(o::Same(r,p.actor+0x110,std::uint8_t{3}))&&
  ok(o::Same(r,w.World()+cfgradarworld::disk::WorldLevels,p.levels))&&ok(p.levelIndex<static_cast<unsigned>(p.levels.count))&&
  ok(o::Same(r,p.levels.data+8ULL*p.levelIndex,p.level))&&
  ok(o::Same(r,p.level+cfgkill::disk::OwningWorld,w.World()))&&
  ok(o::Same(r,p.level+cfgradarworld::disk::LevelActors,p.list))&&ok(p.actorIndex<static_cast<unsigned>(p.list.count))&&
  ok(o::Same(r,p.list.data+8ULL*p.actorIndex,p.actor))&&ok(o::Same(r,p.actor+0x20,p.level))&&
  ok(o::Same(r,p.actor+cfgprobe::ObjectClass,p.type)&&o::Same(r,p.actor+cfgnames::disk::ObjectName,p.name))&&
  ok(r.Read(module+disk::BlockingVolumeReflection,klass)&&r.Read(module+disk::BlockingVolumeConstructor,ctor)&&
  cfgprobe::MatchingClassCache(klass,ctor)&&cfgspawn::ObjectIs(r,p.actor,klass))&&
  ok(r.Read(module+disk::ActorReflection,actorClass)&&r.Read(module+disk::ActorConstructor,actorCtor)&&
  cfgprobe::MatchingClassCache(actorClass,actorCtor)&&cfgspawn::ObjectIs(r,p.actor,actorClass))&&
  ok(r.Read(p.type+cfgspawn::ClassDefaultObject,defaults)&&defaults!=p.actor&&cfgprobe::ObjectPointer(defaults));
 return good?0u:c;
}
template<class R>bool Member(R& r,std::uintptr_t module,const cfgchallengeescape::scope::Witness& w,const Proof& p){
 return MemberClause(r,module,w,p)==0;
}
#if CFG_H279_WALL_LIST_REFRESH
// Clauses 8-10 are the level actor TArray header and cell. The actor list legitimately grows/relocates when anything
// spawns; membership is a current strong link, not an immutable header. Runs only after clauses 1-7 (slot, world,
// level list, level cell, owning world) passed, so p.level is already proven. Bounded unique rescan like MemberCell.
template<unsigned N>struct Rank:Rank<N-1>{};template<>struct Rank<0>{};
template<class R>auto Left(const R& r,Rank<3>)->decltype(r.Remaining(),unsigned{}){return r.Remaining();}
template<class R>auto Left(const R& r,Rank<2>)->decltype(r.cap,r.count,unsigned{}){return r.count<r.cap?r.cap-r.count:0;}
template<class R>auto Left(const R& r,Rank<1>)->decltype(r.cap,r.reads,unsigned{}){return r.reads<r.cap?r.cap-r.reads:0;}
inline unsigned Left(...){return 0;} // unknown budget is not a reservation
template<class R>bool RefreshActorList(R& r,Proof& p){
 actors::List fresh;std::uintptr_t cell=0;
 if(!r.Read(p.level+cfgradarworld::disk::LevelActors,fresh)||!fresh.Sane(16000)||!fresh.count)return false;
 const std::uintptr_t old=p.list.count>=0&&p.actorIndex<static_cast<unsigned>(p.list.count)?p.list.data+8ULL*p.actorIndex:0;
 // Same policy as cfgchallengeboundary::MemberCell: the old cell if the list is unchanged and still holds the actor, else one bounded unique rescan.
 if(fresh==p.list&&old>=fresh.data&&old-fresh.data<8ULL*static_cast<unsigned>(fresh.count)&&actors::Same(r,old,p.actor))cell=old;
 else{unsigned matches=0;
  if(Left(r,Rank<3>{})<static_cast<unsigned>(fresh.count)+16384u)return false;
  for(int i=0;i<fresh.count;++i){const auto at=fresh.data+8ULL*static_cast<unsigned>(i);std::uintptr_t value=0;
   if(!r.Read(at,value))return false;if(value==p.actor){cell=at;++matches;}}
  if(matches!=1)return false;}
 p.list=fresh;p.actorIndex=static_cast<unsigned>((cell-fresh.data)/8);return true;
}
// Same proof as Member, but a moved/grown actor list is re-resolved (and written back into p) instead of failing forever.
template<class R>unsigned MemberRefresh(R& r,std::uintptr_t module,const cfgchallengeescape::scope::Witness& w,Proof& p,bool& refreshed){
 refreshed=false;const auto clause=MemberClause(r,module,w,p);
 if(clause>=8&&clause<=10){Proof moved=p;
  if(RefreshActorList(r,moved)){refreshed=true;const auto again=MemberClause(r,module,w,moved);if(!again)p=moved;return again;}
 }
 return clause;
}
#endif
class Runtime {
 cfgchallengeescape::scope::Runtime scope_;cfgmenu::active::Context source_{};
 cfgchallengeescape::scope::Witness witness_{};std::uint64_t generation_=0;bool bound_=false,stop_=false;
 actors::List levels_{};std::array<actors::List,128> lists_{};std::array<std::uintptr_t,128> levelObjects_{};
 unsigned level_=0,cell_=0,work_=0;bool scanning_=false,scanned_=false;
 std::array<Proof,Capacity> proofs_{};
public:
 Diagnostics report{};
 const auto& Source()const{return source_;}auto& Witness(){return witness_;}const auto& Witness()const{return witness_;}
 // Apply-stage level-membership proof: refreshes the cached actor list in place when it moved, records the first failing clause.
 template<class R>bool ProveMember(R& r,unsigned index){
  if(index>=report.count)return false;
#if CFG_H279_WALL_LIST_REFRESH
  bool refreshed=false;const auto clause=MemberRefresh(r,source_.module,witness_,proofs_[index],refreshed);
#if CFG_J218_FINISHING
  if(refreshed)++report.memberRefreshed;if(clause){report.memberClause=clause;++report.memberRefreshFailed;}
#else
  (void)refreshed;
#endif
  return clause==0;
#else
  return Member(r,source_.module,witness_,proofs_[index]);
#endif
 }
 auto& Authority(){return scope_;}const Proof& Actor(unsigned index)const{return proofs_[index];}
 bool RememberWorld(unsigned index,const Bounds& value){
  if(index>=report.count||!Finite(value))return false;
  report.records[index].bounds=value;
#if CFG_J217_FINISHING
#if CFG_J218_FINISHING
  if(!proofs_[index].worldSampleValid)++report.sampled;
#endif
  proofs_[index].worldSample=value;proofs_[index].worldSampleValid=true;
#endif
  return true;
 }
 template<class Host>bool ApplyWorldSample(unsigned index,const Bounds& fresh,Host& host){
  if(index>=report.count)return false;auto& record=report.records[index];
#if CFG_J217_FINISHING
  const auto& proof=proofs_[index];
  if(!proof.worldSampleValid||!SameWorldBounds(proof.worldSample,fresh)){
#if CFG_J218_FINISHING
   if(report.mismatchCount<report.mismatches.size())report.mismatches[report.mismatchCount++]={index,proof.worldSample,fresh};
   else report.mismatchTruncated=1;
#endif
   record.decision=Decision::Changed;return true;}
#else
  if(!SameWorldBounds(record.bounds,fresh)){record.decision=Decision::Changed;return true;}
#endif
  return Apply(record,report,host)||record.state||record.decision!=Decision::Edge;
 }
#if CFG_J218_FINISHING
 bool OwnedForSampling(unsigned index)const{
  if(index>=report.count)return false;const auto owner=report.records[index].owner;
  return owner<report.ownerCount&&report.owners[owner].challenge>0&&report.owners[owner].challenge<=2;
 }
 bool NeedSample(unsigned index)const{return OwnedForSampling(index)&&!proofs_[index].worldSampleValid;}
 bool SamplesPending()const{for(unsigned i=0;i<report.count;++i)if(NeedSample(i))return true;return false;}
 bool SampleFailed(unsigned index){
  if(!NeedSample(index))return false;++report.sampleFailures;
  if(++proofs_[index].sampleAttempts>=8){report.failure=61;Cancel();}return false;
 }
 template<class R,class Host>bool SampleWall(R& r,unsigned index,Host& host){
  if(!NeedSample(index))return true;Bounds world;
  if(host.ReadBounds(world)&&CaptureGeometry(r,index)&&RememberWorld(index,world))return true;
  return SampleFailed(index);
 }
#endif
 bool Armed()const{return report.enabled!=0;}bool Bound()const{return bound_;}bool Stopping()const{return stop_;}
 bool Pending()const{for(unsigned i=0;i<report.count;++i)if(report.records[i].state==1)return true;return false;}
 void Cancel(){stop_=true;}
#if CFG_J215_CHALLENGE_WALL_OWNERSHIP
 // Non-persisting membership check for const proofs (owner/geometry reads); ProveMember writes a refreshed list back.
 template<class R>bool MemberOf(R& r,const Proof& p){
#if CFG_H279_WALL_LIST_REFRESH
  Proof copy=p;bool refreshed=false;return MemberRefresh(r,source_.module,witness_,copy,refreshed)==0;
#else
  return Member(r,source_.module,witness_,p);
#endif
 }
 template<class R>bool Typed(R& r,std::uintptr_t object,std::uintptr_t cache){
  std::array<std::uintptr_t,2> classes{};life::SlotObservation slot;
  return SlotClear(r,witness_.registry,object,slot)&&r.Read(source_.module+cache,classes)&&
   cfgprobe::MatchingClassCache(classes[0],classes[1])&&cfgspawn::ObjectIs(r,object,classes[0]);
 }
 template<std::size_t N>static bool Label(std::array<char,N>& out,std::u16string_view value){
  if(value.empty()||value.size()>=N)return false;out={};
  for(unsigned i=0;i<value.size();++i){if(value[i]<32||value[i]>=127)return false;out[i]=static_cast<char>(value[i]);}return true;
 }
 template<class R>bool ReadOwner(R& r,const Proof& p,LevelOwner& out){
  namespace d=cfgj215walls::disk;out={};std::uintptr_t persistent=0;life::SlotObservation package;
  cfgnames::Observation<R> names(r,source_.module);cfgnames::Text levelName;cfgnames::PathText path,map;
  if(!MemberOf(r,p)||!Typed(r,p.level,d::LevelReflection)||
     !r.Read(p.level+0x20,out.outerWorld)||!Typed(r,out.outerWorld,d::WorldReflection)||
     !r.Read(out.outerWorld+0x20,out.package)||!SlotClear(r,witness_.registry,out.package,package)||
     !r.Read(witness_.World()+d::PersistentLevel,persistent)||!names.Object(p.level,levelName)||
     !names.Object(out.package,path)||!names.Decode(witness_.map,map)||
     !Label(out.levelName,levelName.View())||!Label(out.packageName,path.View()))return false;
  out.level=p.level;out.persistent=persistent==p.level?1u:0u;
  out.challenge=path.View()==map.View()?1u:ChallengePackage(map.View(),path.View())?2u:3u;
  // Exact map package identity cannot be inferred from a copied package name.
  if(out.challenge==1&&out.package!=witness_.mapOuter)return false;
  return MemberOf(r,p)&&cfgmenu::offlineowner::Same(r,p.level+0x20,out.outerWorld)&&
   cfgmenu::offlineowner::Same(r,out.outerWorld+0x20,out.package);
 }
 template<class R>bool OwnerCurrent(R& r,unsigned index){
  if(index>=report.count)return false;const auto owner=report.records[index].owner;
  if(owner>=report.ownerCount)return false;LevelOwner fresh;
  if(!ReadOwner(r,proofs_[index],fresh))return false;const auto& old=report.owners[owner];
  return fresh.level==old.level&&fresh.outerWorld==old.outerWorld&&fresh.package==old.package&&
   fresh.persistent==old.persistent&&fresh.challenge==old.challenge&&fresh.levelName==old.levelName&&fresh.packageName==old.packageName;
 }
 template<class R>bool SampleGeometry(R& r,unsigned index,life::SlotObservation& root,Vec& rotation){
  if(index>=report.count||!OwnerCurrent(r,index))return false;
  namespace d=cfgj215walls::disk;const auto& p=proofs_[index];std::uintptr_t component=0;
  if(!r.Read(p.actor+d::RootComponent,component)||!cfgmenu::offlineowner::Same(r,p.actor+d::BrushComponent,component)||
     !Typed(r,component,d::BrushComponentReflection)||!SlotClear(r,witness_.registry,component,root)||
     !cfgmenu::offlineowner::Same(r,component+0xE0,p.actor)||!r.Read(component+d::RelativeRotation,rotation))return false;
  for(const float v:{rotation.x,rotation.y,rotation.z})if(!std::isfinite(v)||std::fabs(v)>360000.f)return false;
  return cfgmenu::offlineowner::Same(r,p.actor+d::RootComponent,component)&&MemberOf(r,p);
 }
 template<class R>bool CaptureGeometry(R& r,unsigned index){
  life::SlotObservation root;Vec rotation;
  if(!SampleGeometry(r,index,root,rotation))return false;
  proofs_[index].root=root;report.records[index].rotation=rotation;report.records[index].geometry|=1u;
  Bounds local;if(ReadLocalBounds(r,index,local)){proofs_[index].local=local;report.records[index].localExtent=local.extent;report.records[index].geometry|=2u;}
  return true;
 }
 template<class R>bool ReadLocalBounds(R& r,unsigned index,Bounds& output){
  output={};if(index>=report.count||!OwnerCurrent(r,index))return false;
  namespace d=cfgj215walls::disk;const auto& p=proofs_[index];std::uintptr_t component=0,body=0,table=0;
  if(!r.Read(p.actor+d::RootComponent,component)||!cfgmenu::offlineowner::Same(r,p.actor+d::BrushComponent,component)||
     !Typed(r,component,d::BrushComponentReflection)||!r.Read(component,table)||table!=source_.module+d::BrushVtable||
     !cfgmenu::offlineowner::Same(r,table+d::CalcBoundsSlot,source_.module+d::BrushCalcBounds)||
     !Pin(r,source_.module,d::BrushNativeConstructor,d::BrushNativeConstructorCode)||!Pin(r,source_.module,d::BrushCalcBounds,d::BrushCalcBoundsCode)||
     !Pin(r,source_.module,d::AggregateAabb,d::AggregateAabbCode)||!Pin(r,source_.module,d::ConvexAabb,d::ConvexAabbCode)||
     !r.Read(component+d::BrushBodySetup,body)||!Typed(r,body,d::BodySetupReflection))return false;
  // Brush collision is convex data. Other element kinds are explicitly
  // unverified here, never silently omitted from a claimed local aggregate.
  std::array<actors::List,4> arrays{};
  for(unsigned i=0;i<4;++i)if(!r.Read(body+d::AggGeom+16ULL*i,arrays[i])||!arrays[i].Sane(64))return false;
  if(arrays[0].count||arrays[1].count||arrays[2].count||!arrays[3].count)return false;
  bool first=true;
  for(int i=0;i<arrays[3].count;++i){const auto element=arrays[3].data+d::ConvexStride*static_cast<unsigned>(i);
   cfgj215walls::Box box{},again{};cfgj215walls::Transform transform{},fresh{};Bounds local;
   if(!r.Read(element+d::ConvexBox,box)||!r.Read(element+d::ConvexTransform,transform)||
      !cfgj215walls::TransformBox(box,transform,local)||!r.Read(element+d::ConvexBox,again)||
      !r.Read(element+d::ConvexTransform,fresh)||std::memcmp(&box,&again,sizeof(box))||std::memcmp(&transform,&fresh,sizeof(transform)))return false;
   if(first){output=local;first=false;}else cfgj215walls::Union(output,local);
  }
  for(unsigned i=0;i<4;++i)if(!cfgmenu::offlineowner::Same(r,body+d::AggGeom+16ULL*i,arrays[i]))return false;
  return Finite(output)&&Typed(r,body,d::BodySetupReflection)&&cfgmenu::offlineowner::Same(r,component+d::BrushBodySetup,body)&&
   cfgmenu::offlineowner::Same(r,p.actor+d::RootComponent,component)&&OwnerCurrent(r,index);
 }
 template<class R>bool GeometryCurrent(R& r,unsigned index){
  life::SlotObservation root;Vec rotation;if(!SampleGeometry(r,index,root,rotation))return false;
  const auto& expected=proofs_[index].root;const auto& old=report.records[index].rotation;
  if(!cfgchallengeboundary::BorrowSlot(expected,root)||old.x!=rotation.x||old.y!=rotation.y||old.z!=rotation.z)return false;
  if(report.records[index].geometry&2u){Bounds fresh;if(!ReadLocalBounds(r,index,fresh))return false;
   const auto& local=proofs_[index].local;
   if(local.extent.x!=fresh.extent.x||local.extent.y!=fresh.extent.y||local.extent.z!=fresh.extent.z||
      local.center.x!=fresh.center.x||local.center.y!=fresh.center.y||local.center.z!=fresh.center.z)return false;
  }
  return true;
 }
#endif
 bool Arm(const cfgmenu::active::Context& source,bool enabled,std::uint64_t now){
  if(Pending()||generation_==UINT64_MAX)return false;
  source_=source;report={};report.enabled=enabled&&CFG_J36_CHALLENGE_WALLS;bound_=stop_=scanning_=scanned_=false;
  levels_={};lists_={};levelObjects_={};proofs_={};level_=cell_=work_=0;witness_={};
  return scope_.Arm(source,++generation_,report.enabled!=0,now);
 }
 template<class R>unsigned WorldStatus(R& r){std::uintptr_t world=0;
  if(!bound_||!cfgmenu::offlineowner::Same(r,source_.module+cfgprobe::EngineGlobalCandidate,source_.engine)||
     !cfgmenu::offlineowner::Same(r,source_.engine+cfgprobe::EngineGameViewport,source_.viewport)||
     !r.Read(source_.viewport+cfgprobe::ViewportWorld,world)||!cfgprobe::ObjectPointer(world))return 0;
  return world==witness_.World()?1u:2u;
 }
 template<class R>bool WorldCurrent(R& r){return WorldStatus(r)==1;}
 template<class R>bool Current(R& r,std::uint32_t thread){
  if(!bound_||!WorldCurrent(r)||!cfgprimary::LookupVetoClearForObservation(r,source_.module))return false;
  auto fresh=witness_;
  if(!life::ReadArrayHeader(r,source_.module+life::ObjectArrayRva,fresh.registry)||
     !r.Read(fresh.Mode()+0x3F8,fresh.phase)||!scope_.Current(r,thread,fresh))return false;
  witness_=fresh;return true;
 }
 template<class R>bool Observe(R& r,std::uint32_t thread,std::uint64_t now){
  if(!Armed())return false;
  if(bound_){
   const auto world=WorldStatus(r);if(!world){report.failure=6;return false;}
   if(world==2){for(unsigned i=0;i<report.count;++i)if(report.records[i].state==1){report.records[i].state=4;++report.departed;}
    report.enabled=0;report.stage=7;scope_.Retire();return false;}
   if(!Current(r,thread)){report.failure=2;return false;}
   cfgnames::Observation<R> names(r,source_.module);cfgnames::Text phase;
   if(!names.Decode(witness_.phase,phase)){report.failure=3;return false;}
   if(phase.View()!=u"InProgress")stop_=true;
  }else{if(!scope_.Observe(r,thread,now,witness_)){report.failure=1;return false;}bound_=true;}
  report.failure=0;return true;
 }
 template<class R>bool Scan(R& r){
  if(scanned_)return true;
  if(!scanning_){unsigned total=0;
   if(!r.Read(witness_.World()+cfgradarworld::disk::WorldLevels,levels_)||!levels_.Sane(128)||!levels_.count)return false;
   for(int i=0;i<levels_.count;++i){const auto n=static_cast<unsigned>(i);
    if(!r.Read(levels_.data+8ULL*n,levelObjects_[n])||!cfgprobe::ObjectPointer(levelObjects_[n])||
       !cfgmenu::offlineowner::Same(r,levelObjects_[n]+cfgkill::disk::OwningWorld,witness_.World())||
       !r.Read(levelObjects_[n]+cfgradarworld::disk::LevelActors,lists_[n])||!lists_[n].Sane(16000)||
       static_cast<unsigned>(lists_[n].count)>16000-total)return false;
    total+=static_cast<unsigned>(lists_[n].count);
   }scanning_=true;
  }
  std::uintptr_t base=0,ctor=0;
  if(!r.Read(source_.module+disk::BlockingVolumeReflection,base)||!r.Read(source_.module+disk::BlockingVolumeConstructor,ctor)||!cfgprobe::MatchingClassCache(base,ctor))return false;
  unsigned batch=0;cfgnames::Observation<R> names(r,source_.module);
  while(level_<static_cast<unsigned>(levels_.count)&&batch<128){
   auto& list=lists_[level_];if(cell_==static_cast<unsigned>(list.count)){++level_;cell_=0;continue;}
   if(!cfgmenu::offlineowner::Same(r,witness_.World()+cfgradarworld::disk::WorldLevels,levels_)||
      !cfgmenu::offlineowner::Same(r,levels_.data+8ULL*level_,levelObjects_[level_])||
      !cfgmenu::offlineowner::Same(r,levelObjects_[level_]+cfgradarworld::disk::LevelActors,list))return false;
   const auto index=cell_++;std::uintptr_t actor=0;++batch;
   if(!r.Read(list.data+8ULL*index,actor))return false;if(!actor)continue;
   if(!cfgspawn::ObjectIs(r,actor,base))continue;
   if(report.count==Capacity){report.failure=5;stop_=true;return false;}
   for(unsigned i=0;i<report.count;++i)if(proofs_[i].actor==actor)return false;
   auto& p=proofs_[report.count];p.actor=actor;p.level=levelObjects_[level_];p.levelIndex=level_;p.actorIndex=index;p.levels=levels_;p.list=list;
   cfgnames::Text label;
   if(!r.Read(actor+cfgprobe::ObjectClass,p.type)||!r.Read(actor+cfgnames::disk::ObjectName,p.name)||!names.Object(actor,label)||
      !SlotClear(r,witness_.registry,actor,p.slot)||!Member(r,source_.module,witness_,p))return false;
   auto& out=report.records[report.count++];out.actor=actor;out.index=p.slot.index;out.serial=p.slot.serial;
   out.nameHash=cfgj29walls::Census::Fingerprint(label.View());
   for(unsigned i=0;i+1<out.name.size()&&i<label.View().size();++i)out.name[i]=label.View()[i]<128?static_cast<char>(label.View()[i]):'?';
#if CFG_J215_CHALLENGE_WALL_OWNERSHIP
   for(unsigned i=0;i<report.ownerCount;++i)if(report.owners[i].level==p.level){out.owner=i;break;}
   if(out.owner==UINT32_MAX&&report.ownerCount<OwnerCapacity){LevelOwner owner;
    if(ReadOwner(r,p,owner)){out.owner=report.ownerCount;report.owners[report.ownerCount++]=owner;}}
   if(out.owner==UINT32_MAX)++report.unknownOwners;
#endif
  }
  if(level_==static_cast<unsigned>(levels_.count)){scanned_=true;report.stage=2;work_=0;}
  return true;
 }
 bool Scanned()const{return scanned_;}unsigned Work()const{return work_;}void Advance(){++work_;}
 void ResetWork(){work_=0;}void Stopped(){if(!Pending()){report.enabled=0;report.stage=7;scope_.Retire();}}
};
}
