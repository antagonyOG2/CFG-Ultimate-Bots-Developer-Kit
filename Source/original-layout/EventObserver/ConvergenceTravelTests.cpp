#include "../Core/ConvergenceTravelPolicy.hpp"
#include "ConvergenceBrainObservation.hpp"
#include "ConvergenceTravelParameters.hpp"
#include <map>
#include <cstring>
#include <cassert>
#include <cstdio>
struct BrainFixture {
 std::map<std::uintptr_t,std::array<unsigned char,16>> memory;
 cfgprobe::lifetime::ArrayHeader array{0x60000000,64};
 cfgprobe::lifetime::SlotObservation controller{0x40000000,1,100};
 std::uintptr_t brain=0x40010000,aiClass=0x50000000,brainClass=0x50010000;
 unsigned reads=0;bool mutate=false;
 template<class T>void Put(std::uintptr_t at,T value){auto& cell=memory[at];cell={};std::memcpy(cell.data(),&value,sizeof(value));}
 template<class T>bool Read(std::uintptr_t at,T& value){
  if(++reads>256)return false;const auto i=memory.find(at);if(i==memory.end())return false;
  std::memcpy(&value,i->second.data(),sizeof(value));
  if(mutate&&at==brain+cfgjason::disk::ComponentOwner){mutate=false;Put(controller.object+0x418,std::uintptr_t{0x40020000});}
  return true;
 }
 void Slot(std::uintptr_t object,int index,int serial){Put(object+0xC,index);Put(array.data+index*24,object);Put(array.data+index*24+8,std::uint32_t{0});Put(array.data+index*24+16,serial);}
 BrainFixture(){Slot(controller.object,1,100);Slot(brain,2,0);
  Put(controller.object+0x10,aiClass);Put(brain+0x10,brainClass);
  Put(aiClass+cfgprobe::ClassSuper,std::uintptr_t{0});Put(brainClass+cfgprobe::ClassSuper,std::uintptr_t{0});
  Put(controller.object+0x418,brain);Put(brain+0x20,controller.object);Put(brain+cfgjason::disk::ComponentOwner,controller.object);
 }
 bool Check(){cfgprobe::lifetime::SlotObservation out{};reads=0;return cfgconvergencetravel::Brain(*this,array,controller,aiClass,brainClass,out)&&out.object==brain;}
};
int main(){using namespace cfgconvergencetravel;Policy p;
 {const auto packet=MovePacket({10,20,30});assert(packet.destination.x==10&&packet.destination.y==20&&packet.destination.z==30);
  assert(packet.acceptance==100&&!packet.stopOnOverlap&&packet.pathfinding&&packet.projectNavigation&&packet.strafe&&packet.allowPartial&&!packet.filter);}
 {BrainFixture f;assert(f.Check());assert(f.reads<256);f.Slot(f.brain,2,123);assert(f.Check());}
 for(unsigned n=0;n<7;++n){BrainFixture f;
  if(n==0)f.Put(f.brain+0x20,std::uintptr_t{0x40020000});
  if(n==1)f.Put(f.brain+cfgjason::disk::ComponentOwner,std::uintptr_t{0x40020000});
  if(n==2)f.Put(f.brain+0x10,f.aiClass);
  if(n==3)f.Slot(f.controller.object,1,101);
  if(n==4)f.Put(f.array.data+2*24,std::uintptr_t{0x40020000});
  if(n==5)f.mutate=true;
  if(n==6)f.Put(f.array.data+2*24+8,std::uint32_t{0x20000000});
  assert(!f.Check());
 }
 assert(p.Decide(301,0,true)==Action::Acquire);p.Acquired(1000,0,1000,0);
 assert(p.Decide(1000,0,true,1000,0)==Action::Move);p.Moved(0);
 assert(p.Decide(950,1000,true,950,0)==Action::None);
 assert(p.Decide(910,2000,true,910,0)==Action::None);
 assert(p.Decide(910,3500,true,910,0)==Action::None); // donor6001ms persistent route, not3500ms replanning
 assert(p.Decide(910,5500,true,910,0)==Action::Recover);p.Resumed(5500,true);
 assert(p.Decide(900,6500,true)==Action::None);
 assert(p.Decide(900,6501,true)==Action::Acquire);p.Acquired(900,6501);
 assert(p.Decide(250,6600,true)==Action::Move);
 assert(p.Decide(220,6700,true)==Action::Resume);p.Resumed(6700,false);
 assert(p.Decide(250,6800,true)==Action::None);
 p.Acquired(900,7000);assert(p.Decide(900,17000,true)==Action::Move); // never recover before first submitted path
 assert(p.Decide(900,17001,false)==Action::Resume);
 {Policy detour;detour.Acquired(1000,0,1000,0);detour.Moved(0);
  assert(detour.Decide(1050,3000,true,1000,100)==Action::None); // farther, but actually moving around obstruction
  assert(detour.Decide(1100,5500,true,1000,200)==Action::None);
  assert(detour.Decide(1150,6001,true,1000,300)==Action::Move);
  detour.Moved(6001);assert(detour.Decide(1150,6200,true,1000,300,401,0)==Action::Move); // Jason moved>4m
  assert(detour.Decide(NAN,6201,true)==Action::Resume);
 }
 puts("Convergence travel hysteresis, persistent path, progress and recovery tests PASS");
}
