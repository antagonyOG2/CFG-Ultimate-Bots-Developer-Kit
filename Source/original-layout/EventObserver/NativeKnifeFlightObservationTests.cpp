#define main CfgExistingNativeFlightFixtureMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
#include "../RuntimeProbe/KnifeSequenceBindings.generated.hpp"
#include "NativeKnifeFlightObservation.hpp"
int main(){
 Fixture f;const auto driver=f.Object(),root=f.Object(),movement=f.Object();std::int32_t index=0;
 CHECK(f.Read(driver+0xC,index));f.Put(f.objects.data+24ULL*index+16,std::int32_t{123});
 f.Put(driver+cfgknife::disk::ThrownBy,f.pawn);f.Put(driver+0x160,root);f.Put(root+0x20,driver);
 f.Put(root+0x190,cfgspawn::Vec3{10,20,30});f.Put(driver+0x378,movement);
 constexpr std::uintptr_t table=0x9000000;f.Put(movement,table);f.Put(movement+0x20,driver);
 f.Put(movement+0x104,cfgspawn::Vec3{12000,0,0});f.Put(movement+0xF0,root);f.Put(movement+0xAA,std::uint8_t{1});
 cfgknife::NativeFlightObservation sample{};f.reads=0;
 const auto observe=[&](Fixture& r,cfgknife::Identity id){return cfgknife::ObserveNativeFlight(r,r.objects,id,f.pawn,table,sample);};
 CHECK(observe(f,{driver,123})&&sample.flight.x==10&&sample.flight.speed==12000&&sample.flight.updated==root);const auto reads=f.reads;
 for(unsigned fault=0;fault<6;++fault){auto bad=f;bad.reads=0;
  if(fault==0)bad.Put(root+0x20,std::uintptr_t{0});
  if(fault==1)bad.Put(movement+0x20,std::uintptr_t{0});
  if(fault==2)bad.Put(movement,table+8);
  if(fault==3)bad.Put(movement+0xF0,std::uintptr_t{0x7000000});
  if(fault==4)bad.Put(driver+cfgknife::disk::ThrownBy,std::uintptr_t{0});
  if(fault==5)bad.cap=0;
  CHECK(!observe(bad,{driver,123})&&sample.flight.speed==0);
 }
 f.reads=0;CHECK(!observe(f,{driver,124}));f.reads=0;
 f.Put(movement+0x104,cfgspawn::Vec3{});f.Put(movement+0xF0,std::uintptr_t{0});
 CHECK(observe(f,{driver,123})&&sample.flight.speed==0&&!sample.flight.updated);
 std::printf("Native knife launch origin/velocity read-only member: %u failures, %u reads/sample; changed identity/owner/component/budget refuse; no native calls.\n",failures,reads);
 return failures?1:0;
}
