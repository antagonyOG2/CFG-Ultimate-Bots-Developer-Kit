#pragma once
#include <cstdint>
namespace cfgloadoutresource {
struct Pending {
    bool requested=false,ready=false,failed=false;std::uint64_t deadline=0,next=0;
    bool Poll(std::uint64_t now){if(ready||failed||now<next)return false;if(requested&&now>=deadline){failed=true;return false;}next=now+250;return true;}
    bool Request(std::uint64_t now){if(requested||failed||ready)return false;requested=true;deadline=now+30000;return true;}
};
constexpr std::int32_t LatentUuid=0x43464C01; // separate from counselor43 46 47 xx
}
