#include "../Core/KnifeAimGeometry.hpp"
#include <cstdio>
#include <cmath>
#include <initializer_list>
int main(){unsigned failures=0;
    // Independent ray/plane intersection: a direction from the hand must hit
    // the target torso, including equal ground, slopes and close throws.
    for(float source:{0.f,100.f,500.f})for(float target:{-200.f,100.f,900.f})for(float range:{100.f,1000.f,3000.f}){
        float pitch=0;
        if(!cfgknife::AimPitch(source,target,range,pitch)||
           std::fabs(source+75.f+range*std::tan(pitch/57.2957795f)-(target+60.f))>.02f)++failures;
    }
    float pitch=0;if(cfgknife::AimPitch(NAN,0,100,pitch)||cfgknife::AimPitch(0,0,0,pitch))++failures;
    if(!cfgknife::AimPitch(100,100,1000,pitch)||pitch>=0)++failures; // hand is ABOVE torso
    for(float x:{-18000.f,0.f,24000.f})for(float y:{-8000.f,6000.f})for(float z:{-300.f,300.f}){
        cfgknife::LaunchGeometry launch{};
        const cfgknife::LaunchVector source{x,y,z},target{x+1200.f,y-500.f,z+70.f};
        if(!cfgknife::WorldLaunch(source,target,12000.f,launch)){++failures;continue;}
        const float dx=target.x-launch.origin.x,dy=target.y-launch.origin.y,dz=target.z+60.f-launch.origin.z;
        const float time=std::sqrt(dx*dx+dy*dy+dz*dz)/12000.f;
        if(std::fabs(launch.origin.x+launch.velocity.x*time-target.x)>.01f||
           std::fabs(launch.origin.y+launch.velocity.y*time-target.y)>.01f||
           std::fabs(launch.origin.z+launch.velocity.z*time-target.z-60.f)>.01f)++failures;
    }
    cfgknife::LaunchGeometry launch{};
    if(cfgknife::WorldLaunch({0,0,0},{0,0,0},12000,launch)||
       cfgknife::WorldLaunch({NAN,0,0},{1000,0,0},12000,launch)||
       cfgknife::WorldLaunch({0,0,0},{1000,0,0},INFINITY,launch)||
       cfgknife::WorldLaunch({0,0,0},{1000,0,0},99,launch))++failures;
    std::printf("Knife hand-to-torso geometry: %u failures; no native/game access.\n",failures);return failures?1:0;
}
