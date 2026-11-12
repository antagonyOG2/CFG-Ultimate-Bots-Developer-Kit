#pragma once
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace cfgport::traps {
enum class MapFamily { Unknown, Ground, Grendel };
struct MapScope { std::uint64_t worldIdentity=0,catalogueGeneration=0;MapFamily family=MapFamily::Unknown; };
inline bool Enabled(const MapScope& scope,std::uint64_t worldIdentity,std::uint64_t catalogueGeneration) noexcept {
    // Grendel AND Grendel Large resolve to the same validated family. Never
    // guess from a display label/substring or use a stale previous-map choice.
    return worldIdentity&&catalogueGeneration&&scope.worldIdentity==worldIdentity&&
        scope.catalogueGeneration==catalogueGeneration&&scope.family==MapFamily::Ground;
}
enum class Target { PhoneBoxFront, CarDoorSide, CarHoodFront };
struct Position { float x, y, z; };
struct Anchor {
    Target target;
    std::uint64_t objectiveIdentity;
    std::uint64_t interactionIdentity;
    Position position;
    // Future CFG adapter must validate the actual interaction anchor, ground
    // projection, collision clearance and native trap eligibility, once on setup.
    bool nativePlacementVerified;
};
inline bool Valid(const Anchor& anchor) noexcept {
    return anchor.objectiveIdentity && anchor.interactionIdentity &&
        anchor.nativePlacementVerified && std::isfinite(anchor.position.x) &&
        std::isfinite(anchor.position.y) && std::isfinite(anchor.position.z) &&
        static_cast<unsigned>(anchor.target) <= static_cast<unsigned>(Target::CarHoodFront);
}
// Caller-owned output. One event-driven plan; no tick/scanning or hard-coded map
// coordinates. Every native car-door interaction gets its own verified anchor.
inline bool BuildExactPlan(const Anchor* anchors, std::size_t count,
    unsigned availableTraps, Anchor* output, std::size_t capacity,
    std::size_t& outputCount) noexcept {
    outputCount = 0;
    if (!anchors || !output || !count || count > capacity || count > availableTraps)
        return false; // Do not silently grant unlimited traps or a partial layout.
    for (std::size_t i = 0; i < count; ++i) {
        if (!Valid(anchors[i])) return false;
        for (std::size_t j = 0; j < i; ++j)
            if (anchors[i].objectiveIdentity == anchors[j].objectiveIdentity &&
                anchors[i].interactionIdentity == anchors[j].interactionIdentity)
                return false;
    }
    for (std::size_t i = 0; i < count; ++i) output[i] = anchors[i];
    outputCount = count;
    return true;
}
inline bool BuildScopedPlan(const MapScope& scope,std::uint64_t worldIdentity,std::uint64_t catalogueGeneration,
    const Anchor* anchors,std::size_t count,unsigned availableTraps,Anchor* output,std::size_t capacity,std::size_t& outputCount) noexcept {
    outputCount=0;
    return Enabled(scope,worldIdentity,catalogueGeneration)&&BuildExactPlan(anchors,count,availableTraps,output,capacity,outputCount);
}
} // namespace cfgport::traps
