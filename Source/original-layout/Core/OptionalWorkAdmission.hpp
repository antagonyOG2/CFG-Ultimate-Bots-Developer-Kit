#pragma once
#include <cstdint>
namespace cfgoptionalwork {
inline bool RadioDue(bool finished,bool activeBirth,std::uint64_t next,std::uint64_t now){
    return !finished&&!activeBirth&&now>=next;
}
inline bool WheelTurn(bool active,bool pressed,bool exclusive,bool knifeDue,bool pending,bool previous){
    return active&&!pressed&&!exclusive&&!knifeDue&&pending&&!previous;
}
inline bool SummaryTurn(bool active,bool pressed,bool exclusive,bool urgent,bool previous,bool other,bool pending){
    return active&&!pressed&&!exclusive&&!urgent&&!previous&&!other&&pending;
}
}
