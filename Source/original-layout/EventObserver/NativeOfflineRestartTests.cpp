#define CFG_OFFLINE_OWNER_FIXTURE_ONLY
#include "NativeOfflineOwnerObservationTests.cpp"
#include "NativeOfflineRestart.hpp"
int main(){unsigned failures=0,checks=0;const auto check=[&](bool ok){++checks;if(!ok)++failures;};
 for(unsigned fault=0;fault<9;++fault){Fixture f;f.source.nativeOffline=true;
  constexpr auto package=std::uintptr_t{0x800000};
  f.Name(4,"/Game/Maps/CrystalLake");f.r.Set(f.Names+cfgnames::disk::ElementCount,std::array<int,2>{{5,1}});
  f.r.Set(f.World+0x20,package);f.r.Set(package+cfgnames::disk::ObjectName,cfgnames::Words{4,0});
  cfgmenu::promotion::Context prior,fresh;cfgnames::Text expected;
  check(f.Observe(prior)&&cfgmenu::offlinerestart::Package(f.r,prior,expected));
  prior.identities[0].serial+=10;prior.identities[1].serial+=10;
  if(fault==1)prior.identities[0].serial-=10;
  if(fault==2)prior.identities[1].serial-=10;
  if(fault==3)expected.data[11]=u'X';
  if(fault==4)f.r.Set(f.World+cfgprobe::WorldNetDriver,f.Mode);
  if(fault==5)f.source.nativeOffline=false;
  if(fault==6)f.r.Set(f.Controller+cfgprobe::ObjectClass,f.ModeType);
  if(fault==7)f.r.Set(f.Registry+16,int{0});
  const auto receiver=fault==8?f.PS:f.Mode;
  const bool accepted=cfgmenu::offlinerestart::Observe(f.r,f.source,prior,expected,receiver,42,fresh);
  check(accepted==(fault==0));check(accepted?fresh.world==f.World:!fresh.world);
 }
 std::printf("Native offline fresh same-map restart ownership: %u checks %u failures\n",checks,failures);return failures?1:0;
}
