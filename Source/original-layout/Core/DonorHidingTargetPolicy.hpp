#pragma once
#include <cstdint>
namespace cfgh241hiding {
constexpr std::uintptr_t HidingTarget(bool hosted,std::uintptr_t donorTarget,std::uint64_t donorAt,
    std::uint64_t now,bool donorCurrent,std::uintptr_t scheduledTarget) noexcept {
    return hosted&&donorTarget&&donorAt&&now>=donorAt&&now-donorAt<=500&&donorCurrent?
        donorTarget:scheduledTarget;
}
}
