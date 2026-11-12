#include "../Core/J21FrameBudget.hpp"
#include "../Core/LeanConvergencePolicy.hpp"
#include "../Core/ConvergenceTravelPolicy.hpp"
#include "../Core/GameplayBundlePolicy.hpp"
#include "../Core/StartupTrapRuntime.hpp"
#include <cstdio>
struct Slot {std::uintptr_t object=0;int serial=0;bool HasAssignedSerial()const{return serial>0;}};
struct StarvationTrapHost {
    cfgtrap::Vector position{155,0,85};unsigned attempted=0;int traps=1;
    bool Ready(){return true;}bool Discover(unsigned& n){n=1;return true;}
    bool Count(int& n){n=traps;return true;}
    bool AnchorAt(unsigned,cfgtrap::startup::Anchor& a){a={cfgtrap::startup::Kind::Phone,{0,0,0},{0,-1,0}};return true;}
    bool Position(cfgtrap::Vector& p){p=position;return true;}
    bool Move(cfgtrap::Vector p,unsigned& result){position={p.x,p.y,p.z+85};result=2;return true;}
    bool Stop(){return true;}bool Face(cfgtrap::Vector,cfgtrap::Vector){return true;}
    bool Probe(cfgtrap::CanPlaceParameters& p){p={};p.result=1;p.impact={100,0,0};return true;}
    bool Attempt(){++attempted;--traps;return true;}
};
int main(){unsigned failures=0,checks=0;const auto check=[&](bool ok){++checks;if(!ok)++failures;};
    using namespace cfgj21;
    check(!EarnedSweater(false,false,false));check(!EarnedSweater(false,true,false));
    check(EarnedSweater(false,true,true));check(EarnedSweater(true,false,false));
    Backoff backoff;unsigned attempts=0,radio=0,hud=0;bool previous=false;
    for(std::uint64_t now=0;now<10000;now+=10){
        if(now>=backoff.next){++attempts;backoff.Finish(now,false);}
        const bool objective=ObjectiveTurn(true,false,false,false,previous,false,now%100==0);
        if(objective){++radio;++hud;}previous=objective;
    }
    check(attempts<=9&&radio==100&&hud==100);backoff.Finish(10000,true);
    check(backoff.failures==0&&backoff.next==11000);
    check(!ObjectiveTurn(true,true,false,false,false,false,true));
    check(!ObjectiveTurn(true,false,true,false,false,false,true));
    check(!ObjectiveTurn(true,false,false,true,false,false,true));
    EpochBudget b;b.Tick(1,0,true,0.02f);check(b.epoch==1&&b.primaryTicks==1);
    b.Charge(800);b.Tick(1,1,true,0.02f);check(b.epoch==1&&b.used==800&&b.extraTicks==1);
    b.bothAlive=true;check(b.Choose(0,7)==3);check(b.Choose(1,7)==0);check(b.Choose(1,7)==3);
    b.Charge(1500);check(!b.Slice());b.Tick(1,0,true,0.02f);
    check(b.epoch==2&&b.frames.count==1&&b.frames.maximum==2300&&b.used==0);
    check(b.Choose(0,7)==0);b.Tick(1,0,true,0.02f);b.Tick(1,0,true,0.02f);check(b.Choose(0,7)==1);
    b.Tick(1,1,false,0.02f);check(b.epoch==4&&b.handoffDeferrals==1);b.Tick(1,1,false,0.02f);
    check(b.source==1&&b.epoch==5);b.Fallback(1,true);check(b.epoch==5);
    b.Fallback(1,false);check(b.epoch==6&&b.source==2&&b.peFallbacks==1);
    b.Tick(2,0,true,0.01f);check(b.world==2&&b.epoch==1&&b.frames.count==0);
    b.Charge(3000);check(b.OwedHunt(0,1000,0,false));check(!b.OwedHunt(0,1099,0,false));
    check(b.OwedHunt(0,1100,0,false)&&!b.OwedHunt(0,1200,1100,true));
    Cost cost;cost.Observe(4000);cost.Observe(4001);cost.Observe(16668);
    check(cost.count==3&&cost.total==24669&&cost.maximum==16668&&cost.over4ms==2&&cost.overFrame==1);
    NameMemo<Slot,unsigned> memo;unsigned name=0;memo.Begin(1);memo.Put({12,0},6);check(!memo.Find(1,12,name));
    memo.Put({12,7},6);check(memo.Find(1,12,name)&&name==6);check(!memo.Find(2,12,name));
    memo.Clear();check(!memo.Find(1,12,name));memo.Put({12,7},8);memo.End();check(!memo.Find(1,12,name));
    memo.Begin(2);check(!memo.Find(2,12,name));
    cfglean::Cache cache;check(cache.Needs(2,1,1000,true));cache.Confirm(2,1,1000);
    check(!cache.Needs(2,1,3999,true)&&cache.Needs(2,1,4000,true));
    check(cache.Needs(2,0,1001,true)&&cache.Needs(2,1,1001,false));
    cache.Quarantine(2);check(!cache.Needs(2,0,9000,true));
    cfglean::Schedule schedule;check(!schedule.Due(1000,true,false,false)&&!schedule.Due(1000,false,true,false));
    check(schedule.Due(1000,true,true,false));schedule.Continue(1000);check(!schedule.Due(1024,true,true,false));
    check(schedule.Due(1025,true,true,false));schedule.Finish(1025);check(schedule.next==1358);
    check(cfglean::Driving(true,false,false,false));check(!cfglean::Driving(true,true,false,false));
    check(!cfglean::Driving(true,false,true,false)&&!cfglean::Driving(false,false,false,false));
    cfgconvergencetravel::Policy travel;
    check(travel.Decide(800,1000,true)==cfgconvergencetravel::Action::Acquire);
    travel.Acquired(800,1000);check(travel.Decide(800,1025,true)==cfgconvergencetravel::Action::Move);
    travel.Moved(1025);check(travel.Decide(200,1050,true)==cfgconvergencetravel::Action::Resume);
    check(travel.Decide(800,1050,false)==cfgconvergencetravel::Action::Resume);travel.Resumed(1050,false);check(!travel.owned);
    // #290: six milliseconds of mandatory native work per Tick is telemetry,
    // never optional B/C debt. Exercise the real accepted PE schedule while
    // both optional features run; this is a scheduler fixture, not engine proof.
    EpochBudget busy;Cost native;cfgbundle::Schedule core;check(core.Start(0,0));
    unsigned aiPhases=0;cfgtrap::startup::Runtime traps;StarvationTrapHost trapHost;
    check(traps.Start(0,cfgtrap::MapFamily::GroundMap,true));
    for(std::uint64_t now=16;now<24000;now+=16){
        busy.Tick(9,0,true,0.016f);native.Observe(6000);
        if(busy.Choose(0,6)==1)busy.Charge(1200);else busy.Charge(800);
        busy.CorePE();const auto lane=core.Due(now);
        if(lane!=cfgbundle::Lane::None){++aiPhases;if(traps.Due(now))traps.Step(trapHost,now);}
    }
    check(native.maximum==6000&&native.over4ms==native.count&&native.count>600);
    check(aiPhases>20&&!traps.Busy()&&traps.report.stage==cfgtrap::startup::Stage::Complete&&
        trapHost.attempted==1&&traps.report.consumed==1&&busy.peOpportunities==native.count&&busy.peStarvedEpochs==0);
    check(busy.frames.maximum<=1200); // excludes mandatory 6000us native scope
    std::printf("J2.1 latch/backoff/permanent-failure objective fairness/epoch-budget/parity/round-robin/cache: %u checks, %u failures. No engine calls; no live performance claim.\n",checks,failures);
    return failures?1:0;
}
