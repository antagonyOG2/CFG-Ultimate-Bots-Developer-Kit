#pragma once
#include "../RuntimeProbe/ObjectLifetime.hpp"
namespace cfgh241sweater {
struct ClassProofMemo {
    std::uintptr_t world=0,module=0,classObject=0,cdo=0;
    cfgprobe::lifetime::SlotObservation classSlot{},cdoSlot{};
    void Clear() noexcept {*this={};}
};
inline bool Matches(const ClassProofMemo& memo,std::uintptr_t world,std::uintptr_t module,
    std::uintptr_t classObject,std::uintptr_t cdo,const cfgprobe::lifetime::SlotObservation& classSlot,
    const cfgprobe::lifetime::SlotObservation& cdoSlot) noexcept {
    return world&&module&&memo.world==world&&memo.module==module&&memo.classObject==classObject&&memo.cdo==cdo&&
        memo.classSlot==classSlot&&memo.cdoSlot==cdoSlot;
}
inline ClassProofMemo memo{};
}
