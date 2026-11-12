#include "TrapNativeParameters.hpp"
#include <limits>
#include <cstdio>
int main(){
    unsigned failures=0;const auto check=[&](bool ok){if(!ok)++failures;};
    using namespace cfgtrap;MoveLocationParameters m;RotationParameters r;
    check(!MapAllowed(MapFamily::Unknown,true));check(!MapAllowed(MapFamily::Grendel,true));
    check(!MapAllowed(MapFamily::GrendelLarge,true));check(!MapAllowed(MapFamily::GroundMap,false));
    check(!MapAllowed(static_cast<MapFamily>(99),true));
    check(MoveToAnchor({1,2,3},50,MapFamily::GroundMap,true,m)&&m.pathfinding==1&&m.projectNavigation==1&&m.allowPartial==0&&m.filter==0&&m.result==0);
    check(!MoveToAnchor({1,2,3},-1,MapFamily::GroundMap,true,m)&&m.destination.x==0);
    check(!MoveToAnchor({std::numeric_limits<float>::infinity(),0,0},1,MapFamily::GroundMap,true,m));
    check(!MoveToAnchor({},1,MapFamily::GrendelLarge,true,m));
    check(FaceAnchor({0,0,0},{0,10,0},MapFamily::GroundMap,true,r)&&std::fabs(r.rotation.y-90)<0.001f&&!r.teleport);
    check(!FaceAnchor({},{},MapFamily::GroundMap,true,r));
    CanPlaceParameters c;check(!PlacementObserved(c));c.result=1;check(PlacementObserved(c));
    c.result=2;check(!PlacementObserved(c));c.result=1;c.rotation.x=std::numeric_limits<float>::quiet_NaN();check(!PlacementObserved(c));
    std::printf("TrapNativeTests: %u failures; packet-only, no game access\n",failures);return failures?1:0;
}
