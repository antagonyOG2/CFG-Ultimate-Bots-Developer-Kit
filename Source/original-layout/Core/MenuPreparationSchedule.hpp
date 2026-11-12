#pragma once
#include <cstdint>
namespace cfgmenu::preparation {
enum class State {Idle,Working,Ready,Refused};
// One cold preparation stage per paced owner callback. No catch-up bursts,
// retries, native widget calls, workers or gameplay work live in this scheduler.
class Schedule {
    State state_=State::Idle;unsigned next_=0,count_=0;std::uint64_t last_=0;bool paced_=false;
public:
    bool Begin(unsigned count){if(state_!=State::Idle||!count||count>8)return false;count_=count;state_=State::Working;return true;}
    template<class Host> bool Step(Host& host,std::uint64_t now){
        if(state_!=State::Working||(paced_&&(now<last_||now-last_<25)))return false;
        paced_=true;last_=now;
        if(!host.PreparationCurrent()||!host.PrepareStage(next_)){state_=State::Refused;return false;}
        if(++next_==count_)state_=State::Ready;
        return true;
    }
    unsigned Stage()const noexcept{return next_;}
    State Current()const noexcept{return state_;}
};
}
