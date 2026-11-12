#define CFG_VEHICLE_STOP_FIXTURE_ONLY
#include "VehicleStopRuntimeMembersTests.cpp"
#include "../Core/VehicleProgressPolicy.hpp"
namespace cfgclosedtrace {template<class A,class B>void Mark(const char*,A,B){}}
struct ProgressFixture:StopFixture {
    cfgvehicleprogress::Runtime bundleVehicleProgress;
    struct {unsigned bots=0;std::uintptr_t target=0;}bundleReport;
    struct {void Clear(){}}bundleFollowPath,bundlePathCooldown,bundlePursuit;
    struct {}bundleRoadMorph,bundleCarActionPending;
    unsigned bundleTargetIndex=0;std::uint64_t bundleTargetLockUntil=0,bundleMoveAt=0;
    bool scope=true,alternative=false;unsigned retired=0;
    bool Context(){return scope;}bool Jason(){return scope;}
    bool CarSource(std::uintptr_t,std::uintptr_t car){return car==fixtureCar;}
    bool CarOccupancy(std::uintptr_t,cfgvehicleoccupancy::Proof& p){p=fixtureProof;return true;}
    bool Known(unsigned index,std::uintptr_t& candidate){candidate=index?plan.pawn:fixtureProof.driver;return index==0||alternative;}
    bool Alive(std::uintptr_t,bool& alive){alive=true;return true;}
    bool Manager(){return true;}bool Held(std::uintptr_t& p){p=0;return true;}
    bool CarExtraction(bool& value){value=false;return true;}
    void CarRetire(std::uint64_t){++retired;}
#include "VehicleProgressRuntimeMembers.hpp"
    ProgressFixture(){Serial(fixtureCar);Serial(fixtureProof.driver);
        CHECK(cfgspawn::ObjectSlot(r_,context.objectArray,fixtureCar,fixtureProof.carSlot));}
    bool Admit(std::uint64_t now){cfgprobe::lifetime::SlotObservation d{};
        CHECK(cfgspawn::ObjectSlot(r_,context.objectArray,fixtureProof.driver,d));
        const bool search=bundleVehicleProgress.cars[0].Observe(fixtureCar,fixtureProof.carSlot.serial,fixtureProof.driver,d.serial,8841.57f,now);
        if(search){bundleVehicleProgress.pending=1;bundleVehicleProgress.cursor=0;bundleVehicleProgress.next=now;}return search;}
};
int main(){
 {ProgressFixture f;CHECK(!f.Admit(1000)&&f.Admit(13000));
  CHECK(f.VehicleProgressPhase(13000)&&f.bundleVehicleProgress.pending==1);
  CHECK(f.VehicleProgressPhase(13100)&&!f.bundleVehicleProgress.pending);
  CHECK(f.bundleVehicleProgress.cars[0].deadline==25100&&!f.bundleVehicleProgress.cars[0].ignoredUntil);
  CHECK(!f.Admit(13200)&&!f.retired); // production admission can reach road/stop actions again
  CHECK(f.Admit(25100));}
 {ProgressFixture f;CHECK(!f.Admit(1000)&&f.Admit(13000));f.scope=false;
  CHECK(f.VehicleProgressPhase(13000)&&!f.bundleVehicleProgress.pending);
  CHECK(f.bundleVehicleProgress.cars[0].deadline==13000);}
 {ProgressFixture f;CHECK(!f.Admit(1000)&&f.Admit(13000));f.bundleVehicleActions.extractionCommitted=true;
  CHECK(f.VehicleProgressPhase(13000)&&!f.bundleVehicleProgress.pending&&!f.retired);}
 std::printf("Included recovery search + caller admission: %d failures; native reads mocked.\n",failures);return failures?1:0;
}
