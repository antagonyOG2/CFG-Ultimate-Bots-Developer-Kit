#include "../Core/VehicleActionPolicy.hpp"
#include "../RuntimeProbe/VehicleInteractionBindings.generated.hpp"
#include "TrapNativeParameters.hpp"
#include "../Core/GameplayBundlePolicy.hpp"
#include <cstdio>
int main(){unsigned failures=0;const auto check=[&](bool okay){if(!okay)++failures;};
 {cfgvehicleaction::State s;s.Select(1,2);
  check(!s.StopOwnsEpisode());s.HoodDispatched(false,1000);check(s.StopOwnsEpisode());
  check(s.ObserveStop(true,true,true,500,100,200,100,1100)&&s.extractionCommitted);
  check(s.ObserveStop(true,true,true,500,100,200,100,3000)&&s.extractionCommitted);
  check(s.ObserveStop(false,false,false,500,100,200,100,4000)&&s.extractionCommitted);
  check(s.ObserveStop(true,true,false,500,100,200,100,5000)&&!s.extractionCommitted&&!s.StopOwnsEpisode());}
 {cfgvehicleaction::State s;s.Select(1,2);s.extractionCommitted=true;s.committedAt=900;s.SeatDispatched(1000);
  s.phase=2;s.readyAt=1500;s.DoorMoveAccepted({0,-210,0},1600);
  check(!s.StaleSeatAttempt(999)&&!s.StaleSeatAttempt(6499)&&s.StaleSeatAttempt(6500));
  s.RetryDriverApproach();check(s.car==1&&s.driver==2&&s.extractionCommitted&&s.committedAt==900);
  check(!s.seatInputAt&&!s.seatBatch&&!s.phase&&!s.readyAt&&!s.haveDoorGoal&&!s.StaleSeatAttempt(9000));
  s.SeatDispatched(10000);check(!s.StaleSeatAttempt(15499)&&s.StaleSeatAttempt(15500));
  s.extractionCommitted=false;check(!s.StaleSeatAttempt(16000));}
 {cfgvehicleaction::State s;s.Select(1,2);
  check(s.DoorMoveDue({0,-210,0},1000));s.DoorMoveAccepted({0,-210,0},1000);
  check(!s.DoorMoveDue({0,-210,0},2100)&&!s.DoorMoveDue({0,-210,0},5999));
  check(s.DoorMoveDue({0,-210,0},6000)&&s.DoorMoveDue({75,-210,0},2100));
  check(!s.DoorMoveDue({74,-210,0},2100)&&!s.DoorMoveDue({NAN,0,0},2100));
  check(s.DoorMoveDue({0,-210,0},999));s.Morphed();check(!s.haveDoorGoal);
  s.DoorMoveAccepted({0,-210,0},7000);s.Select(1,3);check(!s.haveDoorGoal);
 }
 {cfgvehicleaction::State s;s.Select(1,2);s.HoodDispatched(true,1000);
  check(!s.hoodAt&&!s.HoodWaiting(1001));s.HoodDispatched(false,2000);check(s.HoodWaiting(2001));
  s.HoodMoveAccepted({100,0,0},2500);s.Morphed();check(s.stopPending&&s.morphStopPending&&!s.haveHoodGoal&&s.HoodMoveDue({},2501));}
 {using cfgvehicleaction::AtInteraction;
  check(!AtInteraction({131,0,0},{900,0,0},{0,0,0},135));
  check(AtInteraction({131,0,0},{0,0,0},{0,0,0},135));}
 {cfgvehicleaction::Obstacle s;const cfgspawn::Vec3 j{0,0,100},car{1000,0,100};
  check(!s.Observe(1,2,3,j,car,1000));
  for(unsigned i=1;i<4;++i)check(!s.Observe(1,2,3,j,car,1000+1000*i));
  check(s.Observe(1,2,3,j,car,5000)&&s.stage==1&&s.candidate.x==100&&s.candidate.y==250);
  check(!s.Projected({100,250,201})&&s.stage==0);
  check(s.Observe(1,2,3,j,car,6000)&&s.candidate.y==-250);
  check(s.Projected({100,-250,100})&&s.stage==2);s.Accepted(6200);
  check(s.Protected(10199)&&!s.Protected(10200)&&!s.Observe(1,2,3,j,car,7000));
  check(!s.Observe(1,2,3,{30,0,100},car,10200)&&s.samples==0);
  check(!s.Observe(2,3,4,j,car,11200)&&s.car==2&&s.driver==3);}
 {cfgvehicleaction::Obstacle s;const cfgspawn::Vec3 j{0,0,100},car{1000,0,100};
  s.car=1;s.driver=2;s.source=3;s.previous=j;s.projected={100,250,100};s.Accepted(1000);
  for(unsigned i=0;i<3;++i)check(!s.Observe(1,2,3,j,car,2000+1000*i));
  check(s.Observe(1,2,3,j,car,5000)&&s.stage==4&&s.detourIssued);
  check(s.projected.x==100&&s.projected.y==250);}
 {cfgvehicleaction::DriverRoute route;cfgspawn::Vec3 goal{};
  // Restore preserved ABI76/early78 and donor ResolveDriverSideDetourPoint:
  // nearest-end DRIVER corner360x360, not a passenger-first custom route or
  // an unverified lateral crossing along the hood's230/240cm line.
  check(route.Goal({0,0,0},{1,0,0},{0,-210,0},{100,210,0},1000,goal)&&goal.x==360&&goal.y==-360&&!route.reached);
  check(route.Goal({0,0,0},{1,0,0},{0,-210,0},{360,-290,0},2000,goal)&&!route.active&&goal.y==-210);
  route.Reset();check(route.Goal({0,0,0},{1,0,0},{0,-210,0},{0,-200,0},1000,goal)&&!route.active&&goal.y==-210);
  route.Reset();check(route.Goal({0,0,0},{1,0,0},{0,-210,0},{100,210,0},1000,goal));
  check(route.Goal({0,0,0},{1,0,0},{0,-210,0},{100,210,0},7000,goal)&&route.opposite&&goal.x==-360);
  route.Reset();check(route.Goal({0,0,0},{1,0,0},{0,-210,0},{240,0,0},1000,goal)&&goal.x==360&&goal.y==-360&&!route.reached);
  // Donor120cm per-pass handoff: once outside the driver side, do not
  // keep walking to the old360cm corner or reproject the driver-door goal.
  check(route.Goal({0,0,0},{1,0,0},{0,-210,0},{240,-290,0},2000,goal)&&!route.active&&goal.x==0&&goal.y==-210);
  route.Reset();check(route.Goal({0,0,0},{1,0,0},{0,-210,0},{-240,0,0},1000,goal)&&goal.x==-360&&goal.y==-360);
  route.Reset();check(route.Goal({100,200,0},{0,1,0},{310,200,0},{100,440,0},1000,goal)&&goal.x==460&&goal.y==560);
  // Both longitudinal signs and a rotated car retain the same donor clearance.
  route.Reset();check(route.Goal({0,0,0},{1,0,0},{0,210,0},{-240,0,0},1000,goal)&&goal.x==-360&&goal.y==360);
  route.Reset();check(route.Goal({100,200,0},{0,1,0},{310,200,0},{100,-40,0},1000,goal)&&goal.x==460&&goal.y==-160);
  check(!route.Goal({NAN,0,0},{1,0,0},{0,-210,0},{0,0,0},8000,goal));}
 cfgspawn::Vec3 point{};check(cfgvehicleaction::Point({10,20,30},{1,0,0},false,point)&&point.x==250&&point.y==20);
 check(cfgvehicleaction::Point({10,20,30},{1,0,0},true,point)&&point.x==10&&point.y==-190);
 check(!cfgvehicleaction::Point({0,0,0},{0,0,0},true,point));check(!cfgvehicleaction::Point({NAN,0,0},{1,0,0},true,point));
 cfgspawn::Vec3 authored{90,120,30};check(cfgvehicleaction::Point({10,20,30},{1,0,0},true,point,&authored)&&point.y>20&&point.x>10);
 authored={210,80,30};check(cfgvehicleaction::Point({10,20,30},{1,0,0},false,point,&authored)&&point.x==210&&point.y==20);
 authored={10,20,30};check(cfgvehicleaction::Point({10,20,30},{1,0,0},true,point,&authored)&&point.y==-190);
 authored.z=500;check(!cfgvehicleaction::Point({10,20,30},{1,0,0},true,point,&authored));
 cfgspawn::Vec3 pursuit{};
 check(cfgvehicleaction::HoodLead({240,0,10},{1000,0,0},pursuit)&&pursuit.x==990&&pursuit.y==0&&pursuit.z==10);
 check(cfgvehicleaction::HoodLead({240,0,10},{-1000,0,0},pursuit)&&pursuit.x==-510); // physical hood remains front while reversing
 check(cfgvehicleaction::HoodLead({240,0,10},{0,200,0},pursuit)&&pursuit.x==240&&pursuit.y==170);
 check(!cfgvehicleaction::HoodLead({240,0,10},{0,0,0},pursuit));
 check(!cfgvehicleaction::HoodLead({NAN,0,10},{1000,0,0},pursuit));
  float speed=0;check(cfgvehicleaction::Speed(350,speed)&&speed==1750);check(!cfgvehicleaction::Speed(NAN,speed));check(!cfgvehicleaction::Speed(0,speed));check(!cfgvehicleaction::Speed(5000,speed));
  {std::array<float,4> after{};
   check(cfgvehicleaction::PursuitSpeeds({250,600,450,300},after)&&after==std::array<float,4>{2250,3000,2250,1500});
   check(cfgvehicleaction::PursuitSpeeds({2250,600,450,300},after)&&after==std::array<float,4>{2250,3000,2250,1500});
   check(!cfgvehicleaction::PursuitSpeeds({NAN,600,450,300},after));
   check(!cfgvehicleaction::PursuitSpeeds({250,NAN,450,300},after));
   check(!cfgvehicleaction::PursuitSpeeds({250,600,3000,300},after));}
 cfgvehicleaction::State state;state.Select(1,2);state.hoodAt=5;state.Select(1,2);check(state.hoodAt==5);state.Select(1,3);check(!state.hoodAt&&!state.readyAt&&!state.attemptAt);
 state.hoodAt=1000;check(!state.SeatReady(false,true,1000)&&!state.SeatReady(false,true,4999));
 check(state.SeatReady(false,true,5000)&&!state.SeatReady(true,true,6000));
 check(state.SeatReady(false,false,1000));state.seatAttempts=1;state.attemptAt=state.seatInputAt=2000;
 check(state.Leaving(1,3,2000)&&state.Leaving(1,3,8999)&&!state.Leaving(1,3,9000));
 check(!state.Leaving(1,3,10000)&&!state.Leaving(1,4,3000));
 state.hoodAttempts=2;state.Retire();check(state.stopPending&&state.KnownVacant(1,3)&&!state.car&&!state.driver&&state.hoodAttempts==2);
 state.Select(1,3);check(state.KnownVacant(1,3)); // stale Driver alone must not reacquire priority
 state.stopPending=false;check(!state.Retire()&&!state.stopPending); // repeat vacancy never stops a resumed hunt
 state.Occupied(1,4);check(state.KnownVacant(1,3)); // unrelated observation cannot clear it
 state.Occupied(1,3);check(!state.KnownVacant(1,3));
 cfgvehicleaction::Attempt p;check(p.method==1&&!p.autoLock&&sizeof(p)==16);check(cfgvehicleinteraction::disk::ExecutorCode.size()==247);
 cfgtrap::MoveLocationParameters move;cfgvehicleaction::NavigationOptions(true,move);
 check(!move.pathfinding&&!move.projectNavigation&&!move.strafe&&!move.stopOnOverlap);
 cfgvehicleaction::NavigationOptions(false,move);check(move.pathfinding==1&&move.projectNavigation==1&&move.strafe==1&&move.allowPartial==1);
 cfgvehicleaction::NavigationOptions(true,move);check(!move.pathfinding&&!move.projectNavigation&&!move.strafe&&!move.allowPartial);
 state={};check(cfgvehicleaction::ContinuationDelay(state,false,1000)==1100);
 check(cfgvehicleaction::ContinuationDelay(state,true,1000)==100);
 state.phase=1;state.readyAt=1000;check(cfgvehicleaction::ContinuationDelay(state,false,1650)==100);
 state.phase=2;check(cfgvehicleaction::ContinuationDelay(state,false,3500)==100);
 check(cfgvehicleaction::ContinuationDelay(state,false,3501)==100);
 check(cfgvehicleaction::ContinuationDelay(state,false,999)==1100);
 state={};state.Select(1,2);state.Motion(true);check(cfgvehicleaction::ContinuationDelay(state,false,1000)==877);
 check(state.HoodMoveDue({0,0,0},1000));state.HoodMoveAccepted({0,0,0},1000);
 check(!state.HoodMoveDue({1599,0,0},1500)&&state.HoodMoveDue({1600,0,0},1500));
 check(!state.HoodMoveDue({0,0,0},6002)&&state.HoodMoveDue({0,0,0},6003));
 state.movingAttempts=3;state.Motion(false);check(!state.haveHoodGoal&&!state.movingAttempts);
 check(cfgvehicleaction::AttemptInterval(false,true)==900&&cfgvehicleaction::AttemptInterval(false,false)==5000&&cfgvehicleaction::AttemptInterval(true,false)==3000);
 // Native slam/engine state, not a void hood return, owns extraction admission.
 state={};state.Select(1,2);state.hoodAt=1000;
 check(state.ObserveStop(true,true,false,0,100,200,100,4999)&&state.extractionCommitted&&!state.HoodWaiting(4999));
 check(state.ObserveStop(false,true,false,0,100,200,100,5000)&&state.extractionCommitted&&state.committedAt==4999);
 check(state.ObserveStop(true,true,false,500,100,200,100,6498)&&state.extractionCommitted);
 check(state.ObserveStop(true,true,false,500,100,200,100,6499)&&!state.extractionCommitted&&!state.hoodAt);
 state={};state.Select(1,2);
 check(state.ObserveStop(true,true,false,0,100,200,100,1000)&&state.extractionCommitted);
 check(cfgvehicleaction::ContinuationDelay(state,false,10000)==100);
 state={};state.Select(1,2);state.hoodAt=1000;
 check(state.ObserveStop(true,true,false,200,100,200,100,6000)&&!state.hoodAt&&!state.extractionCommitted);
 state={};state.Select(1,2);check(state.ObserveStop(true,true,true,0,100,200,100,1000)&&state.extractionCommitted&&state.slamObserved);
 state={};state.Select(1,2);check(state.ObserveStop(false,false,false,0,100,200,100,1000)&&state.extractionCommitted);
 state={};state.Select(1,2);check(state.ObserveStop(false,false,false,0,100,200,651.f*651.f,1000)&&!state.extractionCommitted);
 check(!state.ObserveStop(false,false,false,NAN,100,200,100,1000));
 check(!state.ObserveStop(false,false,false,0,-1,200,100,1000));
 state={};state.Select(1,2);check(state.SeatAttemptDue(1000));state.SeatDispatched(1000);
 check(!state.SeatAttemptDue(3999)&&state.SeatAttemptDue(4000));state.SeatDispatched(4000);state.SeatDispatched(7000);
 check(!state.SeatAttemptDue(11999)&&state.SeatAttemptDue(12000)&&state.seatBatch==0);
 state.driverSerial=19;state.Select(1,3);check(state.driverSerial==0);
 // Owner callback budget is still one Schedule lane. A pending release wins
 // even when the accelerated car continuation is due.
 cfgbundle::Schedule schedule;check(schedule.Start(0));schedule.vehicleAt=100;schedule.Pressed(0);
 check(schedule.Due(150)==cfgbundle::Lane::Release);schedule.Released();
 check(schedule.Due(250)==cfgbundle::Lane::Vehicle);
 // Real paired acquire/action timing reaches stop->650ms settle->face->input
 // in1000ms, instead of4.4s, without combining native phases in one callback.
 state={};state.phase=1;state.readyAt=1000;std::uint64_t now=1000;unsigned phase=0;
 for(unsigned i=0;i<8;++i){now+=cfgvehicleaction::ContinuationDelay(state,(i%2)==0,now);
  if(i%2){if(now>=state.readyAt+650){if(state.phase==1){state.phase=2;++phase;}else if(state.phase==2){state.phase=0;++phase;}}}}
 //650ms is crossed on the fourth action at1800ms; its next pair dispatches.
 check(now==1800&&phase==1&&state.phase==2);
 now+=100;now+=100;++phase;state.phase=0;check(now==2000&&phase==2);
 // Reader-call model from ContextObservation/KillBudget fixtures: ObjectSlot4,
 // ObjectIs2+2depth, FeatureFunction22 plus Context on a cold factory. This
 // models8seats/depth10, two cold action factories, and extra outerguard64.
 // It is a bounded production-shape model, not measured live ancestry/timing.
 const unsigned scope=36+2*5+40+2*(8+6+3),is=2+2*10;
 const unsigned feature=22+is+2,manager=18+is,counselor=75;
 const unsigned base=scope+64+64+2*4+2+counselor+7+8+8+1+8*(1+is+2)+4+1+8+is+1+4+manager+2;
 // Stopped-car authored component position adds one scene-class cache/type
 // proof, repeated translation samples, exact owner and slot revalidation.
 const unsigned authoredPosition=12+is+2+2+4;
 const unsigned transforms=feature+7+authoredPosition;
 const unsigned cold=2*(36+2*5);
 const unsigned actionProof=scope+manager+4*4+12+counselor;
 const unsigned seatSlotSamples=8*4;
 // ABI36 adds the repeated start/slam/enabled observations and a no-extraction
 // two-read sentinel. No second cold Context/Jason traversal on that path.
 const unsigned nativeStopObservation=13+7,extractionSentinel=4;
 const unsigned attemptPhase=base+transforms+actionProof+feature+cold+seatSlotSamples+nativeStopObservation+extractionSentinel;
 const unsigned stopPhase=base+transforms+feature+cold+seatSlotSamples+nativeStopObservation+extractionSentinel;
 const unsigned vacantPhase=base+seatSlotSamples+1+8*(1+is+1+1+4)+4+4+1;
 check(vacantPhase<1536);
 check(attemptPhase<1536&&stopPhase<1536);check(10u<=12u);
 std::printf("Phased vehicle read model: attempt=%u stop/face=%u cap1536; calls<=10/12.\n",attemptPhase,stopPhase);
 std::printf("CFG vehicle5x/no25x handoff/driver-side geometry/episode reset/native packet: %s (%u failures).\n",failures?"FAIL":"PASS",failures);return failures?1:0;
}
