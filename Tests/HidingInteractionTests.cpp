#include "../Core/HidingInteractionPolicy.hpp"
#include <cstdio>
int main(){
    unsigned failures=0;const auto check=[&](bool value){if(!value)++failures;};
    using namespace cfghiding;
    State s;s.Begin(1,2,3,1000);
    check(s.Active()&&s.phase==Phase::Approach&&!s.Recovery());
    for(unsigned i=0;i<6;++i)s.MarkAttempt(1000+i*850);
    check(s.attempts==6&&s.totalAttempts==6&&s.Expired(6000));
    // A sixth native input acquiring its lock must not be canceled by the
    // local attempt cap; lock timeout is measured from episode acquisition.
    check(s.ObserveLock(3,6000)==LockDecision::Owned);
    check(s.ObserveLock(3,12999)==LockDecision::Owned);
    check(s.ObserveLock(3,13000)==LockDecision::Expire);
    check(s.ObserveLock(0,6000)==LockDecision::Expire);
    s.Abandon(13000,true);
    check(s.Recovery()&&s.phase==Phase::Cancel&&s.Ignored(1,2,13000));
    // Ignore remains while owed recovery progresses; it is not an active-lane gate.
    s.phase=Phase::Unlock;check(s.Recovery());s.phase=Phase::Recover;check(s.Recovery());
    s.Clear();check(!s.Active()&&s.Ignored(1,2,37999)&&!s.Ignored(1,2,38000));
    check(s.totalAttempts==6&&s.timeouts==1);
    s.Begin(4,5,6,40000);check(s.ObserveLock(99,41999)==LockDecision::ForeignGrace);
    check(s.ObserveLock(99,42000)==LockDecision::YieldForeign);
    check(s.ObserveLock(0,53999)==LockDecision::Continue&&s.ObserveLock(0,54000)==LockDecision::Expire);
    s.Abandon(54000);s.Clear();check(s.Ignored(4,5,60999)&&!s.Ignored(4,5,61000));
    s.Begin(7,8,9,62000);s.MarkAttempt(62000);
    check(!s.AttemptDue(62849)&&s.AttemptDue(62850)&&s.next==62650);
    s.repositionIndex=4;s.repositionAttempted=true;s.Begin(10,11,12,63000);
    check(s.repositionIndex==0&&!s.repositionAttempted&&!s.repositioned);
    std::printf("CFG hiding episode/lock/recovery/retarget bounds: %s (%u failures).\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
