#pragma once
#include <cstdint>
#include <string_view>
namespace cfgresidentcounselor {
// CFG names Tommy Hunter_Counselor_C; donor builds also use Hero_Counselor_C.
inline bool Ordinary(std::u16string_view name){return !name.empty()&&name!=u"Base_Counselor_C"&&name!=u"Hero_Counselor_C"&&name!=u"Hunter_Counselor_C";}
// Donor Features::ResolveRandomCounselorClass: shuffled resident native
// catalogue first, same local class last. No synchronous or latent asset load
// during play; class/source/AI/CDO proof remains the caller's responsibility.
struct Pass {
    unsigned local=~0u,retry=~0u,reused=~0u;bool fallback=false;
    template<class Pool> bool Next(Pool& pool,unsigned& index){
        if(retry<pool.count){index=retry;retry=~0u;return true;}
        if(pool.cursor<pool.count){index=pool.order[pool.cursor++];return true;}
        if(!fallback&&(reused<pool.count||local<pool.count)){index=reused<pool.count?reused:local;fallback=true;return true;}
        return false;
    }
    void Retry(unsigned index){retry=index;}
    bool Use(std::uintptr_t resolved,std::uintptr_t human,unsigned index,bool used=false){
        if(!resolved)return false;
        if(resolved==human&&!fallback){local=index;return false;}
        if(used&&!fallback){if(reused==~0u)reused=index;return false;}
        return resolved!=human||fallback;
    }
};
}
