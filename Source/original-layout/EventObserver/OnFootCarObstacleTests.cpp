#include "../Core/OnFootCarObstaclePolicy.hpp"
#include <cstdio>
int main(){unsigned failures=0;const auto check=[&](bool v){if(!v)++failures;};
    for(const auto distance:{125.f,180.f,250.f,349.f,350.f,400.f,1000.f}){
        cfgfootcar::Obstacle s;cfgspawn::Vec3 here{0,0,100},target{distance,0,100};
        check(s.Observe(1,2,3,here,target,1000)&&s.stage==5);
        check(!s.Observe(1,2,3,{50,0,100},target,1100)); // one pending heading proof, not repeated paths
        check(s.car==1&&s.driver==2&&s.source==3&&s.samples==1);
    }
    {cfgfootcar::Obstacle shortRange;cfgvehicleaction::Obstacle stock;
        const cfgspawn::Vec3 here{0,0,0},near{250,0,0};
        for(unsigned t=0;t<=5000;t+=1000)stock.Observe(1,2,3,here,near,t);
        check(!stock.stage); // accepted occupied-car behavior remains unchanged
        check(!shortRange.Observe(1,2,3,here,{124,0,0},0)&&!shortRange.car);}
    check(!cfgfootcar::Obstacle::InRange(NAN)&&!cfgfootcar::Obstacle::InRange(INFINITY));
    check(!cfgfootcar::Obstacle::InRange(2001.f*2001.f));
    // PID9092: a capsule can stop just INSIDE the conservative chassis box.
    // Leaving that padding toward the near corner must not be classified as
    // crossing the car. Both heading directions and rotated cars must work.
    for(unsigned rotation=0;rotation<16;++rotation)for(float sign:{-1.f,1.f}){
        const float a=rotation*3.14159265f/8.f,c=std::cos(a),s=std::sin(a);
        const auto at=[&](float x,float y){return cfgspawn::Vec3{c*x-s*y,s*x+c*y,100};};
        for(bool nose:{false,true})for(float contact:{100.f,125.f,139.37f,140.f,150.f,179.f}){
            cfgfootcar::Route r;
            const auto here=nose?at(sign*(contact+150),0):at(27.70f,sign*contact);
            const auto target=nose?at(-sign*400,0):at(50,-sign*250);
            check(r.Begin(at(0,0),{c,s,0},here,target,100));
            check(r.Projection(r.goal));
            check(r.AcceptProjection(r.goal,200));
            check(r.Observe(r.arrival,300)==1);check(r.AcceptProjection(r.goal,400));
            check(r.Observe(r.arrival,500)==2);
            // Reacquire on the opposite side after target goes around again.
            check(r.Begin(at(0,0),{c,s,0},target,here,600));check(r.Projection(r.goal));
        }
        cfgfootcar::Route r;
        check(r.Begin(at(0,0),{c,s,0},at(50,sign*150),at(50,-sign*250),100));
        check(!r.Projection(at(400,-sign*400))); // cannot traverse to the far flank first
    }
    {cfgfootcar::Obstacle s;const cfgspawn::Vec3 goal{0,250,10};
        check(s.Observe(1,2,3,{0,0,10},goal,0)&&s.stage==5);
        s.Reset();check(s.Observe(1,2,3,{30,0,10},goal,1000)&&s.stage==5); // collision sliding cannot starve admission
    }
    {cfgfootcar::RouteHistory h;const cfgspawn::Vec3 car{10,20,0},axis{1,0,0},jason{0,-100,100};
        check(h.Bind(4,20,5,30,car,axis,1,jason));check(h.NextEnd(1)==1);
        check(h.Fail(1,cfgfootcar::Failure::Stall,100)&&h.Tried(1)&&h.NextEnd(1)==-1);
        check(!h.Fail(-1,cfgfootcar::Failure::MoveTo,200)&&h.exhausted&&h.NextEnd(1)==0);
        check(h.Bind(4,20,5,30,car,axis,1,jason)&&h.exhausted&&h.attemptedEnds==3);
        check(h.Bind(4,20,5,31,car,axis,1,jason)&&!h.exhausted&&!h.attemptedEnds);
        check(h.Fail(-1,cfgfootcar::Failure::ProjectionGeometry,300));
        auto moved=car;moved.x+=101;check(h.Bind(4,20,5,31,moved,axis,1,jason)&&!h.attemptedEnds);
        check(h.Fail(1,cfgfootcar::Failure::NoGround,400));
        check(h.Bind(4,20,5,31,moved,{.98f,.2f,0},1,jason)&&!h.attemptedEnds);
        check(h.Fail(1,cfgfootcar::Failure::Stall,500));
        check(h.Bind(4,20,5,31,moved,{.98f,.2f,0},-1,jason)&&!h.attemptedEnds);
        check(h.Fail(-1,cfgfootcar::Failure::MoveTo,600));
        auto far=jason;far.x+=501;check(h.Bind(4,20,5,31,moved,{.98f,.2f,0},-1,far)&&!h.attemptedEnds);
        check(!h.Bind(4,0,5,31,moved,axis,1,jason));
    }
    for(unsigned rotation=0;rotation<16;++rotation)for(float side:{-1.f,1.f})for(float end:{-1.f,1.f}){
        const float angle=rotation*3.14159265f/8.f,c=std::cos(angle),n=std::sin(angle);
        const auto transform=[&](float x,float y){return cfgspawn::Vec3{1000+c*x-n*y,2000+n*x+c*y,100};};
        cfgfootcar::Route r;const auto car=transform(0,0),here=transform(end*150,side*200),target=transform(0,-side*200);
        check(r.Begin(car,{c,n,0},here,target,100));check(r.Current(car,target));
        const auto first=transform(end*400,side*400),second=transform(end*400,-side*400);
        check(cfgfootcar::Route::Squared(first,r.goal)<.01f);check(r.Projection(first));
        // Native projection may not erase the extra clearance by snapping
        // both axes back to the old330/210cm minimum envelope.
        check(!r.Projection(transform(end*330,side*240)));
        check(!r.Projection({first.x+161,first.y,first.z}));check(!r.Projection({first.x,first.y,first.z+181}));
        check(r.Observe(here,200)==0);check(r.Observe(first,500)==1);
        check(cfgfootcar::Route::Squared(second,r.goal)<.01f);check(r.Observe(second,800)==2&&!r.active);
        check(r.Begin(car,{c,n,0},here,target,100));check(r.Observe(here,3099)==0);
        check(r.Observe(here,3100)==3&&!r.active);
        check(r.Begin(car,{c,n,0},here,target,100));auto moved=car;moved.x+=101;check(!r.Current(moved,target));
        check(!r.Current(car,here));check(!r.Begin(car,{0,0,0},here,target,100));
        check(!r.Begin(car,{c,n,0},here,here,100));
        check(!r.Begin(car,{c,n,0},transform(500,200),transform(500,-200),100)); // nearby but no chassis intersection
        const auto front=transform(450,0),back=transform(-450,0);
        check(r.Begin(car,{c,n,0},front,back,100)&&r.Current(car,back));
        auto corner=r.goal;check(r.Observe(corner,200)==1);
        corner=r.goal;check(r.Observe(corner,300)==2&&!r.active);
    }
    std::printf("On-foot short-range wrapper of frozen donor car sampler:%u failures; no game access.\n",failures);
    return failures?1:0;
}
