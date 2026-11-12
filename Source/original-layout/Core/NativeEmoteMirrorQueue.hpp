#pragma once
#include "GameplayLimits.hpp"
#include <array>
#include <cstdint>
namespace cfgemotemirror {
class Queue {
    unsigned cursor_=0,count_=0;std::uint64_t next_=0,started_=0,last_=0,spacing_=SpacingMs,lifetime_=LifetimeMs;
public:
    static constexpr unsigned Capacity=cfglimits::MaximumHelpers;
    static constexpr std::uint64_t SpacingMs=25,LifetimeMs=3000;
    bool Start(unsigned count,std::uint64_t now,std::uint64_t spacing=SpacingMs){
        Clear();if(!count||count>Capacity||spacing<SpacingMs||spacing>250)return false;
        lifetime_=LifetimeMs+(spacing==SpacingMs?0u:static_cast<std::uint64_t>(count)*spacing);
        if(now>UINT64_MAX-lifetime_)return false;
        count_=count;started_=last_=now;next_=now+SpacingMs;spacing_=spacing;return true;
    }
    bool Pending()const{return cursor_<count_;}
    bool Due(std::uint64_t now)const{return Pending()&&now>=next_;}
    bool Take(std::uint64_t now,unsigned& index){
        if(!Pending())return false;
        if(now<last_||now-started_>lifetime_){Clear();return false;}
        last_=now;if(now<next_)return false;
        index=cursor_++;next_=now+spacing_;return true; // never catch-up/drain a burst
    }
    void Clear(){cursor_=count_=0;next_=started_=last_=0;}
};
}
