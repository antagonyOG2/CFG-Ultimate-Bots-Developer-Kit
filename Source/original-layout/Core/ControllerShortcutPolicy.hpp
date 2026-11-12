#pragma once
#include <cstdint>
namespace cfgshortcut {
enum class Action:unsigned {None,Knife,Spray,Radar,Spectator};
constexpr std::uint16_t Up=0x0001,Down=0x0002,Left=0x0004,Right=0x0008,LB=0x0100,RB=0x0200;
class Policy {
    bool armed_=true,timing_=false;Action pending_=Action::None;std::uint64_t since_=0;
public:
    Action Poll(std::uint16_t buttons,bool connected,bool allowed,std::uint64_t now)noexcept{
        const auto held=buttons&(Up|Down|Left|Right|LB|RB);
        if(!connected||!allowed){timing_=false;pending_=Action::None;armed_=false;return Action::None;}
        if(!armed_){if(!held)armed_=true;return Action::None;}
        const auto direction=buttons&(Up|Down|Left|Right);
        const auto action=(buttons&(LB|RB))!=(LB|RB)?Action::None:
            direction==Up?Action::Knife:direction==Down?Action::Spray:direction==Left?Action::Radar:direction==Right?Action::Spectator:Action::None;
        if(action==Action::None){timing_=false;pending_=Action::None;return Action::None;}
        if(!timing_||pending_!=action||now<since_){timing_=true;pending_=action;since_=now;return Action::None;}
        if(now-since_<500)return Action::None;
        armed_=false;timing_=false;pending_=Action::None;return action;
    }
};
}
