#pragma once
#include <cmath>
namespace cfgj22 {
// Lower wins. Same order as the F1 donor lane plus NearbyHuman: the local human only inside 10 m,
// otherwise any other counselor within 80 m horizontally, otherwise the nearest of everyone.
inline float F1TargetScore(bool localHuman,float squared,float horizontalSquared){
    if(localHuman)return squared<=1000.f*1000.f?0.f:squared+1.0e9f;
    return horizontalSquared>8000.f*8000.f?squared+1.0e9f:squared;
}
inline bool NearbyHuman(bool enabled,bool additional,bool localHuman,bool live,bool blocked,bool measured,float distance){
    return enabled&&additional&&localHuman&&live&&!blocked&&measured&&std::isfinite(distance)&&distance>=0&&distance<=1000.f;
}
}
