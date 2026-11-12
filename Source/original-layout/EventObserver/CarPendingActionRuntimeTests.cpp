#define CFG_VEHICLE_STOP_FIXTURE_ONLY
#include "VehicleStopRuntimeMembersTests.cpp"
struct PendingFixture:StopFixture {
    struct Pending {bool moving=false;std::uintptr_t car=0,driver=0,source=0;
        cfgprobe::lifetime::SlotObservation carSlot{},driverSlot{};cfgspawn::Vec3 position{},velocity{};} pending;
    unsigned interactions=0;bool busy=false,sourceCurrent=true,interactionResult=true;
    cfgspawn::Vec3 carPosition{40,0,0};
    bool CarActorBindings(){return true;}bool Manager(){return true;}
    bool Held(std::uintptr_t& held){held=0;return true;}
    bool NativeBool(std::uintptr_t,bool& value){value=busy;return true;}
    bool CarSource(std::uintptr_t,std::uintptr_t){return sourceCurrent;}
    bool Location(std::uintptr_t,cfgspawn::Vec3& value){value=carPosition;return true;}
    bool CarInteraction(std::uintptr_t car,std::uintptr_t driver,cfgspawn::Vec3 position,bool,std::uint64_t now,bool,cfgspawn::Vec3 velocity){
        ++interactions;CHECK(car==fixtureCar&&driver==fixtureProof.driver);
        bool owns=false;return interactionResult&&StopFixture::CarStopBeforeRoad(fixtureProof,position,{240,0,0},velocity,now,owns);
    }
#include "CarPendingActionRuntimeMembers.hpp"
    PendingFixture(){Serial(fixtureCar);Serial(fixtureProof.driver);pending.car=fixtureCar;pending.driver=fixtureProof.driver;
        CHECK(cfgspawn::ObjectSlot(r_,context.objectArray,fixtureCar,pending.carSlot));
        CHECK(cfgspawn::ObjectSlot(r_,context.objectArray,fixtureProof.driver,pending.driverSlot));
        r_.Put(fixtureProof.driver+cfghunter::disk::Health,100.f);
        r_.Put(fixtureProof.driver+cfgescape::disk::bHasEscaped,std::uint8_t{});
        bundleVehicleActions.Select(fixtureCar,fixtureProof.driver);r_.reads=0;}
};
int main(){
 {PendingFixture f;CHECK(f.CarPendingAction(f.pending,5000)&&!f.interactions);}
 {PendingFixture f;f.bundleVehicleActions.extractionCommitted=true;f.bundleVehicleActions.committedAt=1000;
  CHECK(f.CarPendingAction(f.pending,5000)&&f.interactions==1&&f.bundleVehicleActions.extractionCommitted);
  f.r_.reads=0;f.pending.velocity={1000,0,0};CHECK(f.CarPendingAction(f.pending,5100)&&f.interactions==2&&!f.bundleVehicleActions.extractionCommitted);}
 {PendingFixture f;f.bundleVehicleActions.extractionCommitted=true;f.bundleVehicleActions.driver=0x123400;
  CHECK(f.CarPendingAction(f.pending,5000)&&!f.interactions);}
 for(unsigned fault=0;fault<4;++fault){PendingFixture f;f.bundleVehicleActions.extractionCommitted=true;
  if(fault==0)f.sourceCurrent=false;if(fault==1)f.Serial(f.fixtureCar,99);
  if(fault==2)f.r_.Put(f.fixtureProof.driver+cfgescape::disk::bHasEscaped,std::uint8_t{1});if(fault==3)f.busy=true;
  f.CarPendingAction(f.pending,5000);CHECK(!f.interactions);}
 std::printf("Included pending action -> fresh native stop/restart: %d failures; interactions/ownership mocks.\n",failures);return failures?1:0;
}
