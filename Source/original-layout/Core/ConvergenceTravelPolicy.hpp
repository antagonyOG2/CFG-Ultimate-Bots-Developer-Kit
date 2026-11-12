#pragma once
#include <cstdint>
#include <cmath>
namespace cfgconvergencetravel {
enum class Action { None, Acquire, Move, Resume, Recover, Follow };
struct Policy {
    bool owned=false,travelling=false,recovering=false,submitted=false;
    std::uint64_t progressAt=0,recoverUntil=0,pathAt=0;
    float lastDistance=0;
    float sampleX=0,sampleY=0,goalX=0,goalY=0;
    bool haveSample=false,haveGoal=false;
    std::uint64_t followAt=0;bool followAttempted=false;
    Action Decide(float distance,std::uint64_t now,bool eligible,float x=0,float y=0,float gx=0,float gy=0,bool nearFollow=false){
        if(!std::isfinite(distance)||!std::isfinite(x)||!std::isfinite(y)||!std::isfinite(gx)||!std::isfinite(gy))return owned?Action::Resume:Action::None;
        if(!eligible)return owned?Action::Resume:Action::None;
        if(recovering){if(now<recoverUntil)return Action::None;recovering=false;}
        if(nearFollow&&distance<=600){
            if(owned)return Action::Resume; // native melee brain keeps ownership at close range
            const float tx=gx-goalX,ty=gy-goalY;
            return !followAttempted||(now>=followAt&&now-followAt>=1000&&(!haveGoal||tx*tx+ty*ty>150.f*150.f))?Action::Follow:Action::None;
        }
        if(!travelling){if(distance<=300)return Action::None;return Action::Acquire;}
        if(distance<=220)return Action::Resume;
        if(!submitted)return Action::Move;
        // Donor measures pawn displacement, not decrease in target distance.
        // A valid detour around a cabin must not restore the fleeing brain.
        const float dx=x-sampleX,dy=y-sampleY;
        if(!haveSample||dx*dx+dy*dy>80.f*80.f){sampleX=x;sampleY=y;haveSample=true;progressAt=now;}
        if(now-progressAt>=3500)return Action::Recover;
        const float tx=gx-goalX,ty=gy-goalY;
        return !haveGoal||tx*tx+ty*ty>400.f*400.f||now>=pathAt?Action::Move:Action::None;
    }
    void Acquired(float distance,std::uint64_t now,float x=0,float y=0){owned=travelling=true;recovering=submitted=false;lastDistance=distance;progressAt=now;pathAt=now;sampleX=x;sampleY=y;haveSample=true;haveGoal=false;}
    void Moved(std::uint64_t now,float gx=0,float gy=0){if(!submitted)progressAt=now;submitted=true;pathAt=now+6001;goalX=gx;goalY=gy;haveGoal=true;}
    void FollowAttempt(std::uint64_t now){followAt=now;followAttempted=true;}
    void Followed(std::uint64_t now,float gx,float gy){FollowAttempt(now);goalX=gx;goalY=gy;haveGoal=true;}
    void Resumed(std::uint64_t now,bool recover){owned=travelling=false;recovering=recover;recoverUntil=now+1001;}
};
}
