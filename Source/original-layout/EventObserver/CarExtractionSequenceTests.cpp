#define CFG_DRIVER_NAV_FIXTURE_ONLY
#include "DriverNavigationRuntimeTests.cpp"
#undef CFG_DRIVER_NAV_FIXTURE_ONLY
#include "VehicleOccupancyObservation.hpp"
#include "../Core/VehicleRoadInterceptPolicy.hpp"
#include "../RuntimeProbe/AiActionBindings.generated.hpp"
struct ExtractionFixture:DriverFixture {
    std::uintptr_t seat=0,hood=0,manager=0;bool current=true,extracting=false,engineStarted=false;
    unsigned seatInputs=0,stops=0,faces=0,moves=0;
    bool hosted=false,adopted=true;
    bool CarActorBindings(){return !hosted||adopted;}
    struct {unsigned carMoves=0,carStage=0;}bundleReport;
    struct {unsigned moves=0;void Renew(std::uint64_t){}}bundleCarPursuit;
    struct {std::uintptr_t car=0,source=0;}bundleCarActionPending;
    cfgvehicleaction::Obstacle bundleCarObstacle;
    struct {template<class R>bool ChaseWaypoint(R&,std::uintptr_t,cfgspawn::Vec3,cfgspawn::Vec3,cfgroad::Candidate&,unsigned){return false;}}bundleRoads;
    bool CarOccupancy(std::uintptr_t car,cfgvehicleoccupancy::Proof& p){
        return cfgvehicleoccupancy::Observe(r_,context.objectArray,car,navType,bundleCounselorClass,p)==cfgvehicleoccupancy::Result::Occupied;
    }
    bool CarOccupancyCurrent(const cfgvehicleoccupancy::Proof& p){return current&&cfgvehicleoccupancy::Current(r_,context.objectArray,navType,bundleCounselorClass,p);}
    bool CarExtraction(bool& active){active=extracting;return true;}
    bool CarDrain(std::uintptr_t,const cfgvehicleoccupancy::Proof&,std::uint64_t,bool& held){held=false;return true;}
    void CarVacant(std::uintptr_t,std::uint64_t){}
    bool CarStopObservation(std::uintptr_t,std::uintptr_t,bool& started,bool& enabled,bool& slammed,float& doorSpeed,float& slamSpeed){
        started=engineStarted;enabled=true;slammed=false;doorSpeed=100;slamSpeed=200;return true;}
    template<class B,std::size_t N>bool CarCall(const B&,std::uintptr_t executor,const std::array<unsigned char,N>&,std::uintptr_t receiver,void* value){
        ++calls_;
        if(executor==cfgtrap::disk::GetActorForwardVectorExecutor){CHECK(receiver==fixtureCar);*static_cast<cfgspawn::Vec3*>(value)={1,0,0};}
        else if(executor==cfgtrap::disk::MoveToLocationExecutor){++moves;auto& p=*static_cast<cfgtrap::MoveLocationParameters*>(value);CHECK(p.pathfinding==1);p.result=2;}
        else if(executor==cfgtrap::disk::StopMovementExecutor)++stops;
        else if(executor==cfgtrap::disk::SetControlRotationExecutor)++faces;
        else if(executor==cfgvehicleinteraction::disk::Executor){++seatInputs;CHECK(receiver==manager&&static_cast<cfgvehicleaction::Attempt*>(value)->component==seat);}
        return true;
    }
#define CFG_VEHICLE_INTERACTION_MEMBER_FIXTURE_ONLY
#define CarInteraction NativeCarInteraction
#pragma warning(push)
#pragma warning(disable:4458)
#include "VehicleActionRuntimeMembers.hpp"
#pragma warning(pop)
#undef CarInteraction
#undef CFG_VEHICLE_INTERACTION_MEMBER_FIXTURE_ONLY
    bool CarInteraction(std::uintptr_t car,std::uintptr_t driver,cfgspawn::Vec3 position,bool moving,
        std::uint64_t now,bool only=false,cfgspawn::Vec3 velocity={}){
        return NativeCarInteraction(car,driver,position,moving,now,only,velocity);
    }
#include "CarPendingActionRuntimeMembers.hpp"
    ExtractionFixture(){r_.cap=100000;seat=r_.Object(navType);hood=r_.Object(navType);Serial(seat);Serial(hood);Serial(r_.bot);
        r_.Put(plan.module+cfgvehicleinteraction::disk::ILLInteractableComponent,navType);
        r_.Put(plan.module+cfgvehicleinteraction::disk::ILLInteractableComponentConstructor,navType);
        r_.Put(plan.module+cfgposition::disk::SceneClassCache,navType);r_.Put(plan.module+cfgposition::disk::SceneConstructorCache,navType);
        CHECK(r_.Read(bundleJason.pawn.object+cfghunter::disk::Manager,manager));Serial(manager);
        r_.Put(fixtureCar+cfgvehicleinteraction::disk::JasonCarInteractComponent,hood);
        r_.Put(fixtureCar+cfgvehicleinteraction::disk::Seats,cfgspawn::PlayerArray{0x9E0000,1,1});r_.Put(0x9E0000,seat);
        for(const auto c:{seat,hood}){r_.Put(c+0x20,fixtureCar);r_.Put(c+0xE0,fixtureCar);}
        r_.Put(seat+cfgvehicleinteraction::disk::ParentVehicle,fixtureCar);
        r_.Put(seat+cfgvehicleoccupancy::disk::DriverSeat,std::uint8_t{1});r_.Put(seat+cfgvehicleinteraction::disk::CounselorInSeat,r_.bot);
        r_.Put(r_.bot+cfgvehicleoccupancy::disk::CurrentSeat,seat);r_.Put(r_.bot+cfgvehicleoccupancy::disk::LeavingSeat,std::uint8_t{});
        r_.Put(r_.bot+cfghunter::disk::Health,100.f);r_.Put(r_.bot+cfgescape::disk::bHasEscaped,std::uint8_t{});
        r_.Put(seat+cfgposition::disk::SceneWorldTranslation,cfgspawn::Vec3{0,-100,0});
        bundleVehicleActions.Select(fixtureCar,r_.bot);bundleVehicleActions.extractionCommitted=true;bundleVehicleActions.committedAt=1000;
        projectOffset={0,50,0};fixtureHere={0,200,100};Fresh();
    }
    bool Step(std::uint64_t now){Fresh();struct Pending {bool moving=false;std::uintptr_t car=0,driver=0,source=cfgvehicle::disk::Car4;
        cfgprobe::lifetime::SlotObservation carSlot{},driverSlot{};cfgspawn::Vec3 position{},velocity{};}pending;
        pending.car=fixtureCar;pending.driver=r_.bot;
        CHECK(cfgspawn::ObjectSlot(r_,context.objectArray,fixtureCar,pending.carSlot));
        CHECK(cfgspawn::ObjectSlot(r_,context.objectArray,r_.bot,pending.driverSlot));
        const bool ok=CarPendingAction(pending,now);CHECK(r_.reads<=1536&&calls_<=12);return ok;}
};
int main(){
 {ExtractionFixture f;f.hosted=true;
  CHECK(f.Step(900)&&f.bundleReport.carMoves>0);
  const auto prior=f.bundleReport.carMoves;f.adopted=false;
  CHECK(!f.Step(950)&&f.bundleReport.carMoves==prior);}
 {ExtractionFixture f;CHECK(f.Step(1000)&&f.moves==1&&!f.seatInputs);f.fixtureHere=f.bundleVehicleActions.route.waypoint;
  CHECK(f.Step(1100)&&f.moves==2);f.fixtureHere=f.bundleDriverNavigation.door;
  CHECK(f.Step(1200)&&f.stops==1&&f.bundleVehicleActions.phase==1);
  CHECK(f.Step(1849)&&!f.faces&&!f.seatInputs);CHECK(f.Step(1850)&&f.faces==1);
  CHECK(f.Step(1950)&&f.seatInputs==1&&f.bundleVehicleActions.seatAttempts==1);}
 for(unsigned fault=0;fault<3;++fault){ExtractionFixture f;f.fixtureHere={0,-160,0};
  CHECK(f.Step(1000)&&f.bundleVehicleActions.phase==1);CHECK(f.Step(1650)&&f.faces==1);
  if(fault==0)f.current=false;if(fault==1)f.r_.Put(f.manager+cfgvehicleinteraction::disk::LockedComponent,f.hood);
  if(fault==2)f.extracting=true;f.Step(1750);CHECK(!f.seatInputs);}
 {ExtractionFixture f;f.engineStarted=true;f.bundleVehicleActions.ResetApproach();f.fixtureHere={0,-160,0};
  CHECK(f.Step(1000)&&f.bundleVehicleActions.extractionCommitted&&f.stops==1);
  // Interruptions no longer expire the responsive continuation.
  CHECK(cfgvehicleaction::ContinuationDelay(f.bundleVehicleActions,false,9000)==100);
  CHECK(f.Step(9000)&&f.faces==1);CHECK(f.Step(9100)&&f.seatInputs==1);}
 std::printf("Actual included extraction: projected corner -> driver door -> Stop ->650ms -> face -> seat; stale/lock/extraction refusal: %d failures; native calls mocked.\n",failures);
 return failures?1:0;
}
