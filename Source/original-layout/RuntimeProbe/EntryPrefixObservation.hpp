#pragma once
#include <array>
#include <cstdint>

namespace cfgprobe {
// Observational decoding only. No address returned here may be used as a load
// entry point. Recognize one initial jump; never chase its destination's code.
enum class PrefixKind { Unrecognized, RelativeJump, IndirectJump };
struct PrefixObservation {
    PrefixKind kind=PrefixKind::Unrecognized;
    std::uintptr_t destination=0;
    std::uintptr_t pointerSlot=0;
};
constexpr std::uintptr_t MaximumUserAddress=0x00007FFFFFFFFFFF;
inline bool ByteAddress(std::uintptr_t address) noexcept {
    return address>=0x0/*CFG3.5 address withheld*/ && address<=MaximumUserAddress;
}
inline bool ObservedAdd(std::uintptr_t base, std::int64_t delta,
    std::uintptr_t& output) noexcept {
    output=0;
    if (!ByteAddress(base)) return false;
    if (delta>=0) {
        const auto distance=static_cast<std::uint64_t>(delta);
        if (distance>MaximumUserAddress-base) return false;
        output=base+static_cast<std::uintptr_t>(distance);
    } else {
        // Also defined for INT64_MIN, although decoder supplies signed int32.
        const auto distance=static_cast<std::uint64_t>(-(delta+1))+1;
        if (distance>base-0x10000) return false;
        output=base-static_cast<std::uintptr_t>(distance);
    }
    return true;
}
inline std::int64_t ObservedDisp32(const unsigned char* bytes) noexcept {
    const auto value=static_cast<std::uint32_t>(bytes[0]) |
        (static_cast<std::uint32_t>(bytes[1])<<8) |
        (static_cast<std::uint32_t>(bytes[2])<<16) |
        (static_cast<std::uint32_t>(bytes[3])<<24);
    return value<=0x7FFFFFFFu ? static_cast<std::int64_t>(value) :
        static_cast<std::int64_t>(value)-0x100000000ll;
}
inline PrefixObservation ObservePrefix(std::uintptr_t address,
    const std::array<unsigned char,16>& bytes) noexcept {
    PrefixObservation output;
    std::uintptr_t next=0, referenced=0;
    if (bytes[0]==0xE9) {
        if (!ObservedAdd(address,5,next) || !ObservedAdd(next,ObservedDisp32(bytes.data()+1),referenced)) return output;
        output.kind=PrefixKind::RelativeJump; output.destination=referenced;
    } else if (bytes[0]==0xFF && bytes[1]==0x25) {
        if (!ObservedAdd(address,6,next) || !ObservedAdd(next,ObservedDisp32(bytes.data()+2),referenced) ||
            referenced>MaximumUserAddress-sizeof(std::uintptr_t)+1) return output;
        output.kind=PrefixKind::IndirectJump; output.pointerSlot=referenced;
    }
    return output;
}
} // namespace cfgprobe
