#define CFG_RETAINED_GAMEPLAY_BUNDLE
// Compile the REAL retained eligibility gate before the old sparse fixture
// enables its generic observation mode. No completed-tick capability is active.
#include "../RuntimeProbe/PrimaryLookupEligibility.hpp"
#define CFG_SUMMARY_SCOPE_FIXTURE_ONLY
#include "NativeSummaryTests.cpp"
int main(){
    SummaryRuntimeFixture f;using R=decltype(f.r_);
    CHECK(!cfgprimary::LookupSafe(f.r_,f.plan.module));
    f.r_.reads=0;CHECK(f.SummaryObservationScope());
    f.r_.reads=0;CHECK(!f.SummaryScope()&&!f.NativeEnvironmentAllowed());
    {auto module=f.plan.module;cfgprimary::ScopedLookupEligibility<R> scope(module,&f,[](R&,void*){return true;},true);
     CHECK(scope.Entered());f.r_.reads=0;CHECK(f.SummaryPhase(100)&&f.nativeRegistrations==1);}
    CHECK(!cfgprimary::LookupSafe(f.r_,f.plan.module));
    std::uintptr_t row=0;CHECK(f.r_.Read(0x980000,row));
    f.r_.reads=0;CHECK(f.SummaryRowReady(row));
    f.r_.reads=0;CHECK(f.SummaryPresentedState(row,f.state));
    const auto before=f.nativeRegistrations;f.r_.reads=0;(void)f.SummaryPhase(200);CHECK(f.nativeRegistrations==before);
    f.r_.reads=0;CHECK(!f.NativeEnvironmentAllowed());
    for(unsigned fault=0;fault<4;++fault){
        if(fault==0)++f.plan.threadId;
        if(fault==1)f.r_.Put(f.plan.world+cfgprobe::WorldNetDriver,f.plan.controller);
        if(fault==2)f.r_.Put(f.plan.module+cfgprimary::GarbageCollectionCounter,std::uint32_t{1});
        if(fault==3)f.r_.Put(row+cfgsummary::disk::RowPlayerState,f.plan.controller);
        f.r_.reads=0;CHECK(!f.SummaryRowReady(row));
        if(fault==0)--f.plan.threadId;
        if(fault==1)f.r_.Put(f.plan.world+cfgprobe::WorldNetDriver,std::uintptr_t{});
        if(fault==2)f.r_.Put(f.plan.module+cfgprimary::GarbageCollectionCounter,std::uint32_t{});
    }
    std::printf("Actual summary callbacks with REAL retained LookupSafe gate: %d failures; owned row observation without tick, native registration still refused, thread/world/GC/state guards retained.\n",failures);
    return failures?1:0;
}
