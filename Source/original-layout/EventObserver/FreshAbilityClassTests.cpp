#define CFG_UNVERIFIED_RUNTIME_FIXTURE
#include "FreshAbilityClassObservation.hpp"
#include "../Core/SweaterLifecyclePolicy.hpp"
#include "RetiredOwnedAbility.hpp"
#define main CfgResourceFixtureMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
int main(){
 Fixture f;f.cap=1536;const auto& binding=cfgfeature::disk::Classes[cfgfeature::disk::Ability];
 const auto native=f.Object(),derived=f.Object(),ability=f.Object(derived),defaults=f.Object(derived);
 f.Put(f.module+binding.reflection,native);f.Put(f.module+binding.constructor,native);
 f.Put(native+cfgprobe::ClassSuper,std::uintptr_t{});f.Put(derived+cfgprobe::ClassSuper,native);
 f.Put(derived+cfgspawn::ClassDefaultObject,defaults);
 int index=0;CHECK(f.Read(derived+12,index));const auto item=f.objects.data+24*static_cast<std::uintptr_t>(index);
 f.Put(item+16,int{7});f.Put(item+8,std::uint32_t{0});
 cfgsweater::ClassLease lease;f.reads=0;CHECK(lease.Capture(f,f.objects,f.module,ability));CHECK(f.reads<160);
 // Consumed INSTANCE disappears; no member of the old instance is touched by
 // Current. Its earned positive CLASS/defaults still admit a fresh constructor.
 f.Put(ability+cfgprobe::ObjectClass,std::uintptr_t{});f.reads=0;CHECK(lease.Current(f,f.objects,f.module));
 const auto peak=f.reads;CHECK(peak<120);
 {bool attempted=false;unsigned cursor=42;auto retained=lease;f.reads=0;
  CHECK(cfgsweater::RetainCurrentClassForDiscovery(f,f.objects,f.module,retained,attempted,cursor)&&attempted&&retained.slot.object==lease.slot.object);
  CHECK(!cfgsweater::RetainCurrentClassForDiscovery(f,f.objects,f.module,retained,attempted,cursor));
  attempted=false;f.Put(item+16,int{99});f.reads=0;
  CHECK(!cfgsweater::RetainCurrentClassForDiscovery(f,f.objects,f.module,retained,attempted,cursor)&&!attempted&&!retained.slot.object&&cursor==0);
  f.Put(item+16,int{7});
 }
 f.Put(item+16,int{8});CHECK(!lease.Current(f,f.objects,f.module));f.Put(item+16,int{7});
 f.Put(item+8,std::uint32_t{0x10000000});CHECK(!lease.Current(f,f.objects,f.module));f.Put(item+8,std::uint32_t{0});
 f.Put(defaults+cfgprobe::ObjectClass,native);CHECK(!lease.Current(f,f.objects,f.module));f.Put(defaults+cfgprobe::ObjectClass,derived);
 f.Put(f.module+binding.constructor,derived);CHECK(!lease.Current(f,f.objects,f.module));f.Put(f.module+binding.constructor,native);
 f.Put(item+16,int{0});cfgsweater::ClassLease unassigned;CHECK(!unassigned.Capture(f,f.objects,f.module,defaults));
 CHECK(unassigned.failure==5&&unassigned.observedSerial==0);
 // A transient exhausted reader must not consume the only retention attempt.
 f.Put(item+16,int{7});cfgsweater::ClassLease retried;f.reads=f.cap;
 CHECK(!retried.Capture(f,f.objects,f.module,defaults)&&!retried.slot.object);
 f.reads=0;CHECK(retried.Capture(f,f.objects,f.module,defaults));
 f.Put(item+16,int{0});
 // CFG blueprint CLASS can be unassigned while its CDO has a real serial.
 // The CDO's independently retained lifetime plus reciprocal links identify
 // the earned class after its consumed instance disappears. Both-zero fails.
 int defaultIndex=0;CHECK(f.Read(defaults+12,defaultIndex));const auto defaultItem=f.objects.data+24*static_cast<std::uintptr_t>(defaultIndex);
 f.Put(defaultItem+16,int{22});f.Put(defaultItem+8,std::uint32_t{0});cfgsweater::ClassLease cdoLease;f.reads=0;
 CHECK(cdoLease.Capture(f,f.objects,f.module,defaults)&&cdoLease.anchor==3);
 f.reads=0;CHECK(cdoLease.Current(f,f.objects,f.module));
 f.Put(defaultItem+16,int{23});CHECK(!cdoLease.Current(f,f.objects,f.module));f.Put(defaultItem+16,int{22});
 f.Put(defaultItem+8,std::uint32_t{0x10000000});CHECK(!cdoLease.Current(f,f.objects,f.module));f.Put(defaultItem+8,std::uint32_t{0});
 f.Put(item+8,std::uint32_t{0x20000000});CHECK(!cdoLease.Current(f,f.objects,f.module));f.Put(item+8,std::uint32_t{0});
 f.Put(derived+cfgspawn::ClassDefaultObject,native);CHECK(!cdoLease.Current(f,f.objects,f.module));f.Put(derived+cfgspawn::ClassDefaultObject,defaults);
 f.Put(defaults+cfgprobe::ObjectClass,native);CHECK(!cdoLease.Current(f,f.objects,f.module));f.Put(defaults+cfgprobe::ObjectClass,derived);
 f.Put(defaultItem+16,int{0});f.reads=0;cfgsweater::ClassLease bothZero;CHECK(!bothZero.Capture(f,f.objects,f.module,defaults)&&bothZero.failure==5);
 // A real earned INSTANCE with positive identity can anchor its own class.
 // Neither class/CDO serial0 nor a CDO-as-instance can manufacture that lease.
 int abilityIndex=0;CHECK(f.Read(ability+12,abilityIndex));const auto abilityItem=f.objects.data+24*static_cast<std::uintptr_t>(abilityIndex);
 f.Put(ability+cfgprobe::ObjectClass,derived);f.Put(abilityItem+16,int{29});f.Put(abilityItem+8,std::uint32_t{});
 cfgsweater::ClassLease earnedLease;f.reads=0;CHECK(earnedLease.Capture(f,f.objects,f.module,ability)&&earnedLease.anchor==4);
 f.reads=0;CHECK(earnedLease.Current(f,f.objects,f.module));
 f.Put(abilityItem+16,int{30});CHECK(!earnedLease.Current(f,f.objects,f.module));f.Put(abilityItem+16,int{29});
 f.Put(abilityItem+8,std::uint32_t{0x10000000});CHECK(!earnedLease.Current(f,f.objects,f.module));f.Put(abilityItem+8,std::uint32_t{});
 f.Put(ability+cfgprobe::ObjectClass,native);CHECK(!earnedLease.Current(f,f.objects,f.module));f.Put(ability+cfgprobe::ObjectClass,derived);
 f.Put(abilityItem+16,int{});CHECK(!earnedLease.Current(f,f.objects,f.module));
 // Actual CFG case: class/CDO/earned ability all have zero weak serials.
 // Only the repeated reciprocal edge of a positive original wearer anchors it.
 const auto wearer=f.Object(f.pawnClass);int wearerIndex=0;CHECK(f.Read(wearer+12,wearerIndex));
 f.Put(f.objects.data+24*static_cast<std::uintptr_t>(wearerIndex)+16,int{41});
 f.Put(f.objects.data+24*static_cast<std::uintptr_t>(wearerIndex)+8,std::uint32_t{});
 const auto& counselorBinding=cfgfeature::disk::Classes[cfgfeature::disk::Counselor];
 f.Put(f.module+counselorBinding.reflection,f.pawnClass);f.Put(f.module+counselorBinding.constructor,f.pawnClass);
 f.Put(f.module+0x03199662,std::uint8_t{});f.Put(f.module+0x03198110,std::uint32_t{});
 f.Put(wearer+cfgfeature::disk::AbilityField,ability);f.Put(ability+cfgfeature::disk::AbilityOwner,wearer);
 cfgprobe::lifetime::SlotObservation wearerSlot{};CHECK(cfgspawn::ObjectSlot(f,f.objects,wearer,wearerSlot));
 cfgsweater::ClassLease ownedLease;f.reads=0;
 CHECK(ownedLease.Capture(f,f.objects,f.module,ability,wearerSlot)&&ownedLease.anchor==5);
 f.reads=0;CHECK(ownedLease.Current(f,f.objects,f.module)&&f.reads<350);
 for(unsigned fault=0;fault<7;++fault){auto bad=f;bad.reads=0;
  switch(fault){case 0:bad.Put(wearer+cfgfeature::disk::AbilityField,std::uintptr_t{});break;
   case 1:bad.Put(ability+cfgfeature::disk::AbilityOwner,bad.bot);break;
   case 2:bad.Put(bad.objects.data+24*static_cast<std::uintptr_t>(wearerIndex)+16,int{42});break;
   case 3:bad.Put(abilityItem+8,std::uint32_t{0x10000000});break;
   case 4:bad.Put(bad.module+0x03199662,std::uint8_t{1});break;
   case 5:bad.Put(bad.module+0x03198110,std::uint32_t{1});break;
   case 6:bad.Put(ability+cfgprobe::ObjectClass,native);break;}
  CHECK(!ownedLease.Current(bad,bad.objects,bad.module));
 }
 // Production Fresh admission's expired-anchor transition must reopen bounded
 // discovery, NOT authorize a constructor or retain a historical success flag.
 {auto expired=ownedLease;bool attempted=true;unsigned cursor=97;
  auto cleaned=f;cleaned.reads=0;cleaned.Put(wearer+cfgfeature::disk::AbilityField,std::uintptr_t{});
  CHECK(!expired.Current(cleaned,cleaned.objects,cleaned.module));
  cfgsweater::ExpireClassForDiscovery(expired,attempted,cursor);
  CHECK(!attempted&&!cursor&&!expired.slot.object&&!expired.anchor);}
 f.reads=0;CHECK(ownedLease.Current(f,f.objects,f.module)); // another human owner does not invalidate a still-owned original edge
 // Tommy mask case: original wearer edge is cleared, but a CURRENT native
 // world's sweater independently holds the exact derived ability class.
 {auto worldFixture=f;worldFixture.reads=0;
  const auto sweaterType=worldFixture.Object(),source=worldFixture.Object(sweaterType),level=worldFixture.Object();
  const auto world=worldFixture.Object(),items=worldFixture.Object();
  const auto& sweaterBinding=cfgfeature::disk::Classes[cfgfeature::disk::Sweater];
  worldFixture.Put(worldFixture.module+sweaterBinding.reflection,sweaterType);
  worldFixture.Put(worldFixture.module+sweaterBinding.constructor,sweaterType);
  worldFixture.Put(world+cfgprobe::WorldGameState,worldFixture.state);
  struct List {std::uintptr_t data;int count,capacity;};
  worldFixture.Put(worldFixture.state+cfgfeature::disk::WorldItems,List{items,1,1});worldFixture.Put(items,source);
  worldFixture.Put(source+0x20,level);worldFixture.Put(level+cfgkill::disk::OwningWorld,world);
  worldFixture.Put(source+cfgfeature::disk::SweaterAbility,ability);
  worldFixture.Put(wearer+cfgfeature::disk::AbilityField,std::uintptr_t{});
  cfgsweater::ClassLease::WorldSource edge;edge.world=world;edge.state=worldFixture.state;
  CHECK(cfgspawn::ObjectSlot(worldFixture,worldFixture.objects,source,edge.source));
  CHECK(cfgspawn::ObjectSlot(worldFixture,worldFixture.objects,ability,edge.ability));
  cfgsweater::ClassLease worldLease;worldFixture.reads=0;
  CHECK(worldLease.Capture(worldFixture,worldFixture.objects,worldFixture.module,ability,{},&edge)&&worldLease.anchor==6);
  worldFixture.reads=0;CHECK(worldLease.Current(worldFixture,worldFixture.objects,worldFixture.module)&&worldFixture.reads<400);
  CHECK(!ownedLease.Current(worldFixture,worldFixture.objects,worldFixture.module)); // original edge still must expire
  for(unsigned fault=0;fault<9;++fault){auto bad=worldFixture;bad.reads=0;
   switch(fault){case 0:bad.Put(items,std::uintptr_t{});break;
    case 1:bad.Put(source+cfgfeature::disk::SweaterAbility,std::uintptr_t{});break;
    case 2:bad.Put(ability+cfgprobe::ObjectClass,native);break;
    case 3:bad.Put(level+cfgkill::disk::OwningWorld,bad.bot);break;
    case 4:bad.Put(world+cfgprobe::WorldGameState,bad.bot);break;
    case 5:bad.Put(bad.module+sweaterBinding.constructor,native);break;
    case 6:bad.Put(bad.objects.data+24*static_cast<std::uintptr_t>(edge.source.index)+8,std::uint32_t{0x10000000});break;
    case 7:bad.Put(bad.module+0x03198110,std::uint32_t{1});break;
    case 8:bad.Put(source+cfgfeature::disk::SweaterAbility,defaults);break;}
   CHECK(!worldLease.Current(bad,bad.objects,bad.module));}
  auto moved=worldFixture;moved.reads=0;const auto relocated=moved.Object();moved.Put(relocated,source);
  moved.Put(moved.state+cfgfeature::disk::WorldItems,List{relocated,1,1});
  CHECK(worldLease.Current(moved,moved.objects,moved.module)); // repeats CURRENT strong header, not stale list storage
  // Instance cleanup is irrelevant to this separate declaration-backed edge.
  const auto levelType=worldFixture.Object();worldFixture.Put(level+cfgprobe::ObjectClass,levelType);
  worldFixture.Put(worldFixture.module+cfgkill::disk::LevelReflection,levelType);worldFixture.Put(worldFixture.module+cfgkill::disk::LevelConstructor,levelType);
  worldFixture.Put(source+cfgfeature::disk::SweaterAbilityClass,derived);
  worldFixture.Put(source+cfgfeature::disk::SweaterAbility,std::uintptr_t{});
  worldFixture.Put(abilityItem,std::uintptr_t{});worldFixture.reads=0;
  cfgsweater::ClassLease declaration;
  CHECK(declaration.CaptureDeclared(worldFixture,worldFixture.objects,worldFixture.module,edge)&&declaration.anchor==7);
  worldFixture.reads=0;CHECK(declaration.Current(worldFixture,worldFixture.objects,worldFixture.module)&&worldFixture.reads<400);
  for(unsigned fault=0;fault<8;++fault){auto bad=worldFixture;bad.reads=0;
   switch(fault){case 0:bad.Put(source+cfgfeature::disk::SweaterAbilityClass,native);break;
    case 1:bad.Put(items,std::uintptr_t{});break;
    case 2:bad.Put(level+cfgkill::disk::OwningWorld,bad.bot);break;
    case 3:bad.Put(world+cfgprobe::WorldGameState,bad.bot);break;
    case 4:bad.Put(bad.objects.data+24*static_cast<std::uintptr_t>(edge.source.index)+8,std::uint32_t{0x10000000});break;
    case 5:bad.Put(bad.module+0x03198110,std::uint32_t{1});break;
    case 6:bad.Put(defaults+cfgprobe::ObjectClass,native);break;
    case 7:bad.Put(item+16,int{91});break;}
   CHECK(!declaration.Current(bad,bad.objects,bad.module));}
  worldFixture.reads=worldFixture.cap;CHECK(!declaration.Current(worldFixture,worldFixture.objects,worldFixture.module));
 }
 // Captured index, two stable native registry snapshots. Budget exhaustion
 // must NEVER be mistaken for retirement or a reason to allocate a replacement.
 {auto retired=f;retired.reads=0;cfgprobe::lifetime::SlotObservation held{};
  CHECK(cfgspawn::ObjectSlot(retired,retired.objects,ability,held));
  retired.Put(retired.module+cfgprobe::lifetime::ObjectArrayRva+cfgprobe::lifetime::ArrayDataOffset,retired.objects.data);
  retired.Put(retired.module+cfgprobe::lifetime::ObjectArrayRva+cfgprobe::lifetime::ArrayCountOffset,retired.objects.count);
  retired.Put(abilityItem+12,int{});retired.Put(abilityItem+20,int{});
  CHECK(!cfgsweater::Retired(retired,retired.module,held));
  retired.Put(abilityItem,std::uintptr_t{});CHECK(cfgsweater::Retired(retired,retired.module,held));
  retired.reads=retired.cap;CHECK(!cfgsweater::Retired(retired,retired.module,held));
  retired.reads=0;retired.Put(abilityItem,ability);retired.Put(abilityItem+8,std::uint32_t{0x10000000});
  CHECK(cfgsweater::Retired(retired,retired.module,held));}
 // Only the exact registered native class may use serial zero, through its
 // two CURRENT strong registration caches. No invented weak identity.
 const auto nativeDefaults=f.Object(native),nativeAbility=f.Object(native);
 int nativeIndex=0;CHECK(f.Read(native+12,nativeIndex));f.Put(f.objects.data+24*static_cast<std::uintptr_t>(nativeIndex)+8,std::uint32_t{0});
 f.Put(native+cfgspawn::ClassDefaultObject,nativeDefaults);CHECK(unassigned.Capture(f,f.objects,f.module,nativeAbility));
 f.Put(f.module+binding.reflection,derived);CHECK(!unassigned.Current(f,f.objects,f.module));
 cfgsweater::Policy policy;policy.ConfirmPublish();CHECK(policy.ObserveUsed(100,1));
 // Diagnostic only: current actor->class->CDO->ability template. It does not
 // promote a both-zero ClassLease or enter any native allocation path.
 const auto sweaterClass=f.Object(),sweaterDefaults=f.Object(sweaterClass),source=f.Object(sweaterClass);
 const auto abilityTemplate=f.Object(derived);cfgprobe::lifetime::SlotObservation sourceSlot{};
 CHECK(cfgspawn::ObjectSlot(f,f.objects,source,sourceSlot));
 f.Put(sweaterClass+cfgspawn::ClassDefaultObject,sweaterDefaults);
 f.Put(sweaterDefaults+cfgfeature::disk::SweaterAbility,abilityTemplate);f.Put(abilityTemplate+0x20,sweaterDefaults);
 f.reads=0;auto evidence=cfgsweater::ObserveTemplate(f,f.objects,sourceSlot,derived);
 CHECK(evidence.stage==5&&evidence.abilityTemplate==abilityTemplate&&f.reads<180);
 CHECK(!bothZero.slot.object); // readonly evidence is not a weak lifetime lease
 f.Put(abilityTemplate+0x20,source);CHECK(cfgsweater::ObserveTemplate(f,f.objects,sourceSlot,derived).stage==4);
 f.Put(abilityTemplate+0x20,sweaterDefaults);
 f.Put(sweaterDefaults+cfgfeature::disk::SweaterAbility,std::uintptr_t{});
 CHECK(cfgsweater::ObserveTemplate(f,f.objects,sourceSlot,derived).stage==3);
 f.reads=f.cap;CHECK(cfgsweater::ObserveTemplate(f,f.objects,sourceSlot,derived).stage==1);f.reads=0;
 const auto use=policy.lastUse,at=policy.rearmAt;policy.QueueHud(at,true);policy.ConfirmPublish();
 CHECK(policy.lastUse==use&&policy.rearmAt==at&&policy.clearObserved);
 std::printf("Earned fresh ability class: %d failures; retained reads=%u; no allocation/writes/game access\n",failures,peak);
 return failures?1:0;
}
