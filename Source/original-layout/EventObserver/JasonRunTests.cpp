#include "../Core/JasonRunPolicy.hpp"
#include <cassert>
#include <cstdio>
struct Host{
    float walk=250,run=450;unsigned observations=0,writes=0;bool current=true;
    bool RunObserve(float& w,float& r){++observations;w=walk;r=run;return current;}
    bool RunWrite(float expected,float desired){if(!current||walk!=expected)return false;++writes;walk=desired;return true;}
};
struct ToggleHost:Host {
    bool running=false;unsigned stateCalls=0;
    bool RunState(){if(running)return false;running=true;++stateCalls;return true;}
};
int main(){
    {ToggleHost h;h.walk=h.run=450;cfgrun::Policy p;assert(p.Step(h,100,false)&&h.running);
     h.running=false;assert(!p.Step(h,349,false)&&!h.running);
     assert(p.Step(h,350,false)&&h.running&&h.stateCalls==2);
     h.running=false;assert(!p.Step(h,600,true)&&!h.running); // speed lease owns its turn
     assert(p.Step(h,601,false)&&h.running&&h.stateCalls==3);}
    {Host pursuit;cfgrun::Policy p;
        assert(cfgrun::PursuitMultiplier==5.f);
        assert(p.Step(pursuit,1,false,cfgrun::PursuitMultiplier)&&pursuit.walk==2250&&pursuit.run==450&&pursuit.writes==1);
        for(unsigned i=1;i<=5;++i)assert(!p.Step(pursuit,1+30000*i,false,cfgrun::PursuitMultiplier)&&pursuit.walk==2250&&pursuit.writes==1);
        assert(!p.Step(pursuit,180001,true,cfgrun::PursuitMultiplier)&&pursuit.writes==1);
        pursuit.walk=2250; // exact pre-car value restored; authored Run unchanged
        assert(!p.Step(pursuit,180002,false,cfgrun::PursuitMultiplier)&&pursuit.walk==2250&&pursuit.writes==1);}
    {Host hard;cfgrun::Policy p;assert(p.Step(hard,1,false,2.f)&&hard.walk==900&&hard.writes==1);
        assert(!p.Step(hard,30001,false,2.f)&&hard.walk==900&&hard.writes==1);}
    Host h;cfgrun::Policy p;assert(p.Step(h,1,false)&&h.walk==450&&h.writes==1);
    assert(!p.Step(h,100,false)&&h.observations==1);
    assert(!p.Step(h,30001,false)&&h.writes==1);
    // Boost has its own baseline and restoration; do not alter its values.
    h.walk=1800;h.run=1800;assert(!p.Step(h,60001,true)&&h.observations==2);
    h.walk=h.run=450;assert(!p.Step(h,61000,false)&&h.writes==1);
    Host slow;slow.walk=slow.run=200;cfgrun::Policy nonrunner;
    assert(!nonrunner.Step(slow,1,false)&&!slow.writes);
    {Host restored;cfgrun::Policy quick;assert(quick.Step(restored,1,false));restored.walk=250;
     assert(!quick.Step(restored,1000,false));assert(quick.Step(restored,1001,false)&&restored.walk==450&&restored.writes==2);
     assert(!quick.Step(restored,2001,false));}
    float desired=0;assert(!cfgrun::Desired(NAN,450,desired));
    assert(cfgrun::Desired(2250,450,desired)&&desired==2250); // scaled walk limit must not make pursuit fall back to walking
    assert(cfgrun::Desired(450,250,desired)&&desired==450);
    assert(!cfgrun::Desired(250,3000,desired));assert(!cfgrun::Desired(0,450,desired));
    assert(!cfgrun::DesiredScaled(250,450,10.f,desired));
    assert(!cfgrun::DesiredScaled(INFINITY,450,5.f,desired));
    assert(cfgrun::DesiredScaled(200,200,5.f,desired)&&desired==1000);
    assert(cfgrun::DesiredScaled(250,2500,5.f,desired)&&desired==12500);
    Host stale;stale.current=false;cfgrun::Policy stopped;assert(stopped.Step(stale,1,false)&&!stale.writes&&stopped.refused==1);
    std::puts("CFG pursuit run policy PASS:250ms running-state/1s speed repair, no-op does not steal chase, authored baseline, no25x compounding, variant/car-boost isolation; no game access");
}
