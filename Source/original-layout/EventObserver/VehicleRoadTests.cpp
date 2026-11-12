#include "VehicleRoadRuntime.hpp"
#include <map>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <memory>
static void Check(bool value,const char* label){if(!value){std::printf("FAIL %s\n",label);std::exit(1);}}
struct Memory {
 std::map<std::uintptr_t,unsigned char> bytes;unsigned reads=0;
 template<class T>void Put(std::uintptr_t at,const T& value){const auto p=reinterpret_cast<const unsigned char*>(&value);for(unsigned i=0;i<sizeof(T);++i)bytes[at+i]=p[i];}
 template<class T>bool Read(std::uintptr_t at,T& value){++reads;auto p=reinterpret_cast<unsigned char*>(&value);for(unsigned i=0;i<sizeof(T);++i){auto it=bytes.find(at+i);if(it==bytes.end())return false;p[i]=it->second;}return true;}
};
struct Fixture {
 Memory memory;cfgroad::Runtime runtime;
 static constexpr std::uintptr_t module=0x140000000,world=0x100000,level=0x200000,registry=0x300000,road=0x400000,scene=0x410000,levelType=0x420000,levels=0x500000,actors=0x510000;
 static constexpr std::uintptr_t Actor(unsigned i){return 0x600000+0x1000*i;}
 static constexpr std::uintptr_t Root(unsigned i){return 0x700000+0x1000*i;}
 static constexpr std::uintptr_t Links(unsigned i){return 0x800000+0x100*i;}
 void Slot(std::uintptr_t object,int index){memory.Put(object+12,index);memory.Put(registry+24*index,object);memory.Put(registry+24*index+8,std::uint32_t{0});memory.Put(registry+24*index+16,100+index);}
 Fixture(){using namespace cfgroad;
  memory.Put(module+cfgprobe::lifetime::ObjectArrayRva+16,registry);memory.Put(module+cfgprobe::lifetime::ObjectArrayRva+28,100);
  Slot(world,0);memory.Put(world+cfgradarworld::disk::WorldLevels,List{levels,1,1});memory.Put(levels,level);
  memory.Put(level+cfgprobe::ObjectClass,levelType);memory.Put(level+cfgkill::disk::OwningWorld,world);memory.Put(level+cfgradarworld::disk::LevelActors,List{actors,4,4});
  memory.Put(module+disk::RoadClassCache,road);memory.Put(module+disk::RoadConstructorCache,road);
  memory.Put(module+cfgchallengeescape::disk::SCEscapeVolume,std::uintptr_t{});
  memory.Put(module+cfgchallengeescape::disk::SCEscapeVolumeCtor,std::uintptr_t{});
  memory.Put(module+cfgposition::disk::SceneClassCache,scene);memory.Put(module+cfgposition::disk::SceneConstructorCache,scene);
  memory.Put(module+cfgradarworld::disk::LevelClassCache,levelType);memory.Put(module+cfgradarworld::disk::LevelConstructorCache,levelType);
  memory.Put(road+cfgprobe::ClassSuper,std::uintptr_t{0});memory.Put(scene+cfgprobe::ClassSuper,std::uintptr_t{0});memory.Put(levelType+cfgprobe::ClassSuper,std::uintptr_t{0});
  for(unsigned i=0;i<4;++i){const auto actor=Actor(i),root=Root(i);Slot(actor,static_cast<int>(i+1));Slot(root,static_cast<int>(i+5));
   memory.Put(actors+8*i,actor);memory.Put(actor+cfgprobe::ObjectClass,road);memory.Put(actor+0x20,level);
   memory.Put(actor+cfgposition::disk::ActorRootComponent,root);memory.Put(root+cfgprobe::ObjectClass,scene);memory.Put(root+0x20,actor);memory.Put(root+0xE0,actor);
   memory.Put(root+cfgposition::disk::SceneWorldTranslation,Vector{5000.f*i,0,0});
   const int count=(i==0||i==3)?1:2;memory.Put(actor+disk::Neighbors,List{Links(i),count,count});
   if(i==0)memory.Put(Links(i),Actor(1));else{memory.Put(Links(i),Actor(i-1));if(i<3)memory.Put(Links(i)+8,Actor(i+1));}
  }
 }
 void Prepare(){for(unsigned i=0;i<20&&runtime.stage!=cfgroad::Stage::Ready&&runtime.stage!=cfgroad::Stage::Refused;++i){
  memory.reads=0;runtime.Step(memory,module,world,true);Check(memory.reads<=1536&&runtime.lastReads==memory.reads,"bounded preparation reads");}
  if(runtime.stage!=cfgroad::Stage::Ready)std::printf("stage=%u failure=%u count=%u captured=%u reads=%u\n",static_cast<unsigned>(runtime.stage),runtime.failure,runtime.graph.count,runtime.captured,runtime.lastReads);
  Check(runtime.stage==cfgroad::Stage::Ready,"ready graph");Check(runtime.graph.count==4,"complete road census");}
 bool Intercept(){cfgroad::Candidate c;memory.reads=0;const bool ok=runtime.Intercept(memory,world,{1000,0,0},{1,0,0},1000,true,c);
  Check(memory.reads<=1536&&runtime.lastReads==memory.reads,"bounded intercept reads");if(ok)Check(std::fabs(c.location.x-8500)<.1f,"native graph position");return ok;}
};
static cfgroad::Graph Line(unsigned count,float gap){cfgroad::Graph g;g.count=count;for(unsigned i=0;i<count;++i){g.nodes[i].position={gap*i,0,0};
 if(i)g.nodes[i].neighbors[g.nodes[i].count++]=i-1;if(i+1<count)g.nodes[i].neighbors[g.nodes[i].count++]=i+1;}return g;}
