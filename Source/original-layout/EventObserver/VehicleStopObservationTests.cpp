#include "VehicleStopObservation.hpp"
#include "OwnedByteReader.hpp"
#include <cstdio>
#include <limits>
namespace {
struct Fixture {
 static constexpr std::uintptr_t Module=0x10000000,Car=0x20000000,Hood=0x30000000;
 OwnedByteReader r;
 bool started=false,enabled=false,slammed=false;float door=0,slam=0;
 Fixture(){using namespace cfgaction::disk;
  r.Set(Module+IsBeingSlammedExecutor,IsBeingSlammedExecutorCode);
  r.Set(Module+0x4D59A0,std::array<unsigned char,32>{});
  r.Set(Car+CarStarted,std::uint8_t{1});r.Set(Car+CarSlammed,std::uint8_t{});
  r.Set(Car+CarDoorSpeed,100.f);r.Set(Car+CarSlamSpeed,200.f);r.Set(Hood+HoodEnabled,std::uint8_t{1});
 }
 bool Observe(std::uintptr_t hood=Hood){return cfgvehiclestop::Observe(r,Module,Car,hood,started,enabled,slammed,door,slam);}
};
}
int main(){unsigned failures=0,peak=0;
 const auto check=[&](bool value,const char* label){if(!value){++failures;std::printf("FAIL %s\n",label);}};
 {Fixture f;
  check(!cfgbundle::Code(f.r,Fixture::Module,0x4D59A0,cfgaction::disk::IsBeingSlammedExecutorCode),"retired inline pin rejects otherwise valid occupied car");
  check(f.Observe()&&f.started&&f.enabled&&!f.slammed&&f.door==100&&f.slam==200,"current centralized pin admits moving stop observation");peak=f.r.reads;
  f.r.Set(Fixture::Car+cfgaction::disk::CarSlammed,std::uint8_t{1});
  check(f.Observe()&&f.slammed,"fresh native slam visible");
  check(f.Observe(0)&&!f.enabled,"absent hood stays disabled");
 }
 for(unsigned fault=0;fault<8;++fault){Fixture f;using namespace cfgaction::disk;
  if(fault==0)f.r.Set(Fixture::Module+IsBeingSlammedExecutor,std::uint8_t{});
  if(fault==1)f.r.Set(Fixture::Car+CarStarted,std::uint8_t{2});
  if(fault==2)f.r.Set(Fixture::Car+CarSlammed,std::uint8_t{2});
  if(fault==3)f.r.Set(Fixture::Hood+HoodEnabled,std::uint8_t{2});
  if(fault==4)f.r.Set(Fixture::Car+CarDoorSpeed,std::numeric_limits<float>::quiet_NaN());
  if(fault==5)f.r.Set(Fixture::Car+CarSlamSpeed,-1.f);
  if(fault==6){f.r.changeAt=Fixture::Car+CarSlamSpeed;f.r.changeSlot=Fixture::Car+CarStarted;f.r.newPointer=0;}
  if(fault==7)f.r.bytes.erase(Fixture::Car+CarStarted);
  check(!f.Observe(),"malformed/mutating observation refuses");peak=(std::max)(peak,f.r.reads);
 }
 check(peak<32,"bounded leaf read count");
 std::printf("Vehicle stop observation: %u failures, peak%u reads; actual centralized code gate and fields, no native/game calls.\n",failures,peak);
 return failures?1:0;
}
