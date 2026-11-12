#pragma once
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <type_traits>
namespace cfgspawn {
// Exact CFG SpawnAIFromClass descriptor layout, not the base-game SDK.
struct Vec3 { float x=0,y=0,z=0; };
struct SpawnAIParameters {
    std::uintptr_t world=0,pawnClass=0,behaviorTree=0;
    Vec3 location{},rotation{};
    std::uint8_t noCollisionFail=0;
    std::uint8_t padding[7]{};
    std::uintptr_t result=0;
};
static_assert(std::is_trivially_copyable<SpawnAIParameters>::value && sizeof(SpawnAIParameters)==64,"CFG native spawn ABI");
static_assert(offsetof(SpawnAIParameters,world)==0 && offsetof(SpawnAIParameters,pawnClass)==8 &&
    offsetof(SpawnAIParameters,behaviorTree)==16 && offsetof(SpawnAIParameters,location)==24 &&
    offsetof(SpawnAIParameters,rotation)==36 && offsetof(SpawnAIParameters,noCollisionFail)==48 &&
    offsetof(SpawnAIParameters,result)==56,"CFG descriptor offsets");
inline bool Finite(Vec3 value) noexcept {
    return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}
// This builds a parameter packet only. It never permits a call or forces a
// collision-ignoring spawn. Native bindings, navigation, object lifetimes and
// current owner-thread context must be established separately by the provider.
inline bool BuildParameters(std::uintptr_t world,std::uintptr_t pawnClass,
    std::uintptr_t behaviorTree,Vec3 location,Vec3 rotation,SpawnAIParameters& out) noexcept {
    out={};
    const auto pointer=[](std::uintptr_t p) { return p>=0x0/*CFG3.5 address withheld*/ && p<=0x00007FFFFFFFFFFF && !(p&7); };
    if (!pointer(world) || !pointer(pawnClass) || !pointer(behaviorTree) ||
        !Finite(location) || !Finite(rotation) || std::fabs(location.x)>10000000 ||
        std::fabs(location.y)>10000000 || std::fabs(location.z)>10000000) return false;
    out.world=world; out.pawnClass=pawnClass; out.behaviorTree=behaviorTree;
    out.location=location; out.rotation=rotation;
    return true;
}
} // namespace cfgspawn
