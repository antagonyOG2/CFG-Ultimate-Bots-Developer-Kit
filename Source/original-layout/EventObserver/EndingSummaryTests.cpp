#include "../Core/EndingSummaryPolicy.hpp"
#include "../Core/OptionalWorkAdmission.hpp"
#include <cstdio>
int main(){unsigned failed=0;const auto check=[&](bool value){if(!value)++failed;};
    using namespace cfgsummaryending;
    check(!Admission(false,false,false)&&Admission(true,true,false)&&!Admission(true,true,true)&&
        Admission(true,false,true)&&Admission(true,false,false));
    check(Admission(true,true,true,true)&&!Admission(false,false,true,true)); // only native ending overrides stale input
    check(Phase(u"WaitingPostMatchOutro")&&Phase(u"PostMatchOutro")&&Phase(u"WaitingPostMatch"));
    check(!Phase(u"InProgress")&&!Phase(u"LeavingMap")&&!Phase(u"WaitingToStart")&&!Phase(u""));
    Window window;check(!window.Due(100,false));check(window.Due(100,true));
    Window nativeEnd;check(nativeEnd.Probe(100)&&!nativeEnd.Probe(349)&&nativeEnd.Probe(350));
    // No custom completion flag is needed once the native phase ends.
    check(nativeEnd.Due(350,Phase(u"WaitingPostMatchOutro")));
    check(!window.Due(349,true)&&window.Due(350,true));
    check(window.Due(120100,true)&&!window.Due(120101,true)&&!window.Due(200000,true));
    Window overflow;check(!overflow.Due(UINT64_MAX-10,true));
    // Old active-only maintenance gets no terminal turns. New presentation
    // window continues independently, without ever changing active=false.
    unsigned old=0,ending=0;bool active=false;Window replay;
    for(unsigned now=100;now<10100;now+=50){
        old+=cfgoptionalwork::SummaryTurn(active,false,true,false,false,false,true)?1u:0u;
        if(replay.Due(now,true)&&Phase(u"PostMatchOutro"))++ending;
    }
    check(old==0&&ending==40&&!active);
    std::printf("Ending Summary independent bounded presentation:%u failures; no gameplay restart/native calls.\n",failed);
    return failed?1:0;
}
