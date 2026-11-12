#include "../Core/DonorJasonStartup.hpp"
#include <cassert>
#include <cstdio>
using namespace cfgdonorhunt;
struct Host {
    ChaseState chase;NonAggroStartupState startup;std::uint64_t now=1000,ready=21000;
    unsigned traps=3,resolve=0,stops=0,finishes=0;bool visitedReason=false;
    Host(){chase.Jason=2;startup.Active=true;startup.ObjectivesReady=true;startup.ObjectiveCount=2;}
    auto& Chase(){return chase;}auto& Startup(){return startup;}std::uint64_t Now(){return now;}
    std::uintptr_t World(){return 1;}std::uint64_t JasonAIMorphReadyAt(){return ready;}
    void ResolveCounselorRouteStartupObjectives(std::uintptr_t w){assert(w==1);++resolve;}
    unsigned ReadJasonAITrapCount(std::uintptr_t j){assert(j==2);return traps;}
    void StopJasonAIMovementForKnifeOnGameThread(){++stops;}
    void FinishJasonAIStartupTrapSetupOnGameThread(bool visited){++finishes;visitedReason=visited;startup.Active=false;}
};
int main(){unsigned checks=0;auto check=[&](bool b){++checks;assert(b);};
    {Host h;h.startup.ObjectiveIndex=2;h.startup.TransitActive=true;h.startup.TransitGoal=9;h.startup.TransitNextMoveAt=500;
        for(unsigned i=0;i<1200;++i){DonorPrepareNonAggroStartupTick(h);check(h.stops==1&&!h.finishes);}
        check(!h.startup.TransitActive&&!h.startup.TransitGoal&&!h.startup.TransitNextMoveAt&&h.startup.NextActionAt==21000);
        h.now=20999;DonorPrepareNonAggroStartupTick(h);check(!h.finishes);
        ++h.now;DonorPrepareNonAggroStartupTick(h);check(h.stops==2&&h.finishes==1&&h.visitedReason&&!h.startup.Active);}
    {Host h;h.traps=0;h.now=h.ready;DonorPrepareNonAggroStartupTick(h);check(h.finishes==1&&!h.visitedReason&&h.stops==1);}
    for(unsigned flag=0;flag<4;++flag){Host h;h.traps=0;
        if(flag==0)h.startup.Teleported=true;else if(flag==1)h.startup.AttemptSent=true;
        else if(flag==2)h.startup.CountConsumed=true;else h.startup.PhoneApproachActive=true;
        DonorPrepareNonAggroStartupTick(h);check(!h.stops&&!h.finishes);}
    {Host h;h.startup.ObjectivesReady=false;h.startup.NextActionAt=1001;DonorPrepareNonAggroStartupTick(h);check(!h.resolve);
        ++h.now;DonorPrepareNonAggroStartupTick(h);check(h.resolve==1&&!h.stops);}
    {Host h;h.startup.ObjectiveIndex=2;h.ready=0;DonorPrepareNonAggroStartupTick(h);check(!h.stops&&!h.finishes);}
    {Host h;h.startup.TransitActive=true;h.startup.TransitGoal=9;h.startup.TransitNextMoveAt=500;
        DonorPrepareNonAggroStartupTick(h);check(h.startup.TransitActive&&h.startup.TransitGoal==9&&h.startup.TransitNextMoveAt==500&&!h.stops);}
    std::printf("J1a donor non-aggro startup: %u checks PASS; one stop during1200wait callbacks,ready boundary,objective/empty-trap reasons,accepted transit retained.\n",checks);
}
