#include "../Core/ChallengeEdgeWallsPolicy.hpp"
#include <cstdio>
#include <limits>
using namespace cfgj36walls;
struct Host {bool world=true,current=true,collision=true;unsigned reads=0,writes=0;Write outcome=Write::Returned;
 bool WorldCurrent(){return world;}bool Current(const Record&){++reads;return current;}
 bool ReadCollision(const Record&,bool& v){++reads;v=collision;return true;}
 Write Set(const Record&,bool value){++writes;if(outcome==Write::Returned)collision=value;return outcome;}
};
int main(){unsigned checks=0,failures=0;auto check=[&](bool good,const char* text){++checks;if(!good){++failures;std::printf("FAIL %s\n",text);}};
 Diagnostics d;d.count=7;
 const std::array<Bounds,7> boxes{{{{-5000,0,0},{100,5000,1000}},{{5000,0,0},{100,5000,1000}},
  {{0,-5000,0},{5000,100,1000}},{{0,5000,0},{5000,100,1000}},{{0,0,0},{100,4000,1000}},
  {{0,0,0},{1000,1000,1000}},{{0,0,0},{100,3000,0}}}};
 for(unsigned i=0;i<d.count;++i)d.records[i].bounds=boxes[i];Classify(d);
 check(d.chosen==4,"four exterior sides chosen");for(unsigned i=0;i<4;++i)check(d.records[i].decision==Decision::Edge,"outer long thin tall barrier");
 check(d.records[4].decision==Decision::Interior,"interior wall keeps collision");
 check(d.records[5].decision==Decision::NotThinWall&&d.records[6].decision==Decision::Malformed,"ordinary solid and malformed volume stay collidable");
 auto incomplete=d;incomplete.count=3;Classify(incomplete);check(incomplete.chosen==0,"no guessing without complete boundary");
 auto malformed=d;malformed.records[0].bounds.center.x=std::numeric_limits<float>::quiet_NaN();Classify(malformed);check(malformed.chosen==0,"nonfinite boundary refuses selection");
 {auto r=d.records[0];Host h;check(Apply(r,d,h)&&r.state==1&&h.writes==1&&!h.collision,"one confirmed collision disable");
  check(!Apply(r,d,h)&&h.writes==1,"no repeated disable");check(Restore(r,d,h)&&r.state==2&&h.collision&&h.writes==2,"exact current original collision restored/read back");}
 {auto r=d.records[0];Host h;h.collision=false;check(!Apply(r,d,h)&&r.decision==Decision::OriginallyOff&&h.writes==0,"preexisting disabled collision untouched");}
 {auto r=d.records[0];Host h;h.outcome=Write::Uncertain;check(!Apply(r,d,h)&&r.state==3&&h.writes==1,"ambiguous native result quarantines");
  h.outcome=Write::Returned;check(!Apply(r,d,h)&&!Restore(r,d,h)&&h.writes==1,"no uncertain replay or blind restore");}
 {auto r=d.records[0];Host h;check(Apply(r,d,h),"departure setup");h.world=false;const auto reads=h.reads;
  check(!Restore(r,d,h)&&r.state==4&&h.reads==reads&&h.writes==1,"departed World never dereferences old actor or claims restore");}
 {auto r=d.records[0];Host h;h.outcome=Write::NotEntered;check(!Apply(r,d,h)&&r.state==0,"unentered work may yield");}
 {auto r=d.records[0];Host h;check(Apply(r,d,h),"foreign-change setup");h.collision=true;
  check(!Restore(r,d,h)&&r.state==3&&h.writes==1,"external collision edit preserved");}
 std::printf("J214 edge walls geometry + owned collision state: %u checks %u failures. Native host simulated.\n",checks,failures);return failures?1:0;
}
