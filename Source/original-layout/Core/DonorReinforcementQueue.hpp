#pragma once
#include "GameplayLimits.hpp"
namespace cfgreinforcement {
// Keep donor paced births; requested CFG extension allows filling the same
// ten-total roster as setup. Pending requests consume capacity immediately.
struct Counselors {
    static constexpr unsigned AdditionalLimit=cfglimits::MaximumHelpers;
    unsigned accepted=0,pending=0;
    bool Request(unsigned requestedHelpers){
        if(accepted>=AdditionalLimit||requestedHelpers>cfglimits::MaximumHelpers||
           pending>=cfglimits::MaximumHelpers-requestedHelpers)return false;
        ++accepted;++pending;return true;
    }
    bool Ready(unsigned spawned,unsigned requested)const{return pending&&spawned==requested&&requested<cfglimits::MaximumHelpers;}
    void Started(){if(pending)--pending;}
    void Cancel(){pending=0;} // no refund of an ambiguous allocated native actor
};
}
