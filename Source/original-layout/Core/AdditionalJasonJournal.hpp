#pragma once
#include <cstdint>
namespace cfgadditionaljournal {
struct Pair {std::uintptr_t pawn=0,state=0;};
struct Journal {
    Pair primary{};std::uintptr_t extra=0,genericState=0,killerState=0;
    bool captured=false,dispatched=false,uncertain=false;
    bool Capture(Pair value){if(captured||!value.pawn)return false;primary=value;captured=true;return true;}
    bool Primary(Pair value)const{return captured&&value.pawn==primary.pawn&&value.state==primary.state;}
    bool Attributed(Pair value)const{
        return captured&&extra&&(value.pawn==primary.pawn||value.pawn==extra)&&
            (value.state==primary.state||!value.state||
             (genericState&&value.state==genericState)||(killerState&&value.state==killerState));
    }
    void Returned(bool okay,std::uintptr_t pawn){dispatched=true;if(pawn)extra=pawn;if(!okay)uncertain=true;}
};
}
