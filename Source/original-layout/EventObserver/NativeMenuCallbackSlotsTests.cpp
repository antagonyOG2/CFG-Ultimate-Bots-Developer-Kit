#include "NativeMenuCallbackSlots.hpp"
#include <cstdio>
using namespace cfgmenu;
struct Host {
    std::array<std::uintptr_t,callbackslots::Maximum> actual{};unsigned exchanges=0,failureAt=999;
    bool owner=true,resident=true,foreign=false;
    bool Resident(const callbackslots::Plan&){return resident;}
    bool Current(const callbackslots::Plan& p,unsigned count){
        if(!owner||count>p.count)return false;
        for(unsigned i=0;i<p.count;++i)if(actual[i]!=(i<count?p.cells[i].replacement:p.cells[i].original))return false;return true;
    }
    sandboxstart::WriteResult Exchange(std::uintptr_t at,std::uintptr_t expected,std::uintptr_t next){
        const auto i=static_cast<unsigned>((at-0x100000)/8);++exchanges;
        if(i==failureAt){if(foreign)actual[0]=0x700000;return sandboxstart::WriteResult::Refused;}
        if(i>=actual.size()||actual[i]!=expected)return sandboxstart::WriteResult::Refused;actual[i]=next;return sandboxstart::WriteResult::Changed;
    }
};
int main(){using namespace callbackslots;unsigned checks=0,failures=0;const auto check=[&](bool value){++checks;if(!value)++failures;};
    Plan p;p.count=8;for(unsigned i=0;i<p.count;++i)p.cells[i]={0x100000+8*i,0x200000,0x300000+0x100*i};
    const auto owned=[&](){Host h;h.actual.fill(0x200000);return h;};
    auto h=owned();Transaction t;check(t.Prepare(h,p));check(!t.Prepare(h,p));
    for(unsigned i=0;i<p.count;++i){const auto before=h.exchanges;check(t.Step(h));check(h.exchanges==before+1);check(t.Installed()==i+1);}
    check(t.CurrentState()==State::Installed);check(!t.Step(h));check(t.Restore(h));
    for(unsigned i=0;i<p.count;++i){const auto before=h.exchanges;check(t.Step(h));check(h.exchanges==before+1);}
    check(t.Installed()==0&&t.CurrentState()==State::Restored);check(h.actual[0]==p.cells[0].original);
    for(unsigned at=0;at<p.count;++at){auto failing=owned();failing.failureAt=at;Transaction rollback;check(rollback.Prepare(failing,p));
        for(unsigned i=0;i<at;++i)check(rollback.Step(failing));check(!rollback.Step(failing));
        if(at)check(rollback.CurrentState()==State::Restoring);
        for(unsigned i=0;i<at;++i)check(rollback.Step(failing));check(rollback.CurrentState()==State::Refused&&rollback.Installed()==0);}
    auto changed=owned();changed.failureAt=2;changed.foreign=true;Transaction lost;check(lost.Prepare(changed,p));check(lost.Step(changed));check(lost.Step(changed));
    check(!lost.Step(changed));check(lost.CurrentState()==State::Indeterminate);const auto calls=changed.exchanges;check(!lost.Step(changed));check(changed.exchanges==calls&&changed.actual[0]==0x700000);
    for(unsigned fault=0;fault<6;++fault){auto bad=p;auto denied=owned();if(fault==0)bad.count=0;if(fault==1)bad.count=Maximum+1;
        if(fault==2)bad.cells[1].address=bad.cells[0].address;if(fault==3)bad.cells[1].replacement=bad.cells[1].original;
        if(fault==4)denied.owner=false;if(fault==5)denied.resident=false;Transaction invalid;check(!invalid.Prepare(denied,bad));check(!denied.exchanges);}
    std::printf("Class-local callback publication/conditional rollback: %s (%u checks; owned memory only).\n",failures?"FAIL":"PASS",checks);return failures?1:0;
}
