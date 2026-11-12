#include "../Core/CounselorPursuitPolicy.hpp"
#include <cstdio>
int main(){unsigned failures=0,checks=0;const auto check=[&](bool value){++checks;if(!value)++failures;};
    using namespace cfgpursuit;Ring ring;
    check(!ring.Begin(0,{0,0,0},0));check(!ring.Begin(0x10001,{0,0,0},0));check(!ring.Begin(0x10000,{NAN,0,0},0));
    check(ring.Begin(0x10000,{100,200,300},0));check(!ring.Begin(0x20000,{0,0,0},0));
    check(ring.Current(0x10000,{600,200,300})&&!ring.Current(0x10000,{601,200,300}));
    check(!ring.Current(0x20000,{100,200,300})&&!ring.Current(0x10000,{NAN,0,0}));
    const auto point=ring.Candidate();check(point.x==2300&&point.y==200&&point.z==235);
    check(!ring.Projected({2300,200,331})&&!ring.Projected({2300,200,89}));
    check(!ring.Projected({2651,200,210})&&!ring.Projected({1700,200,210}));
    check(!ring.Projected({NAN,200,210}));check(ring.Projected({2300,200,210})&&ring.projected&&ring.surface.z==210);
    check(ring.ActorDestination().x==2300&&ring.ActorDestination().y==200&&ring.ActorDestination().z==300);
    ring.FailedCandidate(100);check(ring.target==0x10000&&ring.candidate==1&&!ring.projected);
    for(unsigned i=1;i<ExteriorOffsets.size();++i){
        check(ring.candidate==i&&ring.Current(0x10000,{100,200,300}));
        ring.FailedCandidate(100+i);
    }
    check(!ring.target&&ring.retryAt==3115);check(!ring.Begin(0x10000,{0,0,0},3114));
    check(ring.Begin(0x10000,{0,0,0},3115));ring.Refused(4000);check(!ring.target&&ring.retryAt==7000);
    check(!ring.Begin(0x10000,{0,0,0},6999)&&ring.Begin(0x10000,{0,0,0},7000));
    ring.Clear();check(!ring.target&&!ring.projected&&!ring.candidate);
    cfgvehicle::Cooldown shared;check(shared.Due(0,5001.f*5001.f));shared.Attempted(1000);
    check(!shared.Due(20999,1.e8f)&&shared.Due(21000,1.e8f));
    check(!cfgpursuit::MorphDue(shared,7999,1.e8f)&&cfgpursuit::MorphDue(shared,8000,1.e8f));
    check(!cfgpursuit::MorphDue(shared,16000,2000.f*2000.f)&&cfgpursuit::MorphDue(shared,16000,2001.f*2001.f));
    check(!cfgpursuit::MorphDue(shared,16000,NAN));
    check(cfgpursuit::MorphDue(shared,16000,221.f*221.f,true));
    check(!shared.Due(16000,1.e8f)); // trap/default shared20s gate NOT shortened
    cfgvehicle::TailIntercept car;car.Attempted(1000);check(car.next==6000); // requested car5s; counselor/trap clocks independent
    Progress p;p.Observe(0x10000,1,{0,0,0},1000);
    for(unsigned i=1;i<4;++i){p.Observe(0x10000,1,{1,0,0},1000+2000*i);check(!p.Stalled());}
    p.Observe(0x10000,1,{1,0,0},9000);check(p.Stalled());
    check(MorphDue(shared,16000,1001.f*1001.f,false,p.Stalled()));
    check(!MorphDue(shared,7999,1.e8f,false,true)&&!MorphDue(shared,8000,1000.f*1000.f,false,true));
    check(MorphDue(shared,8000,500.f*500.f,false,true,true)); // stalled inside -> outside near target
    check(!MorphDue(shared,7999,500.f*500.f,false,true,true)); // shared counselor cooldown retained
    check(!MorphDue(shared,8000,220.f*220.f,false,true,true)); // no immediate melee teleport
    check(MorphDue(shared,1001,221.f*221.f,true)&&!MorphDue(shared,1001,221.f*221.f,false));
    p.Observe(0x10000,1,{100,0,0},11000);check(!p.Stalled());
    p.Observe(0x10000,2,{100,0,0},13000);check(!p.Stalled()&&p.serial==2);
    p.Observe(0x10000,2,{100,0,0},20000);check(!p.Stalled());
    p.Observe(0x10000,0,{100,0,0},22000);check(!p.observed);
    std::printf("Base-mod exterior ring/ownership/distance/shared cooldown: %u checks, %u failures. No game access.\n",checks,failures);
    return failures?1:0;
}
