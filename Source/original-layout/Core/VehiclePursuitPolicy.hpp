#pragma once
#include "NativeSpawnParameters.hpp"
#include <array>
#include <cstdint>
namespace cfgvehicle {
constexpr std::uint64_t MorphInterval=20000;
constexpr float MorphDistance=5000.f; // Same 50-metre threshold as base-game build.
inline bool Landing(cfgspawn::Vec3 lead,cfgspawn::Vec3 ground,cfgspawn::Vec3& out){
    out={};if(!cfgspawn::Finite(lead)||!cfgspawn::Finite(ground))return false;
    const auto dx=ground.x-lead.x,dy=ground.y-lead.y,dz=ground.z-lead.z;
    if(!std::isfinite(dx*dx+dy*dy)||dx*dx+dy*dy>600.f*600.f||std::fabs(dz)>700.f)return false;
    out=ground;out.z+=90.f; // Actor origin is capsule centre; nav projection is ground.
    return cfgspawn::Finite(out);
}
struct Projection {
    std::uintptr_t world=0;cfgspawn::Vec3 point{},projected{};
    std::uintptr_t nav=0,filter=0;cfgspawn::Vec3 extent{300,300,500};
    std::uint8_t result=0,padding[3]{};
};
struct Morph {cfgspawn::Vec3 point{};std::uint8_t result=0,padding[3]{};};
static_assert(sizeof(Projection)==64&&offsetof(Projection,projected)==20&&offsetof(Projection,extent)==48&&offsetof(Projection,result)==60);
static_assert(sizeof(Morph)==16&&offsetof(Morph,result)==12);
inline bool Lead(cfgspawn::Vec3 position,cfgspawn::Vec3 velocity,cfgspawn::Vec3& result){
    result={};if(!cfgspawn::Finite(position)||!cfgspawn::Finite(velocity))return false;
    const auto speedSquared=velocity.x*velocity.x+velocity.y*velocity.y;
    if(!std::isfinite(speedSquared)||speedSquared<100.f*100.f||speedSquared>10000.f*10000.f)return false;
    const auto speed=std::sqrt(speedSquared);
    result={position.x+velocity.x/speed*1000.f,position.y+velocity.y/speed*1000.f,position.z};
    return cfgspawn::Finite(result)&&std::fabs(result.x)<10000000&&std::fabs(result.y)<10000000&&std::fabs(result.z)<10000000;
}
struct Cooldown {
    std::uint64_t next=0;
    bool Due(std::uint64_t now,float squared)const{return now>=next&&std::isfinite(squared)&&squared>MorphDistance*MorphDistance;}
    void Attempted(std::uint64_t now){next=now+MorphInterval;} // Denial does not spam native Morph.
};
// The donor's occupied-vehicle intercept is independent of its general
// counselor/trap Morph gate. The requested five-second car cadence is local.
struct TailIntercept {
    static constexpr std::uint64_t Interval=5000,BehindDelay=5000,SampleLimit=3500;
    std::uintptr_t car=0,driver=0;std::uint64_t next=0,behindSince=0,lastSample=0;
    bool behind=false;
    void ClearEpisode(){car=driver=0;behindSince=lastSample=0;behind=false;}
    void Vacant(std::uintptr_t value){if(car==value)ClearEpisode();}
    void Attempted(std::uint64_t now){next=now+Interval;behindSince=0;behind=false;}
    bool Observe(std::uintptr_t c,std::uintptr_t d,std::uint64_t now,bool moving,
                 cfgspawn::Vec3 position,cfgspawn::Vec3 velocity,cfgspawn::Vec3 jason){
        const bool continuous=car==c&&driver==d&&now>=lastSample&&now-lastSample<=SampleLimit;
        if(!continuous){behind=false;behindSince=0;}
        car=c;driver=d;lastSample=now;
        const float speed2=velocity.x*velocity.x+velocity.y*velocity.y;
        const float dx=jason.x-position.x,dy=jason.y-position.y;
        const bool trailing=c&&d&&moving&&cfgspawn::Finite(position)&&cfgspawn::Finite(velocity)&&
            cfgspawn::Finite(jason)&&std::isfinite(speed2)&&speed2>=100.f*100.f&&speed2<=10000.f*10000.f&&
            (dx*velocity.x+dy*velocity.y)/std::sqrt(speed2)<-200.f;
        if(!trailing){behind=false;behindSince=0;return false;}
        if(!behind){behind=true;behindSince=now;}
        return now>=next&&now>=behindSince&&now-behindSince>=BehindDelay;
    }
};
struct MotionSample {
    std::uintptr_t car=0,driver=0;std::uint64_t at=0;cfgspawn::Vec3 position{};
    bool Observe(std::uintptr_t c,std::uintptr_t d,std::uint64_t now,cfgspawn::Vec3 p,cfgspawn::Vec3& velocity){
        velocity={};bool valid=false;
        if(c&&d&&car==c&&driver==d&&now>at&&now-at>=20&&now-at<=3500&&cfgspawn::Finite(p)&&cfgspawn::Finite(position)){
            const float seconds=static_cast<float>(now-at)/1000.f;
            velocity={(p.x-position.x)/seconds,(p.y-position.y)/seconds,(p.z-position.z)/seconds};
            const auto squared=velocity.x*velocity.x+velocity.y*velocity.y;
            valid=cfgspawn::Finite(velocity)&&std::isfinite(squared)&&squared<=10000.f*10000.f;
        }
        car=c;driver=d;at=now;position=p;return valid;
    }
};
struct Pursuit {
    std::array<MotionSample,2> samples{};
    std::uint64_t until=0;unsigned positionSamples=0,velocityFallbacks=0,stationary=0,held=0,busy=0,moves=0;
    bool Active(std::uint64_t now)const{return now<until;}
    // Covers the bounded multi-slice native interception transaction, not a
    // single velocity sample. Vacancy/terminal/driver extraction clears it.
    // Still finite if observations stop; do not retain a foreign/empty car.
    static constexpr std::uint64_t LeaseMs=7000;
    void Renew(std::uint64_t now){until=now>UINT64_MAX-LeaseMs?UINT64_MAX:now+LeaseMs;}
};
}
