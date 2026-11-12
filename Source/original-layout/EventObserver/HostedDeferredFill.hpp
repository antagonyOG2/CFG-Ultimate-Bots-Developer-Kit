#pragma once
#include <cstdint>
namespace cfghostedfill {
inline bool DeferredReady(bool hostMatch,bool frameSeen,bool enabled,bool disarmed,bool stopped,
    unsigned pending,bool quarantined,std::uint64_t now,std::uint64_t stableAt,std::uint64_t retryAt){
    return hostMatch&&frameSeen&&enabled&&!disarmed&&!stopped&&!pending&&!quarantined&&
        now>=stableAt&&now-stableAt>=3000&&now>=retryAt;
}
}
