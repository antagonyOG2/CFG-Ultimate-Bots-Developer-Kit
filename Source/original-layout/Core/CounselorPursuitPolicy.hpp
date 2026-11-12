#pragma once
#include "VehiclePursuitPolicy.hpp"
#include "J24FeatureFlags.hpp"
#include "RoofLandingGuard.hpp"
namespace cfgpursuit {
// Ported from the base-game mod's exterior nav ring. User-requested counselor
// pursuit now20m/7s; car5s and shared trap20s timing remain independent.
// One projection OR teleport per service phase, never sixteen calls in a burst.
constexpr float MorphDistance=2000.f;
constexpr std::uint64_t MorphInterval=7000;
static_assert(MorphInterval<=cfgvehicle::MorphInterval);
inline bool MorphDue(const cfgvehicle::Cooldown& shared,std::uint64_t now,float squared,bool urgent=false,bool stalled=false,bool structureStalled=false){
    // All native Morph attempts publish the existing20s shared deadline.
    // Derive the counselor-only7s deadline without changing the trap clock
    // or the occupied-car intercept's separate5s episode timer.
    constexpr auto reduction=cfgvehicle::MorphInterval-MorphInterval;
    const auto ready=shared.next>=reduction?shared.next-reduction:0;
    const float distance=(urgent||structureStalled)?220.f:stalled?1000.f:MorphDistance;
    // Fresh caught victim owns one urgent response, matching donor's scoped
    // trap exemption. Ordinary hunting still requires the7second deadline.
    return (urgent||now>=ready)&&std::isfinite(squared)&&squared>distance*distance;
}
// Donor no-movement recovery, sampled ONLY in the already guarded ordinary
// chase lane. Four2s deferrals prove eight seconds of actual stagnation.
// Interrupted combat/door/trap episodes must not count as failed walking.
struct Progress {
    std::uintptr_t target=0;int serial=0;cfgspawn::Vec3 origin{};
    std::uint64_t sampledAt=0;unsigned deferrals=0;bool observed=false;
    void Clear(){*this={};}
    bool Due(std::uint64_t now)const{return !observed||now<sampledAt||now-sampledAt>=2000;}
    bool Stalled()const{return observed&&deferrals>=4;}
    void Observe(std::uintptr_t object,int lifetime,cfgspawn::Vec3 position,std::uint64_t now){
        if(!object||lifetime<=0||!cfgspawn::Finite(position)){Clear();return;}
        const float dx=position.x-origin.x,dy=position.y-origin.y,dz=position.z-origin.z;
        if(!observed||target!=object||serial!=lifetime||now<sampledAt||now-sampledAt>4000||
           dx*dx+dy*dy+dz*dz>=80.f*80.f){target=object;serial=lifetime;origin=position;deferrals=0;observed=true;}
        else if(now-sampledAt>=2000&&deferrals<4)++deferrals;
        sampledAt=now;
    }
};
constexpr std::array<cfgspawn::Vec3,16> ExteriorOffsets{{
    {2200,0,0},{-2200,0,0},{0,2200,0},{0,-2200,0},
    {1556,1556,0},{1556,-1556,0},{-1556,1556,0},{-1556,-1556,0},
    {3000,0,0},{-3000,0,0},{0,3000,0},{0,-3000,0},
    {2121,2121,0},{2121,-2121,0},{-2121,2121,0},{-2121,-2121,0}}};
struct TeleportPacket {
    cfgspawn::Vec3 location{},rotation{};
    std::uint8_t result=0,padding[3]{};
};
static_assert(sizeof(TeleportPacket)==28&&offsetof(TeleportPacket,result)==24);
enum class Result { None, Pending, Teleported, Unavailable };
struct Ring {
    std::uintptr_t target=0;
    cfgspawn::Vec3 origin{},surface{};
    unsigned candidate=0;
    bool projected=false;
    bool exact=false;
    std::uint64_t retryAt=0;
    cfgroofguard::Diagnostics<cfgspawn::Vec3> roof{}; // per-world, read-only diagnostics
    void Clear(){target=0;candidate=0;projected=exact=false;origin={};surface={};}
    bool Begin(std::uintptr_t pawn,cfgspawn::Vec3 position,std::uint64_t now){
        if(target||now<retryAt||pawn<0x10000||(pawn&7)||!cfgspawn::Finite(position))return false;
        target=pawn;origin=position;candidate=0;projected=exact=false;return true;
    }
    // Same nav/landing transaction for an explicitly selected exit guard point.
    // It must not be displaced by the ordinary counselor ring's 22 m offset.
    bool BeginExact(std::uintptr_t pawn,cfgspawn::Vec3 position,std::uint64_t now){
        if(!Begin(pawn,position,now))return false;exact=true;return true;
    }
    bool Current(std::uintptr_t pawn,cfgspawn::Vec3 position)const{
        if(!target||target!=pawn||!cfgspawn::Finite(position))return false;
        const auto x=position.x-origin.x,y=position.y-origin.y,z=position.z-origin.z;
        return std::isfinite(x*x+y*y+z*z)&&x*x+y*y+z*z<=500.f*500.f;
    }
    cfgspawn::Vec3 Candidate()const{
        if(!target||candidate>=ExteriorOffsets.size())return {};
        if(exact)return {origin.x,origin.y,origin.z-90.f+25.f};
        const auto& offset=ExteriorOffsets[candidate];
        return {origin.x+offset.x,origin.y+offset.y,origin.z-90.f+25.f};
    }
    bool Projected(cfgspawn::Vec3 point){
        if(!target||candidate>=ExteriorOffsets.size()||!cfgspawn::Finite(point))return false;
        const auto query=Candidate();const auto x=point.x-origin.x,y=point.y-origin.y;
        if(std::fabs(point.x-query.x)>350||std::fabs(point.y-query.y)>350||
           std::fabs(point.z-(origin.z-90.f))>120||!std::isfinite(x*x+y*y)||(!exact&&x*x+y*y<1700.f*1700.f))return false;
        surface=point;projected=true;return true;
    }
    void FailedCandidate(std::uint64_t now){
        if(exact){Refused(now);return;}
        projected=false;surface={};
        if(++candidate>=ExteriorOffsets.size()){Clear();retryAt=now+3000;}
    }
    void Refused(std::uint64_t now){Clear();retryAt=now+3000;}
    cfgspawn::Vec3 ActorDestination()const{return {surface.x,surface.y,surface.z+90.f};}
};
}
