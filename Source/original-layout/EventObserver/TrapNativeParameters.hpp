#pragma once
#include "../Core/NativeSpawnParameters.hpp"
#include <cstddef>
#include <cstdint>
#include <cmath>
namespace cfgtrap {
using Vector=cfgspawn::Vec3;
struct CanPlaceParameters { Vector impact{},rotation{};std::uint8_t result=0,padding[3]{}; };
struct RotationParameters { Vector rotation{};std::uint8_t teleport=0,result=0,padding[2]{}; };
struct MoveLocationParameters {
    Vector destination{};float acceptance=0;
    std::uint8_t stopOnOverlap=0,pathfinding=1,projectNavigation=1,strafe=0,padding[4]{};
    std::uintptr_t filter=0;
    std::uint8_t allowPartial=0,result=0,padding2[6]{};
};
static_assert(sizeof(CanPlaceParameters)==28 && offsetof(CanPlaceParameters,result)==24);
static_assert(sizeof(RotationParameters)==16 && offsetof(RotationParameters,teleport)==12 && offsetof(RotationParameters,result)==13);
static_assert(sizeof(MoveLocationParameters)==40 && offsetof(MoveLocationParameters,filter)==24 &&
    offsetof(MoveLocationParameters,allowPartial)==32 && offsetof(MoveLocationParameters,result)==33);
// Frame-zero AttemptPlaceTrap and Controller.StopMovement take nullptr params.
// Typed map family must come from independently validated active world/package
// evidence, never a display-name substring or an assumption from map coordinates.
enum class MapFamily { Unknown,GroundMap,Grendel,GrendelLarge };
inline bool MapAllowed(MapFamily family,bool activeIdentityVerified) {
    return activeIdentityVerified && family==MapFamily::GroundMap;
}
inline bool Point(Vector v) {
    return cfgspawn::Finite(v)&&std::fabs(v.x)<=10000000&&std::fabs(v.y)<=10000000&&std::fabs(v.z)<=10000000;
}
inline bool MoveToAnchor(Vector anchor,float acceptance,MapFamily family,bool verified,MoveLocationParameters& out){
    out={};if(!MapAllowed(family,verified)||!Point(anchor)||!std::isfinite(acceptance)||acceptance<0||acceptance>1000)return false;
    out.destination=anchor;out.acceptance=acceptance;return true;
}
inline bool FaceAnchor(Vector from,Vector anchor,MapFamily family,bool verified,RotationParameters& out){
    out={};if(!MapAllowed(family,verified)||!Point(from)||!Point(anchor))return false;
    const double x=static_cast<double>(anchor.x)-from.x,y=static_cast<double>(anchor.y)-from.y;
    if(x*x+y*y<1.0)return false;
    out.rotation.y=static_cast<float>(std::atan2(y,x)*57.29577951308232);return true;
}
inline bool PlacementObserved(const CanPlaceParameters& p){
    return p.result==1&&Point(p.impact)&&cfgspawn::Finite(p.rotation);
}
// No packet proves an authored placement, valid nav path, facing completion,
// trap count, successful birth, or current Jason identity. Revalidate those at
// each native owner-thread phase; CanPlaceTrap outputs are observations, not a
// location input to the zero-frame AttemptPlaceTrap function.
}
