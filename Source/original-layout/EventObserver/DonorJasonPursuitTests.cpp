#include "../Core/DonorJasonPursuit.hpp"
#include "../Core/DonorJasonNativeOwnerRoute.hpp"
#include <cstdio>
#include <limits>
using namespace cfgdonorhunt;
struct Host {
    ChaseState s;CombatState c;BusyState busy;PursuitBoost boost;std::uint64_t now=1000;
    std::uintptr_t movement=7,held=0;std::array<float,4> speeds{{150,350,300,200}};
    std::array<std::uint8_t,4> flags{};Position there{1000,0,0};
    bool readable=true,write=true,locked=false,special=false,stunned=false,module=true,flagsOK=true,lockedOK=true,specialOK=true,stunnedOK=true,heldOK=true;
    unsigned writes=0,targetScans=0,stunCalls=0,heldCalls=0;
    Host(){s.Jason=2;c.Jason=2;}
    ChaseState& Chase(){return s;}CombatState& Combat(){return c;}BusyState& Busy(){return busy;}
    PursuitBoost& Boost(){return boost;}std::uint64_t Now(){return now;}
    std::uintptr_t CurrentMovement(std::uintptr_t){return movement;}
    bool ReadableMovement(std::uintptr_t m){return readable&&m==7;}
    bool ReadMovementSpeeds(std::uintptr_t,std::array<float,4>& out){out=speeds;return true;}
    bool WriteMovementSpeeds(std::uintptr_t,std::array<float,4> desired,unsigned mask){
        ++writes;if(!write)return false;
        for(unsigned i=0;i<4;++i)if(mask&(1u<<i))speeds[i]=desired[i];return true;}
    bool ReadableActor(std::uintptr_t){return readable;}
    bool GetLockedJasonInteractable(std::uintptr_t,bool& ok){ok=lockedOK;return !ok||locked;}
    bool IsJasonInNativeSpecialMove(std::uintptr_t,bool& ok){ok=specialOK;return !ok||special;}
    bool ReadBusyFlags(std::uintptr_t,std::array<std::uint8_t,4>& out){out=flags;return flagsOK;}
    bool ModuleReady(){return module;}
    bool SafeIsStunned(std::uintptr_t,bool& out){++stunCalls;out=stunned;return stunnedOK;}
    bool SafeGetGrabbedCounselor(std::uintptr_t,std::uintptr_t& out){++heldCalls;out=held;return heldOK;}
    std::uintptr_t FindNearestJasonAICounselorTarget(std::uintptr_t){++targetScans;return 4;}
    bool GetJasonAIActorLocation(std::uintptr_t a,Position& out){out=a==2?Position{}:there;return true;}
};
int main(){unsigned checks=0,failures=0;const auto check=[&](bool x){++checks;if(!x){++failures;std::printf("FAIL check=%u\n",checks);}};
    check(SelectNativeOwnerRoute(true,true)==NativeOwnerRoute::HostedAdoption);
    check(SelectNativeOwnerRoute(true,false)==NativeOwnerRoute::Refuse);
    check(SelectNativeOwnerRoute(false,false)==NativeOwnerRoute::OfflineProof);
    {Host h;std::uint32_t causes=0;check(!DonorIsJasonBridgeCombatBusy(h,1000,false,&causes)&&causes==0&&
        h.stunCalls==1&&h.heldCalls==1);}
    {Host h;DonorSetJasonHighPriorityPursuitBoost(h,false);check(!h.writes&&!h.boost.active);
        DonorSetJasonHighPriorityPursuitBoost(h,true);check(h.boost.active&&h.writes==1&&h.speeds[0]==300&&h.speeds[2]==600&&h.s.MorphCooldownMs==10000);
        for(unsigned frame=0;frame<1800;++frame){h.now=1000+frame*1000/60;DonorSetJasonHighPriorityPursuitBoost(h,true);check(h.writes==1);}
        h.now=31012;DonorSetJasonHighPriorityPursuitBoost(h,true);check(h.writes==1);
        h.now=31013;DonorSetJasonHighPriorityPursuitBoost(h,true);check(h.writes==2&&h.speeds[2]==600);
        DonorSetJasonHighPriorityPursuitBoost(h,false);check(h.writes==3&&!h.boost.active&&h.speeds[2]==300&&h.s.MorphCooldownMs==20000);}
    {Host h;DonorSetJasonHighPriorityPursuitBoost(h,true,3,500);
        check(h.speeds[0]==450&&h.s.MorphCooldownMs==2000);
        DonorMarkJasonAIMorphTeleportUsed(h.s,1000);check(DonorJasonAIMorphReadyAt(h.s)==3000);
        DonorSetJasonHighPriorityPursuitBoost(h,true,1.5f,7000);check(h.speeds[0]==225&&h.s.MorphCooldownMs==7000&&h.writes==2);}
    {Host h;DonorSetJasonHighPriorityPursuitBoost(h,true,std::numeric_limits<float>::quiet_NaN());check(h.speeds[0]==300);}
    {Host h;h.write=false;DonorSetJasonHighPriorityPursuitBoost(h,true);check(!h.boost.active&&h.s.MorphCooldownMs==20000);}
    for(unsigned i=0;i<4;++i){Host h;h.speeds[i]=25;DonorSetJasonHighPriorityPursuitBoost(h,true);check(!h.boost.active&&!h.writes);}
    {Host h;std::uint32_t causes=~0u;check(!DonorIsJasonBridgeCombatBusy(h,1000,false,&causes)&&!causes&&!h.targetScans&&h.stunCalls==1&&h.heldCalls==1);
        h.there.X=300;check(DonorIsJasonBridgeCombatBusy(h,1001,true)&&h.busy.CombatBusyUntil==2001);
        h.there.X=301;check(DonorIsJasonBridgeCombatBusy(h,2000,true));
        check(!DonorIsJasonBridgeCombatBusy(h,2001,true));
        h.busy.CombatBusyUntil=10000;check(!DonorIsJasonBridgeCombatBusy(h,2002,false));}
    for(unsigned flag=0;flag<4;++flag){Host h;h.flags[flag]=1;check(DonorIsJasonBridgeCombatBusy(h,1000,false));}
    for(unsigned state=0;state<11;++state){Host h;
        if(state==0)h.c.AttackPressed=true;else if(state==1)h.c.GrabKillReadyAt=99;else if(state==2)h.busy.KnifeSequenceActive=true;
        else if(state==3)h.busy.DoorBreakActive=true;else if(state==4)h.busy.StartupTrapSetupActive=true;
        else if(state==5)h.busy.StartupTrapAttemptSent=true;else if(state==6)h.busy.StartupTrapCountConsumed=true;
        else if(state==7)h.busy.StartupTrapPhoneApproachActive=true;else if(state==8)h.locked=true;
        else if(state==9)h.special=true;else h.held=4;
        check(DonorIsJasonBridgeCombatBusy(h,1000,false)&&!h.targetScans);}
    {Host h;h.flagsOK=false;std::uint32_t causes=0;check(DonorIsJasonBridgeCombatBusy(h,1000,false,&causes)&&
        (causes&BusyCauseBit(BusyCause::BusyFlagsReadFailure)));}
    {Host h;h.lockedOK=false;h.specialOK=false;std::uint32_t causes=0;check(DonorIsJasonBridgeCombatBusy(h,1000,false,&causes)&&
        (causes&BusyCauseBit(BusyCause::LockedReadFailure))&&!(causes&BusyCauseBit(BusyCause::SpecialMoveReadFailure)));}
    {Host h;h.specialOK=false;std::uint32_t causes=0;check(DonorIsJasonBridgeCombatBusy(h,1000,false,&causes)&&
        (causes&BusyCauseBit(BusyCause::SpecialMoveReadFailure)));}
    {Host h;h.stunnedOK=false;std::uint32_t causes=0;check(DonorIsJasonBridgeCombatBusy(h,1000,false,&causes)&&
        (causes&BusyCauseBit(BusyCause::StunnedReadFailure)));}
    {Host h;h.heldOK=false;std::uint32_t causes=0;check(DonorIsJasonBridgeCombatBusy(h,1000,false,&causes)&&
        (causes&BusyCauseBit(BusyCause::GrabbedReadFailure)));}
    {Host h;h.c.AttackPressed=true;h.busy.StartupTrapCountConsumed=true;h.flags[2]=1;std::uint32_t causes=0;
        check(DonorIsJasonBridgeCombatBusy(h,1000,false,&causes)&&(causes&BusyCauseBit(BusyCause::AttackPressed))&&
            (causes&BusyCauseBit(BusyCause::StartupTrapCount))&&(causes&BusyCauseBit(BusyCause::BusyFlag2)));}
    {ChaseState s;s.ConsecutiveStuckChecks=4;s.LastAcceptedMoveAt=99;s.HaveLastLocation=true;s.PathLocked=true;s.PathLockUntil=999;
        s.ConsecutiveNearTargetStuckDeferrals=1;std::uintptr_t target=4;unsigned noPath=7;
        DonorResetStuckSamplingAfterNativeInteraction(s,1000,target,noPath);
        check(!s.ConsecutiveStuckChecks&&!s.LastAcceptedMoveAt&&!s.HaveLastLocation&&!s.PathLocked&&!s.PathLockUntil&&
            s.NextStuckCheckAt==2375&&!target&&!noPath&&s.ConsecutiveNearTargetStuckDeferrals==1);}
    std::printf("J1a donor pursuit/busy: %u checks, %u failures; no repeated speed scaling,30013ms repair,mutable shared Morph cooldown,300cm/1s nearby latch,native interaction reset.\n",checks,failures);
    return failures?1:0;
}
