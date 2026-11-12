#pragma once
#include "../SourceBaseline/gameplay-backend/port/src/Game/Setup/JasonChaseRetryPolicy.hpp"
namespace cfgchaserecovery {
struct Policy {
    std::uintptr_t target=0;unsigned repeated=0,cursor=0;bool pending=false;
    std::uint64_t until=0;
    void Observe(std::uintptr_t pawn,unsigned result,float squared,std::uint64_t now){
        if(!JasonChaseRetryPolicy::IsSuspiciousAlreadyAtGoal(static_cast<std::uint8_t>(result),squared,true)){
            target=0;repeated=0;pending=false;return;
        }
        repeated=static_cast<unsigned>(JasonChaseRetryPolicy::AdvanceRepeatedResult(static_cast<int>(repeated),pawn==target));
        target=pawn;
        if(repeated<2)pending=false;
        if(repeated>=2){pending=true;cursor=0;until=now+5000;}
    }
};
}
