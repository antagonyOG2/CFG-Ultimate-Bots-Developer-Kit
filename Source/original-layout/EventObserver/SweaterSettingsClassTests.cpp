#define CFG_UNVERIFIED_RUNTIME_FIXTURE
#include "FreshAbilityClassObservation.hpp"
#include "../Core/SweaterLifecyclePolicy.hpp"
#define main CfgResourcesFixtureMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
struct SettingsFixture:Fixture {
 std::uintptr_t world=0,level=0,settings=0,sweater=0,ability=0,old=0,garmentDefault=0,abilityDefault=0,large=0,small=0;
 cfgsweater::ClassLease lease;unsigned cursor=0;
 SettingsFixture(){
  cap=100000;world=Object();const auto levelType=Object(),settingsType=Object(),nativeSweater=Object(),nativeAbility=Object();
  level=Object(levelType);settings=Object(settingsType);sweater=Object();ability=Object();old=Object(ability);
  garmentDefault=Object(sweater);abilityDefault=Object(ability);large=Object();small=Object();const auto other=Object();
  Put(module+cfgoutro::disk::LevelClass,levelType);Put(module+cfgoutro::disk::LevelConstructor,levelType);
  Put(module+cfgoutro::disk::SCWorldSettingsClass,settingsType);Put(module+cfgoutro::disk::SCWorldSettingsConstructor,settingsType);
  Put(world+cfgoutro::disk::PersistentLevel,level);Put(level+cfgoutro::disk::OwningWorld,world);
  Put(level+cfgoutro::disk::WorldSettings,settings);Put(settings+0x20,level);Put(settings+0x110,std::uint8_t{3});
  const auto& s=cfgfeature::disk::Classes[cfgfeature::disk::Sweater];const auto& a=cfgfeature::disk::Classes[cfgfeature::disk::Ability];
  Put(module+s.reflection,nativeSweater);Put(module+s.constructor,nativeSweater);
  Put(module+a.reflection,nativeAbility);Put(module+a.constructor,nativeAbility);
  Put(sweater+cfgprobe::ClassSuper,nativeSweater);Put(ability+cfgprobe::ClassSuper,nativeAbility);
  Put(sweater+cfgspawn::ClassDefaultObject,garmentDefault);Put(ability+cfgspawn::ClassDefaultObject,abilityDefault);
  Put(garmentDefault+cfgfeature::disk::SweaterAbilityClass,ability);
  Put(settings+0x800,cfgsweater::ClassLease::SpawnList{large,1,1});Put(large,sweater);
  Put(settings+0x810,cfgsweater::ClassLease::SpawnList{small,1,1});Put(small,other);
  Put(module+0x03199662,std::uint8_t{});Put(module+0x03198110,std::uint32_t{});
  reads=0;cap=1536;
 }
 bool Capture(){reads=0;return lease.CaptureSettings(*this,objects,module,world,cursor);}
 bool Current(){reads=0;return lease.Current(*this,objects,module);}
};
int main(){unsigned peak=0;
 {SettingsFixture f;CHECK(f.Capture()&&f.lease.anchor==9&&f.lease.slot.object==f.ability);peak=f.reads;
  int oldIndex=0;CHECK(f.Read(f.old+12,oldIndex));f.Put(f.objects.data+24ULL*static_cast<unsigned>(oldIndex),std::uintptr_t{});
  for(unsigned i=0;i<0x2000;++i)f.memory.erase(f.old+i);
  CHECK(f.Current());peak=(std::max)(peak,f.reads);
  cfgsweater::Policy policy;policy.ConfirmPublish();CHECK(policy.ObserveUsed(100,1));
  CHECK(policy.stage==cfgsweater::Stage::Waiting&&f.Current()); // pre-constructor class admission survives use
  f.Put(f.settings+0x800,cfgsweater::ClassLease::SpawnList{f.large,0,1});CHECK(!f.Current());}
 for(unsigned fault=0;fault<5;++fault){SettingsFixture f;CHECK(f.Capture());
  if(fault==0)f.Put(f.garmentDefault+cfgprobe::ObjectClass,f.ability);
  if(fault==1)f.Put(f.sweater+cfgprobe::ClassSuper,std::uintptr_t{});
  if(fault==2)f.Put(f.settings+0x20,f.world);
  if(fault==3)f.Put(f.garmentDefault+cfgfeature::disk::SweaterAbilityClass,f.sweater);
  if(fault==4)f.Put(f.small,f.ability); // same header, unrelated class does not replace the recorded strong class edge
  if(fault<4)CHECK(!f.Current());else CHECK(f.Current());}
 {SettingsFixture f;f.Put(f.large,f.old);CHECK(!f.Capture()&&f.lease.failure==31&&!f.lease.slot.object);}
 {SettingsFixture f;f.Put(f.settings+0x800,cfgsweater::ClassLease::SpawnList{f.large,65,65});CHECK(!f.Capture()&&f.lease.failure==30);}
 std::printf("WorldSettings sweater class declaration survives consumed ability; stale ownership/schema refused: %d failures, peak%u/1536; no native/game access\n",failures,peak);
 return failures?1:0;
}
