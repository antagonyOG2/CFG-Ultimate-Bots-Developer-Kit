#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#define CFG_OFFLINE_OWNER_FIXTURE_ONLY
#include "NativeOfflineOwnerObservationTests.cpp"
#include "NativeHumanJasonMenu.hpp"
#include <limits>
struct HitReader {StageReader& memory;unsigned reads=0,cap=1536;std::uintptr_t missing=0;
 template<class T>bool Read(std::uintptr_t at,T& out){if(reads>=cap)return false;++reads;const bool okay=memory.Read(at,out);if(!okay&&!missing)missing=at;return okay;}
};
struct HitWriter {StageReader& memory;unsigned writes=0;bool fail=false;
 template<class T>bool Write(std::uintptr_t at,const T& old,const T& desired){T actual{};
  if(fail||!memory.Read(at,actual)||actual!=old)return false;memory.Set(at,desired);++writes;return true;}
};
struct HitFixture:Fixture {
 static constexpr std::uintptr_t State=0x700000,Victim=0x710000,Weapon=0x720000,Projectile=0x730000,
   WeaponType=0x740000,ProjectileType=0x750000;
 HitReader reader{r};HitWriter writer{r};cfgmenu::humanjason::Runtime runtime;
 HitFixture(){using namespace cfgprobe;Birth();r.Set(Selected+ClassSuper,Killer);
  r.Set(World+WorldGameState,State);r.Set(State+cfgprimary::CurrentKiller,Pawn);r.Set(State+cfgprimary::CurrentKillerState,PS);
  r.Set(Pawn+cfgofflinesettings::disk::RageUnlockTime,600.f);r.Set(Pawn+cfghunter::disk::Health,100.f);
  r.Set(Pawn+cfgknife::disk::Role,std::uint8_t{3});r.Set(PS+cfgescape::disk::bDead,std::uint8_t{});
  r.Set(Module+lifetime::ObjectArrayRva+28,int{10});
  const std::array<std::uintptr_t,4> objects{{State,Victim,Weapon,Projectile}};
  for(unsigned n=0;n<objects.size();++n){const auto index=6+n;r.Set(objects[n]+12,static_cast<int>(index));
   r.Set(Registry+24*index,objects[n]);r.Set(Registry+24*index+16,static_cast<int>(100+index));}
  for(unsigned index=0;index<10;++index)r.Set(Registry+24*index+8,std::uint32_t{});
  for(const auto object:{Pawn,Victim,Weapon,Projectile})r.Set(object+0x20,Level);
  r.Set(Victim+ObjectClass,Counselor);r.Set(Victim+cfgescape::disk::bHasEscaped,std::uint8_t{});
  r.Set(Victim+cfghunter::disk::Health,100.f);r.Set(Victim+cfgescape::disk::PlayerState,std::uintptr_t{});
  r.Set(Victim+cfgescape::disk::Controller,std::uintptr_t{});
  r.Set(Module+cfgconvergence::disk::SCWeaponReflection,WeaponType);r.Set(Module+cfgconvergence::disk::SCWeaponConstructor,WeaponType);
  r.Set(Weapon+ObjectClass,WeaponType);r.Set(WeaponType+ClassSuper,std::uintptr_t{});
  r.Set(Weapon+cfgloadout::restricteddisk::OwningCharacter,Pawn);
  r.Set(Module+cfgknife::disk::ThrowableReflection,ProjectileType);r.Set(Module+cfgknife::disk::ThrowableConstructor,ProjectileType);
  r.Set(Projectile+ObjectClass,ProjectileType);r.Set(ProjectileType+ClassSuper,std::uintptr_t{});
  r.Set(Projectile+cfgknife::disk::ThrownBy,Pawn);
  Name(4,"InProgress");r.Set(Names+cfgnames::disk::ElementCount,std::array<int,2>{{5,1}});
  r.Set(Mode+cfgescape::disk::MatchState,cfgnames::Words{4,0});
 }
 bool Arm(unsigned difficulty){cfgmenu::active::Context menu;menu.module=Module;menu.engine=Engine;menu.viewport=Viewport;
  menu.player=Player;menu.gameInstance=Instance;menu.world=source.frontendWorld;menu.thread=42;
  cfgmenu::humanjason::Options options;options.difficulty=difficulty;return runtime.Arm(menu,options,0);}
 float Hit(std::uintptr_t causer,std::uintptr_t instigator=Controller){reader.reads=0;
  return runtime.DifficultyDamage(reader,42,Victim,20.f,instigator,causer);}
};
#ifndef CFG_HUMAN_DIFFICULTY_TEST_MAIN
#define CFG_HUMAN_DIFFICULTY_TEST_MAIN main
#endif
int CFG_HUMAN_DIFFICULTY_TEST_MAIN(){unsigned checks=0,failures=0,peak=0,phasePeak=0;
 const auto check=[&](bool okay,const char* label){++checks;if(!okay){++failures;std::printf("FAIL %s\n",label);}};
 const auto specs=cfgmenu::humanjason::Specs();check(specs.size()==6&&specs[1].Valid()&&specs[1].id==cfgmenu::RowId::Difficulty&&
  specs[1].selected==1&&specs[1].count==3,"human-Jason row exposes supported Easy Normal Hard with Normal default");
 {std::array<unsigned,cfgmenu::rows::Capacity> values{};values[0]=2;values[1]=2;values[2]=3;values[3]=1;values[4]=1;values[5]=1;
  cfgmenu::humanjason::Options options;
  check(cfgmenu::humanjason::OptionsForChoices(values,options)&&options.difficulty==2&&options.nativeSkill==2&&
      options.seconds==UINT32_MAX&&options.knives&&options.movement&&options.recharge,"actual submission mapping preserves all6choices");
  for(unsigned index=0;index<6;++index){auto invalid=values;invalid[index]=99;
   check(!cfgmenu::humanjason::OptionsForChoices(invalid,options),"invalid setup choice refuses before Arm");}}
 for(unsigned difficulty:{0u,2u}){HitFixture f;check(f.Arm(difficulty),"difficulty arm");
  check(f.Hit(f.Pawn)==20.f,"no scaling before proved original human killer arrival");
  f.reader.reads=0;f.runtime.Step(f.reader,f.Pawn,42,100,f.writer);phasePeak=(std::max)(phasePeak,f.reader.reads);
  if(f.runtime.report.rageState!=2)std::printf("Rage fixture refusal stage%u rage%u reads%u missing%llX\n",f.runtime.report.failure,f.runtime.report.rageState,f.reader.reads,static_cast<unsigned long long>(f.reader.missing));
  check(f.runtime.report.rageState==2&&f.writer.writes==1,"Rage threshold applied once through current human ownership");
  float threshold=0;f.r.Read(f.Pawn+cfgofflinesettings::disk::RageUnlockTime,threshold);
  check(threshold==(difficulty==0?1200.f:300.f),"donor Rage2/0.5 scaling");
  for(auto causer:{f.Pawn,f.Weapon,f.Projectile}){
   check(f.Hit(causer)==(difficulty==0?10.f:40.f),"owned Jason weapon and projectile share donor damage scale");peak=(std::max)(peak,f.reader.reads);}
  f.reader.reads=0;f.runtime.Step(f.reader,f.Pawn,42,200,f.writer);
  check(f.writer.writes==1&&f.runtime.report.retired,"completed option phase retires without reapplying threshold");
  check(f.Hit(f.Projectile)==(difficulty==0?10.f:40.f),"damage remains active after one-shot maintenance retires");
 }
 for(unsigned fault=0;fault<19;++fault){HitFixture f;check(f.Arm(2),"fault arm");f.runtime.Step(f.reader,f.Pawn,42,100,f.writer);
  check(f.runtime.report.rageState==2,"fault begins from positive current witness");
  if(fault==0)f.r.Set(f.World+cfgprobe::WorldNetDriver,f.Mode);
  if(fault==1)f.r.Set(f.State+cfgprimary::CurrentKiller,f.Victim);
  if(fault==2)f.r.Set(f.Controller+cfgprobe::ControllerPawn,f.Victim);
  if(fault==3)f.r.Set(f.Registry+24*5+16,int{999});
  if(fault==4)f.r.Set(f.Registry+24*9+16,int{0});
  if(fault==5)f.r.Set(f.Projectile+cfgknife::disk::ThrownBy,f.Victim);
  if(fault==6)f.r.Set(f.Pawn+cfghunter::disk::Health,0.f);
  if(fault==7)f.r.Set(f.Victim+cfghunter::disk::Health,0.f);
  if(fault==8)f.r.Set(f.Victim+cfgescape::disk::bHasEscaped,std::uint8_t{1});
  if(fault==9)f.r.Set(f.Level+cfgkill::disk::OwningWorld,f.source.frontendWorld);
  if(fault==10)f.r.Set(f.Module+cfgprimary::SerializationFlag,std::uint8_t{1});
  if(fault==11)f.r.Set(f.Module+cfgprimary::GarbageCollectionCounter,std::uint32_t{1});
  if(fault==12)f.r.Set(f.Pawn+cfgknife::disk::Role,std::uint8_t{2});
  if(fault==13)f.r.Set(f.Mode+cfgescape::disk::MatchState,cfgnames::Words{1,0});
  if(fault==14)f.r.Set(f.PS+cfgescape::disk::bDead,std::uint8_t{1});
  if(fault==15)f.r.Set(f.State+cfgprimary::CurrentKillerState,f.Victim);
  if(fault==16)f.r.Set(f.Viewport+cfgprobe::ViewportWorld,f.source.frontendWorld);
  if(fault==17){f.reader.cap=5;}
  const auto instigator=fault==18?f.Victim:f.Controller;
  check(f.Hit(f.Projectile,instigator)==20.f,"foreign stale dead escaped GC phase budget and ownership failures preserve native damage");
 }
 {HitFixture f;check(f.Arm(2),"CAS fault arm");f.writer.fail=true;f.runtime.Step(f.reader,f.Pawn,42,100,f.writer);
  check(f.runtime.report.rageState==3&&f.runtime.report.retired&&!f.writer.writes,"ambiguous threshold CAS retires without publishing damage");
  f.writer.fail=false;f.reader.reads=0;f.runtime.Step(f.reader,f.Pawn,42,200,f.writer);
  check(!f.writer.writes&&f.Hit(f.Pawn)==20.f,"failed Rage cannot replay or activate scaling");}
 {HitFixture f;check(!f.Arm(3),"invalid difficulty refused");}
 {HitFixture f;check(f.Arm(1),"Normal arm");check(!f.runtime.Armed()&&f.Hit(f.Pawn)==20.f&&!f.writer.writes,
   "Normal preserves stock threshold and damage without maintenance");}
 {HitFixture f;check(f.Arm(2),"weapon fault arm");f.runtime.Step(f.reader,f.Pawn,42,100,f.writer);
  f.r.Set(f.Weapon+cfgloadout::restricteddisk::OwningCharacter,f.Victim);
  check(f.Hit(f.Weapon)==20.f,"another killer or counselor weapon cannot inherit human difficulty");
  f.reader.reads=0;check(f.runtime.DifficultyDamage(f.reader,43,f.Victim,20.f,f.Controller,f.Pawn)==20.f&&f.reader.reads==0,
   "foreign thread performs zero hit-time reads");
  f.r.Set(f.Pawn+cfgprobe::ObjectClass,f.Counselor);
  check(f.Hit(f.Pawn)==20.f,"human counselor cannot satisfy killer witness");}
 check(phasePeak<1536&&peak<1536,"bounded phase and O(1) hit proof stay within1536 reads");
 std::printf("NativeHumanJasonDifficulty: %u checks %u failures; phase%u/1536 hit%u/1536; own fixtures, no game access.\n",checks,failures,phasePeak,peak);
 return failures?1:0;
}
