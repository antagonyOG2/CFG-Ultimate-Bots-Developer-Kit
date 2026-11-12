#include "../Core/StartingLoadoutPolicy.hpp"
#include <cstdio>
struct LoadoutHost {
    cfgloadout::Schedule* schedule=nullptr;unsigned grants=0,checks=0,verifies=0,missing=0;
    bool current=true,ready=true,reentry=false;
    bool Current(std::uint64_t id){return current&&id==7;}
    bool Ready(cfgloadout::Slot,unsigned,std::u16string_view name){++checks;return ready&&!name.empty();}
    void Grant(cfgloadout::Slot,unsigned){++grants;if(reentry)schedule->Service(*this,999999);}
    void Verify(cfgloadout::Slot,unsigned){++verifies;}
    void Unavailable(cfgloadout::Slot,unsigned){++missing;}
};
int main(){unsigned failures=0;const auto check=[&](bool value){if(!value)++failures;};using namespace cfgloadout;
    for(unsigned w=0;w<5;++w)for(unsigned i=0;i<6;++i){Selection out;
        check(Commit({w,i},5,7,out)&&out.weapon==(w==1?4:w)&&out.inventory==(i==1?5:i));}
    Selection invalid;check(!Commit({5,0},0,0,invalid)&&!Commit({0,6},0,0,invalid));
    Schedule schedule;LoadoutHost host{&schedule};host.reentry=true;
    check(schedule.Begin(7,{2,5},100)&&!schedule.Begin(7,{2,5},100));
    check(!schedule.Service(host,3099)&&host.grants==0&&host.checks==0);
    check(schedule.Service(host,3100)&&host.grants==1&&schedule.CurrentState()==State::Verify);
    check(!schedule.Service(host,3100)&&!schedule.Service(host,4099));
    check(schedule.Service(host,4100)&&host.verifies==1&&schedule.CurrentSlot()==Slot::Inventory);
    check(schedule.Service(host,5100)&&host.grants==2);
    check(schedule.Service(host,6100)&&host.verifies==2&&schedule.CurrentState()==State::Complete);
    for(unsigned n=0;n<1000;++n)check(!schedule.Service(host,10000+n*1000));
    check(host.checks==2&&host.grants==2&&host.verifies==2);
    Schedule absent;LoadoutHost unavailable{&absent};unavailable.ready=false;
    check(absent.Begin(7,{0,3},0));for(unsigned n=0;n<16;++n)absent.Service(unavailable,3000+n*1000);
    check(unavailable.checks==15&&unavailable.missing==1&&unavailable.grants==0&&absent.CurrentState()==State::Complete);
    Schedule stale;LoadoutHost lost{&stale};check(stale.Begin(7,{2,3},0));lost.current=false;
    check(!stale.Service(lost,3000)&&stale.CurrentState()==State::Cancelled&&lost.grants==0);
    Schedule late;LoadoutHost skipped{&late};check(late.Begin(7,{2,3},0));
    check(late.Service(skipped,1000000)&&skipped.grants==1&&skipped.verifies==0);
    check(!late.Service(skipped,1000000)); // A long frame never catches up both grants.
    Schedule empty;check(empty.Begin(7,{0,0},0)&&empty.CurrentState()==State::Complete);
    Schedule random;check(!random.Begin(7,{1,1},0));
    std::printf("CFG base-style starting loadout pacing/once-only model: %s (%u failures). No game access.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
