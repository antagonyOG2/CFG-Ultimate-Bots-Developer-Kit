#define main CfgTrapPriorityResourceFixtureMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
#include "TrapPriorityObservation.hpp"
#include "../Core/StartupTrapRuntime.hpp"
static void Assigned(Fixture& f,std::uintptr_t object){int index=0;CHECK(f.Read(object+0xC,index));f.Put(f.objects.data+24*index+16,std::int32_t{99});}
int main(){
    using namespace cfgtrappriority;
    Fixture base;const auto type=base.Object(),trap=base.Object(type);
    Assigned(base,base.bot);base.Put(base.module+disk::TrapReflection,type);base.Put(base.module+disk::TrapConstructor,type);
    base.Put(trap+0x110,std::uint8_t{3});base.Put(trap+disk::TrapArmer,base.pawn);base.Put(base.bot+disk::TriggeredTrap,trap);
    auto capture=[&](Fixture& f){f.reads=0;Slot victim{},owned{};const bool ok=Capture(f,f.objects,f.module,f.pawn,f.bot,trap,victim,owned);CHECK(f.reads<96);return ok;};
    CHECK(disk::RuntimeVerified);
    CHECK(capture(base));
    {Fixture cold=base;cold.Put(cold.module+disk::TrapReflection,std::uintptr_t{});CHECK(capture(cold));}
    // A named, disk-pinned constructor cache can prove the native class when
    // its lazy reflection wrapper is cold. No factory call inside the overlap.
    {Fixture missing=base;missing.Put(missing.module+disk::TrapConstructor,std::uintptr_t{});CHECK(!capture(missing));}
    for(unsigned fault=0;fault<8;++fault){Fixture f=base;
        if(fault==0)f.Put(trap+disk::TrapArmer,f.bot);
        if(fault==1)f.Put(f.bot+disk::TriggeredTrap,std::uintptr_t{});
        if(fault==2)f.Put(trap+0x110,std::uint8_t{2});
        if(fault==3)f.Put(trap+0x10,f.botClass);
        if(fault==4)f.Put(f.module+disk::TrapConstructor,f.botClass);
        if(fault==5){int index=0;CHECK(f.Read(trap+0xC,index));f.Put(f.objects.data+24*index+8,std::uint32_t{0x20000000});}
        if(fault==6){f.trigger=trap+disk::TrapArmer;f.mutation=f.bot+disk::TriggeredTrap;f.newPointer=0;}
        if(fault==7){int index=0;CHECK(f.Read(f.bot+0xC,index));f.Put(f.objects.data+24*index+16,std::int32_t{});}
        CHECK(!capture(f));
    }
    Policy priority;CHECK(!priority.Queue(0,0,1));CHECK(priority.Queue(base.bot,1,100));
    CHECK(priority.pending&&priority.index==1&&priority.Active(60099)&&!priority.Active(60100));
    CHECK(!priority.Queue(base.bot,1,UINT64_MAX));priority.Clear();CHECK(!priority.pending&&!priority.Active(100));
    CHECK(priority.Queue(base.bot,1,100));
    cfgtrap::startup::Runtime startup;startup.report.objectives=4;startup.report.objective=1;
    startup.report.outcome[0]=1;startup.report.consumed=1;
    for(const auto stage:{cfgtrap::startup::Stage::Discover,cfgtrap::startup::Stage::Travel,
        cfgtrap::startup::Stage::Stop,cfgtrap::startup::Stage::Face,cfgtrap::startup::Stage::Probe,
        cfgtrap::startup::Stage::Attempt,cfgtrap::startup::Stage::Confirm,cfgtrap::startup::Stage::Hold,
        cfgtrap::startup::Stage::ApproachStop,cfgtrap::startup::Stage::ApproachFace,cfgtrap::startup::Stage::ApproachProbe}){
        startup.report.stage=stage;
        CHECK(priority.ServiceDue(startup.Busy()&&!startup.CanSuspend())==startup.CanSuspend());
        CHECK(!priority.ServiceDue(startup.Busy()&&!startup.CanSuspend(),true));
        CHECK(priority.pending&&priority.until==60100&&startup.report.skipped==0&&startup.report.consumed==1);
    }
    startup.report.stage=cfgtrap::startup::Stage::Complete;
    CHECK(priority.ServiceDue(startup.Busy())&&priority.Active(60099)&&!priority.Active(60100));
    priority.Clear();CHECK(!priority.ServiceDue(false));
    std::printf("Native owned-trap event identity, mutation, lease and route-obligation tests: %s\n",failures?"FAIL":"PASS");return failures?1:0;
}
