#include "../Core/VehicleProgressPolicy.hpp"
#include <cstdio>
#include <limits>
int main(){unsigned checks=0,failed=0;const auto check=[&](bool value){++checks;if(!value)++failed;};
 using namespace cfgvehicleprogress;
 {Entry p;check(!p.Observe(1,1,2,2,2000,1000));
  for(unsigned t=1100;t<13000;t+=100)check(!p.Observe(1,1,2,2,2000,t));
  check(p.Observe(1,1,2,2,2000,13000));check(!p.BackOff(13000,false,false));check(!p.BackOff(13000,true,true));
  check(p.BackOff(13000,true,false));check(p.Ignored(1,1,27999)&&!p.Ignored(1,1,28000)&&!p.Ignored(1,3,14000));
  check(!p.Observe(1,1,2,2,2000,28000));check(p.deadline==40000);}
 {Entry p;p.Observe(1,1,2,2,2000,1000);check(!p.Observe(1,1,2,2,1900,12000));check(p.deadline==24000);
  check(!p.Observe(1,1,2,2,2600,13000));check(p.deadline==24000);check(p.Observe(1,1,2,2,2600,24000));
  check(!p.Observe(1,2,2,2,2600,25000));check(p.deadline==37000&&!p.ignoredUntil);}
 {Runtime p;for(unsigned t=1000;t<=14000;t+=1000){auto& e=p.cars[t%2000?0:1];e.Observe(t%2000?1:3,1,2,2,1000,t);}
  check(p.cars[0].deadline==13000&&p.cars[1].deadline==14000);}
 {Entry p;check(!p.Observe(1,0,2,2,100,1));check(!p.Observe(1,1,2,2,std::numeric_limits<float>::infinity(),1));}
 std::printf("Donor measured vehicle progress: %u checks, %u failures; no native cleanup assumptions.\n",checks,failed);return failed?1:0;
}
