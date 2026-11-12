#pragma once
#include "NativeSpawnParameters.hpp"
#include <cmath>
namespace cfgdrivernav {
struct Cache {
    std::uintptr_t car=0,seat=0;int carSerial=0,seatSerial=0;cfgspawn::Vec3 origin{},door{},corner{},direction{};
    bool haveDoor=false,haveCorner=false;unsigned radius=0;
    void Clear(){*this={};}
    static float Squared(cfgspawn::Vec3 a,cfgspawn::Vec3 b){const float x=a.x-b.x,y=a.y-b.y;return x*x+y*y;}
    void Select(std::uintptr_t c,std::uintptr_t s,cfgspawn::Vec3 at,int cs,int ss){
        if(!c||!s||cs<=0||ss<=0||!cfgspawn::Finite(at)){Clear();return;}
        if(car!=c||seat!=s||carSerial!=cs||seatSerial!=ss||Squared(origin,at)>75.f*75.f||std::fabs(origin.z-at.z)>75.f){
            Clear();car=c;seat=s;origin=at;carSerial=cs;seatSerial=ss;}
    }
    static bool SameLevel(cfgspawn::Vec3 query,cfgspawn::Vec3 projected,cfgspawn::Vec3 car,float extent){
        return cfgspawn::Finite(query)&&cfgspawn::Finite(projected)&&cfgspawn::Finite(car)&&
            std::fabs(projected.x-query.x)<=extent&&std::fabs(projected.y-query.y)<=extent&&std::fabs(projected.z-car.z)<=180.f;
    }
    bool Door(cfgspawn::Vec3 query,cfgspawn::Vec3 projected){
        const float qx=query.x-origin.x,qy=query.y-origin.y,px=projected.x-origin.x,py=projected.y-origin.y;
        if(!SameLevel(query,projected,origin,120.f)||qx*px+qy*py<=0||px*px+py*py<75.f*75.f)return false;
        door=projected;haveDoor=true;radius=0;return true;
    }
    bool Corner(cfgspawn::Vec3 query,cfgspawn::Vec3 projected){
        if(!SameLevel(query,projected,origin,160.f))return false;
        corner=projected;haveCorner=true;return true;
    }
};
}
