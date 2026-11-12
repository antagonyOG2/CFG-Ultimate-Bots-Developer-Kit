#define main CfgActiveMenuFixtureMain
#include "ActiveMenuObservationTests.cpp"
#undef main
#include "NativeHumanJasonRouteObservation.hpp"
int main(){unsigned failures=0;auto check=[&](bool v){if(!v)++failures;};
 Fixture f;cfgmenu::active::Context source;check(cfgmenu::active::Observe(f.reader,f.Module,source));
 f.reader.Set(f.Mode+cfgmenu::selectiondisk::PendingOfflineMode,f.Type);
 f.reader.reads=0;cfgmenu::humanjason::Top top;
 check(cfgmenu::humanjason::ObserveTop(f.reader,source,f.HUD,top));check(f.reader.reads==20);
 for(unsigned i=0;i<1000;++i){f.reader.reads=0;cfgmenu::humanjason::Top again;check(cfgmenu::humanjason::ObserveTop(f.reader,source,f.HUD,again)&&again==top&&f.reader.reads==20);}
 f.reader.Set(f.Records+cfgmenu::execdisk::RecordBytes+cfgmenu::execdisk::RecordWidget,f.OtherWidget);
 f.reader.Set(f.OtherWidget+cfgprobe::ObjectClass,f.Type);f.reader.reads=0;cfgmenu::humanjason::Top changed;
 check(cfgmenu::humanjason::ObserveTop(f.reader,source,f.HUD,changed)&&!(changed==top));
 f.reader.Set(f.World+cfgprobe::WorldNetDriver,f.Mode);f.reader.reads=0;check(!cfgmenu::humanjason::ObserveTop(f.reader,source,f.HUD,changed));
 f.reader.reads=0;check(!cfgmenu::humanjason::ObserveTop(f.reader,source,f.OtherWidget,changed)&&f.reader.reads==0);
 std::printf("Native Jason routing hint: %u failures,20reads/idle HUD; unrelated callback zero reads\n",failures);return failures?1:0;
}
