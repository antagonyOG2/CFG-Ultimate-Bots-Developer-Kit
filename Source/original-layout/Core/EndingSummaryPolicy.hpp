#pragma once
#include <cstdint>
#include <string_view>
namespace cfgsummaryending {
inline bool Admission(bool started,bool active,bool pressed,bool nativeEnding=false){return started&&(nativeEnding||!active||!pressed);}
inline bool Phase(std::u16string_view phase){
    return phase==u"WaitingPostMatchOutro"||phase==u"PostMatchOutro"||phase==u"WaitingPostMatch";
}
// Independent presentation window. Never changes Schedule.active, input or
// native completion. World/controller/participants/widget are proved by host.
struct Window {
    std::uint64_t until=0,next=0,probeAt=0;
    bool finished=false,servicing=false;
    unsigned phaseTraces=0,scopeTraces=0;
    bool Probe(std::uint64_t now){if(finished||now<probeAt)return false;probeAt=now+250;return true;}
    bool Due(std::uint64_t now,bool terminal){
        if(!terminal||finished)return false;
        if(!until){if(now>UINT64_MAX-120000){finished=true;return false;}until=now+120000;}
        if(now>until){finished=true;return false;}
        if(now<next)return false;
        next=now+250;return true;
    }
};
}
