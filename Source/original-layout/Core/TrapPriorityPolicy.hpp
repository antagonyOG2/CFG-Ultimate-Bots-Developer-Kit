#pragma once
#include <cstdint>
namespace cfgtrappriority {
struct Policy {
    std::uintptr_t victim=0;unsigned index=0;
    std::uint64_t until=0;bool pending=false,morphed=false;
    bool ServiceDue(bool openingUnsuspendable,bool occupiedCar=false)const{return pending&&!openingUnsuspendable&&!occupiedCar;}
    bool Active(std::uint64_t now)const{return victim&&now<until;}
    bool Queue(std::uintptr_t pawn,unsigned rosterIndex,std::uint64_t now){
        if(!pawn||now>UINT64_MAX-60000)return false;
        victim=pawn;index=rosterIndex;until=now+60000;pending=true;morphed=false;return true;
    }
    void Clear(){victim=0;index=0;until=0;pending=false;morphed=false;}
};
}
