#pragma once
#include "NativeChallengeBoundaryScript.hpp"
#include "../Core/J24FeatureFlags.hpp"
#ifndef CFG_H294_BOUNDARY_VERIFY_DEFER
#define CFG_H294_BOUNDARY_VERIFY_DEFER 1 // Challenges boundary: a Verify failure under a still-proven scope keeps the edit and re-verifies for ~4 s (streaming grows the object array mid-read)
#endif
#include "../Core/ChallengeBoundaryWallCensusPolicy.hpp"
namespace cfgchallengeboundary {
namespace actors=cfgchallengeescape::actors;
namespace life=cfgprobe::lifetime;
// The dispatcher itself may have serial0. It is NEVER a native-call receiver:
// each edit is admitted by a current positive Challenge scope plus a complete
// World -> Level roster/cell -> typed actor -> actual class -> declared child
// proof. No weak serial assignment, global registry census or native factory.
struct Borrow {
 bool valid=false;std::uintptr_t level=0,levelCell=0,actorCell=0,native=0,defaults=0;
 actors::List levels{},list{};actors::LevelLinkProof levelType{};
 life::SlotObservation slot{};cfgprobe::Ancestry ancestry{};Owner owner{};
};
template<class R>bool BorrowCurrent(R& r,std::uintptr_t module,const cfgchallengeescape::scope::Witness& w,const Borrow& b){
 life::SlotObservation slot;std::uint32_t internal=0,flags=0;Owner owner;
 return b.valid&&actors::PositiveCurrent(r,w.registry,w.owners[0])&&life::ReadSlot(r,w.registry,b.slot.object,slot)&&slot==b.slot&&
  r.Read(w.registry.data+24ULL*static_cast<unsigned>(slot.index)+8,internal)&&!(internal&0x30000000u)&&
  r.Read(slot.object+8,flags)&&!(flags&0x30u)&&actors::Same(r,slot.object+0x110,std::uint8_t{3})&&
  actors::Same(r,module+cfgchallengeescape::disk::Actor,b.native)&&actors::Same(r,module+cfgchallengeescape::disk::ActorCtor,b.native)&&
  b.ancestry.Contains(b.native)&&cfgprobe::StableAncestry(r,slot.object,b.ancestry)&&
  b.ancestry.classes[0]==b.owner.slot.object&&ObserveOwner(r,module,b.owner.slot.object,owner)&&SameOwner(b.owner,owner)&&
  actors::Same(r,owner.slot.object+cfgspawn::ClassDefaultObject,b.defaults)&&b.defaults!=slot.object&&
  actors::LevelLinkCurrent(r,module,w,b.levelType)&&actors::Same(r,w.World()+cfgradarworld::disk::WorldLevels,b.levels)&&
  actors::Same(r,b.levelCell,b.level)&&actors::Same(r,b.level+cfgkill::disk::OwningWorld,w.World())&&
  actors::Same(r,b.level+cfgradarworld::disk::LevelActors,b.list)&&actors::Same(r,b.actorCell,slot.object)&&
  actors::Same(r,slot.object+0x20,b.level);
}
// Continued membership is a current strong link, not an immutable TArray header.
// Keep the fast unchanged-cell path; on growth, compaction or relocation search
// ONLY the current bounded world/level roster, never old/freed array storage.
template<class R>bool MemberCell(R& r,const actors::List& old,std::uintptr_t oldCell,
 const actors::List& fresh,std::uintptr_t object,std::uintptr_t& cell){
 cell=0;
 if(fresh==old&&oldCell>=fresh.data&&(oldCell-fresh.data)%8==0&&
    (oldCell-fresh.data)/8<static_cast<unsigned>(fresh.count)&&actors::Same(r,oldCell,object)){cell=oldCell;return true;}
 if(r.Remaining()<static_cast<unsigned>(fresh.count)+16384u)return false;
 unsigned matches=0;
 for(int i=0;i<fresh.count;++i){const auto at=fresh.data+8ULL*static_cast<unsigned>(i);std::uintptr_t value=0;
  if(!r.Read(at,value))return false;if(value==object){cell=at;if(++matches>1)return false;}
 }
 return matches==1;
}
template<class R>bool RefreshBorrow(R& r,std::uintptr_t module,const cfgchallengeescape::scope::Witness& w,
 Borrow& old,unsigned& guard,unsigned& refreshes){
 Borrow fresh=old;life::ArrayHeader registry;life::SlotObservation actorSlot,levelSlot;Owner owner;
 guard=2;
 if(!old.valid||!actors::PositiveCurrent(r,w.registry,w.owners[0])||
    !life::ReadArrayHeader(r,module+life::ObjectArrayRva,registry)||!(registry==w.registry))return false;
 guard=3;
 if(!life::ReadSlot(r,registry,old.slot.object,actorSlot)||!BorrowSlot(old.slot,actorSlot))return false;
 guard=4;std::uint32_t internal=0,flags=0;
 if(!r.Read(registry.data+24ULL*static_cast<unsigned>(actorSlot.index)+8,internal)||(internal&0x30000000u)||
    !r.Read(actorSlot.object+8,flags)||(flags&0x30u)||!actors::Same(r,actorSlot.object+0x110,std::uint8_t{3}))return false;
 guard=5;
 if(!actors::Same(r,module+cfgchallengeescape::disk::Actor,old.native)||
    !actors::Same(r,module+cfgchallengeescape::disk::ActorCtor,old.native)||
    !old.ancestry.Contains(old.native)||!cfgprobe::StableAncestry(r,actorSlot.object,old.ancestry))return false;
 guard=6;
 if(old.ancestry.classes[0]!=old.owner.slot.object||!ObserveOwner(r,module,old.owner.slot.object,owner)||
    !BorrowSlot(old.owner.slot,owner.slot))return false;
 Owner normalized=owner;normalized.slot=old.owner.slot;
 if(!SameOwner(old.owner,normalized)||owner.defaults!=old.defaults||old.defaults==actorSlot.object)return false;
 guard=7;
 if(!life::ReadSlot(r,registry,old.level,levelSlot)||!BorrowSlot(old.levelType.slot,levelSlot))return false;
 fresh.slot=actorSlot;fresh.owner=owner;fresh.levelType.slot=levelSlot;
 if(!actors::LevelLinkCurrent(r,module,w,fresh.levelType))return false;
 guard=8;
 if(!r.Read(w.World()+cfgradarworld::disk::WorldLevels,fresh.levels)||!fresh.levels.Sane(128)||!fresh.levels.count)return false;
 guard=9;
 if(!MemberCell(r,old.levels,old.levelCell,fresh.levels,old.level,fresh.levelCell))return false;
 guard=10;
 if(!actors::Same(r,old.level+cfgkill::disk::OwningWorld,w.World())||
    !actors::Same(r,actorSlot.object+0x20,old.level))return false;
 guard=11;
 if(!r.Read(old.level+cfgradarworld::disk::LevelActors,fresh.list)||!fresh.list.Sane(16000)||!fresh.list.count)return false;
 guard=12;
 if(!MemberCell(r,old.list,old.actorCell,fresh.list,actorSlot.object,fresh.actorCell))return false;
 guard=13;life::SlotObservation actorEnd,levelEnd;Owner ownerEnd;
 if(!BorrowCurrent(r,module,w,fresh)||!life::ReadSlot(r,registry,actorSlot.object,actorEnd)||!(actorEnd==actorSlot)||
    !life::ReadSlot(r,registry,old.level,levelEnd)||!(levelEnd==levelSlot)||
    !ObserveOwner(r,module,owner.slot.object,ownerEnd)||!SameOwner(owner,ownerEnd))return false;
 // Latch an engine-assigned first serial only AFTER the full current link,
 // metadata, class/default, GC and double-sampled identity proof. A later
 // different positive serial is never admitted as another upgrade.
 if(!(old.slot==fresh.slot)||!(old.owner.slot==fresh.owner.slot)||!(old.levelType.slot==fresh.levelType.slot)||
    !(old.levels==fresh.levels)||!(old.list==fresh.list)||old.levelCell!=fresh.levelCell||old.actorCell!=fresh.actorCell)++refreshes;
 old=fresh;guard=0;return true;
}
class Scan {
 actors::List levels_{};std::array<std::uintptr_t,128> levelsObjects_{};std::array<actors::List,128> lists_{};
 std::vector<std::uintptr_t> seen_;unsigned level_=0,cell_=0,matches_=0;bool begun_=false,done_=false;
 Borrow borrow_{};cfgj29walls::Census wall_{};
 template<class R>bool Stable(R& r,const cfgchallengeescape::scope::Witness& w){
  if(!actors::Same(r,w.World()+cfgradarworld::disk::WorldLevels,levels_))return false;
  for(int i=0;i<levels_.count;++i)if(!actors::Same(r,levels_.data+8ULL*static_cast<unsigned>(i),levelsObjects_[i])||
   !actors::Same(r,levelsObjects_[i]+cfgradarworld::disk::LevelActors,lists_[i]))return false;
  return true;
 }
public:
 unsigned cells=0,failure=0;
 void Reset(){levels_={};levelsObjects_={};lists_={};seen_.clear();level_=cell_=matches_=0;begun_=done_=false;borrow_={};wall_={};}
 const cfgj29walls::Census& WallReport()const{return wall_;}
 const Borrow& Proof()const{return borrow_;}
 Borrow& Proof(){return borrow_;}
 bool Done()const{return done_;}
 template<class R,class Current>bool Step(R& r,std::uintptr_t module,const cfgchallengeescape::scope::Witness& w,Current&& current){
  failure=1;if(!current())return false;
  if(!begun_){unsigned total=0;
   if(!r.Read(w.World()+cfgradarworld::disk::WorldLevels,levels_)||!levels_.Sane(128)||!levels_.count)return false;
   for(int i=0;i<levels_.count;++i){auto& level=levelsObjects_[i];auto& list=lists_[i];
    if(!r.Read(levels_.data+8ULL*static_cast<unsigned>(i),level)||!cfgprobe::ObjectPointer(level))return false;
    for(int j=0;j<i;++j)if(levelsObjects_[j]==level)return false;
    if(!r.Read(level+cfgradarworld::disk::LevelActors,list)||!list.Sane(16000)||static_cast<unsigned>(list.count)>16000-total)return false;
    total+=static_cast<unsigned>(list.count);
   }seen_.reserve(total);begun_=true;
  }
  failure=2;if(!Stable(r,w))return false;
  cfgnames::Observation<R> names(r,module);unsigned work=0;
  // Keep the same complete roster/identity proof, but do not stretch donor's
  // dispatcher lookup over hundreds of callbacks in a streaming mission.
  while(level_<static_cast<unsigned>(levels_.count)&&work<128){
   const auto level=levelsObjects_[level_];const auto& list=lists_[level_];
   if(cell_==static_cast<unsigned>(list.count)){++level_;cell_=0;continue;}
   const auto actorCell=list.data+8ULL*cell_++;std::uintptr_t actor=0,type=0;cfgnames::Text label;++work;++cells;
   failure=3;if(!r.Read(actorCell,actor))return false;if(!actor)continue;
   if(!cfgprobe::ObjectPointer(actor)||std::find(seen_.begin(),seen_.end(),actor)!=seen_.end())return false;seen_.push_back(actor);
   if(!r.Read(actor+cfgprobe::ObjectClass,type)||!names.Object(type,label))return false;
#if CFG_J29_CHALLENGE_BOUNDARY_FULL
   // No extra memory reads: reuse the class name already decoded by Scan.
   // Reserve the remaining budget for the existing proof and script cleanup.
   (void)wall_.Observe(label.View(),r.Remaining()>16384);
#endif
   if(label.View()!=u"SinglePlayer_Dispatcher_C")continue;
   failure=4;if(++matches_!=1)return false;
   auto& b=borrow_;b.level=level;b.levelCell=levels_.data+8ULL*level_;b.actorCell=actorCell;b.levels=levels_;b.list=list;
   std::uintptr_t ctor=0;
   if(!life::ReadSlot(r,w.registry,actor,b.slot)||!r.Read(module+cfgchallengeescape::disk::Actor,b.native)||
      !r.Read(module+cfgchallengeescape::disk::ActorCtor,ctor)||!cfgprobe::MatchingClassCache(b.native,ctor)||
      !cfgprobe::CaptureAncestry(r,actor,b.ancestry)||!ObserveOwner(r,module,type,b.owner)||
      !r.Read(type+cfgspawn::ClassDefaultObject,b.defaults)||!actors::ObserveLevelLink(r,module,w,level,b.levelType))return false;
   b.valid=true;if(!BorrowCurrent(r,module,w,b))return false;
  }
  failure=5;if(!Stable(r,w)||!current()||(borrow_.valid&&!BorrowCurrent(r,module,w,borrow_)))return false;
  if(level_==static_cast<unsigned>(levels_.count)){if(matches_!=1)return false;done_=true;}
  failure=0;return true;
 }
};
template<class R,class Current>bool Functions(R& r,std::uintptr_t module,const Borrow& b,Current&& current,
 std::array<std::unique_ptr<Evidence>,3>& out){
 out={};if(!current())return false;std::array<std::uintptr_t,512> walked{},links{};unsigned count=0;
 cfgreflection::Observation<R> reflection(r,module);cfgnames::Observation<R> names(r,module);
 for(auto node=b.owner.head;node;){if(count==walked.size()||!cfgprobe::ObjectPointer(node))return false;
  for(unsigned i=0;i<count;++i)if(walked[i]==node)return false;
  std::uintptr_t next=0;cfgnames::Text label;if(!r.Read(node+cfgreflection::disk::Next,next)||!names.Object(node,label))return false;
  walked[count]=node;links[count++]=next;
  for(unsigned i=0;i<Names.size();++i)if(label.View()==Names[i]){if(out[i])return false;
   cfgreflection::Field field;out[i]=std::make_unique<Evidence>();
   if(!reflection.ReadField(node,b.owner.slot.object,field)||!cfgmenu::capture::KnownFunction(r,module,field,Names[i],*out[i])||!Shape(*out[i],i,module))return false;
  }node=next;
 }
 for(unsigned i=0;i<count;++i)if(!actors::Same(r,walked[i]+cfgreflection::disk::Next,links[i]))return false;
 return out[0]&&out[1]&&out[2]&&current();
}
class Runtime {
 cfgchallengeescape::scope::Runtime scope_;Scan scan_;Pair pair_;std::uint64_t generation_=0;
 std::uintptr_t module_=0;bool bound_=false,failed_=false;unsigned incompleteScope_=0;
 std::uint64_t retryAt_=0;
 unsigned verifyDefers_=0; // H2.94 consecutive Verify failures kept under a still-proven scope
 bool DeferIncompleteScope(){
  using F=cfgchallengeescape::scope::Failure;
  if(!(bound_&&scope_.Armed()&&(scope_.failure==F::None||scope_.failure==F::Changed))||++incompleteScope_>3)return false;
  ++deferrals;return true;
 }
public:
 unsigned failure=0,attempts=0,guard=0,refreshes=0,deferrals=0,verifyDeferrals=0;std::uint64_t Generation()const{return generation_;}
 static constexpr unsigned VerifyDeferLimit=80; // ~4 s at the boundary pump cadence
 unsigned ScopeFailure()const{return static_cast<unsigned>(scope_.failure);}
 bool Armed()const{return scope_.Armed();}bool Ready()const{return scope_.Armed()&&pair_.Ready();}bool CleanupPending()const{return pair_.Owned()&&!scope_.Armed();}
 unsigned Applied()const{return pair_.applied;}unsigned Restored()const{return pair_.restored;}unsigned Cells()const{return scan_.cells;}
 const cfgj29walls::Census& WallReport()const{return scan_.WallReport();}
 const ProofDiagnostic& VerifyFailure()const{return pair_.verifyFailure;}
 unsigned VerifyRetries()const{return pair_.verifyRetries;}unsigned VerifyRecoveries()const{return pair_.verifyRecoveries;}
 void Cancel(){scope_.Retire();bound_=false;incompleteScope_=0;scan_.Reset();}
 template<class R,class W>bool Cleanup(R& r,W& writer){const bool ok=pair_.Restore(r,writer);if(!ok)failure=7;return ok;}
 template<class R,class W>bool Arm(R& r,W& writer,const cfgmenu::active::Context& source,bool enabled,std::uint64_t now){
  Cancel();if(!Cleanup(r,writer)||generation_==UINT64_MAX)return false;
  if(!scope_.Arm(source,generation_+1,enabled,now))return false;
  ++generation_;module_=source.module;failure=attempts=guard=refreshes=deferrals=verifyDeferrals=verifyDefers_=0;bound_=failed_=false;retryAt_=0;return true;
 }
 template<class R,class W>void Step(R& r,W& writer,std::uint32_t thread,std::uint64_t now){
  if(!scope_.Armed()){(void)Cleanup(r,writer);return;}
  cfgchallengeescape::scope::Witness w;
  if(!scope_.Observe(r,thread,now,w)){
   failure=1;guard=1;
   // An incomplete read is not proof of departure. Preserve only the existing
   // reversible edits for at most three callbacks; do no new work until fresh
   // scope proof succeeds. Observe retires immediately on proved departure.
   if(DeferIncompleteScope())return;
   if(bound_||!scope_.Armed()){Cancel();(void)Cleanup(r,writer);}return;
  }
  bound_=true;const auto current=[&]{return scope_.Current(r,thread,w);};
  if(pair_.Ready()){
   guard=1;bool valid=current();
   if(!valid&&DeferIncompleteScope()){failure=6;return;}
   if(valid)valid=RefreshBorrow(r,module_,w,scan_.Proof(),guard,refreshes);
   if(valid){guard=14;valid=pair_.Verify(r);
    // One fresh FULL same-callback proof may recover an incomplete streaming
    // observation. Failed evidence grants no authority. Keep the shared65536
    // cap and16384 cleanup reserve; no blind repatch or rediscovery loop.
    if(!valid&&r.Remaining()>=49152){
     ++pair_.verifyRetries;guard=1;valid=current();
     if(valid)valid=RefreshBorrow(r,module_,w,scan_.Proof(),guard,refreshes);
     if(valid){guard=14;valid=pair_.Verify(r);if(valid)++pair_.verifyRecoveries;}
    }
   }
   if(valid){guard=15;valid=current();if(!valid&&DeferIncompleteScope()){failure=6;return;}}
#if CFG_H294_BOUNDARY_VERIFY_DEFER
   // H2.94: Verify re-reads each patched function and demands an unchanged global object-array header across the read.
   // Streaming while the player walks into new map areas grows that array mid-read (live: Verify stage 6, record 1,
   // retry failed, guard undone, game's out-of-bounds ended the Challenge). With the scope itself still proven, keep the
   // existing reversible edit and re-verify on later callbacks (no new writes) for a bounded streaming burst; a scope
   // failure, departure or a still-failing verify after the window keeps the original Cancel + Cleanup.
   if(!valid&&guard==14&&scope_.Armed()&&bound_&&verifyDefers_<VerifyDeferLimit&&
      pair_.verifyFailure.difference==0&& // H2.96 (audit 2 P2): only a failed READ, never a positively detected difference
      pair_.StillOursPatched(r)&&++verifyDefers_){failure=6;++verifyDeferrals;++deferrals;return;} // capture: counted in ChallengesBoundaryProof Deferrals
   if(valid)verifyDefers_=0;
#endif
   if(!valid){failure=6;Cancel();(void)Cleanup(r,writer);}else{incompleteScope_=0;guard=failure=0;}return;
  }
  if(failed_||now<retryAt_)return;
  if(!scan_.Step(r,module_,w,current)){
   failure=2;scan_.Reset();
   // Donor PumpChallengeJasonBoundary retries ten times at ONE SECOND, not
   // ten adjacent50ms pumps. Changing native actor lists must not exhaust the
   // whole installation allowance during the initial streaming burst.
   retryAt_=now>UINT64_MAX-1000?UINT64_MAX:now+1000;
   if(++attempts==10){failed_=true;Cancel();}return;
  }
  if(!scan_.Done())return;
  const auto lease=[&]{return current()&&BorrowCurrent(r,module_,w,scan_.Proof());};
  std::array<std::unique_ptr<Evidence>,3> functions;
  if(!Functions(r,module_,scan_.Proof(),lease,functions)){failure=3;failed_=true;Cancel();return;}
  if(!pair_.Apply(r,writer,module_,functions,lease)){failure=4;failed_=true;Cancel();(void)Cleanup(r,writer);return;}
  incompleteScope_=0;failure=guard=0;
 }
};
}
