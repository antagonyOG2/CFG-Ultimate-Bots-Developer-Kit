#pragma once
#include <cstdint>

namespace cfghostedend {
inline bool DriverActive(bool frozen,bool adopted)noexcept{return adopted&&!frozen;}
inline bool Freeze(bool& frozen,std::uint64_t& at,std::uint32_t& count,std::uint64_t now)noexcept{
    if(frozen)return false;frozen=true;at=now;++count;return true;
}
inline void ClearForActivation(bool& frozen,std::uint64_t& watchAt)noexcept{frozen=false;watchAt=0;}
inline void ClearForRetirement(bool& frozen,std::uint64_t& watchAt)noexcept{frozen=false;watchAt=0;}
}
