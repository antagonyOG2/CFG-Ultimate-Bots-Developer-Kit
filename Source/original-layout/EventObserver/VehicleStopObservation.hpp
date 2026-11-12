#pragma once
#include "BundleObservations.hpp"
#include "../RuntimeProbe/AiActionBindings.generated.hpp"
namespace cfgvehiclestop {
// Read-only leaf of the existing guarded occupied-car path. Caller retains
// world/car/hood ownership and reciprocal occupancy before effects.
template<class R>bool Observe(R& r,std::uintptr_t module,std::uintptr_t car,std::uintptr_t hood,
    bool& started,bool& enabled,bool& slammed,float& doorSpeed,float& slamSpeed){
    using namespace cfgaction::disk;
    std::uint8_t start=2,on=2,slam=2;
    const auto sameByte=[&](std::uintptr_t at,std::uint8_t value){std::uint8_t fresh=2;return r.Read(at,fresh)&&fresh==value;};
    if(!cfgbundle::Code(r,module,IsBeingSlammedExecutor,IsBeingSlammedExecutorCode)||
       !r.Read(car+CarStarted,start)||start>1||!r.Read(car+CarSlammed,slam)||slam>1||
       !r.Read(car+CarDoorSpeed,doorSpeed)||!std::isfinite(doorSpeed)||doorSpeed<0||doorSpeed>10000||
       !r.Read(car+CarSlamSpeed,slamSpeed)||!std::isfinite(slamSpeed)||slamSpeed<0||slamSpeed>10000||
       (hood&&(!r.Read(hood+HoodEnabled,on)||on>1))||
       !sameByte(car+CarStarted,start)||!sameByte(car+CarSlammed,slam)||
       (hood&&!sameByte(hood+HoodEnabled,on)))return false;
    started=start!=0;slammed=slam!=0;enabled=hood&&on;return true;
}
}
