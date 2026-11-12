#pragma once
#include "NativeSpawnParameters.hpp"
namespace cfgspawn::deferred {
// Verified ONLY for this CFG GameplayStatics owner and its Transform descriptor.
// These packets do not authorize native calls, instance writes or AI handoffs.
struct alignas(16) Transform {
    float rotation[4]{0,0,0,1};
    Vec3 translation{}; float translationPadding=0;
    Vec3 scale{1,1,1}; float scalePadding=0;
};
struct alignas(16) BeginParameters {
    std::uintptr_t world=0,actorClass=0;
    Transform transform{};
    std::uint8_t collisionOverride=0; // Undefined: preserve native class policy.
    std::uint8_t collisionPadding[7]{};
    std::uintptr_t owner=0,result=0;
    std::uint8_t tailPadding[8]{};
};
struct alignas(16) FinishParameters {
    std::uintptr_t actor=0;
    std::uint8_t actorPadding[8]{};
    Transform transform{};
    std::uintptr_t result=0;
    std::uint8_t tailPadding[8]{};
};
static_assert(sizeof(Transform)==48 && alignof(Transform)==16 && offsetof(Transform,translation)==16 && offsetof(Transform,scale)==32,"CFG native Transform frame");
static_assert(sizeof(BeginParameters)==96 && offsetof(BeginParameters,world)==0 && offsetof(BeginParameters,actorClass)==8 &&
    offsetof(BeginParameters,transform)==16 && offsetof(BeginParameters,collisionOverride)==64 && offsetof(BeginParameters,owner)==72 &&
    offsetof(BeginParameters,result)==80,"CFG GameplayStatics BeginDeferredActorSpawnFromClass frame");
static_assert(sizeof(FinishParameters)==80 && offsetof(FinishParameters,actor)==0 && offsetof(FinishParameters,transform)==16 &&
    offsetof(FinishParameters,result)==64,"CFG GameplayStatics FinishSpawningActor frame (not the other owner16)");
static_assert(std::is_trivially_copyable<BeginParameters>::value && std::is_trivially_copyable<FinishParameters>::value,"Fixed native parameter packets");
inline bool Pointer(std::uintptr_t value) noexcept { return value>=0x0/*CFG3.5 address withheld*/ && value<=0x00007FFFFFFFFFFF && !(value&7); }
inline bool Location(Vec3 value) noexcept {
    return Finite(value) && std::fabs(value.x)<=10000000 && std::fabs(value.y)<=10000000 && std::fabs(value.z)<=10000000;
}
inline bool BuildBegin(std::uintptr_t world,std::uintptr_t actorClass,Vec3 location,BeginParameters& output) noexcept {
    output={};
    if (!Pointer(world) || !Pointer(actorClass) || !Location(location)) return false;
    output.world=world; output.actorClass=actorClass; output.transform.translation=location; return true;
}
inline bool BuildFinish(std::uintptr_t actor,Vec3 location,FinishParameters& output) noexcept {
    output={};
    if (!Pointer(actor) || !Location(location)) return false;
    output.actor=actor; output.transform.translation=location; return true;
}
} // namespace cfgspawn::deferred
