#pragma once
#include <cstdint>
namespace cfgurination {
class MovementSample {
    bool moving_=false;std::uint64_t sampled_=0;
public:
    void Observe(bool foreground,bool connected,std::int16_t x,std::int16_t y,std::uint64_t now,int deadzone)noexcept{
        sampled_=now;const int wideX=x,wideY=y;
        moving_=foreground&&connected&&deadzone>=0&&
            (wideX>deadzone||wideX< -deadzone||wideY>deadzone||wideY< -deadzone);
    }
    bool Recent(std::uint64_t now)const noexcept{return moving_&&now>=sampled_&&now-sampled_<100;}
};
// Only the exact retained replacement montage is eligible. No gameplay writes,
// input unlocks or arbitrary montage stops; the host calls native cancellation.
class PlaybackPolicy {
    std::uintptr_t pawn_=0,world_=0;std::int32_t serial_=0;std::uint64_t since_=0;bool consumed_=false;
public:
    // A consumed cancellation belongs to one native playback, not the pawn
    // forever. Only the existing before/after native-start proof can rearm it;
    // another attempted/failed selection must not replay an old cancellation.
    bool NewNativePlayback(bool completed,unsigned before,unsigned after)noexcept{
        if(!completed||after<=before)return false;*this={};return true;
    }
    void Observe(std::uintptr_t world,std::uintptr_t pawn,std::int32_t serial,bool replacement,std::uint64_t now)noexcept{
        if(!world||!pawn||serial<=0||!replacement){*this={};return;}
        if(world!=world_||pawn!=pawn_||serial!=serial_){world_=world;pawn_=pawn;serial_=serial;since_=now;consumed_=false;}
    }
    bool Due(std::uint64_t now,bool movement)const noexcept{return pawn_&&!consumed_&&now>=since_&&
        (now-since_>=25000||(movement&&now-since_>=250));}
    bool Consume(std::uint64_t now,bool movement)noexcept{
        if(!Due(now,movement))return false;consumed_=true;return true;
    }
    bool Active()const noexcept{return pawn_&&!consumed_;}
    bool ConsumeWheel()noexcept{if(!Active())return false;consumed_=true;return true;}
};
}
