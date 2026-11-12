#define CFG_FOOT_BURST_FIXTURE_ONLY
#include "OnFootBurstRuntimeMembersTests.cpp"
#include "../Core/CounselorPursuitPolicy.hpp"
#include "../Core/CabinBoundsPolicy.hpp"
#include "../RuntimeProbe/TrapPriorityBindings.generated.hpp"
namespace cfgmorphfixturetrace {template<class A,class B>void Mark(const char*,A,B){}}
#define cfgclosedtrace cfgmorphfixturetrace
struct MorphFixture:FootFixture {
    struct {std::uintptr_t target=0;unsigned pursuitChecks=0,pursuitProjections=0,pursuitAttempts=0,pursuitReturns=0;}bundleReport;
    struct {bool Active(std::uint64_t){return false;}bool morphed=false;std::uintptr_t victim=0;}bundleTrapPriority;
    cfgprobe::lifetime::SlotObservation bundleTriggeredTrapSlot;
    cfgpursuit::Progress bundlePursuitProgress;cfgpursuit::Ring bundlePursuit;
    cfgvehicle::Cooldown bundleCarMorph;
    struct {cfgcabin::Cache cache;}bundleCabins;
    struct {void Clear(){}}bundleFollowPath,bundlePathCooldown;
    struct {std::uint64_t traversalUntil=0;bool breaking=false;unsigned phase=0;}bundleDoorRecovery;
    struct {}bundleDoorBreakSelection;
    struct {std::uint64_t huntAt=0;}bundleSchedule;
    std::uint64_t bundleMoveAt=0;std::uintptr_t bundleCounselorMorphTarget=0,navType=0,nav=0;
    cfgspawn::Vec3 fixtureHere{},fixtureThere{3000,0,100};bool outside=true,contextValid=true;
    unsigned projections=0,teleports_=0;bool projectionOkay=true;
    bool Context(){return contextValid&&FootFixture::Context();}
    bool Manager(){return true;}bool NativeBindings(){return true;}
    bool Location(std::uintptr_t actor,cfgspawn::Vec3& result){++calls_;result=actor==bundleJason.pawn.object?fixtureHere:fixtureThere;return true;}
    bool Distance(std::uintptr_t,float& result){result=(fixtureThere.x-fixtureHere.x)*(fixtureThere.x-fixtureHere.x)+(fixtureThere.y-fixtureHere.y)*(fixtureThere.y-fixtureHere.y);return true;}
    bool ExteriorLanding(cfgspawn::Vec3 p){return outside&&bundleCabins.cache.Allows(plan.world,p);}
    bool TeleportFunction(std::uintptr_t& fn){fn=0x456700;return true;}
    bool FeatureFunction(const cfgvehicle::disk::Binding&,std::uintptr_t& fn,std::uintptr_t& type){fn=0x123400;type=navType;return true;}
    bool Invoke(std::uintptr_t,std::uintptr_t fn,void* data,bool=true){++calls_;
        if(fn==0x123400){auto& p=*static_cast<cfgvehicle::Projection*>(data);p.projected=p.point;p.result=projectionOkay?1u:0u;++projections;return true;}
        if(fn==0x456700){auto& p=*static_cast<cfgpursuit::TeleportPacket*>(data);fixtureHere=p.location;p.result=1;++teleports_;return true;}
        return false;
    }
#include "CounselorMorphRuntimeMembers.hpp"
    MorphFixture(){r_.cap=100000;navType=r_.Object();nav=r_.Object(navType);Serial(nav);
        r_.Put(plan.world+cfgtrap::disk::WorldNavigationSystem,nav);
        bundleCabins.cache.world=plan.world;bundleCabins.cache.stage=cfgcabin::Stage::Ready;
        r_.Put(r_.bot+cfgswim::disk::WaterVolume,std::uintptr_t{});
        bundleReport.target=r_.bot;Fresh();r_.cap=1536;}
};
#undef cfgclosedtrace
int main(){unsigned peak=0;
 {MorphFixture f;const cfgspawn::Vec3 guard{2000,700,100};
  f.Fresh();CHECK(f.CounselorMorphService(10000,f.r_.bot,&guard)&&f.projections==1&&!f.teleports_&&f.bundlePursuit.exact);
  CHECK(f.bundlePursuit.surface.x==guard.x&&f.bundlePursuit.surface.y==guard.y);
  f.Fresh();CHECK(f.CounselorMorphService(10100,f.r_.bot,&guard)&&f.teleports_==1&&f.bundleReport.pursuitReturns==1);
  CHECK(f.fixtureHere.x==guard.x&&f.fixtureHere.y==guard.y&&f.bundleCarMorph.next>10100);}
 {MorphFixture f;const cfgspawn::Vec3 guard{2000,700,100};f.projectionOkay=false;
  f.Fresh();CHECK(f.CounselorMorphService(10000,f.r_.bot,&guard)&&f.projections==1&&!f.teleports_&&!f.bundlePursuit.target&&!f.bundleCarMorph.next);}
 const auto step=[&](MorphFixture& f,std::uint64_t now){f.Fresh();f.bundleCounselorMorphTarget=0;
    const bool result=f.CounselorMorphService(now,f.r_.bot);peak=(std::max)(peak,f.r_.reads);
    CHECK(f.r_.reads<=1536&&f.calls_<=12);return result;};
 {MorphFixture f;CHECK(step(f,10000)&&f.projections==1&&!f.teleports_&&f.bundleCounselorMorphTarget==f.r_.bot);
  CHECK(step(f,10100)&&f.teleports_==1&&f.bundleReport.pursuitReturns==1&&!f.bundleCounselorMorphTarget);}
 {MorphFixture f;f.fixtureThere={500,0,100};f.bundleCabins.cache.count=f.bundleCabins.cache.captured=1;
  f.bundleCabins.cache.bounds[0]={{0,0,100},{100,100,150}};f.fixtureHere={0,0,100};
  for(unsigned i=0;i<4;++i){CHECK(step(f,10000+i*2000));CHECK(!f.projections&&!f.teleports_);}
  CHECK(step(f,18000)&&f.projections==1);CHECK(step(f,18100)&&f.teleports_==1);}
 for(unsigned fault=0;fault<6;++fault){MorphFixture f;
  if(fault==0)f.contextValid=false;
  if(fault==1)f.nativeBusy=true;
  if(fault==2)f.r_.Put(f.r_.bot+cfgvehicleoccupancy::disk::CurrentSeat,f.plan.pawn);
  if(fault==3)f.bundleCabins.cache.stage=cfgcabin::Stage::Capture;
  if(fault==4)f.bundleDoorRecovery.traversalUntil=20000;
  if(fault==5)f.outside=false;
  for(unsigned i=0;i<2;++i)step(f,10000+100*i);
  CHECK(!f.teleports_);}
 std::printf("Included counselor exterior Morph/structure-stall service:%d failures,peak%u/1536; native calls mocked.\n",failures,peak);
 return failures?1:0;
}
