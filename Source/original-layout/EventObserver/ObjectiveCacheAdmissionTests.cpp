#include "../Core/ObjectiveCacheAdmissionPolicy.hpp"
#include <cstdio>
int main(){unsigned checks=0,failures=0;const auto check=[&](bool v){++checks;if(!v)++failures;};
    cfgobjective::CacheAdmission p;check(p.Due(100));check(!p.Acquired(100,false));
    check(!p.Due(1099)&&p.Due(1100));check(!p.Acquired(1100,false));
    check(p.Acquired(2100,true)&&p.index==1&&p.attempts==0); // consume BEFORE native write
    check(p.Acquired(2200,true)&&p.index==2&&!p.Due(3000)); // never replay publication
    cfgobjective::CacheAdmission cold;
    for(unsigned i=0;i<20;++i)check(!cold.Acquired(i*1000,false));
    check(cold.Acquired(20000,false)&&cold.index==1);
    check(!cold.Acquired(20001,false));check(cold.Acquired(40001,false)&&cold.index==2);
    cfgobjective::CacheAdmission budget;budget.BudgetDeferred(10);
    check(budget.attempts==0&&!budget.Due(1009)&&budget.Due(1010));
    check(budget.Acquired(20010,false)&&budget.index==1); // finite even under read pressure
    cfgobjective::CacheAdmission rollback;check(!rollback.Acquired(1000,false));
    check(rollback.Acquired(999,false)&&rollback.index==1);
    cfgobjective::PublicationWindow commit;commit.Begin(0,100);
    check(commit.pending&&!commit.Due(124)&&commit.Due(125));
    check(commit.Defer(125)&&!commit.Due(1124)&&commit.Due(1125));
    check(commit.MarkAttempt()&&!commit.MarkAttempt());
    check(!commit.Defer(1125)&&!commit.pending); // ambiguous CAS cannot retry
    commit.Begin(1,2000);check(commit.Expired(22000)&&!commit.Defer(22000));
    commit.Begin(1,4000);check(commit.Expired(3999)&&!commit.Defer(3999));
    commit.Begin(1,5000);commit.Finish();check(!commit.pending&&!commit.Due(6000));
    commit.Begin(0,UINT64_MAX-10);check(commit.next==UINT64_MAX&&!commit.Due(UINT64_MAX-1));
    check(commit.Defer(UINT64_MAX-5)&&commit.next==UINT64_MAX);
    std::printf("Bounded read-only car-cache warmup/no publication replay: %u checks, %u failures.\n",checks,failures);
    return failures?1:0;
}
