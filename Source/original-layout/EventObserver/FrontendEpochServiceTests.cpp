#include "../Core/FrontendEpochService.hpp"
#include <cstdio>
int main(){unsigned failures=0;const auto check=[&](bool value){if(!value)++failures;};
 using namespace cfgmenu::epochservice;
 check(Candidate(42,42,7,false,true));
 check(Candidate(42,42,8,false,true));
 check(!Candidate(43,42,7,false,true));check(!Candidate(42,42,7,true,true));
 check(!Candidate(42,42,7,false,false));check(!Candidate(42,0,7,false,true));
 for(unsigned state=0;state<=10;++state)if(state!=7&&state!=8)check(!Candidate(42,42,state,false,true));
 check(UnclaimedTravel(8,false,false,true,true,true,true));
 check(!UnclaimedTravel(7,false,false,true,true,true,true));
 check(!UnclaimedTravel(8,true,false,true,true,true,true));
 check(!UnclaimedTravel(8,false,true,true,true,true,true));
 check(!UnclaimedTravel(8,false,false,false,true,true,true));
 check(!UnclaimedTravel(8,false,false,true,false,true,true));
 check(!UnclaimedTravel(8,false,false,true,true,false,true));
 check(!UnclaimedTravel(8,false,false,true,true,true,false));
 std::uint64_t next=0;check(Due(1000,next)&&next==1250);
 check(!Due(1249,next)&&next==1250);check(Due(1250,next)&&next==1500);
 check(!Due(UINT64_MAX,next)&&next==1500);
 unsigned attempts=0;
 for(unsigned i=0;i<100;++i)check(!AdmitAttempt(false,attempts)&&attempts==0);
 for(unsigned i=0;i<8;++i)check(AdmitAttempt(true,attempts)&&attempts==i+1);
 check(!AdmitAttempt(true,attempts)&&attempts==8);
 std::printf("Frontend rebuilt-HUD missed-event service: %u failures; owner/returned/nesting/state/timer guards, no native authority\n",failures);
 return failures?1:0;
}
