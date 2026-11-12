#include "../Core/OptionalWorkAdmission.hpp"
#include "../Core/RBEmoteInputPolicy.hpp"
#include <cstdio>
struct Host {unsigned native=0;bool Source(unsigned& id){id=1;return true;}bool Current(unsigned id){return id==1;}
    bool Warm(bool){++native;return true;}bool Call(unsigned,bool){++native;return true;}};
int main(){unsigned failed=0;const auto check=[&](bool v){if(!v)++failed;};
    using namespace cfgoptionalwork;
    check(RadioDue(false,false,100,100));check(!RadioDue(true,false,100,100));
    check(!RadioDue(false,true,100,100));check(!RadioDue(false,false,101,100));
    check(!WheelTurn(true,true,false,false,true,false));check(!WheelTurn(true,false,true,false,true,false));
    check(!WheelTurn(true,false,false,true,true,false));check(!WheelTurn(true,false,false,false,true,true));
    // Dense gameplay demand cannot expire a tapped RB edge. Wheel is selected
    // before gameplay Due(), so no gameplay deadline is consumed/discarded.
    cfgrbemote::EdgeCapture edges;cfgrbemote::Policy<unsigned> policy;Host host;bool previous=false;unsigned gameplay=0;
    for(unsigned now=0;now<600;now+=16){edges.Observe({true,true,now==0},now,true);
        const bool pending=edges.pending||(policy.phase!=cfgrbemote::Phase::Idle&&
            (policy.phase!=cfgrbemote::Phase::Open||!edges.latest.down));
        const bool wheel=WheelTurn(true,false,false,false,pending,previous);
        check(!(wheel&&previous));previous=wheel;
        if(wheel){if(edges.pending)policy.CapturedPress();policy.Step(host,edges.Take(now),true);}else ++gameplay;
    }
    check(policy.shows==1&&policy.hides==2&&edges.expired==0&&gameplay>=20&&host.native==5);
    for(unsigned fault=0;fault<8;++fault)
        check(SummaryTurn(fault!=1,fault==2,fault==3,fault==4,fault==5,fault==6,fault!=7)==(fault==0));
    unsigned summaryTurns=0,gameplayTurns=0;previous=false;
    for(unsigned i=0;i<20;++i){const bool summary=SummaryTurn(true,false,false,false,previous,false,true);
        check(!(summary&&previous));if(summary)++summaryTurns;else ++gameplayTurns;previous=summary;}
    check(summaryTurns==10&&gameplayTurns==10);
    std::printf("Optional radio and fair RB scheduling:%u failures; native calls mocked.\n",failed);return failed?1:0;
}
