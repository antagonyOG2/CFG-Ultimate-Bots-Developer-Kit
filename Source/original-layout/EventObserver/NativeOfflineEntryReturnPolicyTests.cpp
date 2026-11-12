#include "NativeOfflineEntryReturnPolicy.hpp"
#include <cstdio>
namespace {
using namespace cfgmenu;
struct Host {
    active::Context actual{};unsigned checks=0;bool graph=true,descriptors=true,executors=true,children=true,labels=true;
    bool ReturnCurrent(const active::Context&,const active::Context& supplied)noexcept{
        ++checks;return graph&&descriptors&&executors&&children&&actual==supplied;
    }
    bool RelabeledCurrent(const active::Context& supplied)noexcept{return labels&&actual==supplied;}
};
active::Context Source(){active::Context c;c.module=0x100000;c.thread=5;c.engine=0x101000;c.viewport=0x102000;
    c.world=0x110000;c.mode=0x120000;c.controller=0x130000;c.hud=0x140000;c.stack=0x150000;c.gameInstance=0x160000;
    c.players=0x161000;c.player=0x162000;c.widget=0x170000;c.type=0x180000;c.array={0x190000,2,3};
    c.identities={{{c.world,1,11},{c.controller,2,12},{c.widget,3,13},{c.hud,4,14}}};return c;}
constexpr std::uintptr_t Counselor=0x200000;
active::Context Picker(){auto c=Source();c.widget=0x210000;c.type=Counselor;c.array.count=3;c.identities[2]={c.widget,5,15};return c;}
active::Context Returned(){auto c=Source();c.widget=0x220000;c.array={0x230000,2,4};c.identities[2]={c.widget,6,16};return c;}
bool Await(entryreturn::Policy& policy){const auto p=Picker();return policy.Bind(Source(),Counselor)&&policy.Begin(p,p.identities[2],p.identities[1]);}
}
int main(){using namespace cfgmenu::entryreturn;unsigned checks=0,failures=0;
    const auto check=[&](bool value,const char* why){++checks;if(!value){++failures;std::printf("FAIL %s\n",why);}};
    {Policy p;auto source=Source();source.array.count=3;
     auto picker=Picker();picker.array={0x190000,4,4};
     Host h;h.actual=Returned();h.actual.array.count=3;Ticket t;
     check(p.Bind(source,Counselor)&&p.Begin(picker,picker.identities[2],picker.identities[1]),"new epoch keeps actual entry depth3");
     auto wrong=h.actual;wrong.array.count=2;check(!p.Observe(h,wrong,t),"old hardcoded depth cannot authorize new-epoch Back");
     check(p.Observe(h,h.actual,t)&&p.CommitRelabeled(h,t),"native Back depth is relative to captured epoch");}
    {Policy p;Host h;h.actual=Returned();Ticket t;check(Await(p),"positively owned Back begins return only");
        check(p.Observe(h,h.actual,t)&&p.NeedsRelabel(),"new native instance and relocated stack need fresh labels");
        check(p.Source()==Source(),"observation cannot prematurely replace source authority");
        h.labels=false;check(!p.CommitRelabeled(h,t)&&p.NeedsRelabel(),"no commit before actual label readback");h.labels=true;
        check(p.CommitRelabeled(h,t)&&p.Source()==h.actual&&!p.NeedsRelabel(),"commit fresh actual source only after host proof");
        check(!p.CommitRelabeled(h,t),"return ticket consumed once");
        auto picker=Picker();check(p.Begin(picker,picker.identities[2],picker.identities[1]),"second owned counselor visit may return");
        Ticket again;h.actual=Source();h.actual.widget=0x240000;h.actual.identities[2]={h.actual.widget,7,17};
        check(p.Observe(h,h.actual,again)&&!p.CommitRelabeled(h,t)&&p.CommitRelabeled(h,again),"old generation never commits second return");}
    {Policy p;Host h;h.actual=Source();Ticket t;check(Await(p)&&p.Observe(h,h.actual,t)&&p.NeedsRelabel(),"same instance must also relabel after native reconstruction");}
    {Policy p;Host h;h.actual=Source();++h.actual.identities[2].serial;Ticket t;
        check(Await(p)&&p.Observe(h,h.actual,t)&&p.CommitRelabeled(h,t),"same address fresh positive serial admitted only through full fresh return proof");}
    for(unsigned fault=0;fault<20;++fault){Policy p;check(Await(p),"mutation fixture");Host h;h.actual=Returned();
        if(fault==0)++h.actual.module;if(fault==1)++h.actual.thread;if(fault==2)++h.actual.engine;
        if(fault==3)++h.actual.viewport;if(fault==4)++h.actual.world;if(fault==5)++h.actual.mode;
        if(fault==6)++h.actual.gameInstance;if(fault==7)++h.actual.players;if(fault==8)++h.actual.player;
        if(fault==9)++h.actual.controller;if(fault==10)++h.actual.hud;if(fault==11)++h.actual.stack;
        if(fault==12)++h.actual.identities[0].serial;if(fault==13)++h.actual.identities[1].serial;
        if(fault==14)++h.actual.identities[3].serial;if(fault==15)h.actual.identities[2].serial=0;
        if(fault==16)++h.actual.identities[2].object;if(fault==17)h.actual.type=Counselor;
        if(fault==18)h.actual.array.count=3;if(fault==19)h.actual.array.capacity=1;
        Ticket t;check(!p.Observe(h,h.actual,t)&&!t.generation&&!p.NeedsRelabel(),"changed owner/route identity cannot rebind");}
    for(unsigned fault=0;fault<4;++fault){Policy p;check(Await(p),"host proof fixture");Host h;h.actual=Returned();
        if(fault==0)h.graph=false;if(fault==1)h.descriptors=false;if(fault==2)h.executors=false;
        if(fault==3)h.children=false;Ticket t;
        check(!p.Observe(h,h.actual,t),"unchanged source/native graph, descriptors, callbacks and children required");}
    {Policy p;Host h;h.actual=Returned();Ticket t;check(p.Bind(Source(),Counselor)&&!p.Observe(h,h.actual,t),"unsolicited entry instance is not Back authority");
        auto picker=Picker();auto bad=picker.identities[1];++bad.serial;
        check(!p.Begin(picker,picker.identities[2],bad),"foreign owning player refuses Back");
        bad=picker.identities[2];++bad.serial;check(!p.Begin(picker,bad,picker.identities[1]),"foreign picker receiver refuses Back");}
    {Policy p;Host h;h.actual=Returned();Ticket t;check(Await(p)&&p.Observe(h,h.actual,t),"fresh ticket fixture");
        auto changed=t;++changed.entry.identities[2].serial;check(!p.CommitRelabeled(h,changed),"changed ticket refuses");
        ++h.actual.identities[2].serial;check(!p.CommitRelabeled(h,t),"actual owner changing during relabel refuses");
        p.Retire();check(!p.CommitRelabeled(h,t)&&!p.Observe(h,h.actual,t),"retirement is terminal");}
    std::printf("Native OfflinePlay Back/relabel policy: %s (%u checks; owned fixtures, no game access/calls/writes).\n",failures?"FAIL":"PASS",checks);
    return failures?1:0;
}
