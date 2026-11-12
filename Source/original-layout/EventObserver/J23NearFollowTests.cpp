#include "../Core/ConvergenceTravelPolicy.hpp"
#include "../Core/ConvergenceArrivalReport.hpp"
#include <cstdio>
int main(){using namespace cfgconvergencetravel;unsigned n=0,failed=0;const auto check=[&](bool b){++n;if(!b)++failed;};
 Policy p;check(p.Decide(600,1000,true,0,0,500,0,true)==Action::Follow);p.Followed(1000,500,0);
 check(!p.owned&&!p.travelling);check(p.Decide(500,1999,true,100,0,700,0,true)==Action::None);
 check(p.Decide(500,2000,true,100,0,650,0,true)==Action::None);
 check(p.Decide(500,2000,true,100,0,651,0,true)==Action::Follow);p.Followed(2000,651,0);
 check(p.Decide(500,3000,true,100,0,651,0,true)==Action::None);
 check(p.Decide(700,3000,true,100,0,800,0,true)==Action::Acquire);
 p.Acquired(700,3000,100,0);check(p.Decide(500,3001,true,100,0,800,0,true)==Action::Resume);
 p.Resumed(3001,false);check(p.Decide(500,3002,true,100,0,800,0,true)==Action::Follow);
 check(p.Decide(500,3003,false,100,0,800,0,true)==Action::None);
 check(p.Decide(NAN,3003,true,100,0,800,0,true)==Action::None);
 Policy legacy;check(legacy.Decide(500,1000,true)==Action::Acquire);
 legacy.Acquired(500,1000);check(legacy.Decide(221,2000,true)==Action::Move);
 check(legacy.Decide(220,2000,true)==Action::Resume);
 Policy failedPath;check(failedPath.Decide(600,1000,true,0,0,500,0,true)==Action::Follow);failedPath.FollowAttempt(1000);
 check(failedPath.Decide(600,1001,true,0,0,500,0,true)==Action::None);
 check(failedPath.Decide(600,1999,true,0,0,500,0,true)==Action::None);
 check(failedPath.Decide(600,2000,true,0,0,500,0,true)==Action::Follow);
 p.FollowAttempt(3000);check(p.Decide(500,3001,true,100,0,900,0,true)==Action::None);
 cfglean::ArrivalReport r;check(cfglean::ValidArrival(r));r.held=1;check(!cfglean::ValidArrival(r));
 r.weaponKnown=1;r.weapon=r.weaponClass=0x100000;check(cfglean::ValidArrival(r));
 r.knownKeys=1u<<cfgconvergence::KeyCount;check(!cfglean::ValidArrival(r));
 std::printf("J23 near-Jason native-brain follow: %u checks,%u failures;6m/1s/1.5m gates, no chase brain suspension, old travel preserved.\n",n,failed);return failed?1:0;}
