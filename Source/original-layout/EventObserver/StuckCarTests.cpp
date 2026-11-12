#define CFG_VEHICLE_STOP_FIXTURE_ONLY
#include "VehicleStopRuntimeMembersTests.cpp"
#undef CFG_VEHICLE_STOP_FIXTURE_ONLY
#include "../Core/StuckOccupiedCarPolicy.hpp"
namespace cfgclosedtrace {template<class A,class B>void Mark(const char*,A,B){}}
struct StuckFixture:StopFixture {
    cfgstuckcar::Runtime bundleStuckCars;bool lastMoving=true;cfgspawn::Vec3 lastVelocity{};unsigned interactions=0;
    bool NativeCarStopBeforeRoad(const cfgvehicleoccupancy::Proof& p,cfgspawn::Vec3 car,cfgspawn::Vec3 jason,cfgspawn::Vec3 v,std::uint64_t now,bool& owns){
        return StopFixture::CarStopBeforeRoad(p,car,jason,v,now,owns);
    }
    bool NativeCarInteraction(std::uintptr_t,std::uintptr_t,cfgspawn::Vec3,bool moving,std::uint64_t,bool,cfgspawn::Vec3 v){
        ++interactions;lastMoving=moving;lastVelocity=v;return true; // frozen native dispatch is independently tested
    }
#include "StuckCarRuntimeMembers.hpp"
    StuckFixture(){Serial(fixtureCar);Serial(fixtureProof.driver);CHECK(cfgspawn::ObjectSlot(r_,context.objectArray,fixtureCar,fixtureProof.carSlot));}
    bool Sample(std::uint64_t now,cfgspawn::Vec3 p={}){r_.reads=0;bool owns=false;
        CHECK(CarStopBeforeRoad(fixtureProof,p,{240,0,0},{1000,0,0},now,owns));return owns;}
};
int main(){
    {cfgstuckcar::Clock c;for(unsigned i=0;i<3;++i)CHECK(!c.Observe(1,1,2,2,{float(i*10),0,0},1000+i*1000));
     CHECK(c.Observe(1,1,2,2,{20,0,0},4000));CHECK(!c.Ready(1,1,2,3,{20,0,0},4100));
     CHECK(!c.Ready(1,1,2,2,{20,0,0},7501));CHECK(!c.Observe(1,1,2,2,{101,0,0},5000));}
    {cfgstuckcar::Clock c;CHECK(!c.Observe(1,1,2,2,{},1000));CHECK(!c.Observe(1,1,2,2,{},3000));
     CHECK(c.Observe(1,1,2,2,{},5000));CHECK(c.Ready(1,1,2,2,{},7000));} // two-car round robin, not every second
    {cfgstuckcar::Clock c;for(unsigned i=0;i<8;++i)CHECK(!c.Observe(1,1,2,2,{float(i*100),0,0},1000+i*1000));
     CHECK(!c.Observe(1,1,2,0,{},10000));}
    {StuckFixture f;CHECK(!f.Sample(1000));CHECK(!f.Sample(2000));CHECK(!f.Sample(3000));
     CHECK(f.Sample(4000)&&f.bundleVehicleActions.extractionCommitted&&f.fixtureStarted);
     CHECK(f.CarInteraction(f.fixtureCar,f.fixtureProof.driver,{},true,4100,false,{1000,0,0}));
     CHECK(f.interactions==1&&!f.lastMoving&&f.lastVelocity.x==0&&f.fixtureStarted);
     CHECK(f.Sample(5000)&&f.bundleVehicleActions.committedAt==4000); // no re-commit/route-reset loop
     CHECK(!f.Sample(6000,{200,0,0})&&!f.bundleVehicleActions.extractionCommitted);}
    {StuckFixture f;for(unsigned i=0;i<4;++i)f.Sample(1000+i*1000);f.Serial(f.fixtureProof.driver,99);
     CHECK(f.CarInteraction(f.fixtureCar,f.fixtureProof.driver,{},true,4100,false,{1000,0,0})&&f.lastMoving);}
    std::printf("Measured stalled running-car driver extraction: %d failures; frozen stop logic included, interaction dispatch mocked.\n",failures);
    return failures?1:0;
}
