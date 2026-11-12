#pragma once
#include <array>
#include <cmath>
#include <cstdint>
namespace cfgroofguard {
// Absent means the native projection returned normally with result=0; Unsafe
// means entry/return, owner, registry, position or nav identity is ambiguous.
enum class Observation:unsigned {Absent,Present,Unsafe};
enum class Layer:unsigned {Ground,Elevated,Unsafe};
template<class Vec>struct Diagnostics {
    struct DestinationRecord {unsigned kind=0;std::uint64_t at=0;Vec point{},counselor{};};
    std::uint64_t RoofGuardChecks=0,RoofGuardRejects=0,destinations=0;
    Vec RoofGuardLastRejected{},lastDestination{},lastCounselor{};unsigned lastKind=0;
    std::array<DestinationRecord,16> recent{}; // bounded per-world destination log
    void Destination(unsigned kind,Vec point,Vec counselor,std::uint64_t now=0){
        recent[destinations%recent.size()]={kind,now,point,counselor};++destinations;
        lastKind=kind;lastDestination=point;lastCounselor=counselor;
    }
};
template<class Vec>bool Finite(Vec p){return std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z);}
template<class Vec,class Probe>Layer Inspect(Vec point,Probe& probe){
    if(!Finite(point)||point.z< -100000.f||point.z>100000.f)return Layer::Unsafe;
    Vec query{point.x,point.y,point.z-400.f},below{};
    const auto status=probe(query,below);
    if(status==Observation::Absent)return Layer::Ground;
    if(status!=Observation::Present||!Finite(below))return Layer::Unsafe;
    const float x=point.x-below.x,y=point.y-below.y,z=point.z-below.z;
    if(!std::isfinite(x*x+y*y)||x*x+y*y>150.f*150.f||!std::isfinite(z)||z<0)return Layer::Unsafe;
    return z>=200.f?Layer::Elevated:Layer::Unsafe; // not a proven lower layer
}
template<class Vec,class Probe>bool Safe(Diagnostics<Vec>& report,Vec landing,Vec counselorGround,Probe&& probe){
    ++report.RoofGuardChecks;const auto layer=Inspect(landing,probe);
    const bool good=layer==Layer::Ground||(layer==Layer::Elevated&&Inspect(counselorGround,probe)==Layer::Elevated);
    if(!good){++report.RoofGuardRejects;report.RoofGuardLastRejected=landing;}
    return good;
}
}
