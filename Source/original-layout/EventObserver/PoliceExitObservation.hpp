#pragma once
#include "PrimarySpawnObservation.hpp"
#include "../RuntimeProbe/SnapshotBindings.hpp"
#include "../RuntimeProbe/PostMatchRecoveryBindings.generated.hpp"
#include "../RuntimeProbe/Compatibility/CfgPoliceExit35.generated.hpp"
#include "../Core/JasonEscapeGuardPolicy.hpp"
namespace cfgpoliceexit {
namespace disk=cfgcompat::cfg35::policeexit;
using Slot=cfgprobe::lifetime::SlotObservation;
struct List {
 std::uintptr_t data=0;int count=0,capacity=0;
 bool operator==(const List& b)const{return data==b.data&&count==b.count&&capacity==b.capacity;}
 bool Sane()const{return count>=0&&count<=16&&capacity>=count&&capacity<=16&&(!capacity||cfgprobe::ObjectPointer(data));}
};
struct Entry {Slot volume{},sphere{};cfgspawn::Vec3 at{};std::uint64_t sampledAt=0;bool active=false;unsigned sourceIndex=0;};
struct Cache {
 std::array<Entry,cfgescapeguard::MaximumExits> entries{};unsigned count=0,cursor=0,probe=0;
 List source{};std::uintptr_t world=0,mode=0;std::uint64_t next=0;bool complete=false,policeCalled=false;
 void Clear(){*this={};}
};
struct Person {Slot pawn{};cfgspawn::Vec3 at{};std::uint64_t sampledAt=0;};
struct Runtime {
 Cache cache;cfgescapeguard::Guard guard;
 std::array<Person,cfgescapeguard::MaximumCounselors> people{};
 cfgspawn::Vec3 goal{};std::uintptr_t target=0;unsigned exit=0,traces=0;
 std::uint64_t moveAt=0;bool morphPending=false,followFallback=false;
 void ClearOwnership(){guard.Clear();morphPending=false;target=0;}
 void Clear(){*this={};}
};
inline bool HuntAvailable(bool carEpisode,bool carStop,bool carSpeed,bool trap,bool hiding,bool knife,bool door,bool kill,bool death,bool complete){
 return !carEpisode&&!carStop&&!carSpeed&&!trap&&!hiding&&!knife&&!door&&!kill&&!death&&!complete;
}
inline bool CombatOwns(float squared,std::uint64_t now,std::uint64_t meleeUntil,std::uintptr_t grabbed,bool kill){
 return squared<=400.f*400.f||now<meleeUntil||grabbed||kill;
}
template<class R,class T>bool Same(R& r,std::uintptr_t address,const T& expected){T value{};return r.Read(address,value)&&value==expected;}
template<class R>bool Pair(R& r,std::uintptr_t module,std::uintptr_t field,std::uintptr_t& value){
 std::uintptr_t ctor=0;return r.Read(module+field,value)&&r.Read(module+field+8,ctor)&&cfgprobe::MatchingClassCache(value,ctor);
}
template<class R>bool Current(R& r,const cfgprobe::lifetime::ArrayHeader& objects,std::uintptr_t module,
 std::uintptr_t world,std::uintptr_t mode,const List& list,const Entry& e,bool objective=false){
 Slot fresh{};std::uintptr_t volumeClass=0,sphereClass=0,level=0;std::uint8_t kind=255;
 return e.volume.HasAssignedSerial()&&e.sourceIndex<static_cast<unsigned>(list.count)&&list.Sane()&&
  Pair(r,module,disk::EscapeClassReflection,volumeClass)&&Pair(r,module,disk::SphereClassReflection,sphereClass)&&
  cfgspawn::ObjectSlot(r,objects,e.volume.object,fresh)&&fresh==e.volume&&
  cfgspawn::ObjectIs(r,e.volume.object,volumeClass)&&
  r.Read(e.volume.object+0x20,level)&&cfgspawn::ObjectSlot(r,objects,level,fresh)&&
  Same(r,level+cfgoutro::disk::OwningWorld,world)&&
  Same(r,world+cfgprobe::WorldGameMode,mode)&&Same(r,mode+disk::ModeVolumes,list)&&
  Same(r,list.data+8ULL*e.sourceIndex,e.volume.object)&&
  r.Read(e.volume.object+disk::EscapeType,kind)&&(objective?(kind>=1&&kind<=4):kind==0)&&
  Same(r,e.volume.object+disk::EscapeSphere,e.sphere.object)&&
  cfgspawn::ObjectSlot(r,objects,e.sphere.object,fresh)&&fresh==e.sphere&&
  cfgspawn::ObjectIs(r,e.sphere.object,sphereClass)&&Same(r,e.sphere.object+0x20,e.volume.object)&&
  cfgspawn::ObjectSlot(r,objects,e.volume.object,fresh)&&fresh==e.volume;
}
// One volume location or one active query per callback. This observer supplies
// no native authority: the host retains its usual callback/context/call gates.
template<class R,class H>bool Step(R& r,H& host,const cfgprobe::lifetime::ArrayHeader& objects,
 std::uintptr_t module,std::uintptr_t world,std::uintptr_t mode,std::uintptr_t state,Cache& c,std::uint64_t now){
 int timer=1;if(!r.Read(state+disk::PoliceTimeRemaining,timer))return false;
 if(timer>0){c.policeCalled=true;for(auto& e:c.entries)e.active=false;return false;}
 if(!c.policeCalled||now<c.next)return false;
 if(!host.Current())return false;
 List list{};
 if(!r.Read(mode+disk::ModeVolumes,list)||!list.Sane()){c.Clear();c.policeCalled=true;c.next=now+1000;return true;}
 if(c.world!=world||c.mode!=mode||!(c.source==list)){c.Clear();c.policeCalled=true;c.world=world;c.mode=mode;c.source=list;}
 c.next=now+(c.complete?1000u:100u);
 if(!c.complete){
  if(c.cursor>=static_cast<unsigned>(list.count)||c.count==c.entries.size()){c.complete=true;c.next=now;return true;}
  const auto index=c.cursor++;Entry e;e.sourceIndex=index;std::uintptr_t actor=0,type=0;std::uint8_t kind=255;
  if(!r.Read(list.data+8ULL*index,actor)||!actor||!Pair(r,module,disk::EscapeClassReflection,type)||
     !cfgspawn::ObjectIs(r,actor,type)||!r.Read(actor+disk::EscapeType,kind)||kind!=0)return true;
  if(!cfgspawn::ObjectSlot(r,objects,actor,e.volume)||!r.Read(actor+disk::EscapeSphere,actor)||
     !cfgspawn::ObjectSlot(r,objects,actor,e.sphere)||!Current(r,objects,module,world,mode,list,e))return true;
  if(!host.Location(e.volume.object,e.at)||!cfgspawn::Finite(e.at)||!host.Current()||
     !Current(r,objects,module,world,mode,list,e))return true;
  for(unsigned i=0;i<c.count;++i)if(c.entries[i].volume.object==e.volume.object)return true;
  c.entries[c.count++]=e;return true;
 }
 if(!c.count)return true;
 auto& e=c.entries[c.probe++%c.count];e.active=false;e.sampledAt=0;
 if(!Current(r,objects,module,world,mode,list,e)){c.Clear();c.policeCalled=true;c.next=now+1000;return true;}
 std::uint8_t enabled=255;
 if(!host.Collision(e.sphere.object,enabled)||enabled>3||!host.Current()||
    !Current(r,objects,module,world,mode,list,e))return true;
 e.active=enabled!=0;e.sampledAt=now;host.Observed(e);return true;
}
}
