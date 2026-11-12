#pragma once
#include <cstdint>
namespace cfgkillerstart {
enum class BootstrapState { Empty, Pending, Ready, Refused };
// Native reflection initialization only, never asset loading or actor spawning.
// One getter/factory at most, on separate owner-thread callbacks. Cold cache is
// not a missing actor proof. A failed or ambiguous call is never retried.
struct Bootstrap {
    bool getterAttempted=false,factoryAttempted=false;
    unsigned calls=0;BootstrapState state=BootstrapState::Empty;
    template<class Host> BootstrapState Step(Host& host){
        if(state==BootstrapState::Refused||state==BootstrapState::Ready)return state;
        std::uintptr_t reflection=0,constructor=0;
        if(!host.Current()||!host.Caches(reflection,constructor)||
            (reflection&&constructor&&reflection!=constructor))return state=BootstrapState::Refused;
        if(reflection&&constructor)return state=BootstrapState::Ready;
        const bool getter=!constructor;
        if(getter?getterAttempted:factoryAttempted)return state=BootstrapState::Refused;
        if(getter)getterAttempted=true;else factoryAttempted=true;
        ++calls;
        std::uintptr_t result=0;
        if(!host.InitializeClass(getter,result)||!result||!host.Current()||
            !host.Caches(reflection,constructor)||
            result!=(getter?constructor:reflection)||
            (reflection&&constructor&&reflection!=constructor))return state=BootstrapState::Refused;
        return state=BootstrapState::Pending; // no scan/factory stacked in this slice
    }
};
}
