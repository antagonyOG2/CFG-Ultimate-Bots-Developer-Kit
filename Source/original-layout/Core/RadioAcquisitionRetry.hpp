#pragma once
#include <cstdint>
namespace cfgradioretry {
constexpr unsigned MutationLimit=32;
inline bool Allowed(bool extended,bool attempted,unsigned failure,unsigned retries,std::uint64_t elapsed){
    if(attempted||elapsed>=90000)return false;
    if(extended&&failure==5)return retries<MutationLimit;
    return retries<8&&(failure==2||failure==4||failure==5);
}
inline std::uint64_t Delay(bool extended,unsigned failure,unsigned retries){
    return extended&&failure==5?(retries<4?1000u:2500u):1000u;
}
}
