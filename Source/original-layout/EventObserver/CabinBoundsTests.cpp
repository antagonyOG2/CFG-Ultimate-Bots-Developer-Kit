#include "CabinBoundsRuntime.hpp"
#include <map>
#include <cassert>
#include <cstdio>
#include <limits>
struct Reader {
 std::map<std::uintptr_t,unsigned char> bytes;unsigned reads=0;
 template<class T>void Put(std::uintptr_t at,const T& value){const auto* b=reinterpret_cast<const unsigned char*>(&value);for(unsigned i=0;i<sizeof(T);++i)bytes[at+i]=b[i];}
 template<class T>bool Read(std::uintptr_t at,T& value){if(reads>=1536)return false;++reads;auto* b=reinterpret_cast<unsigned char*>(&value);for(unsigned i=0;i<sizeof(T);++i){auto it=bytes.find(at+i);if(it==bytes.end())return false;b[i]=it->second;}return true;}
};
struct Fixture {
 static constexpr std::uintptr_t Module=0x140000000,World=0x200000,Level=0x210000,Actor=0x220000,Other=0x230000,Levels=0x240000,Actors=0x250000,LevelClass=0x300000,CabinClass=0x310000,BP=0x320000;
 Reader r;cfgprobe::lifetime::ArrayHeader registry{0x100000,256};cfgcabin::Runtime runtime{};
 unsigned calls=0,prepare=0,peak=0,scopeCalls=0;bool scope=true,failCall=false,mutate=false;unsigned prepareResult=1;
 Fixture(){
  r.Put(Module+cfgradarworld::disk::LevelClassCache,LevelClass);r.Put(Module+cfgradarworld::disk::LevelConstructorCache,LevelClass);
  r.Put(Module+cfgradarworld::disk::CabinClassCache,CabinClass);r.Put(Module+cfgradarworld::disk::CabinConstructorCache,CabinClass);
  r.Put(LevelClass+0x30,std::uintptr_t{});r.Put(CabinClass+0x30,std::uintptr_t{});r.Put(BP+0x30,CabinClass);
  r.Put(Level+0x10,LevelClass);r.Put(Actor+0x10,BP);r.Put(Other+0x10,LevelClass);r.Put(Actor+0x20,Level);
  r.Put(World+cfgradarworld::disk::WorldLevels,cfgshacktrap::List{Levels,1,1});r.Put(Levels,Level);
  r.Put(Level+cfgkill::disk::OwningWorld,World);List(2048);
  Slot(Actor,0);
 }
 void Slot(std::uintptr_t object,int index){r.Put(object+0xC,index);r.Put(registry.data+24*index,object);r.Put(registry.data+24*index+8,std::uint32_t{});r.Put(registry.data+24*index+16,index+1);}
 void List(unsigned count){r.Put(Level+cfgradarworld::disk::LevelActors,cfgshacktrap::List{Actors,static_cast<int>(count),static_cast<int>(count)});for(unsigned i=0;i<count;++i)r.Put(Actors+8*i,i+1==count?Actor:Other);}
 bool Scope(){++scopeCalls;return scope;}
 unsigned PrepareBounds(){++prepare;r.reads+=400;return prepareResult;}
 bool ReadBounds(std::uintptr_t actor,cfgcabin::Packet& packet){assert(actor==Actor&&packet.onlyColliding==0);++calls;r.reads+=400;
  packet.origin={1000,2000,100};packet.extent={500,400,300};if(mutate)r.Put(Actor+0x20,World);return !failCall;}
 void Tick(){r.reads=384;const auto before=calls;runtime.Step(r,registry,Module,World,*this);assert(calls-before<=1&&r.reads<=1536);if(r.reads>peak)peak=r.reads;}
 void Ready(){for(unsigned i=0;i<600&&runtime.cache.stage!=cfgcabin::Stage::Ready&&runtime.cache.stage!=cfgcabin::Stage::Refused;++i)Tick();assert(runtime.cache.stage==cfgcabin::Stage::Ready);}
};
int main(){
 {cfgcabin::Packet p;assert(p.onlyColliding==0&&sizeof(p)==28);cfgcabin::Cache c;c.world=1;c.count=1;c.stage=cfgcabin::Stage::Capture;
  assert(!c.Allows(1,{}));p.origin={1000,2000,100};p.extent={500,400,300};assert(c.Add(p));c.stage=cfgcabin::Stage::Ready;
  assert(!c.Allows(1,{1000,2000,100})&&!c.Allows(1,{1600,2500,9999}));assert(c.Allows(1,{1601,2500,100}));assert(!c.Allows(2,{9999,9999,0}));
  assert(!c.Allows(1,{std::numeric_limits<float>::quiet_NaN(),0,0}));}
 {cfgcabin::Cache c;c.world=1;c.count=c.captured=1;c.stage=cfgcabin::Stage::Ready;
  c.bounds[0]={{1000,2000,100},{500,400,300}};
  assert(c.Encloses(1,{1000,2000,100})&&!c.Encloses(1,{1601,2000,100}));
  assert(!c.Encloses(1,{1000,2000,501})&&!c.Encloses(2,{1000,2000,100}));
  c.stage=cfgcabin::Stage::Capture;assert(!c.Encloses(1,{1000,2000,100}));}
 for(unsigned fault=0;fault<4;++fault){cfgcabin::Cache c;c.count=1;cfgcabin::Packet p;p.extent={1,1,1};if(fault==0)p.extent.x=0;if(fault==1)p.extent.y=-1;if(fault==2)p.extent.z=std::numeric_limits<float>::infinity();if(fault==3)p.origin.x=std::numeric_limits<float>::quiet_NaN();assert(!c.Add(p)&&c.stage==cfgcabin::Stage::Refused);}
 unsigned peak=0;
 {Fixture f;f.Ready();assert(f.calls==1&&f.prepare==1&&f.runtime.cache.count==1&&f.runtime.cache.captured==1);assert(!f.runtime.cache.Allows(f.World,{1000,2000,0})&&f.runtime.cache.Allows(f.World,{5000,5000,0}));peak=f.peak;
  f.r.reads=0;f.runtime.Step(f.r,f.registry,f.Module,f.World,f);assert(f.r.reads==0&&f.calls==1);}
 for(unsigned fault=0;fault<5;++fault){Fixture f;while(f.runtime.cache.stage!=cfgcabin::Stage::Capture)f.Tick();
  if(fault==0)f.r.Put(f.Actor+0x20,f.World);if(fault==1)f.r.Put(f.Level+cfgkill::disk::OwningWorld,f.Actor);if(fault==2)f.r.Put(f.Actors+2047*8,std::uintptr_t{});if(fault==3)f.r.Put(f.registry.data+16,int{99});if(fault==4)f.prepareResult=2;
  for(unsigned i=0;i<3;++i)f.Tick();assert(f.runtime.cache.stage==cfgcabin::Stage::Refused&&f.calls==0);}
 {Fixture f;f.mutate=true;for(unsigned i=0;i<12&&f.runtime.cache.stage!=cfgcabin::Stage::Refused;++i)f.Tick();assert(f.calls==1&&f.runtime.cache.captured==0&&f.runtime.cache.failure==11);f.Tick();assert(f.calls==1);}
 {Fixture f;f.failCall=true;for(unsigned i=0;i<12&&f.runtime.cache.stage!=cfgcabin::Stage::Refused;++i)f.Tick();assert(f.calls==1&&f.runtime.cache.captured==0);}
 {Fixture f;f.prepareResult=0;for(unsigned i=0;i<80;++i)f.Tick();assert(f.prepare==64&&f.calls==0&&f.runtime.cache.failure==10);}
 {Fixture f;f.scope=false;f.Tick();assert(f.runtime.cache.stage==cfgcabin::Stage::Dormant&&f.r.reads==384&&f.calls==0);}
 {Fixture f;f.Tick();f.r.Put(f.Level+cfgradarworld::disk::LevelActors,cfgshacktrap::List{f.Actors+8,2047,2047});f.Tick();assert(f.runtime.cache.stage==cfgcabin::Stage::Dormant&&f.calls==0&&f.runtime.rebuilds==1);f.Ready();}
 {Fixture f;f.Ready();f.r.reads=0;assert(f.runtime.Allows(f.r,f.World,{9000,9000,0}));assert(f.r.reads==5);
  for(unsigned i=0;i<100;++i){f.List(2049+i);f.r.reads=0;assert(!f.runtime.Allows(f.r,f.World,{9000,9000,0})&&f.runtime.cache.stage==cfgcabin::Stage::Dormant);
   f.Ready();assert(f.calls==1&&f.runtime.cache.calls==1&&f.runtime.rebuilds==i+1);}
  // Growth with stable cabin prefix is a single appended non-cabin actor.
  const auto count=2149;f.r.Put(f.Actors+8*(count-1),f.Other);
  f.r.Put(f.Level+cfgradarworld::disk::LevelActors,cfgshacktrap::List{f.Actors,count,count});
  f.r.reads=0;assert(!f.runtime.Allows(f.r,f.World,{9000,9000,0}));const auto before=f.runtime.slices;f.Ready();
  assert(f.calls==1&&f.runtime.slices-before<=2);}
 {Fixture f;f.r.Put(f.Actors+2047*8,f.Other);f.Ready();assert(f.runtime.cache.count==0&&f.calls==0);f.r.reads=0;assert(f.runtime.Allows(f.r,f.World,{1000,2000,0}));
  // A newly appended cabin must be admitted/captured, not lost in prefix reuse.
  f.r.Put(f.Actors+2048*8,f.Actor);f.r.Put(f.Level+cfgradarworld::disk::LevelActors,cfgshacktrap::List{f.Actors,2049,2049});
  assert(!f.runtime.Allows(f.r,f.World,{1000,2000,0}));f.Ready();assert(f.calls==1&&f.runtime.cache.count==1&&!f.runtime.cache.Allows(f.World,{1000,2000,0}));
  // TArray relocation retains captured bounds, but refreshes all membership proofs.
  constexpr std::uintptr_t relocated=0x400000;
  for(unsigned i=0;i<2049;++i)f.r.Put(relocated+8*i,i==2048?f.Actor:f.Other);
  f.r.Put(f.Level+cfgradarworld::disk::LevelActors,cfgshacktrap::List{relocated,2049,2049});
  assert(!f.runtime.Allows(f.r,f.World,{9000,9000,0}));f.Ready();assert(f.calls==1&&f.runtime.cache.captured==1);}
 {Fixture f;f.r.Put(f.World+cfgradarworld::disk::WorldLevels,cfgshacktrap::List{f.Levels,32,32});
  for(unsigned i=1;i<32;++i){const auto level=std::uintptr_t{0x800000}+i*0x1000;f.r.Put(f.Levels+8*i,level);f.r.Put(level+0x10,f.LevelClass);f.r.Put(level+cfgkill::disk::OwningWorld,f.World);f.r.Put(level+cfgradarworld::disk::LevelActors,cfgshacktrap::List{});}
  f.Ready();f.r.reads=0;assert(f.runtime.Allows(f.r,f.World,{9000,9000,0})&&f.r.reads==98);if(f.peak>peak)peak=f.peak;}
 {Fixture f;f.r.Put(f.Level+cfgradarworld::disk::LevelActors,cfgshacktrap::List{f.Actors,129,129});
  for(unsigned i=0;i<129;++i){const auto actor=std::uintptr_t{0x900000}+i*0x1000;f.r.Put(f.Actors+8*i,actor);f.r.Put(actor+0x10,f.BP);f.r.Put(actor+0x20,f.Level);f.Slot(actor,static_cast<int>(i));}
  for(unsigned i=0;i<600&&f.runtime.cache.stage!=cfgcabin::Stage::Refused;++i)f.Tick();assert(f.runtime.cache.failure==7&&f.calls==0&&f.runtime.cache.count==128);}
 std::printf("Cabin bounds tests PASS: packet,2048-actor census,one-call phases,scope/mutation/failure gates; peak%u/1536 incl384 outer+400 native proof reserve\n",peak);
}
