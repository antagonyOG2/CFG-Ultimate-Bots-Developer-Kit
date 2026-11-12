#include "../Core/RandomCounselorPolicy.hpp"
#include "../Core/ResidentCounselorSelection.hpp"
#include <cstdio>
int main(){
    unsigned failures=0;
    const auto check=[&](bool okay,const char* label){if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    const std::uintptr_t human=0x10,nativeA=0x20,nativeB=0x30,fresh=0x40;
    cfgbundle::RandomCounselors r;
    check(r.Begin(8,7),"catalogue begins");
    r.Reserve(nativeA);r.Reserve(nativeB);r.Reserve(nativeA);r.Reserve(0);
    check(r.reservedCount==2,"native cohort classes reserved once, null ignored");
    check(r.Used(nativeA)&&r.Used(nativeB)&&!r.Used(fresh),"reserved classes count as used");
    check(!r.ClassAllowed(nativeA,human)&&!r.ClassAllowed(human,human)&&r.ClassAllowed(fresh,human),
        "F3 pick avoids native cohort and human, allows a new character");
    // Resident pass: a reserved character is skipped (remembered as reuse
    // fallback) unless the pass has fallen back because all are taken.
    cfgresidentcounselor::Pass pass;
    check(!pass.Use(nativeA,human,3,r.Used(nativeA))&&pass.reused==3,"duplicate of native bot skipped on first pass");
    check(pass.Use(fresh,human,4,r.Used(fresh)),"different character accepted");
    cfgspawn::Vec3 at{1,2,3};check(r.Confirm(fresh,at)&&r.Used(fresh),"F3 birth recorded");
    check(!r.ClassAllowed(fresh,human),"second F3 avoids the first F3 character");
    std::printf("RandomCounselorPolicyTests: %u failures; distinct F3 characters vs native cohort\n",failures);
    return failures?1:0;
}
