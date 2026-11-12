#pragma once
#include <array>
#include <cmath>
#include <cstdint>
namespace cfgvehicleprogress {
// FrozenJasonBridge::DriveOccupiedVehicleInterception: accepted path/void
// input/lock waits are not measured progress. Each car retains its own clock.
struct Entry {
    std::uintptr_t car=0,driver=0;std::int32_t serial=0,driverSerial=0;
    float best=0;std::uint64_t deadline=0,ignoredUntil=0,last=0;
    bool Same(std::uintptr_t c,std::int32_t cs,std::uintptr_t d,std::int32_t ds)const{
        return car==c&&serial==cs&&driver==d&&driverSerial==ds;
    }
    bool Observe(std::uintptr_t c,std::int32_t cs,std::uintptr_t d,std::int32_t ds,float distance,std::uint64_t now){
        if(!c||!d||cs<=0||ds<=0||!std::isfinite(distance)||distance<0)return false;
        if(!Same(c,cs,d,ds)||!deadline||now<last){*this={c,d,cs,ds,distance,now+12000,0,now};return false;}
        last=now;
        if(distance+100.f<=best){best=distance;deadline=now+12000;}
        else if(distance>best+500.f)best=distance; // rebaseline WITHOUT renewal
        return now>=deadline;
    }
    bool Ignored(std::uintptr_t c,std::int32_t cs,std::uint64_t now)const{return car==c&&serial==cs&&now<ignoredUntil;}
    bool BackOff(std::uint64_t now,bool usableAlternative,bool nativeOwned){
        if(!usableAlternative||nativeOwned||!deadline||now<deadline)return false;
        ignoredUntil=now+15000;deadline=ignoredUntil+12000;return true;
    }
    void SearchExhausted(std::uint64_t now){
        // A sole driver remains a valid pursuit target. Do not repeatedly
        // preempt road/stop actions with an already exhausted search.
        deadline=now+12000;last=now;
    }
};
struct Runtime {std::array<Entry,2> cars{};unsigned pending=0,cursor=0;std::uint64_t next=0;};
}
