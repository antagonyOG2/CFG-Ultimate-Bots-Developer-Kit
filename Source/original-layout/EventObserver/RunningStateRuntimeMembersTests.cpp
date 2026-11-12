#define CFG_SWIM_MEMBER_FIXTURE_ONLY
#include "SwimRuntimeMembersTests.cpp"
#include "../RuntimeProbe/RunningStateBindings.hpp"
#include "../Core/JasonRunPolicy.hpp"
#include "../Core/OnFootBurstPolicy.hpp"
namespace cfgrunningfixturetrace {template<class A,class B>void Mark(const char*,A,B){}}
struct RunFixture : SwimFixture {
    struct {unsigned runStateChecks=0,runStateAttempts=0,runStateReturns=0,runStateFailure=0;} bundleReport;
    bool busy_=false,stunned_=false,held_=false;
    unsigned runCalls=0;float speed_=450;unsigned mutation_=0;
    bool Held(std::uintptr_t& held){held=held_?r_.bot:0;return true;}
    bool NativeBool(std::uintptr_t function,bool& result){result=function==cfghunter::disk::Stunned?stunned_:false;return true;}
    bool GrabBusy(bool& busy){busy=busy_;return true;}
    bool CarSpeedWrite(std::uintptr_t,float,float){return false;}
    bool Invoke(std::uintptr_t object,std::uintptr_t function,void* parameters){++calls_;
        if(object!=bundleJason.pawn.object||function!=functions[cfgrun::disk::ServerSetRunning.cache])return false;
        CHECK(*static_cast<std::uint8_t*>(parameters)==1);++runCalls;r_.Put(object+cfgrun::disk::Running,std::uint8_t{1});
        if(mutation_==1)r_.Put(plan.world+cfgprobe::WorldNetDriver,plan.controller);
        if(mutation_==2)r_.Put(object+cfgrun::disk::Running,std::uint8_t{});
        return true;}
    bool bundleCarSpeedActive_=false;
    struct {bool active=false;} bundleCarSpeed;
    cfgfootburst::Policy bundleFootBurst;
#define cfgclosedtrace cfgrunningfixturetrace
#include "JasonRunRuntimeMembers.hpp"
#undef cfgclosedtrace
    RunFixture(){r_.cap=100000;
        Binding(cfgrun::disk::ServerSetRunning,cfgrun::disk::Executor);
        r_.Put(plan.module+cfgrun::disk::ServerSetRunning.ownerReflection,r_.counselorBase);
        r_.Put(plan.module+cfgrun::disk::ServerSetRunning.ownerConstructor,r_.counselorBase);
        // SCCharacter is the shared ancestor, modeled explicitly here.
        r_.Put(r_.killerBase+0x30,r_.counselorBase);
        r_.Put(functions[cfgrun::disk::ServerSetRunning.cache]+0x20,r_.counselorBase);
        r_.Put(plan.module+cfgrun::disk::Executor,cfgrun::disk::ExecutorCode);
        r_.Put(plan.module+cfgrun::disk::Implementation,cfgrun::disk::ImplementationCode);
        r_.Put(plan.module+cfgrun::disk::Toggle,cfgrun::disk::ToggleCode);
        r_.Put(plan.module+cfgrun::disk::SetBit,cfgrun::disk::SetBitCode);
        r_.Put(bundleJason.pawn.object+cfgswim::disk::WaterVolume,std::uintptr_t{});
        r_.Put(bundleJason.pawn.object+cfgrun::disk::Running,std::uint8_t{});
        r_.cap=1536;r_.reads=0;calls_=0;
    }
};
int main(){RunFixture good;CHECK(good.RunState()&&good.runCalls==1&&good.bundleReport.runStateReturns==1);
    CHECK(cfgrun::disk::Executor!=cfgrun::disk::Implementation&&cfgrun::disk::Implementation==0x30B270&&
        cfgrun::disk::ImplementationCode.size()==121&&cfgrun::disk::Toggle==0x30B8A0&&cfgrun::disk::ToggleCode.size()==114);
    const auto peak=good.r_.reads;CHECK(peak<=1536&&good.calls_<=12);CHECK(!good.RunState()&&good.runCalls==1);
    for(unsigned fault=0;fault<8;++fault){RunFixture f;
        if(fault==0)f.busy_=true;if(fault==1)f.stunned_=true;if(fault==2)f.held_=true;
        if(fault==3)f.r_.Put(f.bundleJason.pawn.object+cfgswim::disk::WaterVolume,f.water_);
        if(fault==4)f.r_.Put(f.plan.module+cfgrun::disk::Executor,std::uint8_t{});
        if(fault==5)f.r_.Put(f.plan.world+cfgprobe::WorldNetDriver,f.plan.controller);
        if(fault==6)f.r_.Put(f.plan.module+cfgrun::disk::Implementation,std::uint8_t{});
        if(fault==7)f.r_.Put(f.plan.module+cfgrun::disk::Toggle,std::uint8_t{});
        CHECK(!f.RunState()&&!f.runCalls);}
    for(unsigned mutation=1;mutation<=2;++mutation){RunFixture f;f.mutation_=mutation;CHECK(f.RunState());CHECK(!f.bundleReport.runStateReturns&&f.bundleReport.runStateFailure==4);}
    std::printf("Included native always-run member: failures=%d peak=%u/1536; owned fixture/native invoke mocked\n",failures,peak);return failures?1:0;
}
