#include "../Core/J22TargetPolicy.hpp"
#include <cstdio>
#include <limits>
int main(){unsigned checks=0,failed=0;const auto check=[&](bool x){++checks;if(!x)++failed;};
 for(unsigned flags=0;flags<64;++flags)check(cfgj22::NearbyHuman(flags&1,flags&2,flags&4,flags&8,flags&16,flags&32,999.f)==(flags==47));
 check(cfgj22::F1TargetScore(true,900.f*900.f,900.f*900.f)==0.f);
 check(cfgj22::F1TargetScore(false,7000.f*7000.f,7000.f*7000.f)<cfgj22::F1TargetScore(true,1100.f*1100.f,1100.f*1100.f));
 check(cfgj22::F1TargetScore(false,500.f*500.f,500.f*500.f)>cfgj22::F1TargetScore(true,999.f*999.f,999.f*999.f));
 check(cfgj22::F1TargetScore(true,1100.f*1100.f,1100.f*1100.f)<cfgj22::F1TargetScore(false,9000.f*9000.f,9000.f*9000.f));
 check(cfgj22::NearbyHuman(true,true,true,true,false,true,1000.f));
 check(!cfgj22::NearbyHuman(true,true,true,true,false,true,1000.1f));
 check(!cfgj22::NearbyHuman(true,true,true,true,false,true,-1.f));
 check(!cfgj22::NearbyHuman(true,true,true,true,false,true,std::numeric_limits<float>::quiet_NaN()));
 check(!cfgj22::NearbyHuman(true,true,true,true,false,true,std::numeric_limits<float>::infinity()));
 std::printf("J22 F1 near-human policy: %u checks,%u failures; 10m only, original Jason/unobserved/blocked refused.\n",checks,failed);return failed?1:0;}
