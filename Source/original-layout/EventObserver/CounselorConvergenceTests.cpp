#include "../Core/CounselorConvergencePolicy.hpp"
#include <cstdio>
int main(){
    using namespace cfgconvergence;unsigned failures=0;
    const auto check=[&](bool value){if(!value)++failures;};
    Policy p;unsigned index=99;check(!p.Pick(0,3,index));check(p.ProbeDue(0));check(!p.ProbeDue(210));check(p.ProbeDue(211));
    check(p.Decide(0,true,true,0)==Action::None);p.Acquired();check(!p.ProbeDue(1000));
    check(!p.Pick(0,0,index));check(!p.Pick(0,MaximumBots+1,index));check(p.Pick(0,3,index)&&index==0);
    check(!p.Pick(332,3,index));check(p.Pick(333,3,index)&&index==1);check(p.Pick(666,3,index)&&index==2);
    check(p.Pick(999,3,index)&&index==0);check(p.Decide(0,true,false,0)==Action::None);
    check(p.Decide(0,false,true,0)==Action::None);check(p.Decide(0,true,true,0)==Action::Publish);
    p.Confirm(0,Action::Publish,100);check(p.Decide(0,true,true,432)==Action::None);
    check(p.Decide(0,true,true,433)==Action::Publish);check(p.Decide(0,true,false,101)==Action::Clear);
    p.Confirm(0,Action::Clear,102);check(p.Decide(0,true,false,103)==Action::None);
    p.Refuse(1);check(p.Decide(1,true,true,10000)==Action::None);
    check(!Desired(SeekWeapon,Action::Publish)&&!Desired(Flee,Action::Publish)&&!Desired(Hide,Action::Publish));
    check(Desired(Fight,Action::Publish)&&Desired(Armed,Action::Publish)&&Desired(Melee,Action::Publish)&&Desired(Orient,Action::Publish));
    check(Desired(SeekWeapon,Action::Clear)&&Desired(Flee,Action::Clear)&&!Desired(Fight,Action::Clear));
    Policy late;late.Acquired();check(late.Pick(100000,8,index));check(!late.Pick(100000,8,index)); // no catch-up burst
    check(late.Decide(MaximumBots,true,true,100000)==Action::None);
    std::printf("CFG donor armed-only convergence/pacing/clear/packet tests: %s (%u failures). No game access.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
