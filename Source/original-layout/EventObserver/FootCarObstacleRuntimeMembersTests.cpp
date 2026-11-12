#define CFG_FOOT_BURST_FIXTURE_ONLY
#include "OnFootBurstRuntimeMembersTests.cpp"
#include "../Core/OnFootCarObstaclePolicy.hpp"
#include "../Core/NativeFunctionFrame.hpp"
#include "../RuntimeProbe/VehicleInteractionBindings.generated.hpp"
#include "TrapNativeParameters.hpp"
#include "OfflineDiagnosticTrace.hpp"
// Milestone log is replaced; this fixture never creates a game/log file.
#define cfgclosedtrace cfgfootobstaclefixturetrace
namespace cfgfootobstaclefixturetrace {inline void Mark(const char*,std::uintptr_t,std::uint64_t){}}
struct FootCarFixture:FootFixture {
    cfgfootcar::Obstacle bundleFootObstacle;
    cfgfootcar::RouteHistory bundleFootObstacleHistory;
    cfgprobe::lifetime::SlotObservation bundleFootObstacleCarSlot{};
    bool bundleFootObstacleAlternate=false;
    std::uintptr_t bundleFootObstacleTarget=0;
    struct {std::uint64_t huntAt=0;}bundleSchedule;
    struct {void Clear(){}}bundleFollowPath;
    cfgbundle::MoveCooldown bundlePathCooldown;
    std::uint64_t bundleMoveAt=0,bundleFootObstacleCheckAt=0;
    cfgspawn::Vec3 fixtureHere{0,-200,100},fixtureGoal{0,200,100},carPoint{0,0,0};
    std::uintptr_t fixtureCar=0,vehicleType=0,navType=0,nav=0;bool fixtureSource=true,holding=false;
    unsigned moveCalls=0,stopCalls=0,headingCalls=0;std::uint8_t projectResult=1,moveResult=2;
    bool moveReturns=true,stopReturns=true;
    unsigned projectCalls=0;bool terrainProjection=false;cfgspawn::Vec3 projectOffset{};
    unsigned Reads()const{return r_.reads;}
    bool FootObstacleBindings(){return true;} // independently pinned by production native-binding suites
    bool Manager(){return true;}
    bool Held(std::uintptr_t& held){held=holding?plan.pawn:0;return true;}
    bool CarResolve(std::uintptr_t offset,std::uintptr_t& result){result=offset==cfgvehicle::disk::Car4?fixtureCar:0;return true;}
    bool CarSource(std::uintptr_t offset,std::uintptr_t value){return fixtureSource&&offset==cfgvehicle::disk::Car4&&value==fixtureCar;}
    bool Location(std::uintptr_t value,cfgspawn::Vec3& out){++calls_;out=value==fixtureCar?carPoint:value==bundleJason.pawn.object?fixtureHere:fixtureGoal;return true;}
    bool FeatureFunction(const cfgvehicle::disk::Binding&,std::uintptr_t& function,std::uintptr_t& type){function=0x123400;type=navType;return true;}
    bool Invoke(std::uintptr_t,std::uintptr_t,void* value,bool){++calls_;++projectCalls;auto& p=*static_cast<cfgvehicle::Projection*>(value);p.result=projectResult;p.projected=p.point;
        p.projected.x+=projectOffset.x;p.projected.y+=projectOffset.y;
        if(terrainProjection){p.projected.z=carPoint.z;if(std::fabs(p.point.z-carPoint.z)>p.extent.z)p.result=0;}return true;}
    template<class B,std::size_t N>bool CarCall(const B&,std::uintptr_t executor,const std::array<unsigned char,N>&,std::uintptr_t receiver,void* value){
        ++calls_;
        if(executor==cfgtrap::disk::GetActorForwardVectorExecutor){CHECK(receiver==fixtureCar);
            *static_cast<cfgspawn::Vec3*>(value)={1,0,0};++headingCalls;return true;}
        CHECK(receiver==bundleJason.killerController.object);
        if(executor==cfgtrap::disk::MoveToLocationExecutor){auto& p=*static_cast<cfgtrap::MoveLocationParameters*>(value);
            CHECK(p.acceptance==65&&p.pathfinding==1);p.result=moveResult;++moveCalls;return moveReturns;}
        ++stopCalls;return stopReturns;
    }
#include "FootCarObstacleRuntimeMembers.hpp"
    FootCarFixture(){r_.cap=100000;vehicleType=r_.Object();fixtureCar=r_.Object(vehicleType);Serial(fixtureCar);
        navType=r_.Object();nav=r_.Object(navType);Serial(nav);
        r_.Put(plan.module+cfgvehicle::disk::VehicleReflection,vehicleType);r_.Put(plan.module+cfgvehicle::disk::VehicleConstructor,vehicleType);
        r_.Put(plan.world+cfgtrap::disk::WorldNavigationSystem,nav);
        const auto manager=r_.Object();r_.Put(bundleJason.pawn.object+cfghunter::disk::Manager,manager);
        r_.Put(manager+cfgvehicleinteraction::disk::LockedComponent,std::uintptr_t{});Fresh();r_.cap=1536;
    }
};
#undef cfgclosedtrace
#ifndef CFG_FOOT_CAR_FIXTURE_ONLY
int main(){unsigned peak=0;
 const auto step=[&](FootCarFixture& f,std::uint64_t now){f.Fresh();const bool result=f.FootCarObstacle(now,f.r_.bot);
     peak=(std::max)(peak,f.r_.reads);CHECK(f.r_.reads<=1536&&f.calls_<=12);return result;};
 const auto start=[&](FootCarFixture& f){CHECK(step(f,5000));
     CHECK(f.bundleFootObstacle.stage==5);CHECK(step(f,5100)&&f.bundleFootObstacle.stage==1);
     CHECK(step(f,5200)&&f.bundleFootObstacle.stage==2);CHECK(step(f,5300)&&f.bundleFootObstacle.stage==3);
     CHECK(step(f,5400)&&f.bundleFootObstacle.stage==4&&f.moveCalls==1);
     CHECK(f.bundleFootObstacleTarget==f.r_.bot&&f.bundleSchedule.huntAt==5500);};
 {FootCarFixture f;f.terrainProjection=true;f.fixtureHere.z=103.72f;f.fixtureGoal.z=82.96f;f.carPoint.z=1.79f;
  start(f);CHECK(f.projectCalls==1&&f.bundleFootObstacle.projected.z==1.79f&&f.moveCalls==1);}
 {FootCarFixture f;f.fixtureHere={280,0,100};f.fixtureGoal={-400,0,100};start(f);
  CHECK(f.bundleFootObstacle.route.arrival.x>=330&&f.projectCalls==1&&f.moveCalls==1);}
 {FootCarFixture f;f.fixtureHere={50,150,100};f.fixtureGoal={50,-250,100};start(f);
  CHECK(f.bundleFootObstacle.route.arrival.y>=210&&f.projectCalls==1&&f.moveCalls==1);}
 {FootCarFixture f;f.projectOffset={150,0,0};start(f);
  CHECK(cfgfootcar::Route::Squared(f.bundleFootObstacle.route.goal,f.bundleFootObstacle.projected)>115.f*115.f);
  f.fixtureHere=f.bundleFootObstacle.projected;CHECK(step(f,5500)&&f.bundleFootObstacle.stage==1);
  CHECK(step(f,5600)&&step(f,5700)&&step(f,5800)&&f.moveCalls==2);
  f.fixtureHere=f.bundleFootObstacle.projected;CHECK(step(f,5900)&&!f.bundleFootObstacle.stage);}
 {FootCarFixture f;f.projectResult=0;CHECK(step(f,5000));CHECK(step(f,5100));
  const float originalEnd=f.bundleFootObstacle.route.end;
  CHECK(step(f,5200)&&f.bundleFootObstacle.stage==1&&f.bundleFootObstacle.route.end==-originalEnd);
  CHECK(f.bundleFootObstacleHistory.Tried(originalEnd)&&f.bundleFootObstacleTarget==f.r_.bot);
  CHECK(!step(f,5300)&&!f.bundleFootObstacle.stage&&f.projectCalls==2&&!f.moveCalls&&!f.stopCalls);
  CHECK(f.bundleFootObstacleHistory.exhausted&&f.bundleFootObstacleHistory.episodes==2);
  CHECK(step(f,10300)&&f.bundleFootObstacle.stage==5);CHECK(!step(f,10400));
  CHECK(f.projectCalls==2&&!f.moveCalls&&!f.stopCalls&&f.bundleFootObstacleHistory.exhausted);}
 {FootCarFixture f;f.moveResult=0;CHECK(step(f,5000));CHECK(step(f,5100));CHECK(step(f,5200));CHECK(step(f,5300));
  const float failedEnd=f.bundleFootObstacle.route.end;
  CHECK(step(f,5400)&&f.bundleFootObstacle.stage==1&&f.bundleFootObstacle.route.end==-failedEnd);
  CHECK(f.bundleFootObstacleHistory.Tried(failedEnd)&&f.moveCalls==1);
  f.moveResult=2;CHECK(step(f,5500)&&f.bundleFootObstacle.stage==2);
  CHECK(step(f,5600)&&f.bundleFootObstacle.stage==3);
  CHECK(step(f,5700)&&f.bundleFootObstacle.stage==4&&f.moveCalls==2);
  CHECK(!f.bundleFootObstacleHistory.Tried(-failedEnd));}
 {FootCarFixture f;f.stopReturns=false;CHECK(step(f,5000));CHECK(step(f,5100));CHECK(step(f,5200));
  const float failedEnd=f.bundleFootObstacle.route.end;
  CHECK(step(f,5300)&&f.bundleFootObstacle.stage==1&&f.bundleFootObstacle.route.end==-failedEnd);
  CHECK(f.bundleFootObstacleHistory.Tried(failedEnd));
  f.stopReturns=true;CHECK(step(f,5400)&&f.bundleFootObstacle.stage==2);
  CHECK(step(f,5500)&&f.bundleFootObstacle.stage==3);CHECK(step(f,5600)&&f.bundleFootObstacle.stage==4);}
 {FootCarFixture f;
  start(f);CHECK(f.headingCalls==1&&f.stopCalls==1&&!f.teleports);
  CHECK(step(f,5500)&&f.moveCalls==1&&f.stopCalls==1); // accepted path is retained while moving
  f.fixtureHere=f.bundleFootObstacle.route.goal;CHECK(step(f,6000)&&f.bundleFootObstacle.stage==1);
  CHECK(f.bundleFootObstacle.candidate.y>0); // arrival advances to opposite-side corner
  CHECK(step(f,6100)&&step(f,6200)&&step(f,6300)&&f.moveCalls==2);
  f.fixtureHere=f.bundleFootObstacle.route.goal;CHECK(step(f,6400)&&!f.bundleFootObstacle.stage&&f.bundleMoveAt==6400&&!f.teleports);}
 {FootCarFixture f;start(f);CHECK(step(f,8200)&&f.bundleFootObstacle.stage==1&&f.bundleFootObstacle.candidate.x<0);
  CHECK(step(f,8300)&&step(f,8400)&&step(f,8500)&&f.moveCalls==2);
   CHECK(!step(f,11300)&&!f.bundleFootObstacle.stage&&f.bundleMoveAt==11300&&f.bundleFootObstacleHistory.exhausted&&!f.teleports);}
 {FootCarFixture f;start(f);f.carPoint.x+=101;CHECK(!step(f,5500)&&!f.bundleFootObstacle.stage&&f.moveCalls==1);}
 {FootCarFixture f;start(f);f.fixtureGoal=f.fixtureHere;CHECK(!step(f,5500)&&!f.bundleFootObstacle.stage&&f.moveCalls==1);}
 {FootCarFixture f;start(f);f.Serial(f.fixtureCar,0);CHECK(!step(f,5500)&&!f.bundleFootObstacle.stage&&f.moveCalls==1);}
 for(unsigned fault=0;fault<5;++fault){FootCarFixture f;
  if(fault==0)f.r_.Put(f.r_.bot+cfgvehicleoccupancy::disk::CurrentSeat,f.plan.pawn);
  if(fault==1)f.nativeBusy=true;
  if(fault==2)f.holding=true;
  if(fault==3)f.carPoint={10000,0,0};
  if(fault==4)f.r_.cap=1;
  for(unsigned i=0;i<6;++i){f.Fresh();f.FootCarObstacle(1000+1000*i,f.r_.bot);}
  CHECK(!f.moveCalls&&!f.stopCalls&&!f.teleports);}
 std::printf("Included two-corner car adapter/arrival/stall/source/serial: %d failures,fixture peak%u/1536; native calls/source lookup mocked.\n",failures,peak);return failures?1:0;
}
#endif
