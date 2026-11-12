#include "../Core/NativeCounselorPopulationPolicy.hpp"
#include "../Core/GameplayBundlePolicy.hpp"
#include <cstdio>
struct Host {
    bool human=true,bots=true;unsigned step=0,count=99;
    bool Human(){if(step!=0)return false;step=1;return human;}
    bool Population(unsigned requested){if(step!=1)return false;step=2;count=requested;return bots;}
};
int main(){
    unsigned checks=0,failures=0;
    const auto check=[&](bool value){++checks;if(!value){++failures;std::printf("FAIL check %u\n",checks);}};
    for(unsigned total=1;total<=10;++total){const auto helpers=total-1;const auto p=cfgpopulation::Make(helpers);
        check(p.valid&&p.native<=6&&p.deferred<=3&&p.native+p.deferred==helpers);
        Host host;check(cfgpopulation::Birth(host,helpers)&&host.step==2&&host.count==p.native);
        cfgbundle::Schedule schedule;check(schedule.Start(1000,helpers)&&schedule.AdoptNativeHelpers(p.native));
        check(schedule.spawned==p.native&&schedule.requestedBots==helpers&&schedule.helpersActive==(p.deferred!=0));
        cfgbundle::HelperStartup spawn;check(spawn.AdoptNative(p.native,helpers));
        check(spawn.Active()==(p.deferred!=0));
        for(unsigned done=p.native;done<helpers;++done){
            spawn.stage=cfgbundle::HelperStage::Spawn;spawn.Confirm(done+1,helpers);++schedule.spawned;
            check(spawn.Active()==(done+1<helpers));
        }
        check(schedule.spawned==helpers);
        check(schedule.AdditionalHelper(2000)==(helpers<9));
    }
    {Host host;host.human=false;check(!cfgpopulation::Birth(host,9)&&host.step==1&&host.count==99);}
    {Host host;host.bots=false;check(!cfgpopulation::Birth(host,9)&&host.step==2&&host.count==6);}
    {Host host;check(!cfgpopulation::Birth(host,10)&&host.step==0);}
    {cfgbundle::Schedule s;check(s.Start(1000,9)&&!s.AdoptNativeHelpers(10));}
    {cfgbundle::HelperStartup s;check(!s.AdoptNative(7,6));s.Refuse(cfgbundle::HelperFailure::Dispatch,true);check(!s.AdoptNative(0,9));}
    check(cfgbundle::HelpersSettled(6,9,cfgbundle::HelperStage::Unavailable));
    check(!cfgbundle::HelpersSettled(6,9,cfgbundle::HelperStage::Indeterminate));
    check(!cfgbundle::HelpersSettled(6,9,cfgbundle::HelperStage::Project));
    check(!cfgbundle::HelpersSettled(6,9,cfgbundle::HelperStage::Complete));
    check(cfgbundle::HelpersSettled(9,9,cfgbundle::HelperStage::Complete));
    check(!cfgbundle::HelpersSettled(10,9,cfgbundle::HelperStage::Unavailable));
    check(cfgbundle::HelpersSettled(0,0,cfgbundle::HelperStage::Project));
    std::printf("Native human-first/cohort/deferred population: %u checks %u failures; no game accessed\n",checks,failures);
    return failures?1:0;
}
