#pragma once
#include "NativeChallengeEscapeScope.hpp"
#include "../RuntimeProbe/RadarWorldBindings.generated.hpp"
#include "../RuntimeProbe/VehicleInteractionBindings.generated.hpp"
#include "../RuntimeProbe/VehicleOccupancyBindings.generated.hpp"
#include "../RuntimeProbe/KillBindings.generated.hpp"
#include <vector>

namespace cfgchallengeescape::actors {
namespace life=cfgprobe::lifetime;
template<class R,class T>bool Same(R& r,std::uintptr_t at,const T& value){T actual{};return r.Read(at,actual)&&actual==value;}
struct List {
 std::uintptr_t data=0;int count=0,capacity=0;
 bool Sane(int maximum)const{return count>=0&&count<=maximum&&capacity>=count&&capacity<=maximum&&
  (!capacity||cfgprobe::ObjectPointer(data))&&(!capacity||life::Extent(data,8ULL*static_cast<unsigned>(capacity)));}
 bool operator==(const List& b)const{return data==b.data&&count==b.count&&capacity==b.capacity;}
};
template<class R>bool Positive(R& r,const life::ArrayHeader& registry,std::uintptr_t object,life::SlotObservation& slot){
 slot={};
 // Two fresh registry observations, including flags in BOTH. Read the exact
 // twenty-byte object/flags/cluster/serial span; cluster confers no authority.
 // This preserves index-before-item ordering and never retains a lifetime pin.
 life::SlotObservation first,again;
 if(!scope::ReadIdentity(r,registry,object,first)||!scope::ReadIdentity(r,registry,object,again)||!(first==again))return false;slot=first;return true;
}
template<class R>bool PositiveCurrent(R& r,const life::ArrayHeader& registry,const life::SlotObservation& expected){
 life::SlotObservation actual;return expected.HasAssignedSerial()&&Positive(r,registry,expected.object,actual)&&actual==expected;
}
template<class R,class Current>bool ScopeCurrent(R& r,std::uintptr_t module,const scope::Witness& w,Current&& callerCurrent){
 if(!module||!w.generation||!w.registry.Sane()||!callerCurrent())return false;
 for(const auto& slot:w.owners)if(!PositiveCurrent(r,w.registry,slot))return false;
 life::ArrayHeader actual;
 return life::ReadArrayHeader(r,module+life::ObjectArrayRva,actual)&&actual==w.registry;
}
struct NativeProof {
 bool valid=false;std::uintptr_t cache=0,ctor=0,type=0,defaults=0;
 life::SlotObservation slot{};cfgprobe::Ancestry ancestry{};
};
template<class R>bool NativeCurrent(R& r,std::uintptr_t module,const scope::Witness& w,const NativeProof& p){
 std::uint32_t flags=0;
 return p.valid&&PositiveCurrent(r,w.registry,p.slot)&&r.Read(p.slot.object+8,flags)&&!(flags&0x30u)&&Same(r,module+p.cache,p.type)&&Same(r,module+p.ctor,p.type)&&
  p.ancestry.Contains(p.type)&&cfgprobe::StableAncestry(r,p.slot.object,p.ancestry)&&
  cfgprobe::ObjectPointer(p.defaults)&&p.defaults!=p.slot.object&&Same(r,p.ancestry.classes[0]+cfgspawn::ClassDefaultObject,p.defaults);
}
template<class R>bool ObserveNative(R& r,std::uintptr_t module,const scope::Witness& w,std::uintptr_t object,
 std::uintptr_t cache,std::uintptr_t ctor,NativeProof& out){
 out={};NativeProof p;p.cache=cache;p.ctor=ctor;std::uintptr_t constructed=0;
 if(!Positive(r,w.registry,object,p.slot)||!r.Read(module+cache,p.type)||!r.Read(module+ctor,constructed)||
  !cfgprobe::MatchingClassCache(p.type,constructed)||!cfgprobe::CaptureAncestry(r,object,p.ancestry)||!p.ancestry.Contains(p.type)||
  !r.Read(p.ancestry.classes[0]+cfgspawn::ClassDefaultObject,p.defaults)||!cfgprobe::ObjectPointer(p.defaults)||p.defaults==object)return false;
 p.valid=true;if(!NativeCurrent(r,module,w,p))return false;out=p;return true;
}
// A Level is a typed strong-link borrow, NEVER an action receiver/weak lease.
// CFG's persistent level has an unassigned serial. Only exact current World ->
// Levels and positive Actor -> Outer/cell links authorize reading it; native
// actors/components continue to require assigned serials. No allocator calls.
struct LevelLinkProof {
 bool valid=false;std::uintptr_t type=0,defaults=0;
 life::SlotObservation slot{};cfgprobe::Ancestry ancestry{};
};
template<class R>bool LevelLinkCurrent(R& r,std::uintptr_t module,const scope::Witness& w,const LevelLinkProof& p){
 life::SlotObservation slot,again;std::uint32_t flags=0,objectFlags=0;
 return p.valid&&PositiveCurrent(r,w.registry,w.owners[0])&&life::ReadSlot(r,w.registry,p.slot.object,slot)&&slot==p.slot&&
  r.Read(w.registry.data+24ULL*static_cast<unsigned>(slot.index)+8,flags)&&!(flags&0x30000000u)&&
  r.Read(slot.object+8,objectFlags)&&!(objectFlags&0x30u)&&
  Same(r,module+cfgradarworld::disk::LevelClassCache,p.type)&&Same(r,module+cfgradarworld::disk::LevelConstructorCache,p.type)&&
  p.ancestry.Contains(p.type)&&cfgprobe::StableAncestry(r,slot.object,p.ancestry)&&
  cfgprobe::ObjectPointer(p.defaults)&&p.defaults!=slot.object&&
  Same(r,p.ancestry.classes[0]+cfgspawn::ClassDefaultObject,p.defaults)&&
  life::ReadSlot(r,w.registry,p.slot.object,again)&&again==slot;
}
template<class R>bool ObserveLevelLink(R& r,std::uintptr_t module,const scope::Witness& w,std::uintptr_t object,LevelLinkProof& out){
 out={};LevelLinkProof p;std::uintptr_t ctor=0;
 if(!life::ReadSlot(r,w.registry,object,p.slot)||!r.Read(module+cfgradarworld::disk::LevelClassCache,p.type)||
  !r.Read(module+cfgradarworld::disk::LevelConstructorCache,ctor)||!cfgprobe::MatchingClassCache(p.type,ctor)||
  !cfgprobe::CaptureAncestry(r,object,p.ancestry)||!p.ancestry.Contains(p.type)||
  !r.Read(p.ancestry.classes[0]+cfgspawn::ClassDefaultObject,p.defaults))return false;
 p.valid=true;if(!LevelLinkCurrent(r,module,w,p))return false;out=p;return true;
}
struct Membership {
 bool valid=false;std::uintptr_t world=0,level=0,levelCell=0,actorCell=0;
 List levels{},actors{};NativeProof actor{};LevelLinkProof levelType{};
 const life::SlotObservation& Slot()const{return actor.slot;}
};
template<class R>bool MembershipCurrent(R& r,std::uintptr_t module,const scope::Witness& w,const Membership& p){
 life::ArrayHeader registry;
 return p.valid&&p.world==w.World()&&PositiveCurrent(r,w.registry,w.owners[0])&&
  life::ReadArrayHeader(r,module+life::ObjectArrayRva,registry)&&registry==w.registry&&
  NativeCurrent(r,module,w,p.actor)&&LevelLinkCurrent(r,module,w,p.levelType)&&
  Same(r,p.actor.slot.object+0x20,p.level)&&Same(r,p.level+cfgkill::disk::OwningWorld,p.world)&&
  Same(r,p.world+cfgradarworld::disk::WorldLevels,p.levels)&&Same(r,p.levelCell,p.level)&&
  Same(r,p.level+cfgradarworld::disk::LevelActors,p.actors)&&Same(r,p.actorCell,p.actor.slot.object)&&
  PositiveCurrent(r,w.registry,w.owners[0])&&PositiveCurrent(r,w.registry,p.actor.slot)&&
  life::ReadArrayHeader(r,module+life::ObjectArrayRva,registry)&&registry==w.registry;
}
// Known actor only: finite world/level rosters, never a global registry search.
// The census establishes a cell once. Action-time rechecks only its owning
// headers/cells/positive slots; unloaded streaming levels cannot retain a lease.
template<class R>bool ObserveMembership(R& r,std::uintptr_t module,const scope::Witness& w,std::uintptr_t actor,Membership& out){
 out={};Membership p;p.world=w.World();
 if(!ObserveNative(r,module,w,actor,disk::Actor,disk::ActorCtor,p.actor)||!r.Read(actor+0x20,p.level)||
  !r.Read(p.world+cfgradarworld::disk::WorldLevels,p.levels)||!p.levels.Sane(128)||!p.levels.count)return false;
 unsigned total=0,matches=0;std::array<std::uintptr_t,128> levels{};std::array<List,128> lists{};
 for(int i=0;i<p.levels.count;++i){const auto cell=p.levels.data+8ULL*static_cast<unsigned>(i);auto& level=levels[i];
  if(!r.Read(cell,level)||!cfgprobe::ObjectPointer(level))return false;
  for(int j=0;j<i;++j)if(levels[j]==level)return false;
  if(!r.Read(level+cfgradarworld::disk::LevelActors,lists[i])||!lists[i].Sane(16000)||
     static_cast<unsigned>(lists[i].count)>16000-total)return false;
  total+=static_cast<unsigned>(lists[i].count);
  if(level==p.level){p.levelCell=cell;p.actors=lists[i];++matches;}
 }
 if(matches!=1||!ObserveLevelLink(r,module,w,p.level,p.levelType)||
  !Same(r,p.level+cfgkill::disk::OwningWorld,p.world))return false;
 matches=0;
 for(int i=0;i<p.actors.count;++i){std::uintptr_t value=0;const auto cell=p.actors.data+8ULL*static_cast<unsigned>(i);
  if(!r.Read(cell,value))return false;if(value==actor){p.actorCell=cell;++matches;}}
 if(matches!=1)return false;
 for(int i=0;i<p.levels.count;++i)if(!Same(r,p.levels.data+8ULL*static_cast<unsigned>(i),levels[i])||
  !Same(r,levels[i]+cfgradarworld::disk::LevelActors,lists[i]))return false;
 p.valid=true;if(!MembershipCurrent(r,module,w,p))return false;out=p;return true;
}
struct Spec {std::u16string_view name,kind;int offset,bytes;unsigned char mask=0;};
struct Member {cfgreflection::Property property{};std::uintptr_t head=0;std::array<unsigned char,4> boolean{};cfgnames::Words kind{};};
template<class R>bool MemberCurrent(R& r,std::uintptr_t module,const NativeProof& native,const Member& p){
 const auto& expected=p.property;const auto& field=expected.field;cfgreflection::Observation<R> reflection(r,module);
 if(!native.ancestry.Contains(expected.field.owner)||!Same(r,expected.field.owner+cfgreflection::disk::Children,p.head)||
  !reflection.CurrentProperty(expected)||!Same(r,field.type+cfgnames::disk::ObjectName,p.kind))return false;
 return field.kind.View()!=u"BoolProperty"||Same(r,field.object+0x70,p.boolean);
}
// Targeted native lookup supports counselor classes with >256 fields. Resolve
// all requested fields in ONE walk, stop once found, and recheck walked links.
// The512 bound is per declaring class, not the entire hierarchy. The paused
// CFG challenge capture has365 counselor fields plus272 character fields
// before the inherited Pawn/Actor ownership fields can be reached.
// Subsequent Current uses these exact metadata objects and full sampled flags.
template<class R,std::size_t N>bool ObserveMembers(R& r,std::uintptr_t module,const NativeProof& native,
 const std::array<Spec,N>& specs,std::array<Member,N>& out){
 out={};std::array<bool,N> found{};unsigned remaining=N;
 constexpr unsigned FieldsPerClass=512,FieldsPerHierarchy=4096;
 std::vector<std::uintptr_t> seen,links;seen.reserve(FieldsPerClass);links.reserve(FieldsPerClass);
 cfgreflection::Observation<R> reflection(r,module);cfgnames::Observation<R> names(r,module);
 for(std::size_t depth=0;depth<native.ancestry.count&&remaining;++depth){const auto owner=native.ancestry.classes[depth];std::uintptr_t head=0,next=0;
  if(!r.Read(owner+cfgreflection::disk::Children,head))return false;next=head;unsigned declared=0;
  while(next&&remaining){if(declared++==FieldsPerClass||seen.size()==FieldsPerHierarchy||!cfgprobe::ObjectPointer(next))return false;
   for(const auto visited:seen)if(visited==next)return false;
   cfgnames::Text label;std::uintptr_t following=0;
   if(!r.Read(next+cfgreflection::disk::Next,following)||!names.Object(next,label))return false;
   seen.push_back(next);links.push_back(following);
   for(unsigned i=0;i<N;++i)if(label.View()==specs[i].name){if(found[i])return false;auto& p=out[i];cfgreflection::Field f;
    if(!reflection.ReadField(next,owner,f)||!cfgmenu::capture::RawProperty(r,f,p.property)||f.kind.View()!=specs[i].kind||
     p.property.dimension!=1||p.property.elementBytes!=specs[i].bytes||p.property.offset!=specs[i].offset)return false;
    p.head=head;if(!r.Read(f.type+cfgnames::disk::ObjectName,p.kind))return false;
    if(specs[i].mask){if(!r.Read(next+0x70,p.boolean)||p.boolean!=std::array<unsigned char,4>{{1,0,1,specs[i].mask}})return false;}
    found[i]=true;--remaining;
   }
   next=following;
  }
  if(!Same(r,owner+cfgreflection::disk::Children,head))return false;
 }
 if(remaining||!cfgprobe::StableAncestry(r,native.slot.object,native.ancestry))return false;
 for(std::size_t i=0;i<seen.size();++i)if(!Same(r,seen[i]+cfgreflection::disk::Next,links[i]))return false;
 for(const auto& p:out)if(!MemberCurrent(r,module,native,p))return false;
 return true;
}
enum class ControllerKind {AI,Human,Unpossessed,Unknown};
struct CounselorOwnershipProof {
 bool valid=false;Membership member{},controllerMember{};NativeProof counselor{},controllerType{};
 Member controllerField{},pawnField{};std::uintptr_t controller=0;
 ControllerKind kind=ControllerKind::Unknown;
 bool HumanOrUnknown()const{return controller&&(kind==ControllerKind::Human||kind==ControllerKind::Unknown);}
};
template<class R,class Current>bool CurrentCounselorOwnership(R& r,std::uintptr_t module,const scope::Witness& w,Current&& current,
 const CounselorOwnershipProof& p){
 if(!p.valid||!ScopeCurrent(r,module,w,current)||!MembershipCurrent(r,module,w,p.member)||
  !NativeCurrent(r,module,w,p.counselor)||!Same(r,p.member.Slot().object+0x110,std::uint8_t{3})||
  !MemberCurrent(r,module,p.counselor,p.controllerField)||!Same(r,p.member.Slot().object+0x3A0,p.controller))return false;
 if(p.controllerType.valid&&(!NativeCurrent(r,module,w,p.controllerType)||!MembershipCurrent(r,module,w,p.controllerMember)))return false;
 if(p.kind==ControllerKind::AI&&(!Same(r,p.controller+0x370,p.member.Slot().object)||!MemberCurrent(r,module,p.controllerType,p.pawnField)))return false;
 return ScopeCurrent(r,module,w,current);
}
template<class R,class Current>bool ObserveCounselorOwnership(R& r,std::uintptr_t module,const scope::Witness& w,Current&& current,
 std::uintptr_t actor,CounselorOwnershipProof& out){
 out={};if(!ScopeCurrent(r,module,w,current))return false;CounselorOwnershipProof p;std::array<Member,1> field{};
 if(!ObserveMembership(r,module,w,actor,p.member)||!ObserveNative(r,module,w,actor,disk::SCCounselorCharacter,disk::SCCounselorCharacterCtor,p.counselor)||
  !Same(r,actor+0x110,std::uint8_t{3})||!ObserveMembers(r,module,p.counselor,std::array<Spec,1>{{{u"Controller",u"ObjectProperty",0x3A0,8}}},field)||
  !r.Read(actor+0x3A0,p.controller))return false;
 p.controllerField=field[0];
 if(!p.controller)p.kind=ControllerKind::Unpossessed;
 else {
  NativeProof control;
  if(ObserveNative(r,module,w,p.controller,disk::AIController,disk::AIControllerCtor,control)&&ObserveMembership(r,module,w,p.controller,p.controllerMember)){
   p.controllerType=control;
   if(Same(r,p.controller+0x370,actor)&&ObserveMembers(r,module,control,std::array<Spec,1>{{{u"Pawn",u"ObjectProperty",0x370,8}}},field)){
    p.kind=ControllerKind::AI;p.pawnField=field[0];
   }
  }else if(ObserveNative(r,module,w,p.controller,disk::PlayerController,disk::PlayerControllerCtor,control)&&ObserveMembership(r,module,w,p.controller,p.controllerMember)){
   p.controllerType=control;p.kind=ControllerKind::Human;
  }
 }
 p.valid=true;if(!CurrentCounselorOwnership(r,module,w,current,p))return false;out=p;return true;
}
struct CounselorProof {
 bool valid=false;Membership member{},controllerMember{};NativeProof counselor{},root{},movement{},controllerType{};
 std::array<Member,8> fields{};std::array<Member,1> pawnField{};
 ControllerKind controllerKind=ControllerKind::Unknown;
 std::uintptr_t controller=0,seat=0,spline=0;std::uint8_t dying=0,escaped=0,dead=0,grabbed=0,leaving=0;
 std::uintptr_t Actor()const{return member.actor.slot.object;}
 bool PreCommit()const{return valid&&!spline&&!dead&&!grabbed&&!escaped&&!(dying&1)&&!leaving;}
 bool OnFoot()const{return PreCommit()&&!seat;}
};
inline constexpr std::array<Spec,8> CounselorSpecs{{
 {u"Controller",u"ObjectProperty",0x3A0,8}, {u"CurrentSeat",u"ObjectProperty",0x15C0,8},
 {u"bHasEscaped",u"BoolProperty",0x1525,1,0xFF}, {u"bIsDying",u"BoolProperty",0xE54,1,1},
 {u"bIsGrabbed",u"BoolProperty",0x161C,1,0xFF}, {u"bLeavingSeat",u"BoolProperty",0x15D4,1,0xFF},
 {u"RootComponent",u"ObjectProperty",0x160,8}, {u"CharacterMovement",u"ObjectProperty",0x3D0,8}
}};
template<class R>bool CounselorLocalCurrent(R& r,std::uintptr_t module,const scope::Witness& w,const CounselorProof& p){
 if(!p.valid||!MembershipCurrent(r,module,w,p.member)||!NativeCurrent(r,module,w,p.counselor)||!NativeCurrent(r,module,w,p.root)||
  !Same(r,p.Actor()+0x110,std::uint8_t{3})||!NativeCurrent(r,module,w,p.movement)||
  !Same(r,p.Actor()+0x3D0,p.movement.slot.object)||!Same(r,p.movement.slot.object+0x20,p.Actor())||
  !Same(r,p.Actor()+0x160,p.root.slot.object)||!Same(r,p.root.slot.object+0x20,p.Actor())||
  !Same(r,p.Actor()+0x3A0,p.controller)||!Same(r,p.Actor()+0x15C0,p.seat)||!Same(r,p.Actor()+0x1518,p.spline)||
  !Same(r,p.Actor()+0x1525,p.escaped)||!Same(r,p.Actor()+0xE54,p.dying)||!Same(r,p.Actor()+0x1070,p.dead)||
  !Same(r,p.Actor()+0x161C,p.grabbed)||!Same(r,p.Actor()+0x15D4,p.leaving))return false;
 for(const auto& field:p.fields)if(!MemberCurrent(r,module,p.counselor,field))return false;
 if(p.controllerType.valid&&(!NativeCurrent(r,module,w,p.controllerType)||!MembershipCurrent(r,module,w,p.controllerMember)))return false;
 if(p.controllerKind==ControllerKind::AI){
  if(!p.controller||!p.controllerType.valid||!Same(r,p.controller+0x370,p.Actor())||!MemberCurrent(r,module,p.controllerType,p.pawnField[0]))return false;
 }
 return true;
}
template<class R,class Current>bool CurrentCounselor(R& r,std::uintptr_t module,const scope::Witness& w,Current&& current,const CounselorProof& p){
 return ScopeCurrent(r,module,w,current)&&CounselorLocalCurrent(r,module,w,p)&&ScopeCurrent(r,module,w,current);
}
template<class R,class ScopeCheck>bool Current(R& r,std::uintptr_t module,const scope::Witness& w,ScopeCheck&& current,const CounselorProof& p){
 return CurrentCounselor(r,module,w,current,p);
}
template<class R,class Current>bool ObserveCounselor(R& r,std::uintptr_t module,const scope::Witness& w,Current&& current,
 std::uintptr_t actor,CounselorProof& out){
 out={};if(!ScopeCurrent(r,module,w,current))return false;CounselorProof p;
 if(!ObserveMembership(r,module,w,actor,p.member)||!ObserveNative(r,module,w,actor,disk::SCCounselorCharacter,disk::SCCounselorCharacterCtor,p.counselor)||
  !Same(r,actor+0x110,std::uint8_t{3})||!ObserveMembers(r,module,p.counselor,CounselorSpecs,p.fields)||!r.Read(actor+0x3A0,p.controller)||
  !r.Read(actor+0x15C0,p.seat)||!r.Read(actor+0x1518,p.spline)||!r.Read(actor+0x1525,p.escaped)||p.escaped>1||
  !r.Read(actor+0xE54,p.dying)||!r.Read(actor+0x1070,p.dead)||p.dead>1||
  !r.Read(actor+0x161C,p.grabbed)||p.grabbed>1||!r.Read(actor+0x15D4,p.leaving)||p.leaving>1)return false;
 std::uintptr_t root=0,movement=0;
 if(!r.Read(actor+0x160,root)||!ObserveNative(r,module,w,root,disk::PrimitiveComponent,disk::PrimitiveComponentCtor,p.root)||!Same(r,root+0x20,actor)||
  !r.Read(actor+0x3D0,movement)||!ObserveNative(r,module,w,movement,disk::MovementComponent,disk::MovementComponentCtor,p.movement)||
  !Same(r,movement+0x20,actor))return false;
 if(!p.controller)p.controllerKind=ControllerKind::Unpossessed;
 else {
  // Non-null unfamiliar/malformed controllers never become fresh AI. Human
  // classification survives an eventual null controller in the caller policy.
  NativeProof control;
  if(ObserveNative(r,module,w,p.controller,disk::AIController,disk::AIControllerCtor,control)&&
     ObserveMembership(r,module,w,p.controller,p.controllerMember)){
   p.controllerType=control;
   if(Same(r,p.controller+0x370,actor)&&ObserveMembers(r,module,control,std::array<Spec,1>{{{u"Pawn",u"ObjectProperty",0x370,8}}},p.pawnField))
    p.controllerKind=ControllerKind::AI;
  }else if(ObserveNative(r,module,w,p.controller,disk::PlayerController,disk::PlayerControllerCtor,control)&&
           ObserveMembership(r,module,w,p.controller,p.controllerMember)){
   p.controllerType=control;p.controllerKind=ControllerKind::Human;
  }
 }
 p.valid=true;if(!CurrentCounselor(r,module,w,current,p))return false;out=p;return true;
}
struct VolumeProof {bool valid=false;Membership member{};NativeProof volume{},sphere{};std::array<Member,1> fields{};};
template<class R,class Current>bool CurrentVolume(R& r,std::uintptr_t module,const scope::Witness& w,Current&& current,const VolumeProof& p){
 return p.valid&&ScopeCurrent(r,module,w,current)&&MembershipCurrent(r,module,w,p.member)&&NativeCurrent(r,module,w,p.volume)&&
  Same(r,p.volume.slot.object+0x110,std::uint8_t{3})&&
  NativeCurrent(r,module,w,p.sphere)&&MemberCurrent(r,module,p.volume,p.fields[0])&&
  Same(r,p.volume.slot.object+0x368,p.sphere.slot.object)&&Same(r,p.sphere.slot.object+0x20,p.volume.slot.object)&&ScopeCurrent(r,module,w,current);
}
template<class R,class Current>bool ObserveVolume(R& r,std::uintptr_t module,const scope::Witness& w,Current&& current,
 std::uintptr_t actor,VolumeProof& out){out={};if(!ScopeCurrent(r,module,w,current))return false;VolumeProof p;std::uintptr_t sphere=0;
 if(!ObserveMembership(r,module,w,actor,p.member)||!ObserveNative(r,module,w,actor,disk::SCEscapeVolume,disk::SCEscapeVolumeCtor,p.volume)||
  !Same(r,actor+0x110,std::uint8_t{3})||
  !ObserveMembers(r,module,p.volume,std::array<Spec,1>{{{u"EscapeVolume",u"ObjectProperty",0x368,8}}},p.fields)||
  !r.Read(actor+0x368,sphere)||!ObserveNative(r,module,w,sphere,disk::SphereComponent,disk::SphereComponentCtor,p.sphere)||!Same(r,sphere+0x20,actor))return false;
 p.valid=true;if(!CurrentVolume(r,module,w,current,p))return false;out=p;return true;
}
struct SeatProof {NativeProof seat{};std::array<Member,3> fields{};std::uintptr_t cell=0,occupant=0;std::uint8_t driver=0;CounselorProof counselor{};};
struct CarProof {
 bool valid=false;Membership member{};NativeProof car{},root{},movement{};std::array<Member,3> fields{};
 Member seatInner{};List seats{};std::array<SeatProof,16> occupants{};unsigned occupied=0,drivers=0;
 std::uintptr_t Actor()const{return member.actor.slot.object;}
};
inline constexpr std::array<Spec,3> SeatSpecs{{{u"ParentVehicle",u"ObjectProperty",0x548,8},
 {u"CounselorInSeat",u"ObjectProperty",0x500,8},{u"bIsDriverSeat",u"BoolProperty",0x511,1,0xFF}}};
template<class R>bool SeatInnerCurrent(R& r,std::uintptr_t module,const CarProof& p){
 const auto outer=p.fields[2].property.field.object;const auto& expected=p.seatInner.property;
 cfgreflection::Observation<R> reflection(r,module);cfgreflection::Field field;cfgreflection::Property actual;std::uintptr_t type=0,ctor=0;
 return Same(r,outer+0x70,expected.field.object)&&reflection.ReadField(expected.field.object,outer,field)&&
  cfgmenu::capture::RawProperty(r,field,actual)&&cfgmenu::capture::SameProperty(expected,actual)&&field.kind.View()==u"ObjectProperty"&&
  actual.dimension==1&&actual.elementBytes==8&&!actual.offset&&r.Read(module+disk::SCVehicleSeatComponent,type)&&
  r.Read(module+disk::SCVehicleSeatComponentCtor,ctor)&&cfgprobe::MatchingClassCache(type,ctor)&&Same(r,field.object+0x70,type);
}
// Read-only escape admission can recognize living AI occupants in a scripted
// transition. Physical recovery always retains the full PreCommit restriction.
template<class R,class Current,class Eligible>bool CurrentCar(R& r,std::uintptr_t module,const scope::Witness& w,Current&& current,
 const CarProof& p,Eligible&& eligible,bool recovery=true){
 if(!p.valid||!p.occupied||p.drivers!=1||!ScopeCurrent(r,module,w,current)||!MembershipCurrent(r,module,w,p.member)||
  !NativeCurrent(r,module,w,p.car)||!NativeCurrent(r,module,w,p.root)||!NativeCurrent(r,module,w,p.movement)||
  !Same(r,p.Actor()+0x110,std::uint8_t{3})||!Same(r,p.Actor()+0x47C,std::uint8_t{})||!Same(r,p.Actor()+0x3D8,std::uint8_t{1})||
  !Same(r,p.Actor()+0x160,p.root.slot.object)||!Same(r,p.root.slot.object+0x20,p.Actor())||
  !Same(r,p.Actor()+0x3D0,p.movement.slot.object)||!Same(r,p.movement.slot.object+0x20,p.Actor())||
  !Same(r,p.Actor()+0x4E8,p.seats)||!p.seats.Sane(16)||!p.seats.count)return false;
 for(const auto& member:p.fields)if(!MemberCurrent(r,module,p.car,member))return false;
 if(!SeatInnerCurrent(r,module,p))return false;unsigned occupied=0,drivers=0;
 for(int i=0;i<p.seats.count;++i){const auto& seat=p.occupants[i];
  if(!Same(r,seat.cell,seat.seat.slot.object)||!NativeCurrent(r,module,w,seat.seat)||
   !Same(r,seat.seat.slot.object+0x20,p.Actor())||!Same(r,seat.seat.slot.object+0x548,p.Actor())||
   !Same(r,seat.seat.slot.object+0x500,seat.occupant)||!Same(r,seat.seat.slot.object+0x511,seat.driver))return false;
  for(const auto& member:seat.fields)if(!MemberCurrent(r,module,seat.seat,member))return false;
  if(!seat.occupant)continue;++occupied;drivers+=seat.driver;
  if(!CounselorLocalCurrent(r,module,w,seat.counselor)||
   (recovery?!seat.counselor.PreCommit():(seat.counselor.dead||(seat.counselor.dying&1)))||seat.counselor.seat!=seat.seat.slot.object||
   seat.counselor.controllerKind==ControllerKind::Human||seat.counselor.controllerKind==ControllerKind::Unknown||!eligible(seat.counselor))return false;
 }
 return occupied==p.occupied&&drivers==1&&ScopeCurrent(r,module,w,current);
}
template<class R,class Current,class Eligible>bool ObserveCar(R& r,std::uintptr_t module,const scope::Witness& w,Current&& current,
 std::uintptr_t actor,Eligible&& eligible,CarProof& out,bool recovery=true){
 out={};if(!ScopeCurrent(r,module,w,current))return false;CarProof p;std::uintptr_t root=0,movement=0;
 if(!ObserveMembership(r,module,w,actor,p.member)||!ObserveNative(r,module,w,actor,disk::SCDriveableVehicle,disk::SCDriveableVehicleCtor,p.car)||
  !Same(r,actor+0x110,std::uint8_t{3})||!Same(r,actor+0x47C,std::uint8_t{})||!Same(r,actor+0x3D8,std::uint8_t{1})||
  !ObserveMembers(r,module,p.car,std::array<Spec,3>{{{u"RootComponent",u"ObjectProperty",0x160,8},
   {u"VehicleMovement",u"ObjectProperty",0x3D0,8},{u"Seats",u"ArrayProperty",0x4E8,16}}},p.fields)||
  !r.Read(actor+0x160,root)||!ObserveNative(r,module,w,root,disk::PrimitiveComponent,disk::PrimitiveComponentCtor,p.root)||!Same(r,root+0x20,actor)||
  !r.Read(actor+0x3D0,movement)||!ObserveNative(r,module,w,movement,disk::WheeledVehicleMovementComponent,disk::WheeledVehicleMovementComponentCtor,p.movement)||
  !Same(r,movement+0x20,actor)||!r.Read(actor+0x4E8,p.seats)||!p.seats.Sane(16)||!p.seats.count)return false;
 cfgreflection::Observation<R> reflection(r,module);cfgreflection::Field inner;std::uintptr_t innerAt=0;
 if(!r.Read(p.fields[2].property.field.object+0x70,innerAt)||!reflection.ReadField(innerAt,p.fields[2].property.field.object,inner)||
  !cfgmenu::capture::RawProperty(r,inner,p.seatInner.property)||!SeatInnerCurrent(r,module,p))return false;
 bool allEligible=true;
 for(int i=0;i<p.seats.count;++i){auto& seat=p.occupants[i];seat.cell=p.seats.data+8ULL*static_cast<unsigned>(i);std::uintptr_t object=0;
  if(!r.Read(seat.cell,object)||!ObserveNative(r,module,w,object,disk::SCVehicleSeatComponent,disk::SCVehicleSeatComponentCtor,seat.seat)||
   !Same(r,object+0x20,actor)||!Same(r,object+0x548,actor)||!ObserveMembers(r,module,seat.seat,SeatSpecs,seat.fields)||
   !r.Read(object+0x500,seat.occupant)||!r.Read(object+0x511,seat.driver)||seat.driver>1)return false;
  for(int j=0;j<i;++j)if(p.occupants[j].seat.slot.object==object)return false;
  if(!seat.occupant)continue;
  if(!ObserveCounselor(r,module,w,current,seat.occupant,seat.counselor))return false;
  // Record a human/unknown takeover before recovery-only precommit tests.
  // Escaping/grabbed humans must invalidate historical AI just as free humans.
  const bool allowed=eligible(seat.counselor);
  if(seat.counselor.seat!=object||
   (recovery?!seat.counselor.PreCommit():(seat.counselor.dead||(seat.counselor.dying&1))))return false;
  for(int j=0;j<i;++j)if(p.occupants[j].occupant==seat.occupant)return false;
  ++p.occupied;p.drivers+=seat.driver;
  // Let policy observe even a human (its irreversible tombstone), but never
  // let that predicate promote Human/Unknown to eligible car ownership.
  if(!allowed||seat.counselor.controllerKind==ControllerKind::Human||seat.counselor.controllerKind==ControllerKind::Unknown)allEligible=false;
 }
 if(!allEligible||!p.occupied||p.drivers!=1){out=p;return false;}
 p.valid=true;if(!CurrentCar(r,module,w,current,p,eligible,recovery))return false;out=p;return true;
}
}
