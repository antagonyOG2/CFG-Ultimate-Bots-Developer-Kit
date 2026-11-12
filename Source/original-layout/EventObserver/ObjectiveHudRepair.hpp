#pragma once
#include "ObjectiveHudRuntime.hpp"
#include "ObjectivePanelObservation.hpp"
namespace cfgobjective::repair {
enum class Target:unsigned {Car2,Car4,Radio,Phone,Boat,Count};
enum class Status:unsigned {Pending,Ready,Unavailable};
enum class Stage:unsigned {Dormant,Observe,Prepare,Dispatch,Verify,Complete,Refused,Indeterminate};
enum class Failure:unsigned {None,Timeout,Owner,Snapshot,Finder,Dispatch,DispatchContract,NoChange,UnexpectedChange};
constexpr unsigned TargetCount=static_cast<unsigned>(Target::Count),DefaultMask=15,AllMask=31;
constexpr std::uint64_t PhaseLimitMs=20000;
inline unsigned HelperIndex(Target target){constexpr unsigned values[]{23,24,21,22,20};const auto i=static_cast<unsigned>(target);return i<TargetCount?values[i]:Count;}
using Identity=cfgprobe::lifetime::SlotObservation;
struct Owner {
 std::uintptr_t module=0;Identity world{},controller{},widget{},panel{};
 bool operator==(const Owner& b)const{return module==b.module&&world==b.world&&controller==b.controller&&widget==b.widget&&panel==b.panel;}
 bool Valid()const{return cfgprobe::ObjectPointer(module)&&world.HasAssignedSerial()&&controller.HasAssignedSerial()&&
  widget.HasAssignedSerial()&&cfgprobe::ObjectPointer(panel.object)&&panel.index>=0&&panel.serial>=0&&widget.object!=panel.object;}
};
struct Snapshot {Owner owner{};cfgobjectivepanel::Snapshot panel{};};
struct Finder {
 Patch helper{};std::uint16_t frameBytes=0xffff;std::uint8_t numParms=0xff;
 bool voidReturn=false,helperVerified=false;
};
inline bool ChildSame(const cfgobjectivepanel::Child& a,const cfgobjectivepanel::Child& b){
 return a.slot==b.slot&&a.widget==b.widget&&a.type==b.type&&a.kind==b.kind&&a.slotIdentity==b.slotIdentity&&a.widgetIdentity==b.widgetIdentity;
}
inline bool Sane(const Snapshot& s){
 const auto& p=s.panel;
 if(!s.owner.Valid()||(!s.owner.panel.HasAssignedSerial()&&!p.rootedTopology)||p.widget!=s.owner.widget.object||p.panel!=s.owner.panel.object||
  !(p.rootIdentity==s.owner.widget)||!(p.panelIdentity==s.owner.panel)||
  !cfgprobe::ObjectPointer(p.type)||!cfgprobe::ObjectPointer(p.panelType)||p.slots.count<0||p.slots.count>64||
  p.slots.capacity<p.slots.count||p.slots.capacity>1048576||(p.slots.count&&!cfgprobe::ObjectPointer(p.slots.data)))return false;
 for(int i=0;i<p.slots.count;++i){const auto& c=p.children[static_cast<unsigned>(i)];
  if((!p.rootedTopology&&(!c.slotIdentity.HasAssignedSerial()||!c.widgetIdentity.HasAssignedSerial()))||
   c.slotIdentity.index<0||c.slotIdentity.serial<0||c.widgetIdentity.index<0||c.widgetIdentity.serial<0||c.slot!=c.slotIdentity.object||
   c.widget!=c.widgetIdentity.object||!cfgprobe::ObjectPointer(c.slot)||!cfgprobe::ObjectPointer(c.widget)||!cfgprobe::ObjectPointer(c.type))return false;
  for(int j=0;j<i;++j)if(p.children[static_cast<unsigned>(j)].slot==c.slot||p.children[static_cast<unsigned>(j)].widget==c.widget)return false;
 }return true;
}
inline bool Same(const Snapshot& a,const Snapshot& b){return Sane(a)&&Sane(b)&&a.owner==b.owner&&cfgobjectivepanel::Same(a.panel,b.panel);}
inline bool Matches(Target target,cfgobjectivepanel::Kind kind){
 using K=cfgobjectivepanel::Kind;
 switch(target){case Target::Car2:return kind==K::Car2;case Target::Car4:return kind==K::Car4;
 case Target::Radio:return kind==K::Radio||kind==K::GrendelRadio;
 case Target::Phone:return kind==K::Phone||kind==K::GrendelPhone;case Target::Boat:return kind==K::Boat;default:return false;}
}
inline bool Present(const Snapshot& s,Target target){for(int i=0;i<s.panel.slots.count;++i)if(Matches(target,s.panel.children[static_cast<unsigned>(i)].kind))return true;return false;}
inline bool ValidFinder(const Finder& f,const Snapshot& s,Target target){
 return f.helperVerified&&f.voidReturn&&!f.frameBytes&&!f.numParms&&f.helper.spec==HelperIndex(target)&&
  f.helper.owner==s.panel.type&&f.helper.ownerSlot.object==f.helper.owner&&f.helper.ownerSlot.index>=0&&f.helper.ownerSlot.serial>=0&&
  cfgprobe::ObjectPointer(f.helper.functionSlot.object)&&f.helper.functionSlot.index>=0&&f.helper.functionSlot.serial>=0&&
  cfgprobe::ObjectPointer(f.helper.script.data)&&f.helper.script.count==static_cast<int>(Specification(f.helper.spec).bytes);
}
// The native finder appends a row. Array backing storage may grow, but no old
// child may disappear, move, change kind or change object/slot/class identity.
inline bool AddedOnlyExpected(const Snapshot& before,const Snapshot& after,Target target){
 if(!Sane(before)||!Sane(after)||!(before.owner==after.owner)||Present(before,target)||
  before.panel.type!=after.panel.type||before.panel.panelType!=after.panel.panelType||
  after.panel.slots.count!=before.panel.slots.count+1)return false;
 for(int i=0;i<before.panel.slots.count;++i)if(!ChildSame(before.panel.children[static_cast<unsigned>(i)],after.panel.children[static_cast<unsigned>(i)]))return false;
 return Matches(target,after.panel.children[static_cast<unsigned>(before.panel.slots.count)].kind);
}
struct Report {
 Stage stage=Stage::Dormant;Failure failure=Failure::None;unsigned target=0,attempts=0,confirmed=0,preserved=0,refused=0,deferred=0;
 unsigned attemptedMask=0,confirmedMask=0,preservedMask=0,refusedMask=0;
};
// Host contract, all on the native owner thread and ONE shared1536-read/12-call
// callback budget (no nested fresh reader):
// Observe: typed exact local-player-owned ObjectivesWidget -> named reflected
// ObjectivesVerticleBox -> repeated typed PanelSlots/Content/Parent/backlinks;
// offline world + retained slots + no GC + native GetOwningPlayer are mandatory.
// Prepare: ReadyHelper for this exact owner/function/script and reflected
// NumParms==0/ParmsSize==0/no return; mere name or cached 'patched' bit is not proof.
// Dispatch: repeat the exact Owner + panel Snapshot and ReadyHelper/schema in
// THIS callback, reserve native-call/readback capacity, then mark() immediately
// before invoking that zero-parameter void finder on owner.widget. No Construct,
// no timer/UpdateObjectives replay, no clearing the panel. Ready means native
// returned; Pending requires NO mark/no call. Any ambiguous marked call closes
// this runtime. Host must refuse if these observations cannot fit the budget.
class Runtime {
 unsigned enabled_=DefaultMask,index_=0;std::uint64_t deadline_=0,next_=0;
 bool busy_=false,bound_=false;Owner owner_{};Snapshot before_{};Finder finder_{};
 void Advance(std::uint64_t now){
  ++index_;finder_={};before_={};deadline_=now+PhaseLimitMs;report.stage=Stage::Observe;
  while(index_<TargetCount&&!(enabled_&(1u<<index_)))++index_;
  report.target=index_;if(index_==TargetCount)report.stage=Stage::Complete;
 }
 void Refuse(Failure why,std::uint64_t now){report.failure=why;++report.refused;report.refusedMask|=1u<<index_;Advance(now);}
public:
 Report report{};
 bool Configure(unsigned mask){if(report.stage!=Stage::Dormant||(mask&~AllMask))return false;enabled_=mask;return true;}
 bool Finished()const{return report.stage==Stage::Complete||report.stage==Stage::Refused||report.stage==Stage::Indeterminate;}
 template<class Host> bool Step(Host& h,std::uint64_t now){
  if(busy_||Finished()||now<next_)return false;
  struct Guard{bool& flag;Guard(bool& f):flag(f){flag=true;}~Guard(){flag=false;}} guard(busy_);
  if(now>UINT64_MAX-PhaseLimitMs){report.stage=Stage::Refused;report.failure=Failure::Timeout;return true;}
  next_=now+100;
  if(report.stage==Stage::Dormant){
   while(index_<TargetCount&&!(enabled_&(1u<<index_)))++index_;
   report.target=index_;report.stage=index_==TargetCount?Stage::Complete:Stage::Observe;deadline_=now+PhaseLimitMs;
   if(Finished())return true;
  }
  if(now>=deadline_){
   if(report.stage==Stage::Verify){report.failure=Failure::Timeout;report.stage=Stage::Indeterminate;}
   else Refuse(Failure::Timeout,now);return true;
  }
  const auto target=static_cast<Target>(index_);
  if(report.stage==Stage::Observe||report.stage==Stage::Verify){
   Snapshot fresh;const auto status=h.Observe(fresh);
   if(status==Status::Pending){++report.deferred;return true;}
   if(status!=Status::Ready||!Sane(fresh)|| (bound_&&!(fresh.owner==owner_))){
    report.failure=status==Status::Ready?Failure::Owner:Failure::Snapshot;
    report.stage=report.stage==Stage::Verify?Stage::Indeterminate:Stage::Refused;return true;
   }
   if(!bound_){owner_=fresh.owner;bound_=true;}
   if(report.stage==Stage::Verify){
    if(AddedOnlyExpected(before_,fresh,target)){++report.confirmed;report.confirmedMask|=1u<<index_;Advance(now);}
    else if(Same(before_,fresh))Refuse(Failure::NoChange,now);
    else {report.failure=Failure::UnexpectedChange;report.stage=Stage::Indeterminate;}
    return true;
   }
   if(Present(fresh,target)){++report.preserved;report.preservedMask|=1u<<index_;Advance(now);return true;}
   if(fresh.panel.slots.count==64){Refuse(Failure::Snapshot,now);return true;}
   before_=fresh;report.stage=Stage::Prepare;return true;
  }
  if(report.stage==Stage::Prepare){
   const auto status=h.Prepare(owner_,target,finder_);
   if(status==Status::Pending){++report.deferred;return true;}
   if(status!=Status::Ready||!ValidFinder(finder_,before_,target)){Refuse(Failure::Finder,now);return true;}
   report.stage=Stage::Dispatch;return true;
  }
  if(report.stage==Stage::Dispatch){
   bool marked=false;const auto mark=[&](){
    if(marked||report.stage!=Stage::Dispatch||(report.attemptedMask&(1u<<index_)))return false;
    marked=true;++report.attempts;report.attemptedMask|=1u<<index_;report.stage=Stage::Verify;return true;
   };
   const auto status=h.Dispatch(owner_,finder_,before_,mark);
   if(marked){if(status!=Status::Ready){report.failure=Failure::Dispatch;report.stage=Stage::Indeterminate;}else deadline_=now+PhaseLimitMs;return true;}
   if(status==Status::Pending){++report.deferred;return true;}
   if(status==Status::Unavailable){Refuse(Failure::Dispatch,now);return true;}
   report.failure=Failure::DispatchContract;report.stage=Stage::Indeterminate;return true;
  }
  return false;
 }
};
}
