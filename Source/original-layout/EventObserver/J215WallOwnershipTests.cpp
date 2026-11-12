#define CFG_J215_CHALLENGE_WALL_OWNERSHIP 1
#include "../Core/ChallengeEdgeWallsPolicy.hpp"
#include <cstdio>
#include "../Core/BrushLocalGeometry.hpp"
using namespace cfgj36walls;
int main(){unsigned checks=0,failures=0;const auto check=[&](bool good,const char* label){++checks;if(!good){++failures;std::printf("FAIL %s\n",label);}};
 Diagnostics d;d.count=127;d.ownerCount=2;d.owners[0].challenge=1;d.owners[1].challenge=3;
 // Exact saved #386 world AABBs. The ownership observations below are
 // simulated explicitly; the saved capture itself did NOT prove ownership.
 const std::array<Bounds,5> live{{{{-550.3f,-8928.7f,586.1f},{1574.9f,6052.1f,1500}},
  {{2805.1f,-16884.4f,586.1f},{2006.4f,1999.4f,1500}},{{6839.6f,-17397.f,586.1f},{2432.1f,1439.3f,1500}},
  {{10862.5f,-15946.6f,586.1f},{1725.7f,141.3f,1500}},{{767.4f,-907.5f,586.1f},{2850.4f,2361.f,1500}}}};
 for(unsigned i=0;i<d.count;++i){auto& r=d.records[i];r.owner=1;r.geometry=1;r.bounds={{0,0,0},{100,5000,1500}};
  std::snprintf(r.name.data(),r.name.size(),"BlockingVolume%u",i+10);}
 for(unsigned i=0;i<5;++i){auto& r=d.records[i];r.owner=0;r.bounds=live[i];r.index=175751-static_cast<int>(i)*2;r.rotation.y=45;
  std::snprintf(r.name.data(),r.name.size(),"BlockingVolume%u",299+i);}
 Classify(d);check(d.chosen==5,"all five rotated mission-owned segments selected; no four-side AABB requirement");
 for(unsigned i=0;i<5;++i)check(d.records[i].decision==Decision::Edge,"299-303 selected by proved ownership");
 for(unsigned i=5;i<127;++i)check(d.records[i].decision!=Decision::Edge,"original map ownership keeps collision");
 auto unknown=d;unknown.ownerCount=0;Classify(unknown);check(!unknown.chosen&&unknown.diagnosticOnly==127,"serial-zero/1500/index cluster alone never grants writes");
 for(const auto label:{"Pier_Blocker","corner_Blocker_1","BlockingVolumeX"}){auto special=d;std::snprintf(special.records[0].name.data(),24,"%s",label);Classify(special);check(special.chosen==4,"special blocker excluded even in mission package");}
 auto low=d;low.records[0].bounds.extent.z=100;Classify(low);check(low.chosen==4,"low blocker excluded");
 auto small=d;small.records[0].bounds.extent={100,100,1500};Classify(small);check(small.chosen==4,"small blocker excluded");
 auto root=d;root.records[0].geometry=0;Classify(root);check(root.chosen==4,"unproved component/rotation excluded");
 check(ChallengePackage(u"/Game/Maps/Single_Player/SPC01",u"/Game/Maps/Single_Player/SPC01_Walls"),"current mission sublevel suffix");
 check(!ChallengePackage(u"/Game/Maps/Single_Player/SPC01",u"/Game/Maps/Single_Player/SPC010_Walls"),"prefix collision is not ownership");
 check(!ChallengePackage(u"/Game/Maps/Single_Player/SPC01",u"/Game/Maps/CrystalLake/CrystalLake"),"base-game map never adopted");
 {cfgj215walls::Box box{{-2,-4,-6},{2,4,6},1,{}};cfgj215walls::Transform t{{0,0,0,1},{10,20,30,0},{1,1,1,0}};Bounds out;
  check(cfgj215walls::TransformBox(box,t,out)&&out.center.x==10&&out.center.y==20&&out.extent.x==2&&out.extent.z==6,"actual convex local FBox + element transform; no world bounds involved");
  t.rotation={{0,0,.70710677f,.70710677f}};t.scale={{2,1,1,0}};
  check(cfgj215walls::TransformBox(box,t,out)&&std::fabs(out.extent.x-4)<.001f&&std::fabs(out.extent.y-4)<.001f,"element local rotation/nonuniform scale honored");
  t.rotation={{0,0,0,0}};check(!cfgj215walls::TransformBox(box,t,out),"invalid quaternion refuses local bounds");
  t.rotation={{0,0,0,1}};box.valid=0;check(!cfgj215walls::TransformBox(box,t,out),"invalid native FBox refuses local bounds");
 }
 std::printf("J215 #386 saved geometry + explicit simulated ownership: %u checks, %u failures; no game access.\n",checks,failures);return failures?1:0;
}