int main(){using namespace cfgroad;Candidate c;
 {Fixture f;const auto exitType=0x430000ULL;
  f.memory.Put(Fixture::module+cfgchallengeescape::disk::SCEscapeVolume,exitType);
  f.memory.Put(Fixture::module+cfgchallengeescape::disk::SCEscapeVolumeCtor,exitType);
  f.memory.Put(exitType+cfgprobe::ClassSuper,std::uintptr_t{});
  f.memory.Put(Fixture::level+cfgradarworld::disk::LevelActors,List{Fixture::actors,5,5});
  f.memory.Put(Fixture::actors+32,Fixture::Actor(4));f.Slot(Fixture::Actor(4),11);f.Slot(Fixture::Root(4),12);
  f.memory.Put(Fixture::Actor(4)+cfgprobe::ObjectClass,exitType);f.memory.Put(Fixture::Actor(4)+0x20,Fixture::level);
  f.memory.Put(Fixture::Actor(4)+cfgposition::disk::ActorRootComponent,Fixture::Root(4));
  f.memory.Put(Fixture::Root(4)+cfgprobe::ObjectClass,Fixture::scene);f.memory.Put(Fixture::Root(4)+0x20,Fixture::Actor(4));
  f.memory.Put(Fixture::Root(4)+0xE0,Fixture::Actor(4));
  f.memory.Put(Fixture::Root(4)+cfgposition::disk::SceneWorldTranslation,Vector{15000,0,0});
  f.Prepare();Check(f.runtime.exitCount==1&&f.runtime.ExitCandidate({10000,0,0},{1,0,0},c),"typed exit captured with road survey at50m");
  f.memory.reads=512;
  Check(f.runtime.Intercept(f.memory,Fixture::world,{10000,0,0},{1,0,0},1000,true,c,1536-512-220)&&c.exitBlock&&c.location.x==15000,"native50m exit priority over ordinary intercept");
  Check(f.memory.reads+220<=1536,"exit plus road proof remains inside shared read budget");
  f.runtime.blockedCar=99;f.runtime.blockedExit=0;
  Check(!f.runtime.ExitCandidate({10000,0,0},{1,0,0},c,99),"one exit intercept per occupied-car episode prevents repeated block teleports");
  f.memory.Put(Fixture::Root(4)+cfgposition::disk::SceneWorldTranslation,Vector{15000,250,0});
  f.memory.reads=0;Check(!f.runtime.Intercept(f.memory,Fixture::world,{10000,0,0},{1,0,0},1000,true,c),"moved exit source refuses cached destination");
 }
 {auto straight=Line(5,5000);
  Check(ResolveExit(straight,{15000,0,0},{1,0,0},{20000,0,0},c)&&c.exitBlock&&c.location.x==20000&&c.requestedLead==5000,"50m approaching exit blocks at terminal road centre");
  Check(ResolveExit(straight,{15001,0,0},{1,0,0},{20000,0,0},c)&&c.exitBlock,"inside50m allows exit block");
  Check(!ResolveExit(straight,{14999,0,0},{1,0,0},{20000,0,0},c)&&!c.exitBlock,"just beyond50m keeps ordinary intercept");
  Check(!ResolveExit(straight,{10000,0,0},{1,0,0},{20000,0,0},c),"old100m trigger no longer blocks exit");
  Check(!ResolveExit(straight,{5000,0,0},{1,0,0},{20000,0,0},c),"old150m trigger no longer blocks exit");
  Check(!ResolveExit(straight,{7500,0,0},{1,0,0},{20000,0,0},c),"125m does not trigger exit block");
  Check(!ResolveExit(straight,{16000,0,0},{-1,0,0},{20000,0,0},c),"driving away from exit does not trigger");
  Check(!ResolveExit(straight,{16000,0,0},{1,0,0},{20000,701,0},c),"off-road escape volume cannot create a shoulder block");
  Check(!ResolveExit(straight,{6000,0,0},{1,0,0},{10000,0,0},c),"interior volume is not a road exit");
  Check(ResolveExit(straight,{5000,0,0},{-1,0,0},{0,0,0},c)&&c.location.x==0,"reverse-direction50m exit centre");
  Check(!ResolveExit(straight,{5001,0,0},{-1,0,0},{0,0,0},c),"reverse-direction beyond50m refuses exit block");
 }
 {auto curved=Line(4,2000);curved.nodes[2].position={2000,2000,0};curved.nodes[3].position={4000,2000,0};
  Check(Chase(curved,{500,0,90},{3500,2000,0},c)&&c.location.x==2000&&c.location.y==0,"far pursuit stops at first bend, not a fence-cutting hood chord");
  Check(Chase(curved,{2000,0,90},{3500,2000,0},c)&&c.location.x==2000&&c.location.y==2000,"advance connected next road segment");
  Check(Chase(curved,{500,290,90},{3500,2000,0},c)&&c.location.x==500&&c.location.y==0,"shoulder returns to current road centre first");
  Check(!Chase(curved,{500,701,90},{3500,2000,0},c),"off-road pursuit refuses invented road chord");
  Check(!Chase(curved,{NAN,0,0},{3500,2000,0},c),"nonfinite chase rejected");
  curved.nodes[0].count=9;Check(!Chase(curved,{0,0,0},{3500,2000,0},c),"invalid graph rejected");
 }
 {auto captured=Line(5,1000);
  captured.nodes[0].position={-4976.731f,5866.937f,216.180f};
  captured.nodes[1].position={-5913.163f,5880.442f,165.533f};
  captured.nodes[2].position={-6983.227f,6120.229f,126.484f};
  captured.nodes[3].position={-8677.519f,6739.458f,144.936f};
  captured.nodes[4].position={-10568.639f,7873.312f,383.185f};
  Check(Chase(captured,{-7547.1f,6326.3f,228.8f},{-5536.625f,5887.485f,182.206f},c)&&
    std::fabs(c.location.x+6983.227f)<.1f&&std::fabs(c.location.y-6120.229f)<.1f,
    "PID14344 Morph landing follows node37 around bend toward car, not straight across fence");
 }
 {Fixture f;f.Prepare();f.memory.reads=0;
  Check(f.runtime.ChaseWaypoint(f.memory,Fixture::world,{1000,0,90},{13000,0,0},c)&&c.location.x==5000,"live selected road chase proof");
  Check(f.memory.reads+512+220<=1536,"selected chase proof leaves owner/mutation reserve");
  f.memory.Put(Fixture::Root(1)+cfgposition::disk::SceneWorldTranslation,Vector{5000,100,0});
  Check(!f.runtime.ChaseWaypoint(f.memory,Fixture::world,{1000,0,90},{13000,0,0},c),"moved authored road refuses chase dispatch");
 }
 {Vector result{};
  Check(Landing({8500,100,10},{8500,280,30},result)&&result.x==8500&&result.y==100&&result.z==120,"projection cannot move road X/Y to shoulder");
  Check(!Landing({8500,100,10},{9200,100,10},result),"stale projection rejected");
  Check(!Landing({8500,100,10},{8500,100,231},result),"donor220cm ground height bound");
  Check(!Landing({NAN,0,0},{0,0,0},result),"nonfinite road landing rejected");}
 Check(RequestDue(20000,20000,0,true,0,600.f*600.f),"queue eligible donor road request");
 Check(RequestDue(9000,9000,0,true,0,500.f*500.f,true),"five-second tail may intercept inside old six-meter gate");
 Check(!RequestDue(9000,9000,0,true,1,500.f*500.f,true),"tail does not interrupt paired vehicle interaction");
 Check(!RequestDue(9000,9000,0,true,0,275.f*275.f,true),"tail does not interrupt immediate hood contact");
 Check(MayMorph({},{-500,0,0},{1,0,0},true)&&!MayMorph({},{-275,0,0},{1,0,0},true),"tail dispatch proximity proof");
 Check(!RequestDue(20100,20000,20000+FailedRoadRetry,true,0,800.f*800.f),"failed road yields next tick to physical pursuit");
 Check(RequestDue(20000+FailedRoadRetry,20000,20000+FailedRoadRetry,true,0,800.f*800.f),"bounded retry after physical progress");
 Check(!RequestDue(22000,20000,0,true,1,800.f*800.f)&&!RequestDue(22000,20000,0,true,2,800.f*800.f),"never Morph over owned hood/seat phase");
 Check(!RequestDue(22000,40000,0,true,0,800.f*800.f)&&!RequestDue(22000,20000,0,false,0,800.f*800.f),"cooldown/stationary keeps physical lane");
 Check(RequestedLead(0)==7000&&RequestedLead(1000)==7500&&RequestedLead(3000)==12000,"donor lead clamp");
 Check(!MayMorph({0,0,0},{-599,0,0},{1,0,0})&&MayMorph({0,0,0},{-601,0,0},{1,0,0}),"donor 6m proximity gate");
 Check(!MayMorph({0,0,0},{5000,0,0},{1,0,0})&&MayMorph({0,0,0},{5000,1300,0},{1,0,0}),"already ahead suppression");
 auto g=Line(4,5000);Check(Resolve(g,{1000,0,0},{1,0,0},1000,c)&&c.location.x==8500,"forward projection lead");
 Check(Resolve(g,{14000,0,0},{-1,0,0},1000,c)&&c.location.x==6500,"reverse edge orientation");
 Check(Resolve(g,{1000,100,0},{1,0,0},1000,c)&&c.location.y==100&&c.laneCorrection==100,"donor measured lane corrects authored shoulder graph");
 Check(Resolve(g,{1000,-200,0},{1,0,0},1000,c)&&c.location.y==-200&&c.laneCorrection==-200,"opposite lane calibration sign");
 Check(Resolve(g,{1000,-295,0},{.8f,.6f,0},1000,c)&&c.location.y==-295&&c.laneCorrection==-295,"CFG55 observed .8 alignment retains295cm lane instead of shoulder");
 Check(Resolve(g,{1000,-295,0},{.7f,std::sqrt(.51f),0},1000,c)&&c.laneCorrection==0,"large heading mismatch retains no calibration");
 Check(Resolve(g,{1000,64,0},{1,0,0},1000,c)&&c.location.y==0&&c.laneCorrection==0,"sub65cm offset ignored");
 Check(Resolve(g,{1000,321,0},{1,0,0},1000,c)&&c.location.y==0&&c.laneCorrection==0,"large displacement not calibrated");
 Check(Resolve(g,{14000,100,0},{-1,0,0},1000,c)&&c.location.y==100&&c.laneCorrection==-100,"reverse heading retains measured lane");
 Check(!Resolve(g,{1000,701,0},{1,0,0},1000,c),"off-road rejects without straight fallback");
 g.nodes[2].position={10000,5000,0};g.nodes[3].position={15000,10000,0};
 Check(Resolve(g,{1000,0,0},{1,0,0},1000,c)&&c.location.y>2400&&c.location.y<2500,"follow curved authored road");
 g=Line(2,10000);Check(Resolve(g,{0,0,0},{1,0,0},1300,c)&&std::fabs(c.location.x-7800)<.1f,"endpoint backoff22m");
 Check(!Resolve(g,{8000,0,0},{1,0,0},1300,c),"terminal candidate below12m rejected");
 g=Line(3,5000);Check(Resolve(g,{0,0,0},{1,0,0},2000,c)&&c.location.x==5000,"donor last interior fallback");
 g.count=0;Check(!Resolve(g,{0,0,0},{1,0,0},1000,c),"empty graph no fallback");
 {auto fixture=std::make_unique<Fixture>();auto& f=*fixture;f.Prepare();Check(f.Intercept(),"adapter positive");const auto slices=f.runtime.slices;f.memory.reads=0;
  f.runtime.Step(f.memory,f.module,f.world,true);Check(!f.memory.reads&&f.runtime.slices==slices,"ready never recensus");
  f.memory.Put(f.Links(1)+8,f.Actor(3));Check(!f.Intercept(),"changed neighbor link rejected");}
 {auto fixture=std::make_unique<Fixture>();auto& f=*fixture;f.Prepare();f.memory.Put(f.registry+24+16,999);Check(!f.Intercept(),"recycled actor slot rejected");}
 {auto fixture=std::make_unique<Fixture>();auto& f=*fixture;f.Prepare();f.memory.Put(f.Root(1)+cfgposition::disk::SceneWorldTranslation,Vector{5001,0,0});Check(!f.Intercept(),"changed root position rejected");}
 {auto fixture=std::make_unique<Fixture>();auto& f=*fixture;f.Prepare();f.memory.Put(f.Root(1)+0xE0,f.Actor(2));Check(!f.Intercept(),"wrong root owner rejected");}
 {auto fixture=std::make_unique<Fixture>();auto& f=*fixture;f.Prepare();f.memory.Put(f.module+disk::RoadConstructorCache,f.scene);Check(!f.Intercept(),"changed road class cache rejected");}
 {auto fixture=std::make_unique<Fixture>();auto& f=*fixture;f.Prepare();f.memory.Put(f.actors,f.Actor(3));Check(!f.Intercept(),"changed source roster slot rejected");}
 {auto fixture=std::make_unique<Fixture>();auto& f=*fixture;f.Prepare();f.memory.Put(f.level+cfgkill::disk::OwningWorld,std::uintptr_t{0x990000});Check(!f.Intercept(),"wrong world rejected");}
 {auto fixture=std::make_unique<Fixture>();auto& f=*fixture;
  f.memory.Put(f.level+cfgradarworld::disk::LevelActors,List{f.actors,68,68});
  const std::uintptr_t unrelated=0x900000;f.memory.Put(unrelated+cfgprobe::ClassSuper,std::uintptr_t{0});
  for(unsigned i=0;i<64;++i){const std::uintptr_t object=0xA00000+0x100*i;f.memory.Put(f.actors+8*i,object);f.memory.Put(object+cfgprobe::ObjectClass,unrelated);}
  for(unsigned i=0;i<4;++i)f.memory.Put(f.actors+8*(64+i),f.Actor(i));
  f.Prepare();Check(f.runtime.sources[0].actorIndex==64&&f.Intercept(),"paged census skips unrelated actors");}
 {auto fixture=std::make_unique<Fixture>();auto& f=*fixture;f.Prepare();Candidate rejected;
  f.memory.reads=0;Check(!f.runtime.Intercept(f.memory,f.world,{1000,0,0},{1,0,0},1000,false,rejected)&&!f.memory.reads,"unauthorized scope performs no reads");
  f.memory.Put(f.Root(1)+12,99);Check(!f.Intercept(),"recycled root slot rejected");}
 {auto fixture=std::make_unique<Fixture>();auto& f=*fixture;f.memory.Put(f.Actor(1)+disk::Neighbors,List{f.Links(1),9,9});
  for(unsigned i=0;i<4;++i)f.runtime.Step(f.memory,f.module,f.world,true);
  Check(f.runtime.stage==Stage::Refused,"oversize neighbor list rejected");f.memory.reads=0;f.runtime.Step(f.memory,f.module,f.world,true);Check(!f.memory.reads,"refusal has no ongoing scan");}
 {Memory memory;Budget<Memory> budget(memory);int value=0;for(unsigned i=0;i<2000;++i)budget.Read(0x100000,value);Check(budget.reads==1536&&memory.reads==1536,"absolute hard read cap");}
 {auto fixture=std::make_unique<Fixture>();auto& f=*fixture;f.Prepare();
  // Ordinary projectile/trap/helper births relocate/grow a roster, but the
  // authored road cells remain current. Do not recensus or reject this churn.
  const std::uintptr_t relocated=0x530000;
  for(unsigned i=0;i<4;++i)f.memory.Put(relocated+8*i,f.Actor(i));
  f.memory.Put(relocated+32,std::uintptr_t{0x990000});
  f.memory.Put(f.level+cfgradarworld::disk::LevelActors,List{relocated,5,5});
  Check(f.Intercept(),"unrelated actor birth/roster relocation preserves current authored road cells");}
 {auto fixture=std::make_unique<Fixture>();auto& f=*fixture;
  unsigned peak=0;
  for(unsigned i=0;i<20&&f.runtime.stage!=Stage::Ready&&f.runtime.stage!=Stage::Refused;++i){
   f.memory.reads=512;f.runtime.Step(f.memory,f.module,f.world,true,1536-512);
   peak=(std::max)(peak,f.memory.reads);Check(f.memory.reads<=1536,"shared preparation budget");}
  Check(f.runtime.stage==Stage::Ready,"prepared under shared owner reserve");
  f.memory.reads=512;Candidate measured;
  Check(f.runtime.Intercept(f.memory,f.world,{1000,0,0},{1,0,0},1000,true,measured,1536-512-220),"composed route with mutation reserve");
  peak=(std::max)(peak,f.memory.reads+220);Check(peak<=1536,"shared route plus mutation reserve");
  std::printf("Actual road adapter composed peak=%u/1536 incl512 owner +220 mutation reserve\n",peak);}
 std::puts("Vehicle road policy / source adapter tests PASS");return 0;
}
