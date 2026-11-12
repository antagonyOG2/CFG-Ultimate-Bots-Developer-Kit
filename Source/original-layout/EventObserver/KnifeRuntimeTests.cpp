#include "KnifeRuntime.hpp"
#include <algorithm>
#include <cassert>
#include <cstdio>
struct Host{
 cfgknife::Snapshot s{{1,1},{2,1},{},4,false,true,true,1000.f*1000.f};
 unsigned press=0,release=0,launch=0,finalize=0,aim=0;bool observe=true,launchOk=true;
 unsigned knifeFailure=0,readCount=0,callCount=0;bool aimOkay=true;
 bool aimedThisCallback=false;char order[4]{};unsigned orderCount=0;
 bool KnifeObserve(unsigned,cfgknife::Snapshot& out,bool){aimedThisCallback=false;out=s;knifeFailure=observe?0:2;return observe;}
 bool KnifeAim(unsigned,cfgknife::Identity){aimedThisCallback=true;++aim;if(orderCount<4)order[orderCount++]='A';knifeFailure=aimOkay?0:3;return aimOkay;}
 unsigned KnifeFailureCode()const{return knifeFailure;}unsigned KnifeReadCount()const{return readCount;}unsigned KnifeCallCount()const{return callCount;}
 bool KnifeRefill(const cfgknife::Snapshot& before){++refills;if(s.count!=before.count)return false;s.count=6;return true;}unsigned refills=0;
 bool KnifePress(){++press;if(orderCount<4)order[orderCount++]='P';s.pressed=true;s.driver={3,1};return true;}
 bool KnifeRelease(cfgknife::Identity){++release;s.pressed=false;return true;}
 bool KnifeLaunch(unsigned,cfgknife::Identity,cfgknife::Identity){assert(!aimedThisCallback);++launch;return launchOk;}
 bool KnifeFlight(cfgknife::Identity,cfgknife::FlightSample& out){out={stationary?0.f:100.f*static_cast<float>(samples++),0,0,1000,5,true};return flightAvailable;}unsigned samples=0;bool stationary=false,flightAvailable=true;
 bool KnifeFinalize(cfgknife::Identity expected){assert(s.driver==expected);++finalize;s.driver={};return true;}
};
// Model the observed hosted case: callback observation spends 900 reads from
// the 1536 primary budget, while an aim proof needs 884 fresh reads. The
// production adapter supplies that fresh reader before entering any aim call.
struct HostedBudgetHost:Host{
 unsigned siblingReads=0,maxSiblingReads=0,freshAimReaders=0,observedReads=0;
 bool KnifeObserve(unsigned index,cfgknife::Snapshot& out,bool entry){
  (void)index;
  observedReads=entry?500u:900u;out=s;knifeFailure=0;return true;
 }
 bool KnifeAim(unsigned index,cfgknife::Identity id){
  ++freshAimReaders;siblingReads=884;maxSiblingReads=(std::max)(maxSiblingReads,siblingReads);
  return Host::KnifeAim(index,id);
 }
};
int main(){
 {Host h;h.s.hosted=true;h.aimOkay=false;h.readCount=901;h.callCount=4;cfgknife::Runtime p;
  assert(p.Begin(h,0,100)&&p.stage==cfgknife::Stage::Windup&&h.press==1);
  p.Tick(h,225);assert(p.stage==cfgknife::Stage::Cleanup&&p.firstAbortRecorded&&p.firstAbortStage==cfgknife::Stage::Windup&&
   p.firstAbortFailure==3&&p.firstAbortReads==901&&p.firstAbortCalls==4&&h.launch==0);
 }
 {Host h;h.s.hosted=true;cfgknife::Runtime p;assert(p.Begin(h,0,100));
  assert(h.press==1&&h.aim==0&&h.orderCount==1&&h.order[0]=='P');
  assert(p.stage==cfgknife::Stage::Windup&&p.pressAttempted&&p.readyAt==1000);
  p.Tick(h,225);assert(h.aim==1&&h.orderCount==2&&h.order[1]=='A'&&p.lastAimAt==225);
  h.s.hosted=false;h.s.driver={};h.s.pressed=false;cfgknife::Runtime offline;Host o;assert(offline.Begin(o,0,100));
  assert(o.press==0&&o.aim==0&&offline.stage==cfgknife::Stage::Aim);
 }
 {HostedBudgetHost h;h.s.hosted=true;cfgknife::Runtime p;assert(p.Begin(h,0,100)&&h.press==1);
  for(std::uint64_t now=225;now<=1100;now+=125)p.Tick(h,now);
  assert(p.stage==cfgknife::Stage::Release&&h.freshAimReaders==8&&h.maxSiblingReads==884&&
   h.observedReads==900&&h.press==1&&p.launches==0&&!p.firstAbortRecorded);
  p.Tick(h,1125);assert(p.stage==cfgknife::Stage::Notify&&h.release==1&&h.launch==0);
 }
 {Host h;cfgknife::Runtime p;assert(p.Begin(h,0,100));p.Tick(h,225);p.Tick(h,350);
  assert(p.MaySuppressHeldStop(h.s,3,0,true,400));
  assert(!p.MaySuppressHeldStop(h.s,3,1,true,400)&&!p.MaySuppressHeldStop(h.s,3,0,false,400));
  assert(!p.MaySuppressHeldStop(h.s,30,0,true,400));auto fresh=h.s;fresh.count--;assert(!p.MaySuppressHeldStop(fresh,3,0,true,400));
  fresh=h.s;fresh.pressed=false;assert(!p.MaySuppressHeldStop(fresh,3,0,true,400));
  fresh=h.s;fresh.jason.serial++;assert(!p.MaySuppressHeldStop(fresh,3,0,true,400));
  assert(!p.MaySuppressHeldStop(h.s,3,0,true,p.endAt));
  p.Tick(h,1250);assert(!p.MaySuppressHeldStop(h.s,3,0,true,1251));}
 {struct Roster{unsigned observed=0;bool KnifeCandidate(unsigned index,bool& living,float& squared){
    ++observed;living=index!=2;squared=index==0?900.f*900.f:1100.f*1100.f;return index<4;}}h;
  cfgknife::NearestRefresh refresh;unsigned selected=9;refresh.Start(3,100);
  for(unsigned i=0;i<3;++i)assert(refresh.Step(h,100+25*i,selected)==cfgknife::RefreshResult::Pending);
  assert(refresh.Step(h,175,selected)==cfgknife::RefreshResult::Ready&&selected==0&&h.observed==4);
  refresh.Start(3,200);assert(refresh.Step(h,2200,selected)==cfgknife::RefreshResult::Unavailable);
  refresh.Start(10,100);assert(refresh.Step(h,100,selected)==cfgknife::RefreshResult::Unavailable);}
 {Host h;cfgknife::Runtime p;assert(p.Begin(h,0,100));p.Tick(h,225);p.Tick(h,350);
  const auto initialAim=h.aim;p.Tick(h,475);p.Tick(h,600);p.Tick(h,725);
  assert(h.aim==initialAim+3&&h.press==1&&!h.release&&!h.launch);
  p.Tick(h,1250);p.Tick(h,1375);assert(h.release==1);}
 const auto start=[](Host& h,cfgknife::Runtime& p){assert(p.Begin(h,0,100));p.Tick(h,225);p.Tick(h,350);assert(h.press==1);p.Tick(h,1250);p.Tick(h,1375);assert(h.release==1);};
 {Host h;cfgknife::Runtime p;start(h,p);--h.s.count;p.Tick(h,1500);p.Tick(h,1525);
  assert(p.pollAt==1550);for(std::uint64_t t=1550;t<1675;t+=25){p.Tick(h,t);assert(p.pollAt==t+25);}
  p.Tick(h,1675);assert(p.pollAt==1800&&h.launch==1&&!h.finalize);}
 {Host h;cfgknife::Runtime p;start(h,p);p.Tick(h,1500);assert(!h.launch);--h.s.count;p.Tick(h,1625);assert(p.stage==cfgknife::Stage::Launch&&!h.launch);p.Tick(h,1650);assert(h.launch==1&&p.Busy());
  p.Tick(h,1775);assert(!h.finalize);p.Tick(h,1900);assert(h.finalize==1&&!p.Busy()&&p.flightsMoved==1);p.Tick(h,2025);assert(h.launch==1);assert(!p.Begin(h,0,2025));
  assert(p.Begin(h,0,13000));assert(!p.release.driver.object);}
 {Host h;cfgknife::Runtime p;start(h,p);p.Tick(h,4000);assert(p.Busy());p.Tick(h,4125);assert(h.launch==0&&h.finalize==1&&p.timeouts==1);}
 {Host h;cfgknife::Runtime p;start(h,p);--h.s.count;p.Tick(h,1500);p.Tick(h,1525);h.s.driver={};p.Tick(h,1650);assert(p.Busy());p.Tick(h,1875);assert(!p.Busy()&&p.flightsMoved==1&&h.finalize==0);}
 {Host h;h.stationary=true;cfgknife::Runtime p;start(h,p);--h.s.count;p.Tick(h,1500);p.Tick(h,1525);h.s.driver={};p.Tick(h,1650);p.Tick(h,1875);assert(!p.Busy()&&p.flightsStationary==1&&!p.flightsMoved);}
 {Host h;h.flightAvailable=false;cfgknife::Runtime p;start(h,p);--h.s.count;p.Tick(h,1500);p.Tick(h,1525);h.s.driver={};p.Tick(h,1650);p.Tick(h,1875);assert(!p.Busy()&&p.flightUnavailable==3&&!p.flightsStationary&&!p.flightsMoved);}
 {Host h;cfgknife::Runtime p;start(h,p);h.s.driver.serial=2;p.Tick(h,1500);assert(h.launch==0&&h.finalize==0&&!p.Busy());}
 {Host h;cfgknife::Runtime p;start(h,p);h.s.count-=2;p.Tick(h,1500);p.Tick(h,1625);assert(h.launch==0&&h.finalize==1&&!p.Busy());}
 {Host h;cfgknife::Runtime p;assert(p.Begin(h,0,100));p.Tick(h,225);p.Tick(h,350);h.s.targetLiving=false;p.Tick(h,475);p.Tick(h,600);p.Tick(h,725);assert(h.launch==0&&h.finalize==1&&!p.Busy());}
 {Host h;cfgknife::Runtime p;start(h,p);--h.s.count;h.launchOk=false;p.Tick(h,1500);p.Tick(h,1525);p.Tick(h,1650);assert(h.launch==1&&h.finalize==1&&!p.Busy());}
 {Host h;cfgknife::Runtime p;start(h,p);h.observe=false;p.Tick(h,1500);assert(p.Busy());h.observe=true;p.Tick(h,1625);assert(h.finalize==1&&!p.Busy());}
 {Host h;cfgknife::Runtime p;start(h,p);h.observe=false;h.s.ownerTerminal=true;p.Tick(h,1500);assert(!p.Busy()&&!h.finalize&&!h.launch);}
 {Host h;cfgknife::Runtime p;assert(p.Begin(h,0,100));p.Tick(h,225);p.Tick(h,350);h.s.driver={};p.Tick(h,1250);p.Tick(h,1375);p.Tick(h,1500);assert(!p.Busy()&&!h.s.pressed&&!h.finalize);}
 {Host h;cfgknife::Runtime p;h.s.squared=100.f;assert(!p.Begin(h,0,100)&&!h.press);h.s.squared=2000.f*2000.f;assert(!p.Begin(h,0,100));}
 {Host h;cfgknife::Runtime p;h.s.count=0;assert(!p.Begin(h,0,100)&&h.refills==1&&h.s.count==6&&!h.press);assert(p.Begin(h,0,350));}
 {Host h;cfgknife::Runtime p;h.s.count=21;assert(!p.Begin(h,0,100)&&h.refills==1&&h.s.count==6);}
 {Host h;cfgknife::Runtime p;h.s.count=0;h.s.pressed=true;assert(!p.Begin(h,0,100)&&!h.refills);}
 {Host h;cfgknife::Runtime p;assert(p.Begin(h,0,100));p.Tick(h,225);p.Tick(h,350);p.Tick(h,1250);
  const auto before=h.aim;p.Tick(h,1500);assert(h.aim==before+1&&!h.release);p.Tick(h,1525);assert(h.release==1);
  p.Tick(h,1550);assert(h.aim==before+2&&!h.launch);--h.s.count;p.Tick(h,1675);assert(!h.launch);p.Tick(h,1700);assert(h.launch==1);}
 {Host h;cfgknife::Runtime p;start(h,p);--h.s.count;p.Tick(h,1500);const auto aimed=h.aim;
  p.Tick(h,1800);assert(!h.launch&&h.aim==aimed+1);p.Tick(h,1825);assert(h.launch==1);}
 {Host h;cfgknife::Runtime p;start(h,p);--h.s.count;p.Tick(h,1500);h.s.driver.serial=2;p.Tick(h,1525);assert(!h.launch&&!h.finalize&&!p.Busy());}
 {Host h;cfgknife::Runtime p;start(h,p);--h.s.count;p.Tick(h,1500);--h.s.count;p.Tick(h,1525);p.Tick(h,1650);assert(!h.launch&&h.finalize==1&&!p.Busy());}
 {Host h;cfgknife::Runtime p;start(h,p);--h.s.count;p.Tick(h,1500);h.s.targetLiving=false;p.Tick(h,1525);p.Tick(h,1650);assert(!h.launch&&h.finalize==1&&!p.Busy());}
 std::puts("Knife sequence PASS: donor stock refill,900ms windup, fresh release/notify aim, separate launch callback, launch revalidation, cleanup/identity/cooldown; no native calls");
}
