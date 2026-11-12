#include "../Core/DonorJasonMorph.hpp"
#include "../Core/DonorJasonNativeOwnerRoute.hpp"
#include <cassert>
#include <cstdio>
#include <limits>
#include <vector>
using namespace cfgdonorhunt;
struct Host {
    ChaseState s;std::uint64_t now=1000;std::uintptr_t world=1,target=4;
    Position origin{},destination{9001,0,90};bool actor=true,klass=true,function=true,project=true,
        call=true,result=true,initial=true,traps=false,knife=false,door=false,busy=false,pending=false;
    int projectionFault=0;unsigned projections=0,teleports=0,initials=0,marks=0,busyCalls=0;
    std::vector<Position> candidates;
    Host(){s.Active=true;s.World=1;s.Jason=2;s.InitialTeleportAttempted=true;}
    ChaseState& Chase(){return s;}std::uint64_t Now(){return now;}std::uintptr_t World(){return world;}
    bool ReadableActor(std::uintptr_t){return actor;}bool ReadableActorAndClass(std::uintptr_t){return actor&&klass;}
    std::uintptr_t FindNearestJasonAICounselorTarget(std::uintptr_t){return target;}
    bool GetJasonAIActorLocation(std::uintptr_t who,Position& p){p=who==2?origin:destination;return true;}
    bool TeleportFunction(std::uintptr_t){return function;}
    bool ProjectJasonAINavPointOnGameThread(Position p,Position extent,Position& out){
        assert(extent.X==350&&extent.Y==350&&extent.Z==110);++projections;candidates.push_back(p);out=p;
        if(projectionFault==1)out.Z=destination.Z-90+121;
        if(projectionFault==2)out.X=destination.X,out.Y=destination.Y;
        if(projectionFault==3)out.Z=std::numeric_limits<float>::quiet_NaN();
        return project;}
    bool SafeTeleport(std::uintptr_t,Position,bool& out){++teleports;out=result;return call;}
    bool RunOfflineBotsInitialTeleportOnGameThread(){++initials;return initial;}
    bool NavigationPending(){return pending;}
    void MarkJasonAIMorphTeleportUsed(){++marks;DonorMarkJasonAIMorphTeleportUsed(s,now);}
    bool StartupTrapSetupActive(){return traps;}bool KnifeSequenceActive(){return knife;}
    bool DoorBreakActive(){return door;}bool IsJasonBridgeCombatBusy(std::uint64_t,bool near,std::uint32_t* causes=nullptr){assert(!near);++busyCalls;if(causes)*causes=0;return busy;}
};
int main(){unsigned checks=0;auto check=[&](bool x){++checks;assert(x);};
    {Host h;h.destination={6000,0,0};
        check(SelectNativeOwnerRoute(true,true)==NativeOwnerRoute::HostedAdoption&&
            DonorTrySharedFiftyMeterHuntMorph(h,h.now)&&h.initials==1);}
    {ChaseState hosted;DonorInitializeHostedPrimaryChase(hosted,5000);
        check(hosted.LastMorphTeleportAt==5000&&hosted.NextDistanceTeleportAt==25000&&
            hosted.InitialTeleportAt==25000&&hosted.NextStuckCheckAt==6375);
        check(DonorJasonAIMorphRemainingMs(hosted,24999)==1&&DonorJasonAIMorphRemainingMs(hosted,25000)==0);}
    {Host h;check(DonorRunJasonAINavRingTeleportOnGameThread(h,2,4)&&h.projections==1&&h.teleports==1);
        check(h.candidates[0].X==11201&&h.candidates[0].Y==0&&h.candidates[0].Z==25);}
    {Host h;h.result=false;check(!DonorRunJasonAINavRingTeleportOnGameThread(h,2,4)&&h.projections==16&&h.teleports==16);
        check(h.candidates[1].X==6801&&h.candidates[8].X==12001&&h.candidates[15].X==6880&&h.candidates[15].Y==-2121);}
    for(unsigned fault=0;fault<7;++fault){Host h;
        if(fault==0)h.actor=false;else if(fault==1)h.klass=false;else if(fault==2)h.function=false;
        else if(fault==3)h.project=false;else h.projectionFault=static_cast<int>(fault)-3;
        check(!DonorRunJasonAINavRingTeleportOnGameThread(h,2,4)&&!h.teleports);}
    {Host h;check(DonorRunOfflineBotsInitialTeleportOnGameThread(h)&&h.s.Target==4&&!h.marks);}
    {Host h;h.s.PathLocked=true;h.s.LastAcceptedMoveAt=100;h.s.HaveLastLocation=true;
        check(DonorRunJasonAIDistanceTeleportOnGameThread(h)&&h.initials==1&&h.marks==1&&
            h.s.NextDistanceTeleportAt==21000&&!h.s.PathLocked&&!h.s.LastAcceptedMoveAt&&!h.s.HaveLastLocation);
        h.now=20999;check(!DonorRunJasonAIDistanceTeleportOnGameThread(h)&&h.initials==1);
        ++h.now;check(DonorRunJasonAIDistanceTeleportOnGameThread(h)&&h.initials==2);}
    {Host h;h.destination={9000,0,0};check(!DonorRunJasonAIDistanceTeleportOnGameThread(h)&&!h.initials);
        h.destination.Z=100;check(DonorRunJasonAIDistanceTeleportOnGameThread(h));}
    {Host h;h.initial=false;check(!DonorRunJasonAIDistanceTeleportOnGameThread(h)&&h.initials==1&&h.s.NextDistanceTeleportAt==4000&&!h.marks);
        h.initial=true;h.now=3999;check(!DonorRunJasonAIDistanceTeleportOnGameThread(h)&&h.initials==1);
        ++h.now;check(DonorRunJasonAIDistanceTeleportOnGameThread(h)&&h.initials==2);}
    for(unsigned boundary=0;boundary<4;++boundary){Host h;h.destination={boundary==0?5000.f:boundary==1?5001.f:boundary==2?9000.f:9001.f,0,0};
        check(DonorTrySharedFiftyMeterHuntMorph(h,h.now)==(boundary==1||boundary==2));}
    for(unsigned lock=0;lock<8;++lock){Host h;h.destination={6000,0,0};
        if(lock==0)h.traps=true;else if(lock==1)h.knife=true;else if(lock==2)h.door=true;
        else if(lock==3)h.busy=true;else if(lock==4)h.world=9;else if(lock==5)h.s.InitialTeleportAttempted=false;
        else if(lock==6)h.s.DoorTraversalGraceUntil=h.now+1;else h.s.LastMorphTeleportAt=h.now-1;
        check(!DonorTrySharedFiftyMeterHuntMorph(h,h.now)&&!h.initials);}
    {Host h;h.destination={4900,0,0};check(!DonorTrySharedFiftyMeterHuntMorph(h,h.now));
        h.destination.X=6000;h.now+=499;check(!DonorTrySharedFiftyMeterHuntMorph(h,h.now)&&!h.initials);
        ++h.now;check(DonorTrySharedFiftyMeterHuntMorph(h,h.now)&&h.initials==1);}
    {ChaseState s;check(DonorJasonAIMorphReadyAt(s)==0&&DonorJasonAIMorphRemainingMs(s,100)==0);
        DonorMarkJasonAIMorphTeleportUsed(s,1000);check(DonorJasonAIMorphReadyAt(s)==21000&&
            DonorJasonAIMorphRemainingMs(s,20999)==1&&DonorJasonAIMorphRemainingMs(s,21000)==0);}
    {Host h;h.initial=false;h.pending=true;
        check(DonorRunJasonAIDistanceTeleportOnGameThread(h)&&h.s.NextDistanceTeleportAt==0&&!h.marks);}
    {Host h;h.destination={6000,0,0};h.initial=false;h.pending=true;
        check(DonorTrySharedFiftyMeterHuntMorph(h,h.now)&&h.s.NextDistanceTeleportAt==0&&!h.marks);}
    std::printf("J1a donor Morph/rings: %u checks PASS; exact16candidate order/floor limits, 50/90m boundaries, shared20s recharge,3s failed retry. Native host pending.\n",checks);
}
