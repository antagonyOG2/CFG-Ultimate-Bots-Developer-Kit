#pragma once
#include "PrimarySpawnObservation.hpp"
#include "../RuntimeProbe/PostMatchRecoveryBindings.generated.hpp"
#include "../RuntimeProbe/SnapshotBindings.hpp"
#include "../RuntimeProbe/Compatibility/CfgCameraDiagnostic35.generated.hpp"
#include "../Core/WorldRootRetirement.hpp"
namespace cfgworlddiag {
struct Previous {
 std::uint64_t generation=0; // match observation epoch, never an engine serial
 cfgprobe::lifetime::SlotObservation world{},level{},package{};bool checked=false,checkedAtStart=false,recordAttempted=false;
 // PID1692/19256/7576: UWorld and its level have serial0, so the old
 // HasAssignedSerial gate never recorded and the probe silently never ran.
 // Mailbox51: PendingKill and later serial assignment DO NOT mean freed.
 // Diagnostic identity is index -> exact object + unchanged class, NEVER a
 // lifetime proof or permission for gameplay/native dispatch.
 std::uintptr_t worldClass=0,levelClass=0,packageClass=0;
};

// Deliberately separate from cfgspawn::ObjectSlot and every production
// ownership/GC guard. This diagnostic admits PendingKill/Unreachable objects.
template<class R>bool DiagnosticSlot(R& r,const cfgprobe::lifetime::ArrayHeader& objects,
 std::uintptr_t object,cfgprobe::lifetime::SlotObservation& out,std::uint32_t& flags){
 out={};flags=0;cfgprobe::lifetime::SlotObservation s;s.object=object;
 if(!objects.Sane()||!cfgprobe::ObjectPointer(object)||
    !r.Read(object+cfgprobe::lifetime::ObjectIndexOffset,s.index)||s.index<0||s.index>=objects.count)return false;
 const auto at=objects.data+static_cast<std::uintptr_t>(s.index)*cfgprobe::lifetime::ItemBytes;
 std::uintptr_t first=0,last=0;
 if(!r.Read(at+cfgprobe::lifetime::ItemObjectOffset,first)||first!=object||!r.Read(at+8,flags)||
    !r.Read(at+cfgprobe::lifetime::ItemSerialOffset,s.serial)||
    !r.Read(at+cfgprobe::lifetime::ItemObjectOffset,last)||last!=object)return false;
 out=s;return true;
}
template<class R>bool Alive(R& r,const cfgprobe::lifetime::ArrayHeader& objects,
 const cfgprobe::lifetime::SlotObservation& recorded,std::uintptr_t type,std::uint32_t& flags){
 flags=0;if(!objects.Sane()||!cfgprobe::ObjectPointer(recorded.object)||!cfgprobe::ObjectPointer(type)||
    recorded.index<0||recorded.index>=objects.count)return false;
 const auto at=objects.data+static_cast<std::uintptr_t>(recorded.index)*cfgprobe::lifetime::ItemBytes;
 std::uintptr_t first=0,last=0,currentClass=0;
 return r.Read(at+cfgprobe::lifetime::ItemObjectOffset,first)&&first==recorded.object&&
    r.Read(recorded.object+cfgprobe::ObjectClass,currentClass)&&currentClass==type&&r.Read(at+8,flags)&&
    r.Read(at+cfgprobe::lifetime::ItemObjectOffset,last)&&last==recorded.object;
}
template<class R>bool Camera(R& r,const cfgprobe::lifetime::ArrayHeader& objects,std::uintptr_t module,
 std::uintptr_t controller,std::uintptr_t& out){
 out=0;std::uintptr_t camera=0,type=0,ctor=0,again=0;
 cfgprobe::lifetime::SlotObservation first{},last{};
 if(!r.Read(module+cfgcamera::disk::CameraClassCache,type)||!r.Read(module+cfgcamera::disk::CameraConstructorCache,ctor)||
    !cfgprobe::MatchingClassCache(type,ctor)||!r.Read(controller+cfgprobe::ControllerCameraManager,camera)||
    !cfgspawn::ObjectSlot(r,objects,camera,first)||!cfgspawn::ObjectIs(r,camera,type)||
    !r.Read(controller+cfgprobe::ControllerCameraManager,again)||again!=camera||
    !cfgspawn::ObjectSlot(r,objects,camera,last)||!(first==last))return false;
 out=camera;return true;
}
inline void BeginRecord(Previous& out)noexcept{
 const auto generation=out.generation;out={};out.generation=generation;out.recordAttempted=true;
}
template<class R>bool Record(R& r,const cfgprobe::lifetime::ArrayHeader& objects,std::uintptr_t world,Previous& out,
 std::uintptr_t packageType){
 BeginRecord(out);Previous p;p.recordAttempted=true;
 if(out.generation==UINT64_MAX)return false;p.generation=out.generation+1;
 std::uintptr_t level=0,package=0,outer=1;std::uint32_t flags=0;
 if(!world||!cfgprobe::ObjectPointer(packageType)||!DiagnosticSlot(r,objects,world,p.world,flags)||
    !r.Read(world+cfgprobe::ObjectClass,p.worldClass)||!p.worldClass||
    !r.Read(world+cfgoutro::disk::PersistentLevel,level)||!level||!DiagnosticSlot(r,objects,level,p.level,flags)||
    !r.Read(level+cfgprobe::ObjectClass,p.levelClass)||!p.levelClass||
    !r.Read(world+0x20,package)||package==world||!DiagnosticSlot(r,objects,package,p.package,flags)||
    !r.Read(package+cfgprobe::ObjectClass,p.packageClass)||p.packageClass!=packageType||
    !r.Read(package+0x20,outer)||outer)return false;
 out=p;return true;
}
// Compatibility name for diagnostic callers only; serial is NOT identity here.
template<class R>bool StillCurrent(R& r,const cfgprobe::lifetime::ArrayHeader& objects,const cfgprobe::lifetime::SlotObservation& slot,
 std::uintptr_t type){
 std::uint32_t flags=0;return Alive(r,objects,slot,type,flags);
}
// Read-only evidence of CURRENT Outer ancestry, never a pin or acquisition.
template<class R>bool Reaches(R& r,const cfgprobe::lifetime::ArrayHeader& objects,std::uintptr_t object,const Previous& p){
 std::array<std::uintptr_t,16> seen{};cfgprobe::lifetime::SlotObservation current{},again{};std::uint32_t flags=0;
 for(unsigned depth=0;object&&depth<seen.size();++depth){
  if(!DiagnosticSlot(r,objects,object,current,flags))return false;
  const auto target=[&](const cfgprobe::lifetime::SlotObservation& saved,std::uintptr_t type){
   return current.object==saved.object&&current.index==saved.index&&Alive(r,objects,saved,type,flags);};
  if(target(p.world,p.worldClass)||target(p.level,p.levelClass)||target(p.package,p.packageClass))return true;
  for(unsigned i=0;i<depth;++i)if(seen[i]==object)return false;
  seen[depth]=object;std::uintptr_t outer=0;
  if(!r.Read(object+0x20,outer)||!DiagnosticSlot(r,objects,object,again,flags)||again.object!=current.object||again.index!=current.index)return false;
  object=outer;
 }
 return false;
}
template<class R>cfgretain::MapReference ClassifyMapReference(R& r,const cfgprobe::lifetime::ArrayHeader& objects,
 std::uintptr_t object,const Previous& p){
 std::array<std::uintptr_t,16> seen{};cfgprobe::lifetime::SlotObservation first{},again{};std::uint32_t flags=0;
 for(unsigned depth=0;object&&depth<seen.size();++depth){
  if(!DiagnosticSlot(r,objects,object,first,flags))return cfgretain::MapReference::Unproven;
  const auto target=[&](const auto& slot,std::uintptr_t type){return first.object==slot.object&&first.index==slot.index&&Alive(r,objects,slot,type,flags);};
  if(target(p.world,p.worldClass)||target(p.level,p.levelClass)||target(p.package,p.packageClass))return cfgretain::MapReference::OldMap;
  for(unsigned i=0;i<depth;++i)if(seen[i]==object)return cfgretain::MapReference::Unproven;
  seen[depth]=object;std::uintptr_t outer=0;
  if(!r.Read(object+0x20,outer)||!DiagnosticSlot(r,objects,object,again,flags)||again.object!=first.object||again.index!=first.index)
   return cfgretain::MapReference::Unproven;
  object=outer;
 }
 return object?cfgretain::MapReference::Unproven:cfgretain::MapReference::Unrelated;
}
// Affirmative slot absence/replacement, not a failed read, PendingKill flag or
// later serial assignment. This grants no object access/native-call authority.
template<class R>bool NoLongerCurrent(R& r,const cfgprobe::lifetime::ArrayHeader& objects,const Previous& p){
 if(!objects.Sane()||p.world.index<0||p.world.index>=objects.count||!p.world.object)return false;
 const auto at=objects.data+static_cast<std::uintptr_t>(p.world.index)*cfgprobe::lifetime::ItemBytes;
 std::uintptr_t first=0,again=0;
 return r.Read(at+cfgprobe::lifetime::ItemObjectOffset,first)&&first!=p.world.object&&
  r.Read(at+cfgprobe::lifetime::ItemObjectOffset,again)&&again==first;
}
template<class R,class Emit>unsigned ObserveAtLoad(R& r,const cfgprobe::lifetime::ArrayHeader& objects,const Previous& p,Emit emit){
 std::uint32_t flags=0;const unsigned observed=NoLongerCurrent(r,objects,p)?2u:Alive(r,objects,p.world,p.worldClass,flags)?1u:3u;
 emit(observed);return observed; // never grants or denies the load
}
}
