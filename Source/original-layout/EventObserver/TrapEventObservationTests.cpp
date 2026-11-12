#define CFG_RETAINED_GAMEPLAY_BUNDLE
#include "../RuntimeProbe/PrimaryLookupEligibility.hpp"
#define CFG_SUMMARY_SCOPE_FIXTURE_ONLY
#include "NativeSummaryTests.cpp"
#include "TrapPriorityObservation.hpp"
#include "HostedH1Report.hpp"
#include "DonorCounselorObservation.hpp"
struct TrapFixture:SummaryRuntimeFixture {
    cfgtrappriority::Policy bundleTrapPriority;
    cfgtrappriority::Slot bundleTrapVictimSlot{},bundleTriggeredTrapSlot{};
    struct {std::uint64_t huntAt=0;}bundleSchedule;
    struct {unsigned bots=1;std::uintptr_t target=0;struct Bot{std::uintptr_t pawn=0;};std::array<Bot,1>roster{};}bundleReport;
    unsigned bundleTargetIndex=1;
    std::uintptr_t trapObject=0;bool known=true;
    bool hosted=false,adopted=true;
    cfgh1::Report hostedH1Report{};
    bool HostedRoutineActive()const{return hosted;}
    struct JasonIdentity {cfgtrappriority::Slot pawn{};};
    bool JasonAdmission(JasonIdentity& out){out.pawn=bundleJason.pawn;return adopted;}
    bool Known(unsigned index,std::uintptr_t& pawn){pawn=known&&index==1?r_.bot:0;return pawn!=0;}
#include "TrapEventObservationMembers.hpp"
    TrapFixture(){
        r_.cap=100000;bundleReport.roster[0].pawn=r_.bot;Serial(r_.bot);
        const auto type=r_.Object();trapObject=r_.Object(type);Serial(trapObject);
        r_.Put(plan.module+cfgtrappriority::disk::TrapReflection,type);r_.Put(plan.module+cfgtrappriority::disk::TrapConstructor,type);
        r_.Put(trapObject+0x110,std::uint8_t{3});r_.Put(trapObject+cfgtrappriority::disk::TrapArmer,bundleJason.pawn.object);
        r_.Put(r_.bot+cfgtrappriority::disk::TriggeredTrap,trapObject);r_.reads=0;r_.cap=1536;
    }
};
#ifndef CFG_TRAP_EVENT_TEST_MAIN
#define CFG_TRAP_EVENT_TEST_MAIN main
#endif
int CFG_TRAP_EVENT_TEST_MAIN(){unsigned peak=0;
    {TrapFixture f;f.hosted=true;f.bundleReport.roster[0].pawn=0;f.bundleReport.target=f.r_.bot;
     CHECK(f.TrapEvent(f.r_.bot,f.trapObject,80)&&f.bundleTrapPriority.index==1&&f.bundleTrapPriority.pending);
     f.r_.reads=0;f.bundleReport.target=0;f.known=false;
     CHECK(!f.TrapEvent(f.r_.bot,f.trapObject,85));}
    for(unsigned fault=0;fault<8;++fault){TrapFixture f;
        CHECK(!cfgprimary::LookupSafe(f.r_,f.plan.module));f.r_.reads=0;
        if(fault==1)++f.plan.threadId;
        if(fault==2)f.r_.Put(f.plan.world+cfgprobe::WorldNetDriver,f.plan.controller);
        if(fault==3)f.r_.Put(f.plan.module+cfgprimary::GarbageCollectionCounter,std::uint32_t{1});
        if(fault==4)f.r_.Put(f.trapObject+cfgtrappriority::disk::TrapArmer,f.r_.bot);
        if(fault==5)f.r_.Put(f.r_.bot+cfgtrappriority::disk::TriggeredTrap,std::uintptr_t{});
        if(fault==6)f.known=false;
        if(fault==7)f.bundleReport.roster[0].pawn=0;
        CHECK(f.TrapEvent(f.r_.bot,f.trapObject,100)==(fault==0));
        CHECK(f.bundleTrapPriority.pending==(fault==0));peak=(std::max)(peak,f.r_.reads);
        CHECK(f.r_.reads<1536&&f.calls_==0&&f.nativeRegistrations==0&&!f.selections&&!f.teleports);
        f.r_.reads=0;CHECK(!cfgprimary::LookupSafe(f.r_,f.plan.module));
        if(!fault){CHECK(f.bundleSchedule.huntAt==100&&f.bundleTrapPriority.index==1&&f.bundleTrapVictimSlot.HasAssignedSerial());}
    }
    std::printf("Actual native trap event adapter with REAL retained gate: %d failures, peak%u/1536; queues owned intent only, no native actions/borrowed tick.\n",failures,peak);
    return failures?1:0;
}
