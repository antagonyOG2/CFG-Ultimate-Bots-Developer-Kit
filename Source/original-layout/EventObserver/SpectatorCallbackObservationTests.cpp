#define CFG_RETAINED_GAMEPLAY_BUNDLE
#include "../RuntimeProbe/PrimaryLookupEligibility.hpp"
#define CFG_LABEL_FIXTURE_ONLY
#include "NativeSpectatorLabelTests.cpp"
int main(){
    LabelFixture f;
    CHECK(!cfgprimary::LookupSafe(f.r_,f.plan.module));f.r_.reads=0;CHECK(!f.Context());
    f.r_.reads=0;CHECK(f.NativeJasonLabelProof());
    const auto peak=f.r_.reads;CHECK(f.calls_==0&&!f.teleports&&!f.selections);
    for(const auto direction:{1,-1}){f.r_.reads=0;CHECK(f.SpectatorCycleObservationScope(f.plan.controller,direction));}
    f.r_.reads=0;CHECK(!f.SpectatorCycleObservationScope(f.r_.bot,1));
    f.r_.reads=0;CHECK(!f.SpectatorCycleObservationScope(f.plan.controller,0));
    f.r_.reads=0;CHECK(!f.SpectatorCycleObservationScope(f.plan.controller,2));
    for(unsigned fault=0;fault<4;++fault){
        if(fault==0)++f.plan.threadId;
        if(fault==1)f.r_.Put(f.plan.world+cfgprobe::WorldNetDriver,f.plan.controller);
        if(fault==2)f.r_.Put(f.plan.module+cfgprimary::GarbageCollectionCounter,std::uint32_t{1});
        if(fault==3)f.r_.Put(f.plan.controller+cfgprobe::ControllerPlayer,f.r_.bot);
        f.r_.reads=0;CHECK(!f.NativeJasonLabelProof());
        f.r_.reads=0;CHECK(!f.SpectatorCycleObservationScope(f.plan.controller,1));
        if(fault==0)--f.plan.threadId;
        if(fault==1)f.r_.Put(f.plan.world+cfgprobe::WorldNetDriver,std::uintptr_t{});
        if(fault==2)f.r_.Put(f.plan.module+cfgprimary::GarbageCollectionCounter,std::uint32_t{});
    }
    f.r_.reads=0;CHECK(!cfgprimary::LookupSafe(f.r_,f.plan.module));
    std::printf("Actual spectator callback observations with REAL retained gate: %d failures; label/cycle without borrowed tick, native movement authority still absent, peak%u/1536.\n",failures,peak);
    return failures?1:0;
}
