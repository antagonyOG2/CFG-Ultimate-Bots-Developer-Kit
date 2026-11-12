#include "../Core/DonorJasonChase.hpp"
#include "../Core/DonorJasonPursuit.hpp"
#include "../Core/DonorJasonStartup.hpp"
#include "../Core/DonorNativeMorphCompletion.hpp"
#include "DonorJasonRuntimeState.hpp"
#include <cassert>
#include <cstdio>
#include <limits>
using namespace cfgdonorhunt;
struct Host {
    DonorNativeJasonRuntime runtime;ChaseState& s=runtime.chase;
    std::uint64_t now=10000,morphReady=0;std::uintptr_t world=1,target=4;
    Position at{},targetAt{4000,0,0};float walk=150,run=300;
    bool readable=true,controller=true,position=true,door=false,initial=true,distant=false,failsafe=true,pending=false,pendingOnInitial=false;
    unsigned moves=0,doors=0,initials=0,marks=0,runWrites=0,recoveries=0;
    Host(){s.Active=true;s.InitialTeleportAttempted=true;s.World=1;s.Jason=2;s.Controller=3;}
    ChaseState& Chase(){return s;}std::uintptr_t World(){return world;}
    bool ReadableActor(std::uintptr_t){return readable;}bool ReadableController(std::uintptr_t){return controller;}
    std::uint64_t Now(){return now;}
    bool ReadAuthoredMovementSpeeds(float& w,float& r){w=walk;r=run;return true;}
    void UseAuthoredRunSpeed(float value){++runWrites;walk=value;}
    bool RunOfflineBotsInitialTeleportOnGameThread(){++initials;if(pendingOnInitial)pending=true;return initial;}
    bool NavigationPending(){return pending;}
    void MarkJasonAIMorphTeleportUsed(){++marks;}
    bool RunJasonAIDistanceTeleportOnGameThread(){return distant;}
    bool GetJasonAIActorLocation(std::uintptr_t actor,Position& p){p=actor==s.Jason?at:targetAt;return position;}
    bool RunJasonAIDoorInteractionOnGameThread(){++doors;return door;}
    std::uint64_t JasonAIMorphReadyAt(){return morphReady;}
    std::uintptr_t FindNearestJasonAICounselorTarget(std::uintptr_t){return target;}
    bool TryOfflineBotsFailsafeTeleportOnGameThread(const Position&){++recoveries;return failsafe;}
    void IssueStockReflectedJasonMoveOnGameThread(std::uintptr_t){++moves;s.LastAcceptedMoveAt=now;}
    bool Tick(){return DonorRunJasonAIChaseUpdateOnGameThread(*this);}
    void BeginAcceptedPath(){s.HaveLastLocation=true;s.LastLocation=at;s.LastAcceptedMoveAt=now-1;
        s.PathLocked=true;s.PathLockUntil=now+100000;s.Target=target;}
};
int main(){unsigned checks=0;auto check=[&](bool x){++checks;assert(x);};
    {Host h;h.s.InitialTeleportAttempted=false;h.s.InitialTeleportAt=h.now+2000;
        check(h.Tick()&&!h.initials&&!h.moves);h.now+=1999;check(h.Tick()&&!h.initials);
        ++h.now;check(h.Tick()&&h.initials==1&&h.marks==1&&!h.moves&&!h.s.PathLocked);
        ++h.now;check(h.Tick()&&h.moves==1&&h.initials==1);}
    {Host h;h.s.InitialTeleportAttempted=false;h.initial=false;h.pending=true;
        check(h.Tick()&&h.initials==1&&!h.marks&&!h.moves);}
    {Host h;h.pending=true;check(h.Tick()&&!h.moves&&!h.doors&&!h.recoveries);}
    {Host h;h.BeginAcceptedPath();check(h.Tick());h.now+=1375;h.failsafe=false;h.initial=false;h.pendingOnInitial=true;
        check(h.Tick()&&h.recoveries==1&&h.initials==1&&!h.moves&&h.s.ConsecutiveStuckChecks==2&&
            h.s.NextStuckCheckAt==h.now+1375);}
    for(unsigned fault=0;fault<5;++fault){Host h;
        if(fault==0)h.s.Active=false;else if(fault==1)h.world=9;else if(fault==2)h.s.Jason=0;
        else if(fault==3)h.readable=false;else h.controller=false;
        check(!h.Tick()&&!h.s.Active&&!h.moves&&!h.runWrites);}
    {Host h;check(h.Tick()&&h.runWrites==1&&h.moves==1);h.walk=150;h.now+=29999;
        check(h.Tick()&&h.runWrites==1);++h.now;check(h.Tick()&&h.runWrites==2);}
    for(unsigned bad=0;bad<5;++bad){Host h;
        if(bad==0)h.walk=25;else if(bad==1)h.run=h.walk+25;else if(bad==2)h.run=1500;
        else if(bad==3)h.walk=std::numeric_limits<float>::quiet_NaN();else h.run=std::numeric_limits<float>::infinity();
        check(h.Tick()&&!h.runWrites);}
    {Host h;h.BeginAcceptedPath();check(h.Tick()&&h.s.ConsecutiveStuckChecks==1&&!h.recoveries&&h.doors==1);
        check(h.s.PathLocked&&h.s.LastAcceptedMoveAt==9999&&h.s.NextStuckCheckAt==11375);
        h.now+=1374;check(h.Tick()&&h.doors==1);++h.now;
        check(h.Tick()&&h.doors==2&&h.recoveries==1&&!h.s.HaveLastLocation&&!h.s.PathLocked&&!h.s.LastAcceptedMoveAt);}
    {Host h;h.BeginAcceptedPath();h.door=true;check(h.Tick()&&h.doors==1&&!h.recoveries&&
        !h.s.PathLocked&&!h.s.HaveLastLocation&&h.s.NextStuckCheckAt==h.now+750);}
    {Host h;h.BeginAcceptedPath();h.s.DoorTraversalGraceUntil=h.now+5000;
        check(h.Tick()&&!h.s.ConsecutiveStuckChecks&&h.s.PathLocked&&!h.recoveries);}
    {Host h;h.BeginAcceptedPath();h.at.X=30;
        check(h.Tick()&&!h.s.ConsecutiveStuckChecks&&!h.doors&&!h.recoveries);}
    {Host h;h.BeginAcceptedPath();h.s.LastAcceptedMoveAt=h.now-5001;
        check(h.Tick()&&!h.s.ConsecutiveStuckChecks&&!h.doors);}
    {Host h;h.BeginAcceptedPath();h.targetAt.X=2999;
        check(h.Tick()&&!h.recoveries);h.now+=1375;check(h.Tick()&&!h.recoveries&&h.s.ConsecutiveNearTargetStuckDeferrals==1);
        check(h.s.NextStuckCheckAt==h.now+4750&&h.s.PathLocked&&h.s.LastAcceptedMoveAt==9999);
        h.now=h.s.NextStuckCheckAt;h.s.LastAcceptedMoveAt=h.now-1;
        check(h.Tick()&&h.recoveries==1&&h.s.ConsecutiveNearTargetStuckDeferrals==0);}
    {Host h;h.BeginAcceptedPath();h.morphReady=h.now+100000;h.Tick();h.now+=1375;
        check(h.Tick()&&!h.recoveries&&!h.s.ConsecutiveNearTargetStuckDeferrals&&h.s.PathLocked);}
    {Host h;h.BeginAcceptedPath();h.failsafe=false;h.Tick();h.now+=1375;
        check(h.Tick()&&h.recoveries==1&&h.initials==1&&!h.s.PathLocked);}
    // H2.45: target lost mid-stuck still reaches the failsafe step-out (and retries after a failure)
    {Host h;h.BeginAcceptedPath();check(h.Tick()&&h.s.ConsecutiveStuckChecks==1);h.target=0;h.now+=1375;
        check(h.Tick()&&h.recoveries==1&&!h.s.HaveLastLocation&&!h.s.ConsecutiveStuckChecks);}
    {Host h;h.BeginAcceptedPath();h.failsafe=false;h.initial=false;h.Tick();h.target=0;h.now+=1375;
        check(h.Tick()&&h.recoveries==1&&h.s.ConsecutiveStuckChecks==1&&h.s.NextStuckCheckAt==h.now+4750);
        h.now+=4750;check(h.Tick()&&h.recoveries==2);}   // no accepted move in 5 s, still retried
    {Host h;h.target=0;h.s.HaveLastLocation=true;h.s.LastLocation=h.at;check(h.Tick()&&!h.recoveries&&!h.s.ConsecutiveStuckChecks);} // idle, never stuck
    {Host h;h.s.Target=h.target;h.s.LastAcceptedMoveAt=h.now-6106;
        check(h.Tick()&&!h.moves&&h.s.PathLockUntil==h.s.LastAcceptedMoveAt+6107);
        ++h.now;check(h.Tick()&&h.moves==1);}
    {Host h;h.s.Target=99;h.s.LastAcceptedMoveAt=h.now-1;
        check(h.Tick()&&h.moves==1&&h.s.Target==4);}
    {Host h;h.distant=true;check(h.Tick()&&!h.moves&&!h.doors);}
    for(bool extra:{false,true}){Host h;h.runtime.isAdditional=extra;
        check(h.Tick()&&h.moves==1); // entered MoveTo result is then forced uncertain
        h.runtime.MarkUncertain(UncertainAction::Move,h.now);
        for(unsigned i=1;i<10;++i){h.now=10000+100*i;check(!h.runtime.ClearSettledUncertainty(h.now)&&h.moves==1);}
        h.now=11000;check(h.runtime.ClearSettledUncertainty(h.now)&&!h.s.PathLocked&&!h.s.LastAcceptedMoveAt);
        check(h.Tick()&&h.moves==2&&h.runtime.health.uncertainEvents==1);
        check(!h.runtime.ClearSettledUncertainty(h.now)&&h.moves==2); // new decision, not action replay
    }
    // Actual donor body for 60 s, continuous translated pawn/target. A valid
    // accepted native path survives 6.107 s; no per-callback MoveTo rebuild.
    {Host h;for(unsigned frame=0;frame<3600;++frame){h.now=10000+(frame*1000/60);
        h.at.X=static_cast<float>(frame)*5;h.targetAt.X=h.at.X+500;check(h.Tick());}
        check(h.moves>=9&&h.moves<=11&&!h.recoveries&&!h.doors&&h.runWrites==1);}
    std::printf("J1a donor chase: %u checks PASS; 1375ms samples/30cm, door grace, near/far recovery, 6107ms accepted path, authored run. Native host pending.\n",checks);
}
