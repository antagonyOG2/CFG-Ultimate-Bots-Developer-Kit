#include "../Core/DonorJasonTargeting.hpp"
#include <cassert>
#include <cstdio>
#include <map>
#include <limits>
using namespace cfgdonorhunt;
struct Host {
    struct Actor {Position at{};bool readable=true,controller=true,location=true,blocked=false;};
    std::map<std::uintptr_t,Actor> actors;
    std::uint64_t Now(){return 100;}
    bool ReadableActor(std::uintptr_t actor){const auto i=actors.find(actor);return i!=actors.end()&&i->second.readable;}
    bool ReadablePawnController(std::uintptr_t actor){return actors.at(actor).controller;}
    bool IsJasonAITargetBlocked(std::uintptr_t actor,std::uint64_t){const auto i=actors.find(actor);return i!=actors.end()&&i->second.blocked;}
    bool GetJasonAIActorLocation(std::uintptr_t actor,Position& out){const auto i=actors.find(actor);if(i==actors.end()||!i->second.location)return false;out=i->second.at;return true;}
};
int main(){unsigned checks=0;const auto check=[&](bool x){++checks;assert(x);};
    Host h;h.actors[1]={};h.actors[2].at={200,0,0};h.actors[3].at={100,0,0};
    Targets targets;targets.DonorRegisterJasonAITarget(h,0);targets.DonorRegisterJasonAITarget(h,99);
    check(!targets.count);targets.DonorRegisterJasonAITarget(h,2);targets.DonorRegisterJasonAITarget(h,3);
    targets.DonorRegisterJasonAITarget(h,2);check(targets.count==2);
    check(targets.DonorFindNearestJasonAICounselorTarget(h,1)==3);
    check(targets.DonorFindNearestJasonAICounselorTarget(h,1,3)==2);
    check(targets.DonorHasAlternativeUsableJasonAITarget(h,3));
    h.actors[2].blocked=true;check(!targets.DonorHasAlternativeUsableJasonAITarget(h,3));
    h.actors[2].blocked=false;h.actors[3].controller=false;
    check(targets.DonorFindNearestJasonAICounselorTarget(h,1)==2);
    h.actors[3].controller=true;h.actors[3].at={0,0,300};
    check(targets.DonorFindNearestJasonAICounselorTarget(h,1)==2); // full3D distance, not port's flat-distance shortcut
    h.actors[3].at={200,0,0};check(targets.DonorFindNearestJasonAICounselorTarget(h,1)==2); // first registered wins equal distance
    h.actors[2].readable=false;check(targets.DonorFindNearestJasonAICounselorTarget(h,1)==3);
    h.actors[3].at.X=std::numeric_limits<float>::quiet_NaN();check(!targets.DonorFindNearestJasonAICounselorTarget(h,1));
    h.actors[3].at={1,0,0};h.actors[1].location=false;check(!targets.DonorFindNearestJasonAICounselorTarget(h,1));
    h.actors[1].location=true;h.actors[3].location=false;check(!targets.DonorFindNearestJasonAICounselorTarget(h,1));
    h.actors[3].location=true;h.actors[3].at.X=std::numeric_limits<float>::infinity();check(!targets.DonorFindNearestJasonAICounselorTarget(h,1));
    h.actors[3].at.X=1;for(unsigned i=0;i<64;++i){h.actors[100+i].at={float(i+400),0,0};targets.DonorRegisterJasonAITarget(h,100+i);}
    check(targets.count==64&&targets.DonorFindNearestJasonAICounselorTarget(h,1)==3);
    h.actors[3].blocked=true;check(targets.DonorFindNearestJasonAICounselorTarget(h,1)==100);
    // No fatigue timer is introduced: a reachable nearest counselor remains
    // nearest even after arbitrary repeated decisions without closing distance.
    h.actors[3].blocked=false;for(unsigned i=0;i<600;++i)check(targets.DonorFindNearestJasonAICounselorTarget(h,1)==3);
    std::printf("J1a actual donor targeting bodies: %u checks PASS; native proof boundaries supplied, production wiring pending.\n",checks);
}
