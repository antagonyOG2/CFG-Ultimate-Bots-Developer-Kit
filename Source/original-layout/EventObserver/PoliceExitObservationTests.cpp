#define main CfgAiResourcesFixtureMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
#include "PoliceExitObservation.hpp"
using namespace cfgpoliceexit;
struct ExitFixture:Fixture {
 std::uintptr_t world=0,volumeType=0,sphereType=0,police=0,car=0,sphere=0,listData=0;
 unsigned locations=0,queries=0,observations=0;bool valid=true,queryOkay=true;std::uint8_t collision=1;
 Cache cache;
 Slot Serial(std::uintptr_t object){int index=0;CHECK(Read(object+12,index));Put(objects.data+24ULL*static_cast<unsigned>(index)+16,int{17});Slot out;CHECK(cfgspawn::ObjectSlot(*this,objects,object,out));return out;}
 bool Current(){return valid;}
 bool Location(std::uintptr_t actor,cfgspawn::Vec3& at){++locations;CHECK(actor==police);at={1000,2000,100};return true;}
 bool Collision(std::uintptr_t component,std::uint8_t& value){++queries;CHECK(component==sphere);value=collision;return queryOkay;}
 void Observed(const Entry&){++observations;}
 ExitFixture(){
  cap=100000;world=Object();mode=Object();volumeType=Object();sphereType=Object();police=Object(volumeType);car=Object(volumeType);sphere=Object(sphereType);listData=Object();
  const auto level=Object();Put(police+0x20,level);Put(level+cfgoutro::disk::OwningWorld,world);
  Put(module+disk::EscapeClassReflection,volumeType);Put(module+disk::EscapeClassConstructor,volumeType);
  Put(module+disk::SphereClassReflection,sphereType);Put(module+disk::SphereClassConstructor,sphereType);
  Serial(police);Serial(car);Serial(sphere);
  Put(world+cfgprobe::WorldGameMode,mode);Put(mode+disk::ModeVolumes,List{listData,2,2});
  Put(listData,police);Put(listData+8,car);Put(police+disk::EscapeType,std::uint8_t{0});Put(car+disk::EscapeType,std::uint8_t{1});
  Put(police+disk::EscapeSphere,sphere);Put(sphere+0x20,police);Put(state+disk::PoliceTimeRemaining,int{30});
  Step(0);Put(state+disk::PoliceTimeRemaining,int{-1});
 }
 bool Step(std::uint64_t now){reads=0;return cfgpoliceexit::Step(*this,*this,objects,module,world,mode,state,cache,now);}
};
int main(){
 CHECK(CombatOwns(400.f*400.f,100,0,0,false));
 CHECK(CombatOwns(1000.f*1000.f,100,101,0,false));
 CHECK(CombatOwns(1000.f*1000.f,100,0,1,false));
 CHECK(CombatOwns(1000.f*1000.f,100,0,0,true));
 CHECK(!CombatOwns(401.f*401.f,100,100,0,false));
 CHECK(HuntAvailable(false,false,false,false,false,false,false,false,false,false));
 CHECK(!HuntAvailable(true,false,false,false,false,false,false,false,false,false));
 CHECK(!HuntAvailable(false,true,false,false,false,false,false,false,false,false));
 CHECK(!HuntAvailable(false,false,false,false,false,false,false,true,false,false));
 {ExitFixture f;CHECK(f.Step(1000)&&f.cache.count==1&&f.locations==1&&!f.queries);
  CHECK(f.Step(2000)&&f.cache.count==1&&f.locations==1); // car entry ignored
  CHECK(f.Step(3000)&&f.cache.complete);
  CHECK(f.Step(4000)&&f.queries==1&&f.observations==1&&f.cache.entries[0].active);
  CHECK(!f.Step(4500)&&f.queries==1);f.collision=0;
  CHECK(f.Step(5000)&&f.queries==2&&!f.cache.entries[0].active);
  f.Put(f.state+disk::PoliceTimeRemaining,int{30});CHECK(!f.Step(6000)&&f.queries==2&&!f.cache.entries[0].active);}
 {ExitFixture f;f.cache.Clear();CHECK(!f.Step(1000)&&!f.queries&&!f.locations&&!f.cache.count);}
 for(unsigned fault=0;fault<5;++fault){ExitFixture f;f.Step(1000);f.Step(2000);f.Step(3000);
  if(fault==0){int index=0;f.Read(f.police+12,index);f.Put(f.objects.data+24ULL*static_cast<unsigned>(index)+16,int{18});}
  if(fault==1)f.Put(f.sphere+0x20,f.car);
  if(fault==2)f.Put(f.listData,f.car);
  if(fault==3)f.Put(f.module+disk::EscapeClassConstructor,f.sphereType);
  if(fault==4)f.Put(f.police+disk::EscapeType,std::uint8_t{1});
  CHECK(f.Step(4000)&&!f.queries&&!f.cache.count);}
 {ExitFixture f;f.Step(1000);f.Step(2000);f.Step(3000);f.queryOkay=false;
  CHECK(f.Step(4000)&&!f.cache.entries[0].active&&!f.cache.entries[0].sampledAt);}
 // H2.48: objective mode accepts EscapeType 1..4 (Car/Boat/Pod/GrendelBridge) only; police mode stays type 0.
 for(unsigned t=0;t<6;++t){ExitFixture f;f.Put(f.police+disk::EscapeType,static_cast<std::uint8_t>(t));
  List list{};CHECK(f.Read(f.mode+disk::ModeVolumes,list));Entry e;e.volume=f.Serial(f.police);e.sphere=f.Serial(f.sphere);e.sourceIndex=0;
  CHECK(cfgpoliceexit::Current(f,f.objects,f.module,f.world,f.mode,list,e)==(t==0));
  CHECK(cfgpoliceexit::Current(f,f.objects,f.module,f.world,f.mode,list,e,true)==(t>=1&&t<=4));}
 std::printf("Police exit bounded two-volume discovery, native-query gateway, timer and ownership refusals: %d failures; no game access\n",failures);
 return failures?1:0;
}
