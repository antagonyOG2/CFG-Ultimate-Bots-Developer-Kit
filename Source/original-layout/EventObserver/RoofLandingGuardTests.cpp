#include "../Core/RoofLandingGuard.hpp"
#include <cassert>
#include <cstdio>
struct Point{float x=0,y=0,z=0;};
using cfgroofguard::Observation;
struct Probe{
    bool lowerLanding=false,lowerCounselor=false,fail=false,ambiguous=false;
    unsigned calls=0;
    Observation operator()(Point query,Point& out){
        ++calls;if(fail)return Observation::Unsafe;
        if(ambiguous){out={query.x+200,query.y,query.z};return Observation::Present;}
        if(query.x>1000){if(!lowerLanding)return Observation::Absent;out={query.x,query.y,query.z+0};return Observation::Present;}
        if(!lowerCounselor)return Observation::Absent;
        out={query.x,query.y,query.z};return Observation::Present;
    }
};
int main(){cfgroofguard::Diagnostics<Point> d;const Point roof{2000,0,500},counselor{0,0,100};
 {Probe p;p.lowerLanding=true;assert(!cfgroofguard::Safe(d,roof,counselor,p)&&p.calls==2&&d.RoofGuardRejects==1);}
 {Probe p;assert(cfgroofguard::Safe(d,{2000,0,100},counselor,p)&&p.calls==1);}
 {Probe p;p.lowerLanding=p.lowerCounselor=true;assert(cfgroofguard::Safe(d,roof,{0,0,500},p)&&p.calls==2);}
 {Probe p;p.fail=true;assert(!cfgroofguard::Safe(d,roof,counselor,p)&&p.calls==1);}
 {Probe p;p.ambiguous=true;assert(!cfgroofguard::Safe(d,roof,counselor,p)&&p.calls==1);}
 d.Destination(1,roof,counselor);assert(d.destinations==1&&d.lastDestination.z==500&&d.lastCounselor.z==100&&d.lastKind==1);
 assert(d.RoofGuardChecks==5&&d.RoofGuardRejects==3&&d.RoofGuardLastRejected.z==500);
 std::puts("Roof guard PASS: roof/ground/elevated counselor/failed or ambiguous lower projections; max2 probes");
}
