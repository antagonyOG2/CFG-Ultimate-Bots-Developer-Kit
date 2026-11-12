#pragma once
#include "../RuntimeProbe/ObjectLifetime.hpp"
namespace cfgh241mask {
struct TriggerMemo {
    std::uintptr_t world=0,pawn=0,inventory=0,item=0,itemClass=0,listData=0;
    std::uint32_t inventoryOffset=0,itemIndex=0;
    std::int32_t itemCount=0,itemCapacity=0;
    cfgprobe::lifetime::SlotObservation inventorySlot{},itemSlot{};
    void Clear() noexcept {*this={};}
};
inline TriggerMemo memo{};
}
