#include "../Core/MenuPreparationSchedule.hpp"
#include <array>
#include <cstdio>
struct Host {
    bool current=true;unsigned refuse=9,checks=0,calls=0;std::array<unsigned,4> stages{};
    bool PreparationCurrent(){++checks;return current;}
    bool PrepareStage(unsigned stage){++calls;++stages[stage];return stage!=refuse;}
};
int main(){unsigned failures=0;const auto check=[&](bool okay,const char* label){if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    for(unsigned fault=0;fault<6;++fault){cfgmenu::preparation::Schedule s;Host h;
        check(s.Begin(4)&&!s.Begin(4),"one preparation only");
        if(fault<4)h.refuse=fault;
        if(fault==4)h.current=false;
        for(unsigned stage=0;stage<4&&s.Current()==cfgmenu::preparation::State::Working;++stage){
            const auto before=h.calls;const auto worked=s.Step(h,100+stage*25);
            check(h.calls-before<=1,"at most one cold stage per callback");
            if(worked){check(!s.Step(h,100+stage*25)&&!s.Step(h,101+stage*25),"no same-callback or catchup burst");}
        }
        check(s.Current()==(fault==5?cfgmenu::preparation::State::Ready:cfgmenu::preparation::State::Refused),"complete or terminal refusal");
        const auto calls=h.calls,checks=h.checks;
        for(unsigned i=0;i<1000;++i)check(!s.Step(h,1000+i*25),"no retry or ongoing ready work");
        check(h.calls==calls&&h.checks==checks,"terminal preparation has zero reads/work");
        for(auto count:h.stages)check(count<=1,"each stage runs once");
    }
    std::printf("CFG paced cold-menu preparation: %s (%u failures). No game access.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
