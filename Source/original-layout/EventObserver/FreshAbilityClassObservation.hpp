#pragma once
#include "../RuntimeProbe/AiResourceSnapshot.hpp"
#include "PrimarySpawnObservation.hpp"
#include "../RuntimeProbe/SnapshotBindings.hpp"
#include "../RuntimeProbe/FeatureSnapshotBindings.generated.hpp"
#include "../RuntimeProbe/LoadedCounselorBindings.generated.hpp"
#include "../RuntimeProbe/KillBindings.generated.hpp"
#include "PostMatchRecoveryObservation.hpp"
#include "../RuntimeProbe/Compatibility/CfgSweaterWorldSettings35.generated.hpp"
namespace cfgsweater {
// Read-only evidence, NOT a ClassLease authority. A current sweater CDO may
// contain an independent ability template, but that must be observed in CFG
// before adding any constructor path. No scan, rooting or serial allocation.
struct TemplateEvidence {
 unsigned stage=0;std::uintptr_t sourceClass=0,sourceDefault=0,abilityTemplate=0,abilityClass=0;
 int sourceSerial=0,templateSerial=0;
};
template<class R>bool TemplateSame(R& r,std::uintptr_t at,std::uintptr_t expected){
 std::uintptr_t value=0;return r.Read(at,value)&&value==expected;
}
template<class R>TemplateEvidence ObserveTemplate(R& r,const cfgprobe::lifetime::ArrayHeader& objects,
 const cfgprobe::lifetime::SlotObservation& source,std::uintptr_t earnedClass){
 TemplateEvidence out;cfgprobe::lifetime::SlotObservation fresh{},defaultSlot{},templateSlot{};
 out.stage=1;out.sourceSerial=source.serial;
 if(!source.object||!cfgspawn::ObjectSlot(r,objects,source.object,fresh)||!(fresh==source))return out;
 out.stage=2;
 if(!r.Read(source.object+cfgprobe::ObjectClass,out.sourceClass)||
    !r.Read(out.sourceClass+cfgspawn::ClassDefaultObject,out.sourceDefault)||
    !cfgspawn::ObjectSlot(r,objects,out.sourceDefault,defaultSlot)||
    !TemplateSame(r,out.sourceDefault+cfgprobe::ObjectClass,out.sourceClass))return out;
 out.stage=3;
 if(!r.Read(out.sourceDefault+cfgfeature::disk::SweaterAbility,out.abilityTemplate)||!out.abilityTemplate||
    !cfgspawn::ObjectSlot(r,objects,out.abilityTemplate,templateSlot))return out;
 out.templateSerial=templateSlot.serial;
 if(!r.Read(out.abilityTemplate+cfgprobe::ObjectClass,out.abilityClass))return out;
 out.stage=4;
 if(out.abilityClass!=earnedClass||
    !TemplateSame(r,out.abilityTemplate+0x20,out.sourceDefault)||
    !TemplateSame(r,out.sourceDefault+cfgfeature::disk::SweaterAbility,out.abilityTemplate)||
    !cfgspawn::ObjectSlot(r,objects,source.object,fresh)||!(fresh==source))return out;
 out.stage=5;return out;
}
// Retain an EARNED ability's exact loaded class, never the consumed instance.
// A positive class/CDO serial or the native registration's two strong cache
// cells is mandatory. A serial-backed CDO is an identity anchor, NOT a GC pin.
// There is no serial allocation, root mutation or object scan.
struct ClassLease {
 cfgprobe::lifetime::SlotObservation slot{},defaultSlot{},earnedSlot{},earnedOwner{};std::uintptr_t defaults=0,native=0;
 bool nativeRegistered=false;
 struct WorldSource {std::uintptr_t world=0,state=0;unsigned index=0;
  cfgprobe::lifetime::SlotObservation source{},ability{};} worldSource;
 unsigned failure=0,anchor=0;int observedSerial=0,observedDefaultSerial=0;
 struct SpawnList {std::uintptr_t data=0;int count=0,capacity=0;
  bool operator==(const SpawnList& b)const{return data==b.data&&count==b.count&&capacity==b.capacity;}
  bool Sane()const{return count>=0&&count<=64&&capacity>=count&&capacity<=65536&&(!capacity||cfgprobe::ObjectPointer(data));}};
 struct SettingsSource {std::uintptr_t world=0,actorClass=0;cfgoutro::Owner owner{};
  std::array<SpawnList,2> lists{};unsigned which=0,index=0;
  cfgprobe::lifetime::SlotObservation actorDefault{};} settingsSource;
 std::uint32_t observedClassFlags=0,observedDefaultFlags=0;
 template<class R>bool SettingsCurrent(R& r,const cfgprobe::lifetime::ArrayHeader& objects,std::uintptr_t module){
  namespace d=cfgcompat::cfg35::sweaterworldsettings;cfgoutro::Owner freshOwner{};
  cfgprobe::lifetime::SlotObservation fresh{};SpawnList list{};std::uintptr_t nativeSweater=0,ctor=0,defaultsNow=0;
  const auto& binding=cfgfeature::disk::Classes[cfgfeature::disk::Sweater];const auto& s=settingsSource;
  if(!s.world||s.which>1||!s.lists[s.which].Sane()||s.index>=static_cast<unsigned>(s.lists[s.which].count)||
     !cfgprimary::LookupSafe(r,module)||!cfgoutro::ObserveOwner(r,objects,module,s.world,freshOwner)||
     !(freshOwner.level==s.owner.level)||!(freshOwner.settings==s.owner.settings)||freshOwner.type!=s.owner.type||
     !r.Read(module+binding.reflection,nativeSweater)||!r.Read(module+binding.constructor,ctor)||
     !cfgprobe::MatchingClassCache(nativeSweater,ctor)||!cfgspawn::ClassContains(r,s.actorClass,nativeSweater))return false;
  for(unsigned i=0;i<2;++i)if(!r.Read(s.owner.settings.object+(i?d::SmallItemSpawnData:d::LargeItemSpawnData),list)||!(list==s.lists[i]))return false;
  if(!TemplateSame(r,s.lists[s.which].data+d::ElementBytes*s.index,s.actorClass)||
     !r.Read(s.actorClass+cfgspawn::ClassDefaultObject,defaultsNow)||defaultsNow!=s.actorDefault.object||
     !cfgspawn::ObjectSlot(r,objects,defaultsNow,fresh)||!(fresh==s.actorDefault)||
     !TemplateSame(r,defaultsNow+cfgprobe::ObjectClass,s.actorClass)||
     !TemplateSame(r,defaultsNow+cfgfeature::disk::SweaterAbilityClass,slot.object)||
     !TemplateSame(r,s.lists[s.which].data+d::ElementBytes*s.index,s.actorClass))return false;
  return cfgoutro::ObserveOwner(r,objects,module,s.world,freshOwner)&&freshOwner.level==s.owner.level&&
   freshOwner.settings==s.owner.settings&&cfgprimary::LookupSafe(r,module);
 }
 // Four declared classes per callback; never a global registry/actor scan.
 // The class chain survives native retirement of the worn item and ability.
 template<class R>bool CaptureSettings(R& r,const cfgprobe::lifetime::ArrayHeader& objects,
     std::uintptr_t module,std::uintptr_t world,unsigned& cursor){
  namespace d=cfgcompat::cfg35::sweaterworldsettings;SettingsSource source;source.world=world;
  failure=30;if(!cfgprimary::LookupSafe(r,module)||!cfgoutro::ObserveOwner(r,objects,module,world,source.owner))return false;
  for(unsigned i=0;i<2;++i)if(!r.Read(source.owner.settings.object+(i?d::SmallItemSpawnData:d::LargeItemSpawnData),source.lists[i])||!source.lists[i].Sane())return false;
  std::uintptr_t nativeSweater=0,ctor=0;const auto& sweater=cfgfeature::disk::Classes[cfgfeature::disk::Sweater];
  if(!r.Read(module+sweater.reflection,nativeSweater)||!r.Read(module+sweater.constructor,ctor)||!cfgprobe::MatchingClassCache(nativeSweater,ctor))return false;
  const auto total=static_cast<unsigned>(source.lists[0].count+source.lists[1].count);
  for(unsigned n=0;n<4&&cursor<total;++n,++cursor){
   source.which=cursor<static_cast<unsigned>(source.lists[0].count)?0u:1u;
   source.index=source.which?cursor-static_cast<unsigned>(source.lists[0].count):cursor;
   if(!r.Read(source.lists[source.which].data+d::ElementBytes*source.index,source.actorClass))return false;
   if(!source.actorClass||!cfgspawn::ClassContains(r,source.actorClass,nativeSweater))continue;
   ClassLease candidate;candidate.settingsSource=source;candidate.anchor=9;std::uintptr_t type=0,a=0,b=0,actorDefault=0;
   const auto& ability=cfgfeature::disk::Classes[cfgfeature::disk::Ability];
   if(!r.Read(source.actorClass+cfgspawn::ClassDefaultObject,actorDefault)||
      !cfgspawn::ObjectSlot(r,objects,actorDefault,candidate.settingsSource.actorDefault)||
      !TemplateSame(r,actorDefault+cfgprobe::ObjectClass,source.actorClass)||
      !r.Read(actorDefault+cfgfeature::disk::SweaterAbilityClass,type)||!type||
      !r.Read(module+ability.reflection,a)||!r.Read(module+ability.constructor,b)||
      !(candidate.native=cfgprobe::MatchingClassCache(a,b))||!cfgspawn::ClassContains(r,type,candidate.native)||
      !cfgspawn::ObjectSlot(r,objects,type,candidate.slot)||!r.Read(type+cfgspawn::ClassDefaultObject,candidate.defaults)||
      !cfgspawn::ObjectSlot(r,objects,candidate.defaults,candidate.defaultSlot)||!candidate.Current(r,objects,module))return false;
   *this=candidate;return true;
  }
  failure=cursor>=total?31u:32u;return false; // absent declaration vs paced continuation
 }
 // Native SCPamelaSweater.SweaterAbilityClass (+590), unlike +598 instance,
 // remains a declaration even when a consumed instance has been retired.
 // Same CURRENT world-item/level/class/registry proof, no GC/root mutation.
 template<class R>bool DeclaredWorldCurrent(R& r,const cfgprobe::lifetime::ArrayHeader& objects,std::uintptr_t module){
  struct List{std::uintptr_t data=0;int count=0,capacity=0;
   bool operator==(const List& b)const{return data==b.data&&count==b.count&&capacity==b.capacity;}} list{},again{};
  cfgprobe::lifetime::SlotObservation item{},last{},levelSlot{};std::uintptr_t type=0,ctor=0,level=0,levelType=0;std::uint32_t flags=0;
  const auto& binding=cfgfeature::disk::Classes[cfgfeature::disk::Sweater];
  return worldSource.world&&worldSource.state&&worldSource.source.object&&
   cfgprimary::LookupSafe(r,module)&&
   TemplateSame(r,worldSource.world+cfgprobe::WorldGameState,worldSource.state)&&
   r.Read(worldSource.state+cfgfeature::disk::WorldItems,list)&&list.count>0&&list.count<=256&&
   list.capacity>=list.count&&list.capacity<=65536&&worldSource.index<static_cast<unsigned>(list.count)&&cfgprobe::ObjectPointer(list.data)&&
   TemplateSame(r,list.data+8*worldSource.index,worldSource.source.object)&&
   r.Read(module+binding.reflection,type)&&r.Read(module+binding.constructor,ctor)&&type&&type==ctor&&
   cfgspawn::ObjectIs(r,worldSource.source.object,type)&&cfgspawn::ObjectSlot(r,objects,worldSource.source.object,item)&&item==worldSource.source&&
   r.Read(objects.data+static_cast<std::uintptr_t>(item.index)*24+8,flags)&&!(flags&cfgmenu::loadeddisk::InvalidItemFlags)&&
   r.Read(item.object+0x20,level)&&level&&TemplateSame(r,level+cfgkill::disk::OwningWorld,worldSource.world)&&
   r.Read(module+cfgkill::disk::LevelReflection,levelType)&&r.Read(module+cfgkill::disk::LevelConstructor,ctor)&&levelType&&levelType==ctor&&
   cfgspawn::ObjectIs(r,level,levelType)&&cfgspawn::ObjectSlot(r,objects,level,levelSlot)&&
   r.Read(objects.data+static_cast<std::uintptr_t>(levelSlot.index)*24+8,flags)&&!(flags&cfgmenu::loadeddisk::InvalidItemFlags)&&
   TemplateSame(r,item.object+cfgfeature::disk::SweaterAbilityClass,slot.object)&&
   r.Read(worldSource.state+cfgfeature::disk::WorldItems,again)&&again==list&&
   TemplateSame(r,list.data+8*worldSource.index,item.object)&&TemplateSame(r,item.object+cfgfeature::disk::SweaterAbilityClass,slot.object)&&
   cfgspawn::ObjectSlot(r,objects,item.object,last)&&last==item&&TemplateSame(r,item.object+0x20,level)&&
   r.Read(objects.data+static_cast<std::uintptr_t>(item.index)*24+8,flags)&&!(flags&cfgmenu::loadeddisk::InvalidItemFlags)&&
   TemplateSame(r,level+cfgkill::disk::OwningWorld,worldSource.world)&&
   cfgspawn::ObjectSlot(r,objects,level,last)&&last==levelSlot&&cfgspawn::ObjectIs(r,level,levelType)&&
   TemplateSame(r,worldSource.world+cfgprobe::WorldGameState,worldSource.state)&&
   cfgprimary::LookupSafe(r,module);
 }
 // Independent current world-item->typed sweater->ability->class edge.
 // Class evidence only: never transfers/uses another wearer's ability.
 template<class R>bool StrongWorldCurrent(R& r,const cfgprobe::lifetime::ArrayHeader& objects,std::uintptr_t module){
  struct List {std::uintptr_t data=0;int count=0,capacity=0;
   bool operator==(const List& b)const{return data==b.data&&count==b.count&&capacity==b.capacity;}} list{},again{};
  cfgprobe::lifetime::SlotObservation source{},ability{},last{};
  std::uintptr_t item=0,value=0,type=0,ctor=0,level=0,world=0;std::uint32_t flags=0;
  const auto& sweater=cfgfeature::disk::Classes[cfgfeature::disk::Sweater];
  return worldSource.world&&worldSource.state&&worldSource.source.object&&worldSource.ability.object&&
   cfgprimary::LookupSafe(r,module)&&
   TemplateSame(r,worldSource.world+cfgprobe::WorldGameState,worldSource.state)&&
   r.Read(worldSource.state+cfgfeature::disk::WorldItems,list)&&list.count>0&&list.count<=256&&
   list.capacity>=list.count&&list.capacity<=65536&&worldSource.index<static_cast<unsigned>(list.count)&&cfgprobe::ObjectPointer(list.data)&&
   r.Read(list.data+8*worldSource.index,item)&&item==worldSource.source.object&&
   r.Read(module+sweater.reflection,type)&&r.Read(module+sweater.constructor,ctor)&&type&&type==ctor&&
   cfgspawn::ObjectIs(r,item,type)&&cfgspawn::ObjectSlot(r,objects,item,source)&&source==worldSource.source&&
   r.Read(objects.data+static_cast<std::uintptr_t>(source.index)*24+8,flags)&&!(flags&cfgmenu::loadeddisk::InvalidItemFlags)&&
   r.Read(item+0x20,level)&&level&&r.Read(level+cfgkill::disk::OwningWorld,world)&&world==worldSource.world&&
   r.Read(item+cfgfeature::disk::SweaterAbility,value)&&value==worldSource.ability.object&&value!=defaults&&
   cfgspawn::ObjectSlot(r,objects,value,ability)&&ability==worldSource.ability&&
   r.Read(objects.data+static_cast<std::uintptr_t>(ability.index)*24+8,flags)&&!(flags&cfgmenu::loadeddisk::InvalidItemFlags)&&
   TemplateSame(r,value+cfgprobe::ObjectClass,slot.object)&&
   r.Read(worldSource.state+cfgfeature::disk::WorldItems,again)&&again==list&&
   TemplateSame(r,list.data+8*worldSource.index,item)&&TemplateSame(r,item+cfgfeature::disk::SweaterAbility,value)&&
   cfgspawn::ObjectSlot(r,objects,item,last)&&last==source&&cfgspawn::ObjectSlot(r,objects,value,last)&&last==ability&&
   TemplateSame(r,item+0x20,level)&&TemplateSame(r,level+cfgkill::disk::OwningWorld,worldSource.world)&&
   TemplateSame(r,worldSource.world+cfgprobe::WorldGameState,worldSource.state)&&
   cfgprimary::LookupSafe(r,module);
 }
 template<class R>bool EarnedCurrent(R& r,const cfgprobe::lifetime::ArrayHeader& objects){
  cfgprobe::lifetime::SlotObservation fresh{};std::uintptr_t type=0;std::uint32_t flags=0;
  return earnedSlot.HasAssignedSerial()&&cfgspawn::ObjectSlot(r,objects,earnedSlot.object,fresh)&&fresh==earnedSlot&&
   r.Read(objects.data+static_cast<std::uintptr_t>(fresh.index)*24+8,flags)&&!(flags&cfgmenu::loadeddisk::InvalidItemFlags)&&
   r.Read(earnedSlot.object+cfgprobe::ObjectClass,type)&&type==slot.object&&
   cfgspawn::ObjectSlot(r,objects,earnedSlot.object,fresh)&&fresh==earnedSlot;
 }
 // A zero-serial ability is NOT independently retained. Its exact typed
 // reciprocal strong edge from a positively identified original wearer must
 // still exist in THIS no-GC owner-thread callback. This anchor may survive
 // Tommy promotion, but expires immediately when native cleanup clears it.
 template<class R>bool StrongEarnedCurrent(R& r,const cfgprobe::lifetime::ArrayHeader& objects,std::uintptr_t module){
  cfgprobe::lifetime::SlotObservation pawn{},ability{},again{};std::uintptr_t nativePawn=0,ctor=0,type=0,owner=0,current=0;
  std::uint32_t flags=0;
  const auto& binding=cfgfeature::disk::Classes[cfgfeature::disk::Counselor];
  return earnedOwner.HasAssignedSerial()&&earnedSlot.object&&earnedSlot.object!=defaults&&
   cfgprimary::LookupSafe(r,module)&&
   cfgspawn::ObjectSlot(r,objects,earnedOwner.object,pawn)&&pawn==earnedOwner&&
   r.Read(objects.data+static_cast<std::uintptr_t>(pawn.index)*24+8,flags)&&!(flags&cfgmenu::loadeddisk::InvalidItemFlags)&&
   r.Read(module+binding.reflection,nativePawn)&&r.Read(module+binding.constructor,ctor)&&nativePawn&&ctor==nativePawn&&
   cfgspawn::ObjectIs(r,pawn.object,nativePawn)&&
   r.Read(pawn.object+cfgfeature::disk::AbilityField,current)&&current==earnedSlot.object&&
   cfgspawn::ObjectSlot(r,objects,current,ability)&&ability==earnedSlot&&
   r.Read(objects.data+static_cast<std::uintptr_t>(ability.index)*24+8,flags)&&!(flags&cfgmenu::loadeddisk::InvalidItemFlags)&&
   r.Read(current+cfgfeature::disk::AbilityOwner,owner)&&owner==pawn.object&&
   r.Read(current+cfgprobe::ObjectClass,type)&&type==slot.object&&
   TemplateSame(r,pawn.object+cfgfeature::disk::AbilityField,current)&&
   TemplateSame(r,current+cfgfeature::disk::AbilityOwner,owner)&&
   cfgspawn::ObjectSlot(r,objects,pawn.object,again)&&again==pawn&&
   cfgspawn::ObjectSlot(r,objects,current,again)&&again==ability&&
   cfgprimary::LookupSafe(r,module);
 }
 template<class R>bool Current(R& r,const cfgprobe::lifetime::ArrayHeader& objects,std::uintptr_t module,bool matchRoots=false){
  using namespace cfgfeature::disk;cfgprobe::lifetime::SlotObservation now{},cdo{};std::uintptr_t a=0,b=0;std::uint32_t flags=0,defaultFlags=0;
  const auto& binding=Classes[Ability];
  return slot.object&&r.Read(module+binding.reflection,a)&&r.Read(module+binding.constructor,b)&&a==native&&b==native&&
   cfgspawn::ObjectSlot(r,objects,slot.object,now)&&now==slot&&
   r.Read(objects.data+static_cast<std::uintptr_t>(slot.index)*24+8,flags)&&!(flags&cfgmenu::loadeddisk::InvalidItemFlags)&&
   (slot.HasAssignedSerial()||(nativeRegistered&&slot.object==native)||defaultSlot.HasAssignedSerial()||
    (anchor==4&&EarnedCurrent(r,objects))||(anchor==5&&StrongEarnedCurrent(r,objects,module))||
    (anchor==6&&StrongWorldCurrent(r,objects,module))||(anchor==7&&DeclaredWorldCurrent(r,objects,module))||
    (anchor==9&&SettingsCurrent(r,objects,module))||(anchor==8&&matchRoots))&&
   cfgspawn::ClassContains(r,slot.object,native)&&r.Read(slot.object+cfgspawn::ClassDefaultObject,a)&&a==defaults&&
   cfgspawn::ObjectSlot(r,objects,defaults,cdo)&&cdo==defaultSlot&&
   r.Read(objects.data+static_cast<std::uintptr_t>(cdo.index)*24+8,defaultFlags)&&!(defaultFlags&cfgmenu::loadeddisk::InvalidItemFlags)&&
   r.Read(defaults+cfgprobe::ObjectClass,a)&&a==slot.object&&
   cfgspawn::ObjectSlot(r,objects,slot.object,now)&&now==slot&&
   cfgspawn::ObjectSlot(r,objects,defaults,cdo)&&cdo==defaultSlot&&
   r.Read(objects.data+static_cast<std::uintptr_t>(slot.index)*24+8,flags)&&!(flags&cfgmenu::loadeddisk::InvalidItemFlags)&&
   r.Read(objects.data+static_cast<std::uintptr_t>(cdo.index)*24+8,defaultFlags)&&!(defaultFlags&cfgmenu::loadeddisk::InvalidItemFlags)&&
   (anchor!=4||EarnedCurrent(r,objects))&&(anchor!=5||StrongEarnedCurrent(r,objects,module))&&
   (anchor!=6||StrongWorldCurrent(r,objects,module))&&(anchor!=7||DeclaredWorldCurrent(r,objects,module))&&
   (anchor!=9||SettingsCurrent(r,objects,module))&&(anchor!=8||matchRoots);
 }
 template<class R>bool CaptureDeclared(R& r,const cfgprobe::lifetime::ArrayHeader& objects,std::uintptr_t module,const WorldSource& source){
  ClassLease candidate;candidate.worldSource=source;candidate.anchor=7;
  std::uintptr_t type=0,a=0,b=0;const auto& binding=cfgfeature::disk::Classes[cfgfeature::disk::Ability];
  if(!r.Read(source.source.object+cfgfeature::disk::SweaterAbilityClass,type)||!type||
     !r.Read(module+binding.reflection,a)||!r.Read(module+binding.constructor,b)||!(candidate.native=cfgprobe::MatchingClassCache(a,b))||
     !cfgspawn::ObjectSlot(r,objects,type,candidate.slot)||!r.Read(type+cfgspawn::ClassDefaultObject,candidate.defaults)||
     !cfgspawn::ObjectSlot(r,objects,candidate.defaults,candidate.defaultSlot)||!candidate.Current(r,objects,module))return false;
  *this=candidate;return true;
 }
 template<class R>bool Capture(R& r,const cfgprobe::lifetime::ArrayHeader& objects,std::uintptr_t module,std::uintptr_t earned,
     cfgprobe::lifetime::SlotObservation owner={},const WorldSource* world=nullptr){
  failure=3;observedSerial=observedDefaultSerial=0;observedClassFlags=observedDefaultFlags=0;anchor=0;
  ClassLease candidate;std::uintptr_t type=0,a=0,b=0;const auto& binding=cfgfeature::disk::Classes[cfgfeature::disk::Ability];
  if(!r.Read(earned+cfgprobe::ObjectClass,type)||!r.Read(module+binding.reflection,a)||!r.Read(module+binding.constructor,b)||
   !(candidate.native=cfgprobe::MatchingClassCache(a,b))||!cfgspawn::ObjectSlot(r,objects,type,candidate.slot)||
   !r.Read(type+cfgspawn::ClassDefaultObject,candidate.defaults)||
   !cfgspawn::ObjectSlot(r,objects,candidate.defaults,candidate.defaultSlot))return false;
  observedSerial=candidate.slot.serial;observedDefaultSerial=candidate.defaultSlot.serial;candidate.nativeRegistered=type==candidate.native;
  if(!r.Read(objects.data+static_cast<std::uintptr_t>(candidate.slot.index)*24+8,observedClassFlags)||
     !r.Read(objects.data+static_cast<std::uintptr_t>(candidate.defaultSlot.index)*24+8,observedDefaultFlags))return false;
  candidate.observedClassFlags=observedClassFlags;candidate.observedDefaultFlags=observedDefaultFlags;
  failure=4;
  candidate.anchor=candidate.slot.HasAssignedSerial()?1u:candidate.nativeRegistered?2u:candidate.defaultSlot.HasAssignedSerial()?3u:0u;
  // CFG generated class/CDO may both have serial0 while the earned instance
  // has a positive lifetime identity. Its typed class edge is a callback-local
  // anchor, not a GC root. Recheck it on every constructor admission; if native
  // cleanup/GC retires the instance, this fourth anchor expires immediately.
  if(!candidate.anchor&&earned!=candidate.defaults&&cfgspawn::ObjectSlot(r,objects,earned,candidate.earnedSlot)&&
     candidate.EarnedCurrent(r,objects))candidate.anchor=4;
  if(!candidate.anchor&&owner.HasAssignedSerial()){
   candidate.earnedOwner=owner;
   if(cfgspawn::ObjectSlot(r,objects,earned,candidate.earnedSlot)&&candidate.StrongEarnedCurrent(r,objects,module))candidate.anchor=5;
  }
  if(!candidate.anchor&&world){candidate.worldSource=*world;
   if(candidate.StrongWorldCurrent(r,objects,module))candidate.anchor=6;}
  if(!candidate.anchor){failure=5;return false;}
  if(!candidate.Current(r,objects,module))return false;*this=candidate;return true;
 }
};
// A failed Current preflight never spends construction permission. Clear only
// the historical class success latch/cursor; the caller's existing bounded
// discovery deadline, native-use history and one-constructor guard stay intact.
inline void ExpireClassForDiscovery(ClassLease& lease,bool& attempted,unsigned& cursor){
 lease={};attempted=false;cursor=0;
}
// Promotion resets the discovery latch, not an independently current class.
// Revalidate that retained class in its own phase before scanning WorldItems:
// the original sweater may have been removed from that list by native pickup.
template<class R>bool RetainCurrentClassForDiscovery(R& r,const cfgprobe::lifetime::ArrayHeader& objects,
    std::uintptr_t module,ClassLease& lease,bool& attempted,unsigned& cursor,bool matchRoots=false){
 if(attempted||!lease.slot.object)return false;
 if(!lease.Current(r,objects,module,matchRoots)){ExpireClassForDiscovery(lease,attempted,cursor);return false;}
 attempted=true;return true;
}
}
