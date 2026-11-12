#include "../Core/ChaseRecoveryPolicy.hpp"
#include <cstdio>
int main(){unsigned failed=0;auto check=[&](bool b){if(!b)++failed;};
 cfgchaserecovery::Policy p;p.Observe(1,1,400.f*400.f,1000);check(!p.pending&&p.repeated==1);
 p.Observe(1,1,400.f*400.f,2000);check(p.pending&&p.cursor==0&&p.until==7000);
 p.Observe(2,1,400.f*400.f,3000);check(!p.pending&&p.repeated==1);
 p.Observe(2,2,400.f*400.f,4000);check(!p.pending&&!p.repeated&&!p.target);
 p.Observe(2,1,300.f*300.f,5000);check(!p.pending&&!p.repeated);
 p.Observe(2,0,400.f*400.f,6000);check(!p.pending&&!p.repeated);
 std::printf("Donor distant AlreadyAtGoal recovery: %u failures; alternative is host-proved.\n",failed);return failed?1:0;
}
