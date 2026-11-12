#pragma once
#include "KillPolicy.hpp"
namespace cfgdeathend {
struct Policy {
    cfgkill::Identity identity{};std::uint64_t confirmedAt=0,due=0,next=0;bool dispatched=false;
    bool Confirm(const cfgkill::Policy& kill,bool nativeDying,std::uint64_t now){
        if(dispatched||!nativeDying||!kill.identity.Valid()||!kill.interactionAttempted||now>UINT64_MAX-25000)return false;
        if(confirmedAt)return identity==kill.identity;
        identity=kill.identity;confirmedAt=now?now:1;due=now+25000;return true;
    }
    bool Ready(std::uint64_t now)const{return confirmedAt&&!dispatched&&now>=due;}
    bool Mark(std::uint64_t now){if(!Ready(now))return false;dispatched=true;return true;}
};
}
