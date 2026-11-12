#define CFG_FOOT_BURST_FIXTURE_ONLY
#include "OnFootBurstRuntimeMembersTests.cpp"
#include "../Core/JasonRunPolicy.hpp"
#include "../RuntimeProbe/RunningStateBindings.hpp"
#define cfgclosedtrace cfgrunfixturetrace
namespace cfgrunfixturetrace {template<class A,class B>void Mark(const char*,A,B){}}
struct RunFixture:FootFixture {
    struct {unsigned runStateChecks=0,runStateFailure=0,runStateAttempts=0,runStateReturns=0;}bundleReport;
#include "JasonRunRuntimeMembers.hpp"
    RunFixture(){r_.cap=100000;r_.Put(bundleJason.pawn.object+cfgrun::disk::Running,std::uint8_t{1});
        r_.Put(fixtureMovement+cfghook::disk::Walk,200.f);r_.Put(fixtureMovement+cfghook::disk::Run,200.f);
        r_.Put(fixtureMovement+cfghook::disk::Sprint,250.f);r_.Put(fixtureMovement+cfghook::disk::SlowRun,150.f);Fresh();r_.cap=1536;}
};
#undef cfgclosedtrace
int main(){unsigned peak=0;
    {RunFixture f;cfgrun::Policy policy;
     for(unsigned i=0;i<4;++i){f.Fresh();CHECK(policy.Step(f,1000+100*i,false));peak=(std::max)(peak,f.r_.reads);CHECK(f.r_.reads<=1536);}
     for(const auto field:{cfghook::disk::Walk,cfghook::disk::Run,cfghook::disk::Sprint,cfghook::disk::SlowRun}){
         float value=0;CHECK(f.r_.Read(f.fixtureMovement+field,value)&&value==450.f);}
     CHECK(f.writes==4);f.Fresh();CHECK(!policy.Step(f,2400,false)&&f.writes==4);}
    {RunFixture f;cfgrun::Policy policy;
     f.r_.Put(f.fixtureMovement+cfghook::disk::Walk,2250.f);
     f.r_.Put(f.fixtureMovement+cfghook::disk::Run,450.f);
     f.r_.Put(f.fixtureMovement+cfghook::disk::Sprint,450.f);
     f.r_.Put(f.fixtureMovement+cfghook::disk::SlowRun,450.f);
     f.Fresh();
     for(unsigned i=0;i<3;++i){f.Fresh();CHECK(policy.Step(f,3000+100*i,false));}
     for(const auto field:{cfghook::disk::Walk,cfghook::disk::Run,cfghook::disk::Sprint,cfghook::disk::SlowRun}){
         float value=0;CHECK(f.r_.Read(f.fixtureMovement+field,value)&&value==2250.f);}
     CHECK(f.writes==3);}
    for(unsigned fault=0;fault<5;++fault){RunFixture f;float walk=0,run=0;
     CHECK(f.RunObserve(walk,run)&&run==450.f);
     if(fault==0)f.bundleCarSpeed.active=true;
     if(fault==1)f.bundleFootBurst.stage=cfgfootburst::Stage::Hold;
     if(fault==2)f.Serial(f.fixtureMovement,0);
     if(fault==3)f.r_.Put(f.fixtureMovement+0x20,f.plan.controller);
     if(fault==4)f.r_.Put(f.fixtureMovement+cfghook::disk::Run,201.f);
     f.Fresh();CHECK(!f.RunAuxiliary(run)&&!f.writes);}
    std::printf("Included all-variant pursuit run/native field repair: %d failures,peak%u/1536; no game access.\n",failures,peak);
    return failures?1:0;
}
