#include "../Core/JasonRageDifficultyPolicy.hpp"
#include <cstdio>
#include <limits>
#include <initializer_list>
int main(){unsigned failed=0;auto check=[&](bool b){if(!b)++failed;};
 for(unsigned difficulty=0;difficulty<3;++difficulty){cfgragethreshold::Rage p;
  check(p.Prepare(1,2,difficulty,600));check(p.desired==(difficulty==0?1200.f:difficulty==2?300.f:600.f));
  p.attempted=true;check(!p.Prepare(1,2,difficulty,p.desired));check(p.original==600);}
 for(auto value:{0.f,-1.f,86401.f,std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN()}){
  cfgragethreshold::Rage p;check(!p.Prepare(1,2,1,value));}
 cfgragethreshold::Rage p;check(!p.Prepare(1,0,1,600)&&!p.Prepare(0,1,1,600)&&!p.Prepare(1,2,3,600));
 std::printf("Donor once-per-pawn Rage threshold scales2/1/0.5: %u failures.\n",failed);return failed?1:0;
}
