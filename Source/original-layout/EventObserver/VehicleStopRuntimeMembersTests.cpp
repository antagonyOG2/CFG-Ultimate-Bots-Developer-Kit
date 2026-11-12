#define CFG_SPECTATOR_MEMBER_FIXTURE_ONLY
#include "SpectatorRuntimeMembersTests.cpp"
#include "VehicleOccupancyObservation.hpp"
#include "../Core/VehicleActionPolicy.hpp"
struct StopFixture : SpectatorFixture {
    cfgvehicleaction::State bundleVehicleActions;
    cfgvehicleoccupancy::Proof fixtureProof{};
    bool fixtureStarted=true,fixtureEnabled=true,fixtureSlammed=false,stopReadable=true,occupancyCurrent=true;
    unsigned stopReads=0,occupancyChecks=0;
    float speedLimit=100,slamLimit=200;
    std::uintptr_t fixtureCar=0,fixtureHood=0;
    bool CarStopObservation(std::uintptr_t c,std::uintptr_t h,bool& start,bool& on,bool& slam,float& ds,float& ss){
        ++stopReads;if(!stopReadable||c!=fixtureCar||h!=fixtureHood)return false;
        start=fixtureStarted;on=fixtureEnabled;slam=fixtureSlammed;ds=speedLimit;ss=slamLimit;return true;
    }
    bool CarOccupancyCurrent(const cfgvehicleoccupancy::Proof& p){++occupancyChecks;return occupancyCurrent&&p==fixtureProof;}
#include "VehicleStopRuntimeMembers.hpp"
    StopFixture(){fixtureCar=r_.Object();fixtureHood=r_.Object();
        r_.Put(fixtureCar+cfgvehicleinteraction::disk::JasonCarInteractComponent,fixtureHood);r_.Put(fixtureHood+0xE0,fixtureCar);
        fixtureProof.result=cfgvehicleoccupancy::Result::Occupied;fixtureProof.driver=r_.bot;
        CHECK(cfgspawn::ObjectSlot(r_,context.objectArray,fixtureCar,fixtureProof.carSlot));r_.reads=0;r_.cap=1536;calls_=0;}
    bool Step(bool& owns,std::uint64_t now=1000){return CarStopBeforeRoad(fixtureProof,{0,0,0},{240,0,0},{1000,0,0},now,owns);}
};
#ifndef CFG_VEHICLE_STOP_FIXTURE_ONLY
int main(){
    {StopFixture f;bool owns=true;CHECK(f.Step(owns)&&!owns&&f.stopReads==1&&f.occupancyChecks==1&&!f.calls_);
     f.fixtureSlammed=true;CHECK(f.Step(owns,1100)&&owns&&f.bundleVehicleActions.extractionCommitted);
     // Projection succeeded earlier, but a fresh slam now cancels teleport.
     CHECK(f.Step(owns,4000)&&owns&&f.bundleVehicleActions.extractionCommitted);
     f.fixtureSlammed=false;f.fixtureStarted=false;CHECK(f.Step(owns,5000)&&owns);
     f.fixtureStarted=true;CHECK(f.Step(owns,6000)&&!owns&&!f.bundleVehicleActions.extractionCommitted);}
    {StopFixture f;bool owns=false;f.bundleVehicleActions.Select(f.fixtureCar,f.fixtureProof.driver);
     f.bundleVehicleActions.HoodDispatched(false,900);CHECK(f.Step(owns)&&owns);}
    {StopFixture f;bool owns=false;f.fixtureStarted=false;CHECK(f.Step(owns)&&owns&&!f.bundleVehicleActions.extractionCommitted);
     f.r_.reads=0;CHECK(f.CarStopBeforeRoad(f.fixtureProof,{},{240,0,0},{},1100,owns)&&owns&&f.bundleVehicleActions.extractionCommitted);}
    {StopFixture f;bool owns=false;f.bundleVehicleActions.Select(0x123000,f.fixtureProof.driver);
     f.bundleVehicleActions.extractionCommitted=true;CHECK(f.Step(owns)&&owns&&f.bundleVehicleActions.car==0x123000);}
    for(unsigned fault=0;fault<7;++fault){StopFixture f;bool owns=true;
        if(fault==0)f.fixtureProof.result=cfgvehicleoccupancy::Result::Empty;
        if(fault==1)f.fixtureProof.result=cfgvehicleoccupancy::Result::Exiting;
        if(fault==2)f.stopReadable=false;
        if(fault==3)f.occupancyCurrent=false;
        if(fault==4)f.r_.Put(f.fixtureHood+0xE0,f.plan.controller);
        if(fault==5)f.speedLimit=NAN;
        if(fault==6)f.r_.cap=1;
        CHECK(!f.Step(owns)&&!owns&&!f.bundleVehicleActions.extractionCommitted&&!f.calls_);}
    {StopFixture f;bool owns=true;CHECK(!f.CarStopBeforeRoad(f.fixtureProof,{NAN,0,0},{},{},1000,owns)&&!owns);}
    std::printf("Included stop-before-road adapter: %d failures; fresh stop/occupancy guards, no native calls.\n",failures);
    return failures?1:0;
}
#endif
