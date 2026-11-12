#define CFG_SPECTATOR_MEMBER_FIXTURE_ONLY
#include "SpectatorRuntimeMembersTests.cpp"
#undef CFG_SPECTATOR_MEMBER_FIXTURE_ONLY
#include "KillRuntime.hpp"
#include "../RuntimeProbe/KillContactBindings.generated.hpp"
struct TraceFixture:SpectatorFixture {
 cfgkill::Runtime bundleKill;std::uintptr_t trace=0,contextActor=0;
 bool living=true;
 bool Scope(cfgkill::Identity& id){if(!Context()||!Jason())return false;
  id={plan.world,bundleJason.pawn.object,bundleJason.pawn.serial};return id.Valid();}
 bool KnownLiving(std::uintptr_t pawn){return living&&pawn==r_.bot;}
#include "KillTraceRuntimeMembers.hpp"
 TraceFixture(){
  r_.cap=100000;std::uintptr_t owner=0;r_.Read(r_.bot+cfgprobe::ObjectClass,owner);
  trace=r_.Object(owner);contextActor=r_.Object();Serial(r_.bot);
  using namespace cfgkill::contactdisk;
  r_.Put(trace+0x20,owner);r_.Put(trace+0xB0,plan.module+Executor);
  r_.Put(trace+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{});
  r_.Put(plan.module+AttackTrace.cache,trace);r_.Put(plan.module+AttackTrace.ownerReflection,owner);
  r_.Put(plan.module+AttackTrace.ownerConstructor,owner);r_.Put(plan.module+AttackTrace.factory,AttackTrace.code);
  r_.Put(plan.module+Executor,ExecutorCode);r_.Put(plan.module+Implementation,ImplementationCode);
  r_.Put(r_.bot+Actors,cfgkillcontact::List{0,0,0});
  auto& p=bundleKill.policy;p.stage=cfgkill::Stage::NativeContext;
  p.identity={plan.world,bundleJason.pawn.object,bundleJason.pawn.serial};p.context=contextActor;
  p.weapon=0xABCD;p.finisher=r_.bot;p.usedAt=100;p.acceptedAt=101;p.until=6102;
  r_.reads=0;r_.cap=1536;
 }
 void Hit(std::uintptr_t actor){r_.Put(r_.bot+cfgkill::contactdisk::Actors,cfgkillcontact::List{0x500000,1,4});r_.Put(0x500000,actor);}
};
int main(){unsigned peak=0;
 {TraceFixture f;cfgkillcontact::Proof p;CHECK(f.BeginMeleeTrace(f.r_.bot,f.trace,p));const auto before=p.beforeReads;
  f.Hit(f.bundleJason.pawn.object);f.r_.reads=0;CHECK(f.FinishMeleeTrace(p)&&f.MeleeContactCurrent(p));
  peak=before+f.r_.reads+1;CHECK(peak<=1536&&f.calls_==0);}
 {TraceFixture f;cfgkillcontact::Tracker tracker;cfgkillcontact::Proof first,next;
  f.Hit(f.bundleJason.pawn.object);CHECK(f.BeginMeleeTrace(f.r_.bot,0,first)&&!tracker.Consider(first));
  f.r_.Put(f.r_.bot+cfgkill::contactdisk::Actors,cfgkillcontact::List{});f.r_.reads=0;
  CHECK(f.BeginMeleeTrace(f.r_.bot,0,next)&&!tracker.Consider(next));
  f.Hit(f.bundleJason.pawn.object);f.r_.reads=0;
  CHECK(f.BeginMeleeTrace(f.r_.bot,0,next)&&tracker.Consider(next));
  f.r_.reads=0;CHECK(f.MeleeContinuationBudget(next)&&f.MeleeContactCurrent(next)&&!f.calls_);
 }
 for(unsigned fault=0;fault<10;++fault){TraceFixture f;cfgkillcontact::Proof p;
  if(fault==0)f.bundleKill.policy.stage=cfgkill::Stage::AwaitHit;
  if(fault==1)f.bundleKill.policy.interactionAttempted=true;if(fault==2)f.living=false;
  if(fault==3)f.r_.Put(f.trace+0xB0,std::uintptr_t{});
  if(fault==4)f.r_.Put(f.plan.module+cfgkill::contactdisk::Implementation,std::uint8_t{});
  if(fault==5)f.r_.Put(f.trace+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{8});
  if(fault==6)f.r_.Put(f.r_.bot+cfgkill::contactdisk::Actors,cfgkillcontact::List{0,65,65});
  if(fault==7)f.bundleKill.policy.nativeActiveObserved=true;if(fault==8)f.bundleKill.policy.finishingStrike=true;
  const auto source=fault==9?f.plan.pawn:f.r_.bot;CHECK(!f.BeginMeleeTrace(source,f.trace,p)&&!p.armed&&f.calls_==0);
 }
 {TraceFixture f;cfgkillcontact::Proof p;f.Hit(f.bundleJason.pawn.object);CHECK(f.BeginMeleeTrace(f.r_.bot,f.trace,p));
  f.r_.reads=0;CHECK(!f.FinishMeleeTrace(p));} // previous contact cannot finish again
 {TraceFixture f;cfgkillcontact::Proof p;CHECK(f.BeginMeleeTrace(f.r_.bot,f.trace,p));f.Hit(f.contextActor);
  f.r_.reads=0;CHECK(f.FinishMeleeTrace(p));f.r_.Put(0x500000,f.plan.pawn);CHECK(!f.MeleeContactCurrent(p));}
 std::printf("Included native trace adapter: %d failures before+after peak%u/1536; no native calls/writes.\n",failures,peak);
 return failures?1:0;
}
