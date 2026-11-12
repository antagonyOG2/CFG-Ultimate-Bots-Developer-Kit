#define CFG_J21C_HELD_RECOVERY 1
#define main LegacyHeldFixtureMain
#include "DonorHeldGrabKillTests.cpp"
#undef main
#include "../Core/DonorJasonCombat.hpp"
struct FrozenCombatHost {
    cfgdonorhunt::CombatState state{};std::uint64_t now=2600;unsigned inputs=0;
    FrozenCombatHost(){state.Jason=0x200000;state.GrabKillReadyAt=1300;}
    cfgdonorhunt::CombatState& Combat(){return state;}bool ReadableActor(std::uintptr_t){return true;}
    std::uint64_t Now(){return now;}bool ModuleReady(){return true;}
    bool AttackReleaseReadable(){return true;}void ReleaseAttack(){}
    bool TickJasonAIKnifeSequenceOnGameThread(std::uint64_t){return false;}
    void UseReflectedGrabKill(unsigned){++inputs;}
    std::uintptr_t FindNearestJasonAICounselorTarget(std::uintptr_t){return 0;}
    bool GetJasonAIActorLocation(std::uintptr_t,cfgdonorhunt::Position&){return false;}
    bool TryStartJasonAIKnifeThrowOnGameThread(std::uintptr_t,float,std::uint64_t){return false;}
    bool SelectedInteractableIsSCDoor(){return false;}bool GrabReadable(){return false;}bool TryGrab(){return false;}
    bool AttackPressReadable(){return false;}bool PressAttack(){return false;}
};
int main(){unsigned count=0,fail=0;const auto check=[&](bool v){++count;if(!v){++fail;printf("FAIL recovery check%u\n",count);}};
 {State s;Host h;check(s.Drive(h,0));check(s.Drive(h,100));check(s.probes==2&&!h.kills);
  check(s.Drive(h,125)&&s.probes==4);check(s.Drive(h,150)&&s.probes==6);
  check(s.Drive(h,175)&&s.probes==7&&h.kills==1&&s.attempts==1&&!s.CanFallback(2000));}
 {State s;Host h;h.probeLimit=0;check(s.Drive(h,0));check(s.Drive(h,100));check(s.probes==0);
  check(s.Drive(h,FallbackDeadlineMs-1)&&!s.legacyFallback);check(!s.Drive(h,FallbackDeadlineMs)&&s.legacyFallback&&s.CanFallback(FallbackDeadlineMs));check(cfgj24::HeldFallbackMs==2500&&FallbackDeadlineMs==(CFG_H276_HELD_WALK?9000u:2500u));}
 for(unsigned veto=0;veto<7;++veto){State s;Host h;h.probeLimit=0;s.Drive(h,0);
  if(veto==0)h.observed=false;if(veto==1)h.sample.dead=true;if(veto==2)h.sample.finalContext=true;
  if(veto==3)h.sample.killing=true;if(veto==4)h.sample.water=true;if(veto==5)h.sample.seat=true;if(veto==6)h.sample.extracting=true;
  s.Drive(h,2600);check(!s.legacyFallback&&!s.CanFallback(2600)&&!h.kills);}
 {State s;Host h;s.Drive(h,0);s.stage=Stage::Uncertain;s.Drive(h,3000);check(!s.legacyFallback&&!h.kills);}
 {State s;Host h;h.probeLimit=0;s.Drive(h,1000);FrozenCombatHost frozen;
  const auto timer=frozen.state.GrabKillReadyAt;
  check(!s.Drive(h,1000+FallbackDeadlineMs)&&s.legacyFallback&&frozen.state.GrabKillReadyAt==timer);
  check(cfgdonorhunt::DonorRunJasonAICombatOnGameThread(frozen)&&frozen.inputs==1&&!frozen.state.GrabKillReadyAt);
  s.committedAt=3500;s.stage=Stage::Committed;check(!s.CanFallback(3600));}
 printf("J2.1c bounded probing/deadline/unknown/death/water/seat/uncertain fallback: %u checks,%u failures; owned core only.\n",count,fail);return fail?1:0;}