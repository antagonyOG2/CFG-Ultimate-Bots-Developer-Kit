#pragma once
#include "VehicleActionPolicy.hpp"
namespace cfgfootcar {
enum class Failure : std::uint8_t {
    None=0,ProjectionCall=1,NoGround=2,ProjectionGeometry=3,NavigationOwner=4,
    StopMovement=5,MoveTo=6,Stall=7
};
// Donor two-corner route with40cm additional on-foot clearance. Visit the near
// corner FIRST, then the far-side corner; a diagonal to only the far corner
// can intersect the car. Progress, not MoveTo's return, advances each leg.
struct Route {
    static constexpr float CornerClearance=400.f;
    cfgspawn::Vec3 center{},axis{},chassisAxis{},goal{},sample{},arrival{};
    float side=1,end=1,best=0;unsigned leg=0,retries=0;
    std::uint64_t progressAt=0;bool active=false;
    void Reset(){*this={};}
    static float Squared(cfgspawn::Vec3 a,cfgspawn::Vec3 b){const float x=a.x-b.x,y=a.y-b.y;return x*x+y*y;}
    static bool Crosses(float ax,float ay,float bx,float by){
        float enter=0,leave=1;
        const auto slab=[&](float a,float b,float extent){const float delta=b-a;
            if(std::fabs(delta)<.001f)return std::fabs(a)<=extent;
            float first=(-extent-a)/delta,last=(extent-a)/delta;if(first>last)std::swap(first,last);
            enter=(std::max)(enter,first);leave=(std::min)(leave,last);return enter<=leave;};
        return slab(ax,bx,300.f)&&slab(ay,by,180.f)&&leave>=0&&enter<=1;
    }
    static bool ClearsChassis(cfgspawn::Vec3 a,cfgspawn::Vec3 b){
        if(!Crosses(a.x,a.y,b.x,b.y))return true;
        // The conservative box is not a collision surface. A live capsule
        // may contact different car meshes anywhere inside its padding.
        // Escape through the nearest normalized face, monotonically outward;
        // do not gate it on another guessed physical-mesh inset. Native
        // swept navigation still owns collision. Never cross to the far face.
        if(std::fabs(a.x)>300.01f||std::fabs(a.y)>180.01f)return false;
        const bool endEscape=std::fabs(a.x)/300.f>=std::fabs(a.y)/180.f&&a.x*b.x>0&&
            std::fabs(b.x)>=330.f&&std::fabs(b.x)>=std::fabs(a.x);
        const bool sideEscape=std::fabs(a.y)/180.f>=std::fabs(a.x)/300.f&&a.y*b.y>0&&
            std::fabs(b.y)>=210.f&&std::fabs(b.y)>=std::fabs(a.y);
        return endEscape||sideEscape;
    }
    void Corner(cfgspawn::Vec3 here,std::uint64_t now){
        const float sign=leg==0?side:-side;
        goal={center.x+axis.x*end*CornerClearance-axis.y*sign*CornerClearance,
              center.y+axis.y*end*CornerClearance+axis.x*sign*CornerClearance,center.z};
        sample=here;arrival=goal;best=std::sqrt(Squared(here,arrival));progressAt=now;
    }
    bool Begin(cfgspawn::Vec3 car,cfgspawn::Vec3 forward,cfgspawn::Vec3 here,cfgspawn::Vec3 target,std::uint64_t now){
        Reset();if(!cfgspawn::Finite(car)||!cfgspawn::Finite(forward)||!cfgspawn::Finite(here)||!cfgspawn::Finite(target))return false;
        const float n=std::sqrt(forward.x*forward.x+forward.y*forward.y);if(n<.1f||!std::isfinite(n))return false;
        center=car;axis=chassisAxis={forward.x/n,forward.y/n,0};
        float js=-(here.x-car.x)*axis.y+(here.y-car.y)*axis.x;
        float ts=-(target.x-car.x)*axis.y+(target.y-car.y)*axis.x;
        const float jl=(here.x-car.x)*axis.x+(here.y-car.y)*axis.y;
        const float tl=(target.x-car.x)*axis.x+(target.y-car.y)*axis.y;
        if(!Crosses(jl,js,tl,ts))return false;
        if(js*ts>=0){
            if(jl*tl>=0)return false;
            // Hood-to-trunk obstruction: same perimeter algorithm rotated90
            // degrees, so it walks down one flank rather than through the car.
            axis={-axis.y,axis.x,0};js=-jl;ts=-tl;
        }
        // An actual cross-chassis obstruction, not merely proximity to a car.
        if(js*ts>=0||std::fabs(js)>650.f||std::fabs(ts)>1000.f)return false;
        side=js>=0?1.f:-1.f;end=(here.x-car.x)*axis.x+(here.y-car.y)*axis.y>=0?1.f:-1.f;
        active=true;Corner(here,now);return true;
    }
    bool Current(cfgspawn::Vec3 car,cfgspawn::Vec3 target)const{
        if(!active||!cfgspawn::Finite(car)||!cfgspawn::Finite(target)||Squared(car,center)>100.f*100.f)return false;
        const float ts=-(target.x-center.x)*axis.y+(target.y-center.y)*axis.x;
        return ts*side<0&&std::fabs(target.z-center.z)<280.f;
    }
    // 0 keep path, 1 project new leg, 2 complete, 3 stalled route end.
    unsigned Observe(cfgspawn::Vec3 here,std::uint64_t now){
        if(!active||!cfgspawn::Finite(here))return 3;
        const float d=std::sqrt(Squared(here,arrival));
        if(d<=115.f){if(leg++==1){active=false;return 2;}Corner(here,now);return 1;}
        if(d+25.f<best){best=d;progressAt=now;}
        else if(now<progressAt||now-progressAt>=3000){
            active=false;return 3;
        }
        return 0;
    }
    enum class ProjectionFailure : std::uint8_t { None=0,Inactive=1,NonFinite=2,XRange=3,YRange=4,ZRange=5,Clearance=6,ChassisPath=7 };
    ProjectionFailure ProjectionReason(cfgspawn::Vec3 point)const{
        // Native navigation returns a ground point, not Jason's capsule center.
        // Match the donor driver's180cm same-level envelope around the CAR.
        if(!active)return ProjectionFailure::Inactive;
        if(!cfgspawn::Finite(point))return ProjectionFailure::NonFinite;
        if(std::fabs(point.x-goal.x)>160.f)return ProjectionFailure::XRange;
        if(std::fabs(point.y-goal.y)>160.f)return ProjectionFailure::YRange;
        if(std::fabs(point.z-center.z)>180.f)return ProjectionFailure::ZRange;
        const auto local=[&](cfgspawn::Vec3 p){const float x=p.x-center.x,y=p.y-center.y;
            return cfgspawn::Vec3{x*chassisAxis.x+y*chassisAxis.y,-x*chassisAxis.y+y*chassisAxis.x,0};};
        const auto a=local(sample),b=local(point);
        // Projection may shift within the donor query envelope, but may not
        // put the waypoint/segment through the physical chassis.
        if(std::fabs(b.x)<370.f&&std::fabs(b.y)<250.f)return ProjectionFailure::Clearance;
        if(!ClearsChassis(a,b))return ProjectionFailure::ChassisPath;
        return ProjectionFailure::None;
    }
    bool Projection(cfgspawn::Vec3 point)const{return ProjectionReason(point)==ProjectionFailure::None;}
    bool AcceptProjection(cfgspawn::Vec3 point,std::uint64_t now){
        if(!Projection(point))return false;
        arrival=point;best=std::sqrt(Squared(sample,arrival));progressAt=now;return true;
    }
    bool ReplanEnd(float nextEnd,cfgspawn::Vec3 here,std::uint64_t now){
        if((nextEnd!=1.f&&nextEnd!=-1.f)||!cfgspawn::Finite(here))return false;
        end=nextEnd;leg=0;retries=0;active=true;Corner(here,now);return true;
    }
};
// Bounded per-car/per-target retry memory. It contains no native pointers;
// index/serial keys are refreshed from the current object array on each route
// acquisition. BundleEpochReset clears it at world retirement.
struct RouteHistory {
    std::int32_t carIndex=-1,carSerial=0,targetIndex=-1,targetSerial=0;
    cfgspawn::Vec3 center{},axis{},origin{};
    float side=0;
    std::uint8_t attemptedEnds=0;
    Failure failures[2]{Failure::None,Failure::None};
    unsigned episodes=0;
    std::uint64_t failedAt=0;
    bool valid=false,exhausted=false;
    void Reset(){*this={};}
    static std::uint8_t EndBit(float end){return end>0?1u:2u;}
    static unsigned EndIndex(float end){return end>0?0u:1u;}
    bool Bind(std::int32_t ci,std::int32_t cs,std::int32_t ti,std::int32_t ts,
        cfgspawn::Vec3 car,cfgspawn::Vec3 forward,float routeSide,cfgspawn::Vec3 jason){
        if(ci<0||cs<=0||ti<0||ts<=0||!cfgspawn::Finite(car)||!cfgspawn::Finite(forward)||
           !cfgspawn::Finite(jason)||(routeSide!=1.f&&routeSide!=-1.f))return false;
        const float n=std::sqrt(forward.x*forward.x+forward.y*forward.y);
        if(n<.1f||!std::isfinite(n))return false;
        const cfgspawn::Vec3 normalized{forward.x/n,forward.y/n,0};
        const bool geometryChanged=valid&&(Route::Squared(center,car)>100.f*100.f||
            axis.x*normalized.x+axis.y*normalized.y<.985f||side!=routeSide||
            Route::Squared(origin,jason)>500.f*500.f);
        const bool identityChanged=valid&&(carIndex!=ci||carSerial!=cs||targetIndex!=ti||targetSerial!=ts);
        if(!valid||geometryChanged||identityChanged){
            Reset();carIndex=ci;carSerial=cs;targetIndex=ti;targetSerial=ts;
            center=car;axis=normalized;side=routeSide;origin=jason;valid=true;
        }
        return true;
    }
    bool Tried(float end)const{return (attemptedEnds&EndBit(end))!=0;}
    float NextEnd(float preferred)const{
        if(preferred!=1.f&&preferred!=-1.f)return 0;
        if(!Tried(preferred))return preferred;
        const float other=-preferred;
        return Tried(other)?0:other;
    }
    bool Fail(float end,Failure why,std::uint64_t now){
        if(!valid||(end!=1.f&&end!=-1.f)||why==Failure::None)return false;
        attemptedEnds=static_cast<std::uint8_t>(attemptedEnds|EndBit(end));
        failures[EndIndex(end)]=why;failedAt=now;++episodes;
        exhausted=(attemptedEnds&3u)==3u;
        return !exhausted;
    }
};
// On-foot admission cannot depend on four absolutely stationary samples:
// collision sliding and target changes can reset them indefinitely. Acquire
// the typed nearby car, then prove cross-chassis geometry in the independent
// heading phase before issuing any movement. Occupied-car sampling is frozen.
struct Obstacle : cfgvehicleaction::Obstacle {
    Route route;
    void Reset(){cfgvehicleaction::Obstacle::Reset();route.Reset();}
    static bool InRange(float squared){
        return std::isfinite(squared)&&squared>=125.f*125.f&&squared<=2000.f*2000.f;
    }
    bool Observe(std::uintptr_t c,std::uintptr_t d,std::uintptr_t s,cfgspawn::Vec3 here,cfgspawn::Vec3 target,std::uint64_t now){
        if(stage||now<next)return false;
        if(!cfgspawn::Finite(here)||!cfgspawn::Finite(target)){Reset();return false;}
        const auto dx=target.x-here.x,dy=target.y-here.y;
        const auto squared=dx*dx+dy*dy;
        if(!InRange(squared)){Reset();return false;}
        car=c;driver=d;source=s;origin=previous=here;next=now+1000;
        samples=1;detourIssued=false;stage=5;return true;
    }
};
}
