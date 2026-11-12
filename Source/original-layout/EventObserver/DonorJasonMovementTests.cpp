#include "../Core/DonorJasonMovement.hpp"
#include "../Core/DonorMoveRetentionPolicy.hpp"
#include <cassert>
#include <cstdio>
#include <limits>
#include <array>
struct FunctionReader {std::uintptr_t function=0,executor=0;bool Read(std::uintptr_t address,std::uintptr_t& out){if(address!=function+0xB0)return false;out=executor;return true;}};
using namespace cfgdonorhunt;
struct Host {
    ChaseState s;std::uint64_t now=1000;Position destination{10,20,30},forward{3,4,0},candidate{},extent{},set{};
    cfgtrap::MoveLocationParameters packet{};std::array<unsigned,8> trace{};unsigned traceCount=0,moves=0,sets=0,projects=0,result=2;
    bool current=true,call=true,projection=true,moved=true;unsigned succeedOn=0;
    Host(){s.Jason=2;s.Controller=3;}
    ChaseState& Chase(){return s;}std::uint64_t Now(){return now;}std::uintptr_t World(){return 1;}
    bool ReadableActor(std::uintptr_t){return current;}bool ReadableActorAndClass(std::uintptr_t){return current;}
    bool ReadableControllerAndClass(std::uintptr_t){return current;}bool MoveToLocationFunction(){return true;}
    bool SafeMoveToLocation(cfgtrap::MoveLocationParameters& p){++moves;packet=p;p.result=static_cast<std::uint8_t>(result);return call;}
    bool GetJasonAIActorLocation(std::uintptr_t,Position& p){p=destination;return true;}
    bool GetActorForwardVector(std::uintptr_t,Position& p){p=forward;return call;}
    bool ProjectJasonAINavPointOnGameThread(Position p,Position e,Position& out){++projects;candidate=p;extent=e;out=p;return succeedOn?projects==succeedOn:projection;}
    bool SetActorLocation(std::uintptr_t p,Position at,bool sweep,bool teleport,bool& out){assert(p==2&&!sweep&&teleport);++sets;set=at;out=moved;return call;}
    void StuckRecoveryStage(unsigned stage){if(traceCount<trace.size())trace[traceCount++]=stage;}
};
int main(){unsigned checks=0;const auto check=[&](bool x){++checks;assert(x);};
    FunctionReader moveFrame{0x3000,0x1010};
    check(cfgh241move::HostedFixedMoveFrame(moveFrame,true,moveFrame.function,0x1000,0x10)); // fixture binds UFunction+0xB0 to executor
    check(!cfgh241move::HostedFixedMoveFrame(moveFrame,false,moveFrame.function,0x1000,0x10));
    moveFrame.executor=0x1020;
    check(!cfgh241move::HostedFixedMoveFrame(moveFrame,true,moveFrame.function,0x1000,0x10));
    {Host h;check(DonorIssueStockReflectedJasonMoveOnGameThread(h,4)&&h.moves==1&&h.s.LastAcceptedMoveAt==1000);
        check(h.packet.destination.x==10&&h.packet.destination.y==20&&h.packet.destination.z==30&&h.packet.acceptance==85);
        check(!h.packet.stopOnOverlap&&h.packet.pathfinding&&h.packet.projectNavigation&&!h.packet.strafe&&!h.packet.filter&&h.packet.allowPartial);
        h.result=1;h.now=2000;check(DonorIssueStockReflectedJasonMoveOnGameThread(h,4)&&h.s.LastAcceptedMoveAt==1000);
        h.result=0;check(!DonorIssueStockReflectedJasonMoveOnGameThread(h,4));}
    {Host h;h.call=false;check(!DonorIssueStockReflectedJasonMoveOnGameThread(h,4)&&!h.s.LastAcceptedMoveAt);}
    for(bool exact:{false,true}){Host h;
        check(DonorIssueAIMoveToLocationOnGameThread(h,3,{10,20,30},150,exact));
        check(h.packet.acceptance==150&&!h.packet.stopOnOverlap&&!h.packet.filter);
        check(h.packet.pathfinding==!exact&&h.packet.projectNavigation==!exact&&h.packet.strafe==!exact&&h.packet.allowPartial==!exact);
        check(!h.s.LastAcceptedMoveAt); // generic helper does not stamp primary chase
        h.result=0;check(!DonorIssueAIMoveToLocationOnGameThread(h,3,{10,20,30},150,exact));
    }
    {Host h;check(DonorTryOfflineBotsFailsafeTeleportOnGameThread(h,{10,20,30})&&h.sets==1);
        check(h.traceCount==6&&h.trace[0]==1&&h.trace[1]==2&&h.trace[2]==3&&h.trace[3]==4&&h.trace[4]==5&&h.trace[5]==7);
        check(h.candidate.X==310&&h.candidate.Y==420&&h.candidate.Z==30&&h.extent.X==100&&h.extent.Y==100&&h.extent.Z==10);
        check(h.set.X==310&&h.set.Y==420&&h.set.Z==30);}
    // H2.45: blocked forward projection falls back to side-steps then back
    for(unsigned on=2;on<=4;++on){Host h;h.succeedOn=on;check(DonorTryOfflineBotsFailsafeTeleportOnGameThread(h,{10,20,30})&&h.sets==1&&h.projects==on);
        const float ex[3]={-390,410,-290},ey[3]={320,-280,-380};check(h.set.X==ex[on-2]&&h.set.Y==ey[on-2]&&h.set.Z==30&&h.trace[h.traceCount-1]==7);}
    {Host h;h.succeedOn=9;check(!DonorTryOfflineBotsFailsafeTeleportOnGameThread(h,{10,20,30})&&h.projects==4&&!h.sets&&h.trace[h.traceCount-1]==4);}
    for(unsigned fault=0;fault<6;++fault){Host h;
        if(fault==0)h.current=false;else if(fault==1)h.forward={0,0,0};
        else if(fault==2)h.forward.X=std::numeric_limits<float>::quiet_NaN();else if(fault==3)h.call=false;
        else if(fault==4)h.projection=false;else h.moved=false;
        check(!DonorTryOfflineBotsFailsafeTeleportOnGameThread(h,{10,20,30}));
        check(h.traceCount>0&&h.trace[0]==1);
        if(fault==0)check(h.traceCount==1);else if(fault==3)check(h.trace[h.traceCount-1]==2);
        else if(fault==4)check(h.trace[h.traceCount-1]==4);else if(fault==5)check(h.trace[h.traceCount-1]==5||h.trace[h.traceCount-1]==6);
        if(fault<5)check(!h.sets);
    }
    std::printf("J1a donor movement: %u checks PASS; exact85cm location packet,accepted-only stamp,normalized500cm forward/100-100-10 projection/native set-location.\n",checks);
}
