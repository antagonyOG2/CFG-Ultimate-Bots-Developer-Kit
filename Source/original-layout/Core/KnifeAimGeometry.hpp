#pragma once
#include <cmath>
#include <initializer_list>
namespace cfgknife {
constexpr float HandHeight=75.f,TorsoHeight=60.f;
struct LaunchVector {float x=0,y=0,z=0;};
struct LaunchGeometry {LaunchVector origin{},velocity{};};
inline bool WorldLaunch(LaunchVector sourceRoot,LaunchVector targetRoot,float speed,LaunchGeometry& output){
    output={};
    for(float v:{sourceRoot.x,sourceRoot.y,sourceRoot.z,targetRoot.x,targetRoot.y,targetRoot.z,speed})
        if(!std::isfinite(v))return false;
    if(speed<100.f||speed>20000.f)return false;
    sourceRoot.z+=HandHeight;targetRoot.z+=TorsoHeight;
    const float x=targetRoot.x-sourceRoot.x,y=targetRoot.y-sourceRoot.y,z=targetRoot.z-sourceRoot.z;
    const float length=std::sqrt(x*x+y*y+z*z);
    if(!std::isfinite(length)||length<=111.f)return false;
    // Donor Features41: ignore the attachment-relative knife and stale AI
    // camera origin. Start outside Jason's capsule along the torso ray.
    output.origin={sourceRoot.x+x*110.f/length,sourceRoot.y+y*110.f/length,sourceRoot.z+z*110.f/length};
    output.velocity={x*speed/length,y*speed/length,z*speed/length};
    return true;
}
inline bool AimPitch(float sourceRootZ,float targetRootZ,float horizontal,float& pitch){
    pitch=0;
    if(!std::isfinite(sourceRootZ)||!std::isfinite(targetRootZ)||!std::isfinite(horizontal)||horizontal<1.f)return false;
    pitch=std::atan2(targetRootZ+TorsoHeight-(sourceRootZ+HandHeight),horizontal)*57.2957795f;
    return std::isfinite(pitch);
}
}
