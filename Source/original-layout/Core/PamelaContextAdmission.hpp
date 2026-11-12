#pragma once
#include <cstdint>
namespace cfgpamela {
// Mailbox195: bounded failed *preparation*, never cached native authority.
// Camera/RB/other independent work must not lose an already-selected turn.
struct ContextRetry {
    std::uint64_t useAt=0,retryAt=0;
    bool Due(std::uint64_t use,std::uint64_t now){
        if(useAt!=use){useAt=use;retryAt=0;}
        return now>=retryAt;
    }
    void Failed(std::uint64_t now){retryAt=now+250;}
    void Succeeded(){retryAt=0;}
};
inline bool ContextTurn(bool opportunity,bool spectator,bool independent,bool mirror,bool input){
    (void)spectator;(void)independent;(void)mirror;(void)input;
    // KP0 validation always gets first opportunity. A failed preparation then
    // falls through to the already-selected RB/spectator/independent turn.
    return opportunity;
}
}
