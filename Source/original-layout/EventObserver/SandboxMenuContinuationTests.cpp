#include "../Core/SandboxMenuContinuation.hpp"
#include <cstdio>
using namespace cfgmenu::continuation;
struct Host {
    Plan evidence;std::array<std::uintptr_t,MaximumCells> values{};unsigned observes=0,checks=0,writes=0,fault=0;bool current=true;
    Host(){evidence.settingsClass=0x200000;evidence.count=4;for(unsigned i=0;i<4;++i)evidence.cells[i]={0x300000+8*i,i?std::uintptr_t{0}:std::uintptr_t{0x400000}};values[0]=0x400000;}
    bool Observe(Plan& out){++observes;out=evidence;return fault!=1;}
    bool Current(const Plan&){++checks;return current;}
    bool Published(const Plan&){for(unsigned i=0;i<evidence.count;++i)if(values[i]!=evidence.settingsClass)return false;return current;}
    Write Exchange(const Cell& cell,std::uintptr_t expected,std::uintptr_t replacement){++writes;
        if((fault==2||fault==4||fault==5)&&writes==3)return Write::Refused;if(fault==3&&writes==3)return Write::Indeterminate;
        if(fault==4&&writes==4)return Write::Refused;if(fault==5&&writes==4)return Write::Indeterminate;
        unsigned index=static_cast<unsigned>((cell.address-0x300000)/8);if(index>=evidence.count||values[index]!=expected)return Write::Refused;
        values[index]=replacement;return Write::Changed;
    }
};
int main(){unsigned failures=0;const auto check=[&](bool okay,const char* text){if(!okay){++failures;std::printf("FAIL %s\n",text);}};
    for(unsigned fault=0;fault<12;++fault){Host host;Transaction route;
        if(fault<6)host.fault=fault;
        if(fault==6)host.evidence.count=1;if(fault==7)host.evidence.count=66;
        if(fault==8)host.evidence.cells[2].address=host.evidence.cells[1].address;
        if(fault==9)host.evidence.cells[1].address++;
        if(fault==10)host.evidence.settingsClass=0;if(fault==11)host.evidence.cells[1].previous=3;
        const bool prepared=route.Prepare(host);check(prepared==(fault!=1&&fault<6),"invalid plan never publishes");
        if(!prepared){check(!host.writes,"preparation refusal zero writes");continue;}
        for(unsigned i=0;i<12;++i)route.Step(host,25*i);
        check(route.CurrentState()==(fault==0?State::Ready:fault==3||fault==5?State::Indeterminate:fault==4?State::Detached:State::Refused),"explicit final transaction status");
        if(fault==2)check(host.values[0]==0x400000&&!host.values[1],"definite failure restores only own published cells");
        if(fault==0)for(unsigned i=0;i<4;++i)check(host.values[i]==host.evidence.settingsClass,"native continuation class published to each populated button");
        const auto writes=host.writes,checks=host.checks;
        check(!route.Step(host,1000)&&host.writes==writes&&host.checks==checks,"retired/ready transaction has zero ongoing reads or writes");
        check(!route.Prepare(host)&&host.observes==1,"no automatic retry");
    }
    {Host host;Transaction route;check(route.Prepare(host),"paced plan");route.Step(host,100);const auto writes=host.writes,checks=host.checks;
        check(!route.Step(host,110)&&!route.Step(host,90)&&host.writes==writes&&host.checks==checks,"same interval/backward clock zero work");
        route.Step(host,10000);check(host.writes==writes+1,"long pause never catches up in a burst");
        host.current=false;check(!route.Step(host,10025)&&route.CurrentState()==State::Detached,"lost menu stops before writes");}
    {Host host;Transaction route;check(route.Prepare(host),"final readback plan");
        for(unsigned i=0;i<3;++i)route.Step(host,25*i);host.values[0]=0x400000;
        route.Step(host,75);check(route.CurrentState()==State::Restoring,"reverted earlier cell cannot report Ready");}
    std::printf("Native Sandbox setup continuation transaction: %s (%u failures); no game access.\n",failures?"FAIL":"PASS",failures);return failures?1:0;
}
