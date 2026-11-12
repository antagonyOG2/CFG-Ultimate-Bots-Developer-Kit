#pragma once
#include <cstdint>
namespace cfgdonorturn {
inline bool UseBaseTick(bool driverReady,bool held,bool owedStop,bool water){return !water&&(driverReady||held||owedStop);}
struct WaterCounter {
    std::uint64_t ticks=0;bool active=false;
    bool Observe(bool known,bool original,bool water){
        if(!known)return false; // uncertainty is not a water sample or an exit
        if(original&&water){++ticks;const bool emit=!active||ticks%128==0;active=true;return emit;}
        if(!water&&active){active=false;return true;}return false;
    }
};
// Callback admission/retention is the host's job. Once admitted, B/C must not
// disappear behind hunting's Census/External/lease or original/base choices.
template<class Host>void Service(Host& host){
    const bool releaseBlocked=host.DrainOwedRelease();
    const bool heldOwned=!releaseBlocked&&host.DriveHeld();
    if(!releaseBlocked&&!heldOwned&&!host.HuntExclusive())host.Hunt();
    host.Convergence();host.Emotes();
}
}
