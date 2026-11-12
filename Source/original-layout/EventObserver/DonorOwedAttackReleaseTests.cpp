#include "../Core/DonorOwedAttackRelease.hpp"
#include "../Core/DonorJasonCombat.hpp"
#include "../Core/DonorAdditionalJasonHunt.hpp"
#include <cassert>
#include <cstdio>
using namespace cfgdonorhunt;
struct Host {NativeOutcome outcome=NativeOutcome::NotEntered;unsigned calls=0;
    NativeOutcome ReleaseAttackOutcome(){++calls;return outcome;}};
template<class State>unsigned CheckState(){unsigned checks=0;auto check=[&](bool b){++checks;assert(b);};
    Host h;State s;check(DonorDrainOwedAttackRelease(h,s,1000)==ReleaseDrain::NothingOwed&&!h.calls);
    s.AttackPressed=true;s.AttackReleaseAt=1120;
    check(DonorDrainOwedAttackRelease(h,s,1119)==ReleaseDrain::Waiting&&!h.calls&&s.AttackPressed);
    for(unsigned i=0;i<5;++i)check(DonorDrainOwedAttackRelease(h,s,1120+i)==ReleaseDrain::Deferred&&s.AttackPressed&&s.AttackReleaseAt==1120);
    h.outcome=NativeOutcome::ReturnedTrue;
    check(DonorDrainOwedAttackRelease(h,s,1125)==ReleaseDrain::Returned&&!s.AttackPressed&&!s.AttackReleaseAt&&h.calls==6);
    check(DonorDrainOwedAttackRelease(h,s,1126)==ReleaseDrain::NothingOwed&&h.calls==6);
    s.AttackPressed=true;s.AttackReleaseAt=1200;h.outcome=NativeOutcome::Uncertain;
    check(DonorDrainOwedAttackRelease(h,s,1200)==ReleaseDrain::Uncertain&&!s.AttackPressed&&h.calls==7);
    check(DonorDrainOwedAttackRelease(h,s,1201)==ReleaseDrain::NothingOwed&&h.calls==7);
    return checks;
}
int main(){const auto checks=CheckState<CombatState>()+CheckState<AdditionalHuntState>();
    std::printf("J1a actual owed-release drain: %u checks PASS; primary andF1,120ms boundary,pre-entry refusal preserves latch,uncertain entry never replays.\n",checks);
}
