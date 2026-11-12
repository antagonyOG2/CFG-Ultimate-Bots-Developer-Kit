#define CFG_SPECTATOR_MEMBER_FIXTURE_ONLY
#include "SpectatorRuntimeMembersTests.cpp"
struct HelperOwnerFixture:SpectatorFixture {
    bool living=true,contextOkay=true,known=true;
    bool Context(){context=plan;return contextOkay;}
    bool Known(unsigned index,std::uintptr_t& pawn){pawn=bundleHumanTarget.object;return !index&&known;}
    bool Alive(std::uintptr_t,bool& value){value=living;return true;}
#include "CurrentHelperOwnerMembers.hpp"
    HelperOwnerFixture(){r_.cap=100000;CHECK(cfgspawn::ObjectSlot(r_,plan.objectArray,r_.bot,bundleHumanTarget));
        r_.Put(plan.controller+cfgprobe::ControllerPawn,r_.bot);r_.Put(r_.bot+cfgescape::disk::Controller,plan.controller);
        r_.reads=0;r_.cap=1536;}
};
int main(){
    HelperOwnerFixture good;CHECK(good.plan.pawn!=good.bundleHumanTarget.object);CHECK(good.HelperContext());
    CHECK(good.r_.reads<=1536&&good.helperContextStage_==0);
    for(unsigned fault=0;fault<6;++fault){HelperOwnerFixture f;
        if(fault==0)f.contextOkay=false;if(fault==1)f.living=false;if(fault==2)f.known=false;
        if(fault==3)f.r_.Put(f.plan.controller+cfgprobe::ControllerPawn,f.plan.pawn);
        if(fault==4)f.r_.Put(f.r_.bot+cfgescape::disk::Controller,std::uintptr_t{});
        if(fault==5)f.Serial(f.r_.bot,999);
        CHECK(!f.HelperContext()&&f.helperContextStage_>=1&&f.helperContextStage_<=8);
        if(fault==0)CHECK(f.helperContextStage_==1);
        if(fault==1)CHECK(f.helperContextStage_==8);
        if(fault==2)CHECK(f.helperContextStage_==3);}
    std::printf("Current living local counselor F3 scope: %d failures; original plan unchanged; no game access.\n",failures);
    return failures?1:0;
}
