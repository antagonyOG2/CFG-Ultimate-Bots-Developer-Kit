#pragma once
#include "VehicleRoadInterceptPolicy.hpp"
namespace cfgroad {
// Donor BeginRoadMorphTrace/SampleRoadMorphTrace, using already observed car
// positions only. Diagnostic evidence never authorizes a placement or action.
struct PathTrace {
    std::uintptr_t car=0;int serial=0;Vector landing{},previous{};
    std::uint64_t started=0;unsigned state=0,segments=0;
    float nearest=0,lateral=0;
    void Begin(std::uintptr_t object,int incarnation,Vector position,Vector actual,std::uint64_t now){
        *this={};if(!object||incarnation<=0||!Finite(position)||!Finite(actual))return;
        car=object;serial=incarnation;previous=position;landing=actual;started=now;state=1;
    }
    void Sample(std::uintptr_t object,int incarnation,Vector position,std::uint64_t now){
        if(state!=1)return;
        if(now<started||now-started>30000){state=3;return;}
        if(object!=car)return;
        if(incarnation!=serial){state=4;return;}
        if(!Finite(position)){state=4;return;}
        const auto delta=Sub(position,previous),offset=Sub(landing,previous);previous=position;
        const float squared=Dot(delta,delta);
        if(!std::isfinite(squared)||squared<50.f*50.f||squared>3000.f*3000.f)return;
        const float projection=Dot(offset,delta)/squared,fraction=(std::clamp)(projection,0.f,1.f);
        const Vector difference{offset.x-fraction*delta.x,offset.y-fraction*delta.y,0};
        const float distance=Length(difference);if(!std::isfinite(distance))return;
        if(!segments||distance<nearest){nearest=distance;lateral=(delta.x*difference.y-delta.y*difference.x)/std::sqrt(squared);}
        ++segments;if(projection>=0&&projection<=1&&distance<=1200.f)state=2;
    }
};
}
