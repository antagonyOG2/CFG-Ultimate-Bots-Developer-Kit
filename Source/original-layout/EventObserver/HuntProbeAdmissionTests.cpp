#include "../Core/GameplayBundlePolicy.hpp"
#include "../Core/HuntProbeAdmission.hpp"
#include <cstdio>
struct Counts {unsigned ordinary=0,morph=0,car=0,releases=0;};
Counts Replay(bool corrected,bool melee){
    cfgbundle::Schedule schedule;cfghuntprobe::Policy gate;Counts c;
    schedule.Start(0,0);schedule.helpersActive=false;unsigned pending=0;
    std::uint64_t morphCheck=0,carCheck=0;
    for(std::uint64_t now=100;now<60000;now+=25){
        const auto lane=schedule.Due(now);
        if(lane==cfgbundle::Lane::Release){schedule.pressed=false;++c.releases;continue;}
        if(lane!=cfgbundle::Lane::Hunt)continue;
        if(pending){pending=0;continue;} // actual independent service consumed this Hunt
        schedule.CloseCombat(now,melee?200.f*200.f:900.f*900.f);
        const bool admit=corrected?gate.MayProbe():true;
        if(admit&&(!corrected||gate.MorphFirst(true))&&now>=morphCheck){pending=1;morphCheck=now+1000;schedule.huntAt=now+100;gate.Queued();++c.morph;continue;}
        if(admit&&now>=carCheck){pending=2;carCheck=now+1000;schedule.huntAt=now+100;gate.Queued(true);++c.car;continue;}
        ++c.ordinary;
        if(melee&&schedule.AttackDue(now,200.f*200.f))schedule.Pressed(now);
    }
    return c;
}
int main(){unsigned failed=0;const auto check=[&](bool b){if(!b)++failed;};
    const auto broken=Replay(false,false),fixed=Replay(true,false),combat=Replay(true,true);
    check(broken.morph>0&&broken.car>0&&broken.ordinary==0);
    check(fixed.ordinary>=20&&fixed.morph>0&&fixed.car>0);
    check(combat.ordinary>=30&&combat.releases>=10);
    cfghuntprobe::Policy p;check(p.MayProbe());p.Queued();check(!p.MayProbe());check(p.MayProbe());
    std::printf("Actual Schedule/composed Morph+car replay:old ordinary=%u fixed=%u combat=%u releases=%u failures=%u; no game access.\n",
        broken.ordinary,fixed.ordinary,combat.ordinary,combat.releases,failed);
    return failed?1:0;
}
