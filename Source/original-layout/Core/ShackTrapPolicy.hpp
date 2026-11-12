#pragma once
#include "NativeSpawnParameters.hpp"
#include <cmath>
namespace cfgshacktrap {
using Vector=cfgspawn::Vec3;
inline bool Anchor(Vector cabin,Vector component,Vector& door,Vector& exterior){
    if(!cfgspawn::Finite(cabin)||!cfgspawn::Finite(component))return false;
    float x=component.x-cabin.x,y=component.y-cabin.y,n=std::sqrt(x*x+y*y);
    if(!std::isfinite(n)||n<25.f||n>5000.f)return false;x/=n;y/=n;
    // Working base-mod calibrated threshold in the authored door-component basis.
    door={component.x+x*(-240.812576f)-y*(-776.665588f),component.y+y*(-240.812576f)+x*(-776.665588f),component.z+0.878663f};
    x=door.x-cabin.x;y=door.y-cabin.y;n=std::sqrt(x*x+y*y);
    if(!std::isfinite(n)||n<25.f||n>5000.f)return false;
    exterior={x/n,y/n,0};return cfgspawn::Finite(door);
}
}
