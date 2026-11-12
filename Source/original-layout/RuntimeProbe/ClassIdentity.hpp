#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include "Compatibility/CfgBuildProfile.hpp"

namespace cfgprobe {
// CFG native GetObjectClass / ClassIsChildOf implementations, independently
// checked in Diagnostics/CFG-IdentityBindings.json. No stock SDK layout reuse.
constexpr std::uintptr_t ObjectClass = cfgcompat::cfg35::objects::ObjectClass;
constexpr std::uintptr_t ClassSuper = cfgcompat::cfg35::objects::ClassSuper;
constexpr std::size_t MaximumAncestry = 32;
inline bool ObjectPointer(std::uintptr_t value) noexcept {
    return value >= 0x0/*CFG3.5 address withheld*/ && value <= 0x00007FFFFFFFFFFF && value % 8 == 0;
}
struct Ancestry {
    std::array<std::uintptr_t,MaximumAncestry> classes{};
    std::array<std::uintptr_t,MaximumAncestry> parents{};
    std::size_t count = 0;
    bool Contains(std::uintptr_t nativeClass) const noexcept {
        if (!ObjectPointer(nativeClass)) return false;
        for (std::size_t i=0; i<count; ++i)
            if (classes[i] == nativeClass) return true;
        return false;
    }
};
template<class Reader> bool CaptureAncestry(Reader& reader,
    std::uintptr_t object, Ancestry& output) {
    output = {};
    Ancestry candidate;
    std::uintptr_t current = 0;
    if (!ObjectPointer(object) || !reader.Read(object+ObjectClass,current)) return false;
    while (current) {
        if (!ObjectPointer(current) || candidate.count == MaximumAncestry) return false;
        for (std::size_t i=0; i<candidate.count; ++i)
            if (candidate.classes[i] == current) return false;
        std::uintptr_t parent = 0;
        if (!reader.Read(current+ClassSuper,parent)) return false;
        candidate.classes[candidate.count] = current;
        candidate.parents[candidate.count++] = parent;
        current = parent;
    }
    if (!candidate.count) return false;
    output = candidate;
    return true;
}
template<class Reader> bool StableAncestry(Reader& reader,
    std::uintptr_t object, const Ancestry& ancestry) {
    if (!ObjectPointer(object) || !ancestry.count || ancestry.count > MaximumAncestry) return false;
    std::uintptr_t value = 0;
    if (!reader.Read(object+ObjectClass,value) || value != ancestry.classes[0]) return false;
    for (std::size_t i=0; i<ancestry.count; ++i) {
        if (!reader.Read(ancestry.classes[i]+ClassSuper,value) || value != ancestry.parents[i]) return false;
    }
    return true;
}
using NativeClassBinding=cfgcompat::cfg35::objects::NativeClassBinding;
enum NativeClass : std::size_t {
    Character, Counselor, Killer, PlayerControllerClass, OfflineBotsMode,
    SandboxMode, ChallengesMode, NativeClassCount
};
// This enum is a public observation schema. The central catalogue also has
// GameMode/GameState entries, so its ordinal order is not this schema's order.
constexpr std::array<cfgcompat::cfg35::objects::NativeClassBinding,NativeClassCount> NativeClasses{{
    cfgcompat::cfg35::objects::NativeClasses[0],
    cfgcompat::cfg35::objects::NativeClasses[1],
    cfgcompat::cfg35::objects::NativeClasses[2],
    cfgcompat::cfg35::objects::NativeClasses[3],
    cfgcompat::cfg35::objects::NativeClasses[6],
    cfgcompat::cfg35::objects::NativeClasses[7],
    cfgcompat::cfg35::objects::NativeClasses[8]
}};
inline std::uintptr_t MatchingClassCache(std::uintptr_t reflection,
    std::uintptr_t constructor) noexcept {
    return ObjectPointer(reflection) && reflection == constructor ? reflection : 0;
}
} // namespace cfgprobe
