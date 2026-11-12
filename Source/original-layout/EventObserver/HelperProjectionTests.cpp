#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#define main ProjectionSparseFixtureMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
#include "OfflineContextObservation.hpp"
#include "../Core/HelperSpawnPolicy.hpp"
#include "../Core/RandomCounselorPolicy.hpp"
#include "../Core/VehiclePursuitPolicy.hpp"
#include "../RuntimeProbe/VehicleBindings.generated.hpp"
namespace cfgcounselorstart {struct Marker{std::uintptr_t actor=0;};}
struct ProjectionFixture {
    using BundleAccess=ProjectionFixture;
    Fixture r_;cfgobserver::Plan plan{},context{};
    cfgbundle::HelperStartup bundleHelpers;
    struct {unsigned helperPreparation=0,bots=2;} bundleReport;
    struct {std::uint64_t spawnAt=0;} bundleSchedule;
    cfgbundle::RandomCounselors bundleRandomCounselors;
    unsigned bundleHelperLocationAttempts=0,bundleHelperStart=0,calls=0,resourceCalls=0,classCalls=0;
    bool scope=true,nearStart=false;std::uintptr_t fixtureOwner=0,fixtureCDO=0;
    bool Charge(unsigned n){for(unsigned i=0;i<n;++i){std::uint32_t v=0;if(!r_.Read(0xE00000,v))return false;}return true;}
    struct Starts{unsigned visits=0;bool rejectSecond=false;unsigned Count()const{return 13;}
        bool Current(Fixture&,ProjectionFixture& host,unsigned,cfgcounselorstart::Marker& out){
            ++visits;if(!host.Charge(800)||(rejectSecond&&visits==2))return false;out.actor=0x600000;return true;}
    } bundleCounselorStarts;
    ProjectionFixture(){r_.cap=100000;r_.Put(0xE00000,std::uint32_t{});
        fixtureOwner=r_.Object();fixtureCDO=r_.Object(fixtureOwner);r_.Put(fixtureOwner+cfgspawn::ClassDefaultObject,fixtureCDO);
        plan.world=0x120000;context.objectArray=r_.objects;r_.reads=0;r_.cap=1536;}
    bool HelperContext(){return Charge(140)&&scope;}
    unsigned Reads()const{return r_.reads;}unsigned Calls()const{return calls;}
    bool Current(){return HelperContext();}
    bool Known(unsigned,std::uintptr_t& pawn){pawn=0x500000;return Charge(20);}
    bool Location(std::uintptr_t pawn,cfgspawn::Vec3& point){point=pawn==0x500000||nearStart?cfgspawn::Vec3{0,0,90}:cfgspawn::Vec3{2000,2000,90};return Charge(80);}
    bool FeatureFunction(const cfgvehicle::disk::Binding&,std::uintptr_t& function,std::uintptr_t& type){function=1;type=fixtureOwner;return Charge(60);}
    bool Invoke(std::uintptr_t object,std::uintptr_t function,void* raw,bool){
        if(object!=fixtureCDO||function!=1)return false;++calls;auto& p=*static_cast<cfgvehicle::Projection*>(raw);
        p.result=1;p.projected=p.point;p.projected.z-=25;return true;}
    bool HelperResources(){++resourceCalls;return Charge(500);}
    bool HelperClassCurrent(){++classCalls;return Charge(350);}
#include "HelperMarkerProofRuntimeMembers.hpp"
#include "HelperProjectionRuntimeMembers.hpp"
};
int main(){unsigned failed=0;auto check=[&](bool v){if(!v)++failed;};
    {ProjectionFixture f;f.ProjectHelperStart();
     check(f.bundleHelpers.stage==cfgbundle::HelperStage::Spawn&&f.calls==1&&f.r_.reads<1536);
     check(!f.resourceCalls&&!f.classCalls&&f.bundleHelpers.location.z==90);
     check(f.helperMarkerProofReads_==800&&f.bundleCounselorStarts.visits==2);
     std::printf("Included helper projection budget fixture:%u/1536 reads; native/source/location helpers mocked.\n",f.r_.reads);}
    {ProjectionFixture f;f.bundleCounselorStarts.rejectSecond=true;f.ProjectHelperStart();check(!f.calls&&f.bundleHelpers.stage==cfgbundle::HelperStage::Unavailable);}
    {ProjectionFixture f;f.scope=false;f.ProjectHelperStart();check(!f.calls);}
    {ProjectionFixture f;f.nearStart=true;f.ProjectHelperStart();check(!f.calls&&f.bundleHelpers.Active()&&f.bundleSchedule.spawnAt);}
    std::printf("Helper projection independent callback:%u failures; no actor allocation.\n",failed);return failed?1:0;
}
