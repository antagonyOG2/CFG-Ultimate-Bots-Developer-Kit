#include "NativeCounselorRoutePolicy.hpp"
#include <cstdio>
int main(){using namespace cfgmenu;using namespace counselorroute;unsigned checks=0,failures=0;
    const auto check=[&](bool value){++checks;if(!value)++failures;};
    active::Context source;source.module=0x100000;source.thread=5;source.world=0x110000;source.mode=0x120000;
    source.controller=0x130000;source.hud=0x140000;source.stack=0x150000;source.gameInstance=0x160000;source.widget=0x170000;source.type=0x180000;
    source.identities={{{source.world,1,11},{source.controller,2,12},{source.widget,3,13},{source.hud,4,14}}};
    constexpr std::uintptr_t counselor=0x190000,map=0x200000,settings=0x210000,sandbox=0x220000;
    const auto bind=[&](Transaction& t){return t.Bind(source,counselor,map,settings,sandbox);};
    const auto screen=[&](std::uintptr_t type,int index){auto c=source;c.type=type;c.widget=0x300000+static_cast<std::uintptr_t>(index)*0x100;c.identities[2]={c.widget,index,index+10};return c;};
    {auto deeper=source;deeper.array={0x400000,3,4};Transaction replay;
     check(replay.Bind(deeper,counselor,map,settings,sandbox));
     auto returned=deeper;returned.widget=0x410000;returned.identities[2]={returned.widget,5,15};
     auto wrong=returned;wrong.array.count=2;check(!replay.RebindEntry(wrong));
     check(replay.RebindEntry(returned)&&replay.RequestEntry(returned));}
    Transaction t;Token entry,accept;check(bind(t));check(!bind(t));
    check(!t.Borrow(source,Step::Entry,sandbox,entry));check(!t.RequestAccept(source));check(t.RequestEntry(source));
    check(!t.RequestEntry(source));check(!t.Borrow(source,Step::Entry,map,entry));check(t.Borrow(source,Step::Entry,sandbox,entry));
    auto forged=entry;forged.destination=map;check(!t.Consume(source,forged));
    check(t.Consume(source,entry));check(!t.Consume(source,entry));check(!t.Arrive(screen(map,5)));
    auto picker=screen(counselor,6);check(t.Arrive(picker));check(!t.Borrow(picker,Step::AcceptedCounselor,0,accept));
    check(t.RequestAccept(picker));check(!t.Borrow(picker,Step::AcceptedCounselor,counselor,accept));check(t.Borrow(picker,Step::AcceptedCounselor,0,accept));
    const auto staleAccept=accept;check(t.CancelAccept(picker));check(!t.Consume(picker,staleAccept));
    check(t.CurrentState()==State::CounselorReady);check(t.RequestAccept(picker));check(t.Borrow(picker,Step::AcceptedCounselor,0,accept));
    for(unsigned fault=0;fault<6;++fault){auto altered=picker;if(fault==0)altered.controller+=0x100;if(fault==1)altered.identities[1].serial++;
        if(fault==2)altered.identities[2].serial++;if(fault==3)altered.thread++;if(fault==4)altered.world+=0x100;if(fault==5)altered.stack+=0x100;
        Token refused;check(!t.Borrow(altered,Step::AcceptedCounselor,0,refused));check(!t.Consume(altered,accept));}
    check(t.Consume(picker,accept));check(!t.Consume(picker,accept));check(!t.Arrive(screen(settings,7)));
    auto maps=screen(map,8);auto setup=screen(settings,9);check(t.Arrive(maps));check(!t.Submit(maps));check(t.Arrive(setup));check(t.Submit(setup));check(!t.Submit(setup));
    Transaction cancelled;check(bind(cancelled));check(cancelled.RequestEntry(source));check(cancelled.Borrow(source,Step::Entry,sandbox,entry));check(cancelled.Consume(source,entry));
    check(cancelled.Arrive(picker));check(cancelled.RequestAccept(picker));check(cancelled.Borrow(picker,Step::AcceptedCounselor,0,accept));
    check(cancelled.Cancel(picker));check(!cancelled.Consume(picker,accept));check(!cancelled.Borrow(picker,Step::AcceptedCounselor,0,accept));
    check(cancelled.RequestEntry(source));check(cancelled.Borrow(source,Step::Entry,sandbox,entry));cancelled.Retire();check(!cancelled.Consume(source,entry));check(!cancelled.RequestEntry(source));
    for(unsigned fault=0;fault<5;++fault){auto bad=source;if(fault==0)bad.identities[2].serial=0;if(fault==1)bad.identities[1].serial=0;
        if(fault==2)bad.widget+=0x100;if(fault==3)bad.thread=0;Transaction invalid;check(!invalid.Bind(bad,counselor,map,settings,fault==4?map:sandbox));}
    std::printf("Dedicated counselor route admission: %s (%u checks; no callbacks/writes/native calls).\n",failures?"FAIL":"PASS",checks);return failures?1:0;
}
