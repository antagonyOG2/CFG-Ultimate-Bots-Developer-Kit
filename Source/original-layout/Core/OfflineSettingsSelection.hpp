#pragma once
#include <cstdint>
namespace cfgofflinesettings {
// Separate versioned handoff: zero seconds preserves native CFG settings.
// This is captured only after the menu's exact new-world handoff succeeds.
struct Request {
    std::uint64_t submission=0;
    std::uintptr_t world=0,pawn=0;
    std::int32_t worldSerial=0,pawnSerial=0;
    unsigned seconds=0;
    unsigned nativeSkill=3; // 3 preserves native; CFG Normal=0, Easy=1, Hard=2.
    bool Valid()const noexcept{return submission&&world>=0x0/*CFG3.5 address withheld*/&&!(world&7)&&pawn>=0x0/*CFG3.5 address withheld*/&&!(pawn&7)&&
        worldSerial>0&&pawnSerial>0&&nativeSkill<=3&&(seconds==0||seconds==1800||seconds==3600||seconds==5400||seconds==UINT32_MAX);}
};
enum class State {Empty,Waiting,Applying,Applied,Preserved,Refused,Indeterminate};
// Duration is optional. A refused/partially observed timer transaction is
// reported and retired, not permission to stop the independently owned AI.
// This does NOT authorize another write or hide a failed setting as Applied.
inline bool DurationFinished(State state){
    return state==State::Applied||state==State::Preserved||state==State::Refused||state==State::Indeterminate;
}
constexpr unsigned MaximumArrivalElapsedSeconds=120;
// Reconcile once from native elapsed-in-progress seconds, never reset the
// countdown on a tick. Pre-match/outro timers cannot satisfy this policy.
inline bool Reconcile(unsigned seconds,std::int32_t elapsed,std::int32_t remaining,std::int32_t& output)noexcept{
    output=0;
    if((seconds!=1800&&seconds!=3600&&seconds!=5400&&seconds!=UINT32_MAX)||elapsed<0||elapsed>static_cast<int>(MaximumArrivalElapsedSeconds)||
       remaining < -1||remaining>86400)return false;
    if(seconds==UINT32_MAX){output=-1;return true;}
    output=static_cast<std::int32_t>(seconds)-elapsed;return output>0;
}
}
