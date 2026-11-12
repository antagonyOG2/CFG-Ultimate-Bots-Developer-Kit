#define CFG_FOOT_CAR_FIXTURE_ONLY
#include "FootCarObstacleRuntimeMembersTests.cpp"
#undef CFG_FOOT_CAR_FIXTURE_ONLY
#include "../Core/DriverNavigationPolicy.hpp"
struct DriverFixture:FootCarFixture {
    cfgdrivernav::Cache bundleDriverNavigation;cfgvehicleaction::State bundleVehicleActions;
#include "DriverNavigationRuntimeMembers.hpp"
};
#ifndef CFG_DRIVER_NAV_FIXTURE_ONLY
int main(){
 {DriverFixture f;f.projectOffset={0,60,0};cfgspawn::Vec3 point{0,-210,0};
  f.Fresh();CHECK(f.DriverDoorPoint(f.fixtureCar,123,{},point,1,1)&&point.y==-150&&f.projectCalls==1);
  point={0,-210,0};f.Fresh();CHECK(f.DriverDoorPoint(f.fixtureCar,123,{},point,1,1)&&point.y==-150&&f.projectCalls==1);
  f.projectOffset={150,0,0};auto& route=f.bundleVehicleActions.route;cfgspawn::Vec3 goal{};
  CHECK(route.Goal({},{1,0,0},point,{0,200,100},100,goal));
  f.Fresh();CHECK(f.DriverCornerPoint(goal)&&goal.x==510&&route.waypoint.x==510);
  CHECK(f.projectCalls==2);f.Fresh();CHECK(f.DriverCornerPoint(goal)&&f.projectCalls==2);
  const auto arrival=goal;CHECK(route.Goal({},{1,0,0},point,arrival,200,goal)&&goal.y==point.y);
  // Driver-side handoff must happen even while a corner route is active.
  CHECK(route.Goal({},{1,0,0},point,{0,-125,100},300,goal)&&goal.y==point.y&&goal.x==point.x);
  // A door leg can fail without returning to the passenger side. Emulate a
  // projected corner close to the chassis centerline, then stall at it.
  route.Reset();route.active=true;route.reached=true;route.origin={};route.axis={1,0,0};
  route.endSign=1;route.waypoint={360,0,0};route.best=100;route.progressAt=100;
  const auto stuck=route.waypoint;
  CHECK(route.Goal({},{1,0,0},point,stuck,6200,goal)&&!route.reached&&route.endSign==-1&&goal.x<0);
  f.Fresh();CHECK(f.DriverCornerPoint(goal)&&route.waypoint.x<0);
 }
 {DriverFixture f;f.projectResult=0;cfgspawn::Vec3 point{0,-210,0};
  f.Fresh();CHECK(!f.DriverDoorPoint(f.fixtureCar,123,{},point,1,1)&&!f.bundleDriverNavigation.haveDoor);
  f.projectResult=1;f.projectOffset={0,160,0};point={0,-210,0};f.Fresh();
  CHECK(!f.DriverDoorPoint(f.fixtureCar,123,{},point,1,1)); // outside120cm query envelope
 }
 {cfgdrivernav::Cache c;c.Select(1,2,{},1,1);CHECK(c.Door({0,-210,0},{0,-180,0}));
  c.Select(1,3,{},1,1);CHECK(!c.haveDoor);CHECK(!c.Door({0,-210,0},{0,30,0}));
  CHECK(c.Door({0,-210,0},{0,-180,0}));c.Select(1,3,{76,0,0},1,1);CHECK(!c.haveDoor);
  CHECK(c.Door({76,-210,0},{76,-180,0}));c.Select(1,3,{76,0,0},2,1);CHECK(!c.haveDoor);
  CHECK(!c.Corner({360,360,0},{600,360,0}));CHECK(!c.Door({0,-210,0},{0,-180,181}));
  c.Select(1,3,{76,0,0},0,1);CHECK(!c.car&&!c.haveDoor);
 }
 std::printf("Included donor driver door/corner projection and actual-arrival cache: %d failures; native projection mocked.\n",failures);
 return failures?1:0;
}
#endif
