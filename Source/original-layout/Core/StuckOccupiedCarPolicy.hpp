#pragma once
#include <array>
#include "VehicleActionPolicy.hpp"
namespace cfgstuckcar {
// Measured world progress, never engine-on/velocity alone. Three seconds in
// an80cm envelope is a stalled occupied car, not a moving-car extraction.
struct Clock {
    std::uintptr_t car=0,driver=0;int carSerial=0,driverSerial=0;
    cfgspawn::Vec3 anchor{};std::uint64_t since=0,last=0;unsigned samples=0;
    bool Same(std::uintptr_t c,int cs,std::uintptr_t d,int ds)const{return car==c&&driver==d&&carSerial==cs&&driverSerial==ds;}
    bool Ready(std::uintptr_t c,int cs,std::uintptr_t d,int ds,cfgspawn::Vec3 p,std::uint64_t now)const{
        const float x=p.x-anchor.x,y=p.y-anchor.y,z=p.z-anchor.z;
        return samples>=3&&cs>0&&ds>0&&Same(c,cs,d,ds)&&cfgspawn::Finite(p)&&now>=last&&now-last<=3500&&
            now>=since&&now-since>=3000&&x*x+y*y<=80.f*80.f&&std::fabs(z)<=80.f;
    }
    bool Observe(std::uintptr_t c,int cs,std::uintptr_t d,int ds,cfgspawn::Vec3 p,std::uint64_t now){
        if(!c||!d||cs<=0||ds<=0||!cfgspawn::Finite(p)){*this={};return false;}
        const float x=p.x-anchor.x,y=p.y-anchor.y,z=p.z-anchor.z;
        if(!samples||!Same(c,cs,d,ds)||now<last||now-last>3500||x*x+y*y>80.f*80.f||std::fabs(z)>80.f){
            *this={c,d,cs,ds,p,now,now,1};return false;}
        if(now>last&&samples<100000)++samples;
        last=now;return Ready(c,cs,d,ds,p,now);
    }
};
struct Runtime {
    std::array<Clock,2> cars{};
    Clock* Find(std::uintptr_t car){for(auto& c:cars)if(c.car==car)return &c;for(auto& c:cars)if(!c.car)return &c;return nullptr;}
};
}
