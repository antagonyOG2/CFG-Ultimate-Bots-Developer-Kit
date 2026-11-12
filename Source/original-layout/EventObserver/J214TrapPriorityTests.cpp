#define CFG_J214_TRAP_PRIORITY 1
#define CFG_TRAP_EVENT_TEST_MAIN TrapObservationBaselineMain
#include "TrapEventObservationTests.cpp"
#undef CFG_TRAP_EVENT_TEST_MAIN
#include "../Core/DonorTrapPriorityBridge.hpp"
namespace {
struct Host {
 bool primary=true,usable=true,busy=false,deferred=false,pending=false,uncertain=false;
 std::uint64_t last=0,cooldown=0;unsigned stops=0,morphs=0,busyChecks=0,cooldownChecks=0;
 cfgj214trap::MorphResult result=cfgj214trap::MorphResult::ReturnedTrue;
 bool Primary(){return primary;}bool Usable(std::uintptr_t v){return usable&&v!=0;}
 std::uint64_t LastMorph(){return last;}bool NavigationPending(){return pending;}
 bool Busy(std::uint64_t){++busyChecks;return busy;}
 std::uint64_t MorphRemaining(std::uint64_t now){++cooldownChecks;return cooldown>now?cooldown-now:0;}
 bool Deferred(){return deferred;}bool Uncertain(){return uncertain;}bool Stop(){++stops;return !deferred&&!uncertain;}
 cfgj214trap::MorphResult Morph(std::uintptr_t){++morphs;return result;}
 void MarkMorph(std::uint64_t now){last=now;cooldown=now+20000;}
};
unsigned originalCalls=0,queued=0;
int Original(void*,void*,cfgspawn::Vec3* value){++originalCalls;value->x=17;return 1;}
}
int main(){
 using cfgj214trap::Intent;using cfgj214trap::Service;using cfgj214trap::MorphResult;
 // Real typed trap-event admission, then the driver's single-target intent.
 {TrapFixture f;CHECK(f.TrapEvent(f.r_.bot,f.trapObject,100));Intent s;Host h;
  s.Post(f.bundleTrapPriority.victim,100,h.last);CHECK(Service(s,h,101));
  CHECK(h.stops==1&&h.morphs==1&&s.confirmed==1&&s.completed);
  CHECK(!Service(s,h,20102)&&h.morphs==1);}
 // Both held-grab and kill/attack share donor combat-busy admission.
 for(unsigned combat=0;combat<2;++combat){Intent s;Host h;s.Post(1,100,0);h.busy=true;
  CHECK(!Service(s,h,101)&&h.stops==0&&h.morphs==0&&h.cooldownChecks==0);
  h.busy=false;CHECK(Service(s,h,102)&&h.morphs==1);}
 {Intent s;Host h;h.last=10;h.cooldown=20010;s.Post(1,100,h.last);
  CHECK(!Service(s,h,20009)&&h.stops==0&&h.morphs==0);
  CHECK(Service(s,h,20010)&&h.stops==1&&h.morphs==1);
  CHECK(!Service(s,h,40011)&&h.morphs==1);}
 {Intent s;Host h;s.Post(1,100,0);h.last=150;h.cooldown=20150;
  CHECK(!Service(s,h,151)&&s.sharedSatisfied==1&&s.completed&&h.morphs==0);
  CHECK(!Service(s,h,20200)&&h.morphs==0);}
 {Intent s;Host h;s.Post(1,100,0);h.cooldown=20000;CHECK(!Service(s,h,101));
  h.usable=false;CHECK(!Service(s,h,102)&&s.victim==0&&h.morphs==0);}
 {Intent s;Host h;s.Post(1,100,0);h.primary=false;CHECK(!Service(s,h,101)&&h.morphs==0);}
 {Intent s;Host h;s.Post(1,100,0);CHECK(!Service(s,h,60100)&&s.victim==0);}
 {Intent s;Host h;s.Post(1,100,0);h.result=MorphResult::ReturnedFalse;
  CHECK(!Service(s,h,101)&&h.morphs==1);
  CHECK(!Service(s,h,3100)&&h.morphs==1);
  h.result=MorphResult::ReturnedTrue;CHECK(Service(s,h,3101)&&h.morphs==2&&s.confirmed==1);}
 {Intent s;Host h;s.Post(1,100,0);h.result=MorphResult::Pending;h.pending=true;
  CHECK(Service(s,h,101)&&s.inFlight&&h.morphs==1);
  CHECK(Service(s,h,102)&&h.morphs==1);
  h.pending=false;h.last=103;CHECK(!Service(s,h,104)&&s.completed&&h.morphs==1);}
 {Intent s;Host h;s.Post(1,100,0);h.result=MorphResult::Uncertain;
  CHECK(Service(s,h,101)&&s.completed&&s.ambiguous);
  CHECK(!Service(s,h,40101)&&h.morphs==1);}
 {Intent s;Host h;s.Post(1,100,0);h.usable=false;h.deferred=true;
  CHECK(Service(s,h,101)&&s.victim==1&&h.morphs==0);
  h.usable=true;h.deferred=false;CHECK(Service(s,h,102)&&h.morphs==1);}
 {Intent s;Host h;s.Post(1,100,0);h.uncertain=true;
  CHECK(Service(s,h,101)&&s.completed&&s.ambiguous&&h.morphs==0);
  h.uncertain=false;CHECK(!Service(s,h,102)&&h.stops==1&&h.morphs==0);}
 {Intent s;Host h;s.Post(1,100,0);h.deferred=true;
  CHECK(Service(s,h,101)&&h.stops==1&&h.morphs==0&&s.attempts==0);
  h.deferred=false;CHECK(Service(s,h,102)&&h.morphs==1);}
 {cfgspawn::Vec3 out{};CHECK(cfgj214trap::Forward(static_cast<void*>(nullptr),static_cast<void*>(nullptr),&out,
   &Original,true,[](void*,void*){++queued;})==1);
  CHECK(originalCalls==1&&queued==1&&out.x==17);
  CHECK(cfgj214trap::Forward(static_cast<void*>(nullptr),static_cast<void*>(nullptr),&out,
   &Original,false,[](void*,void*){++queued;})==1);
  CHECK(originalCalls==2&&queued==1);}
 std::printf("J214 trap event -> donor-priority Morph: %d failures; owned event fixture, simulated retained Morph host.\n",failures);
 return failures?1:0;
}
