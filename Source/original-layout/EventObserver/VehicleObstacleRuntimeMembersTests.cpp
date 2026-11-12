// Included PRODUCTION phase orchestration. Native invocation/context admission
// are mocked; real typed slot/ancestry/registry observation remains in use.
#define main SparseAiFixtureMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
#include "VehicleRoadRuntime.hpp"
#include <cassert>
#include "VehicleOccupancyObservation.hpp"
#include "JasonInstanceObservation.hpp"
#include "TrapNativeParameters.hpp"
#include "../RuntimeProbe/StartupTrapBindings.generated.hpp"
#include "../Core/VehicleActionPolicy.hpp"
#include "../Core/CounselorPursuitPolicy.hpp"
namespace cfgbundle {constexpr unsigned ReadCap=1536;}
struct Host:Fixture {
 Fixture& r_=*this;cfgobserver::Plan plan{},context{};
 struct {cfgprobe::lifetime::SlotObservation pawn{},killerController{};}bundleJason;
 struct {struct {bool Exclusive(std::uint64_t){return false;}}policy;}bundleKill;
 bool bundleCompletionReady=false;
 cfgvehicleaction::Obstacle bundleCarObstacle;cfgvehicleaction::State bundleVehicleActions;
 cfgvehicle::Pursuit bundleCarPursuit;struct {void Clear(){}}bundleFollowPath;
 cfgvehicle::Cooldown bundleCarMorph;cfgvehicle::TailIntercept bundleCarIntercept;
 struct {unsigned carMoves=0,carStage=0,morphAttempts=0,morphReturns=0;}bundleReport;
 struct {std::uint64_t vehicleAt=0;}bundleSchedule;
 std::uintptr_t managerAddress=0,systemAddress=0,navType=0,car=0,driver=0;
 unsigned calls=0,stops=0,moves=0,projections=0,teleports=0;bool admitted=true,occupied=true,currentProof=true,nativeOkay=true,exterior=true;
 cfgspawn::Vec3 position{},lastMove{};
 Host(){cap=1536;plan.world=Object();context.objectArray=objects;
  const auto pawnObject=Object(),controllerObject=Object();managerAddress=Object();navType=Object();systemAddress=Object(navType);car=Object();driver=Object();
  Put(navType+cfgprobe::ClassSuper,std::uintptr_t{});
  for(const auto value:{pawnObject,controllerObject,systemAddress}){int index=0;Read(value+cfgprobe::lifetime::ObjectIndexOffset,index);
   Put(objects.data+24*static_cast<unsigned>(index)+16,100+index);}
  cfgspawn::ObjectSlot(*this,objects,pawnObject,bundleJason.pawn);cfgspawn::ObjectSlot(*this,objects,controllerObject,bundleJason.killerController);
  Put(plan.world+cfgtrap::disk::WorldNavigationSystem,systemAddress);
  Put(pawnObject+cfghunter::disk::Manager,managerAddress);Put(managerAddress+cfgvehicleinteraction::disk::LockedComponent,std::uintptr_t{});
  bundleCarObstacle.car=car;bundleCarObstacle.driver=driver;bundleCarObstacle.source=cfgvehicle::disk::Car4;
  bundleCarObstacle.origin={0,0,100};bundleCarObstacle.candidate={100,250,100};position=bundleCarObstacle.origin;bundleCarObstacle.stage=1;reads=0;
 }
 bool Context(){reads+=40;return admitted;}
 bool Jason(){reads+=40;return admitted;}
 bool CarActorBindings(){reads+=40;return admitted;}
 bool Manager(){reads+=20;return admitted;}
 bool Held(std::uintptr_t& value){value=0;return true;}
 bool Alive(std::uintptr_t,bool& value){value=true;return true;}
 template<class B>bool NativeBool(const B&,bool& value){value=false;return true;}
 bool CarSource(std::uintptr_t source,std::uintptr_t value){return source==cfgvehicle::disk::Car4&&value==car;}
 bool CarOccupancy(std::uintptr_t value,cfgvehicleoccupancy::Proof& proof){
  proof={};if(value!=car)return false;proof.result=occupied?cfgvehicleoccupancy::Result::Occupied:cfgvehicleoccupancy::Result::Empty;proof.driver=driver;return true;}
 bool Location(std::uintptr_t,cfgspawn::Vec3& value){value=position;return true;}
 bool ExteriorLanding(cfgspawn::Vec3){return exterior;}
 bool CarOccupancyCurrent(const cfgvehicleoccupancy::Proof&){return currentProof&&occupied;}
 template<class B>bool FeatureFunction(const B&,std::uintptr_t& function,std::uintptr_t& owner){function=0x910000;owner=navType;return admitted;}
 bool Invoke(std::uintptr_t receiver,std::uintptr_t,void* parameters,bool){
  ++calls;++projections;assert(receiver==systemAddress);auto& p=*static_cast<cfgvehicle::Projection*>(parameters);
  assert(p.extent.x==80&&p.extent.z==100&&p.world==plan.world);p.projected=p.point;p.result=nativeOkay?1:0;return nativeOkay;}
 template<class B,std::size_t N>bool CarCall(const B&,std::uintptr_t executor,const std::array<unsigned char,N>&,
   std::uintptr_t receiver,void* parameters){
  assert(reads<=1536);++calls;
  if(executor==cfgtrap::disk::K2_TeleportToExecutor){assert(receiver==bundleJason.pawn.object);++teleports;
   auto& p=*static_cast<cfgpursuit::TeleportPacket*>(parameters);p.result=nativeOkay?1:0;return nativeOkay;}
  assert(receiver==bundleJason.killerController.object);
  if(executor==cfgtrap::disk::StopMovementExecutor){assert(!parameters);++stops;}
  else {assert(executor==cfgtrap::disk::MoveToLocationExecutor);auto& p=*static_cast<cfgtrap::MoveLocationParameters*>(parameters);
   assert(p.acceptance==65&&p.pathfinding&&p.projectNavigation);lastMove=p.destination;p.result=nativeOkay?1:0;++moves;}
  return nativeOkay;
 }
#include "VehicleObstacleRuntimeMembers.hpp"
};
int main(){unsigned ownedFailures=0,checkIndex=0;const auto check=[&](bool value){++checkIndex;if(!value){++ownedFailures;std::printf("FAIL obstacle check%u\n",checkIndex);}};
 {Host h;const auto first=h.CarObstaclePhase(1000);std::printf("Obstacle first=%u stage=%u reads=%u calls=%u projections=%u\n",first,h.bundleCarObstacle.stage,h.reads,h.calls,h.projections);check(first&&h.bundleCarObstacle.stage==2&&h.projections==1&&!h.stops&&!h.moves);
  h.reads=0;check(h.CarObstaclePhase(1100)&&h.bundleCarObstacle.stage==3&&h.stops==1&&!h.moves);
  h.reads=0;check(h.CarObstaclePhase(1200)&&!h.bundleCarObstacle.stage&&h.moves==1&&h.bundleCarObstacle.Protected(5199)&&h.bundleReport.carStage==15);
  check(h.lastMove.x==100&&h.lastMove.y==250&&h.calls==3&&h.reads<1536);}
 for(unsigned failure=0;failure<7;++failure){Host h;
  if(failure==0)h.admitted=false;if(failure==1)h.occupied=false;if(failure==2)h.currentProof=false;
  if(failure==3)h.Put(h.managerAddress+cfgvehicleinteraction::disk::LockedComponent,h.systemAddress);
  if(failure==4)h.bundleVehicleActions.extractionCommitted=true;if(failure==5)h.position.x=101;if(failure==6)h.nativeOkay=false;
  h.CarObstaclePhase(1000);check(!h.stops&&!h.moves&&!h.bundleCarObstacle.stage);}
 {Host h;check(h.CarObstaclePhase(1000));h.currentProof=false;h.reads=0;h.CarObstaclePhase(1100);check(!h.stops&&!h.moves&&!h.bundleCarObstacle.stage);}
 {Host h;check(h.CarObstaclePhase(1000));h.reads=0;check(h.CarObstaclePhase(1100));h.occupied=false;h.reads=0;h.CarObstaclePhase(1200);check(!h.moves&&!h.bundleCarObstacle.stage);}
 {Host h;h.bundleCarObstacle.projected={100,250,100};h.bundleCarObstacle.stage=4;h.bundleCarObstacle.detourIssued=true;
  check(h.CarObstaclePhase(10000)&&h.teleports==1&&h.bundleReport.morphAttempts==1&&h.bundleReport.morphReturns==1&&
   h.bundleVehicleActions.morphStopPending&&!h.bundleCarObstacle.stage&&h.reads<1536);}
 for(unsigned failure=0;failure<4;++failure){Host h;h.bundleCarObstacle.projected={100,250,100};h.bundleCarObstacle.stage=4;h.bundleCarObstacle.detourIssued=true;
  if(failure==0)h.exterior=false;if(failure==1)h.occupied=false;if(failure==2)h.currentProof=false;if(failure==3)h.bundleCarMorph.next=20000;
  h.CarObstaclePhase(10000);check(!h.teleports);}
 std::printf("Included car obstacle phases: ownedFailures=%u, projection/stop/move on separate callbacks, lock/exit/world/motion refusals; native mocks only\n",ownedFailures);return ownedFailures?1:0;
}
