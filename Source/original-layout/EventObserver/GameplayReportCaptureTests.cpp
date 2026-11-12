#include "../Core/GameplayReportCapture.hpp"
#include <cstdio>
int main(){using cfgbundlecapture::Status;unsigned failed=0;
 const auto checkAt=[&](bool okay,int line){if(!okay){++failed;std::printf("Capture failure line%d version%u\n",line,cfgbundle::ReportVersion);}};
#define check(value) checkAt(value,__LINE__)
 cfgbundle::Report a,b;std::memcpy(&b,&a,sizeof(a)); // exact captured bytes, including padding
 check(cfgbundlecapture::Observe(true,true,a,b)==Status::Accepted);
 check(cfgbundlecapture::Observe(false,true,a,b)==Status::Unreadable);
 b.generation=1;check(cfgbundlecapture::Observe(true,true,b,b)==Status::Changing);
 b={};b.phases=1;check(cfgbundlecapture::Observe(true,true,a,b)==Status::Changing);
 b={};b.spectatorRefusals=1;b.spectatorLastRefusal.Capture(10,1,9,8,0,901,3,4,4,2,25);
 check(cfgbundlecapture::Observe(true,true,b,b)==Status::Accepted);
 b.spectatorLastRefusal.binding=5;check(cfgbundlecapture::Observe(true,true,b,b)==Status::Invalid);
 for(unsigned leaf=15;leaf<=34;++leaf){b={};b.spectatorFailure=leaf;b.spectatorRefusals=1;
  b.spectatorLastRefusal.Capture(10,1,leaf,8,0,1252,6,4,4,0,25);
  check(cfgbundlecapture::Observe(true,true,b,b)==Status::Accepted);}
 b.spectatorFailure=35;check(cfgbundlecapture::Observe(true,true,b,b)==Status::Invalid);
 b.spectatorFailure=34;b.spectatorLastRefusal.failure=35;check(cfgbundlecapture::Observe(true,true,b,b)==Status::Invalid);
 b={};b.version=47;check(cfgbundlecapture::Observe(true,true,b,b)==Status::Invalid);
 std::printf("Gameplay report capture: %u failures; stable resource4 refusal retained, invalid/changing/unreadable distinguished; no game access.\n",failed);
 return failed?1:0;
}
