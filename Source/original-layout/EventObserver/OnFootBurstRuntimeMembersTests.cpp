#define CFG_SPECTATOR_MEMBER_FIXTURE_ONLY
#include "SpectatorRuntimeMembersTests.cpp"
#include "../RuntimeProbe/VehicleOccupancyBindings.generated.hpp"
#include "../RuntimeProbe/SwimActionBindings.hpp"
#include "../Core/OnFootBurstPolicy.hpp"
struct FootFixture:SpectatorFixture {
    using BundleAccess=FootFixture;
    cfgfootburst::Policy bundleFootBurst;cfgprobe::lifetime::SlotObservation bundleFootMovement;
    struct {bool active=false;}bundleCarSpeed;
    struct {std::uintptr_t target=0;std::array<std::uint32_t,2> h285Starts{},h285ChaseStarts{};}bundleReport;float bundleFootBurstSquared=-1.f; // H2.85
    unsigned bundleTargetIndex=0,writes=0;std::uintptr_t fixtureMovement=0;float range=900.f*900.f;
    bool nativeBusy=false;
    bool Known(unsigned,std::uintptr_t& out){out=r_.bot;return true;}
    bool Alive(std::uintptr_t target,bool& living){living=target==r_.bot;return living;}
    bool Distance(std::uintptr_t,float& squared){squared=range;return true;}
    bool NativeBool(std::uintptr_t,bool& value){++calls_;value=nativeBusy;return true;}
    bool GrabBusy(bool& value){value=nativeBusy;return true;}
    bool Held(std::uintptr_t& value){value=0;return true;}
    bool CarSpeedWrite(std::uintptr_t at,float before,float after){
        float current=0;if(!r_.Read(at,current)||current!=before)return false;r_.Put(at,after);++writes;return true;
    }
#include "OnFootBurstRuntimeMembers.hpp"
    FootFixture(){
        r_.cap=100000;const auto type=r_.Object();fixtureMovement=r_.Object(type);Serial(fixtureMovement);
        const auto& binding=cfghook::disk::Types[cfghook::disk::Movement];
        r_.Put(plan.module+binding.reflection,type);r_.Put(plan.module+binding.constructor,type);
        r_.Put(fixtureMovement+0x20,bundleJason.pawn.object);r_.Put(bundleJason.pawn.object+cfghook::disk::MovementComponent,fixtureMovement);
        for(const auto field:{cfghook::disk::Walk,cfghook::disk::Sprint,cfghook::disk::Run,cfghook::disk::SlowRun})r_.Put(fixtureMovement+field,450.f);
        r_.Put(r_.bot+cfgvehicleoccupancy::disk::CurrentSeat,std::uintptr_t{});
        r_.Put(bundleJason.pawn.object+cfgswim::disk::WaterVolume,std::uintptr_t{});
        r_.Put(plan.module+cfghunter::disk::Stunned,cfghunter::disk::StunnedBytes);
        r_.Put(plan.module+cfghunter::disk::Attacking,cfghunter::disk::AttackingBytes);
        bundleReport.target=r_.bot;r_.reads=0;r_.cap=1536;
    }
    void Fresh(){r_.reads=0;calls_=0;}
};
#ifndef CFG_FOOT_BURST_FIXTURE_ONLY
int main(){unsigned peak=0;
    {FootFixture f;CHECK(f.FootBurstBegin(100)&&f.bundleFootBurst.Owned()&&!f.writes&&f.calls_==2);peak=f.r_.reads;
     for(unsigned i=0;i<4;++i){f.Fresh();CHECK(f.FootBurstService(101+i,false));peak=(std::max)(peak,f.r_.reads);CHECK(f.calls_==0);}
     float run=0;CHECK(f.r_.Read(f.fixtureMovement+cfghook::disk::Run,run)&&run==2250&&f.writes==4);
     for(unsigned i=0;i<4;++i){f.Fresh();CHECK(f.FootBurstService(4000+i,true));peak=(std::max)(peak,f.r_.reads);}
     CHECK(!f.bundleFootBurst.Owned()&&f.writes==8&&f.r_.Read(f.fixtureMovement+cfghook::disk::Run,run)&&run==450);}
    for(unsigned fault=0;fault<8;++fault){FootFixture f;
        if(fault==0)f.bundleCarSpeed.active=true;
        if(fault==1)f.r_.Put(f.fixtureMovement+0x20,f.plan.controller);
        if(fault==2)f.Serial(f.fixtureMovement,0);
        if(fault==3)f.r_.Put(f.plan.world+cfgprobe::WorldNetDriver,f.plan.controller);
        if(fault==4)f.r_.Put(f.r_.bot+cfgvehicleoccupancy::disk::CurrentSeat,f.plan.controller);
        if(fault==5)f.nativeBusy=true;
        if(fault==6)f.range=100;
        if(fault==7)f.r_.cap=1;
        f.FootBurstBegin(100);CHECK(!f.bundleFootBurst.Owned()&&!f.writes);
    }
    {FootFixture f;CHECK(f.FootBurstBegin(100));
     for(unsigned i=0;i<4;++i){f.Fresh();CHECK(f.FootBurstService(101+i,false));}
     for(const auto now:{1100ULL,2200ULL,3099ULL}){f.Fresh();CHECK(f.FootBurstVehicleTurn(now));
         CHECK(f.bundleFootBurst.stage==cfgfootburst::Stage::Hold&&f.writes==4);
         peak=(std::max)(peak,f.r_.reads);CHECK(f.r_.reads<=1536);}
     for(unsigned i=0;i<4;++i){f.Fresh();CHECK(f.FootBurstVehicleTurn(3100+i));}
     CHECK(!f.bundleFootBurst.Owned()&&f.writes==8);
     f.Fresh();CHECK(!f.FootBurstVehicleTurn(3200)&&!f.r_.reads);}
    {FootFixture f;CHECK(f.FootBurstBegin(100));
     for(unsigned i=0;i<4;++i){f.Fresh();CHECK(f.FootBurstService(101+i,false));}
     f.r_.Put(f.r_.bot+cfgvehicleoccupancy::disk::CurrentSeat,f.plan.controller);
     f.Fresh();CHECK(f.FootBurstVehicleTurn(1100));
     CHECK(f.bundleFootBurst.stage==cfgfootburst::Stage::Restore&&f.writes==5);}
    CHECK(peak<=1536);std::printf("Composed on-foot burst movement proof: %d failures,peak%u/1536; synthetic,OS CAS replaced only.\n",failures,peak);
    return failures?1:0;
}
#endif
