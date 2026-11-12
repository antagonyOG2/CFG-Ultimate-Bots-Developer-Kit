#pragma once
#include <cstdint>
namespace cfgmenu::arrivalwindow {
// A single pending native launch, not an idle gameplay scanner. At most one
// small read sample every100ms; no catch-up after long frames or loading.
class Window {
    std::uint64_t start_=0,next_=0,stable_=0,world_=0,pawn_=0;
    bool armed_=false,finished_=false,ready_=false;
public:
    static constexpr std::uint64_t Interval=100,Timeout=45000,StableFor=750;
    unsigned checks=0;
    bool Start(std::uint64_t now)noexcept{if(armed_||finished_)return false;start_=next_=now;armed_=true;return true;}
    bool Due(std::uint64_t now)noexcept{
        if(!armed_||finished_)return false;
        if(now<start_||now-start_>=Timeout){finished_=true;return false;}
        if(now<next_)return false;next_=now+Interval;++checks;return true;
    }
    bool Sample(std::uint64_t now,std::uint64_t world,std::uint64_t pawn,bool ready)noexcept{
        if(!armed_||finished_)return false;
        ready_=false;
        if(!ready||!world||!pawn){world_=pawn_=0;stable_=now;return false;}
        if(world!=world_||pawn!=pawn_){world_=world;pawn_=pawn;stable_=now;return false;}
        ready_=now>=stable_&&now-stable_>=StableFor;return ready_;
    }
    // Readiness sampling may happen on another actor's callback. The next
    // exact pawn callback may consume it without waiting for another Due slot.
    bool ReadyFor(std::uint64_t object)const noexcept{return !finished_&&ready_&&object==pawn_;}
    void Finish()noexcept{finished_=true;}
    bool Finished()const noexcept{return finished_;}
};
}
