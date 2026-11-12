#pragma once
#include <cmath>
#include <cstdint>
#include "J22FeatureFlags.hpp"
#ifndef CFG_H286_BURST_LEAD
#define CFG_H286_BURST_LEAD 1 // during a speed burst: aim 2 s ahead of the running counselor (capped at the Jason-counselor gap) and re-aim every 500 ms instead of holding a 6 s path lock
#endif
namespace cfgchaselead {
constexpr float LeadSeconds=2.0f,MinimumSpeed=150.0f,OvershootCm=180.0f;constexpr std::uint64_t BurstRefreshMs=500;
// goal + heading*(min(speed*LeadSeconds, horizontal gap Jason->goal) + 6 ft). False (unchanged) below walking pace or on bad input.
inline bool Lead(float gx,float gy,float jx,float jy,float vx,float vy,float& outX,float& outY){
    const float speed=std::sqrt(vx*vx+vy*vy),gap=std::sqrt((gx-jx)*(gx-jx)+(gy-jy)*(gy-jy));
    if(!std::isfinite(speed)||!std::isfinite(gap)||speed<MinimumSpeed)return false;
    // H2.95 (user): cap relaxed to 1.5x the gap so a close chase still aims past the runner; then the 6 ft overshoot (H2.89).
    float lead=speed*LeadSeconds;const float cap=CFG_H295_LEAD_CAP_RELAX?gap*1.5f:gap;if(lead>cap)lead=cap;lead+=OvershootCm;
    outX=gx+vx/speed*lead;outY=gy+vy/speed*lead;return std::isfinite(outX)&&std::isfinite(outY);
}
}
namespace cfgdonorhunt {
// Optional host seams (fixture hosts without them keep donor behaviour).
template<class Host,class P>static auto DonorChaseLead(Host& host,std::uintptr_t goal,P& point,int)->decltype(host.ChaseLead(goal,point),void()){
    if(CFG_H286_BURST_LEAD)host.ChaseLead(goal,point);
}
template<class Host,class P>static void DonorChaseLead(Host&,std::uintptr_t,P&,long){}
template<class Host>static auto DonorChaseBursting(Host& host,int)->decltype(host.ChaseBursting(),bool()){return CFG_H286_BURST_LEAD&&host.ChaseBursting();}
template<class Host>static bool DonorChaseBursting(Host&,long){return false;}
}
