#include "../Core/DonorReinforcementQueue.hpp"
#include "../Core/HelperSpawnPolicy.hpp"
#include <algorithm>
#include <cstdio>
int main(){unsigned checks=0,failures=0;const auto check=[&](bool x){++checks;if(!x)++failures;};
 for(unsigned initial=0;initial<=cfglimits::MaximumHelpers;++initial){cfgreinforcement::Counselors q;
   const auto room=cfglimits::MaximumHelpers-initial;
   for(unsigned press=0;press<12;++press)check(q.Request(initial)==(press<room));
   check(q.pending==room&&q.accepted==room);
   unsigned requested=initial,spawned=initial;
   while(q.Ready(spawned,requested)){q.Started();++requested;
      check(!q.Ready(spawned,requested));++spawned;}
   check(!q.pending&&requested==initial+room&&requested<=cfglimits::MaximumHelpers);
 }
 {cfgreinforcement::Counselors q;check(!q.Request(10));check(q.Request(1));q.Cancel();check(!q.pending&&q.accepted==1);}
 cfgbundle::HelperStartup helper;const auto point=helper.Candidate({0,0,100},0);
 check(point.x==700&&point.y==0&&point.z==35);check(helper.Projected(point,{700,0,10})&&helper.location.z==100);
 std::printf("Donor F3 queue/cap/near-spawn policy: %u checks,%u failures; no game access.\n",checks,failures);return failures?1:0;
}
