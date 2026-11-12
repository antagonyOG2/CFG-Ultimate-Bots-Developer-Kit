#include "../Core/DonorJasonRoster.hpp"
#include <cassert>
#include <cstdio>
#include <map>
#include <vector>
using namespace cfgdonorhunt;
struct Host {
    struct Pawn {float health=100;std::uintptr_t controller=0;std::uint8_t dead=0;bool escaped=false,typed=true,readable=true;};
    std::map<std::uintptr_t,Pawn> pawns;
    std::map<std::uintptr_t,std::uintptr_t> controllers;
    std::vector<std::vector<std::uintptr_t>> levels;
    std::uintptr_t held=0;unsigned resets=0,retired=0,levelReads=0;
    void Add(std::uintptr_t pawn,std::uintptr_t controller){pawns[pawn].controller=controller;controllers[controller]=pawn;}
    bool ReadableActor(std::uintptr_t a){return pawns.count(a)&&pawns[a].readable;}
    bool IsCounselorCharacter(std::uintptr_t a){return pawns.count(a)&&pawns[a].typed;}
    bool ReadCounselorHealth(std::uintptr_t a,float& h){if(!ReadableActor(a))return false;h=pawns[a].health;return true;}
    bool ReadPawnController(std::uintptr_t a,std::uintptr_t& c){if(!ReadableActor(a))return false;c=pawns[a].controller;return true;}
    bool ReadCounselorDead(std::uintptr_t a,std::uint8_t& d){if(!ReadableActor(a))return false;d=pawns[a].dead;return true;}
    bool HasCounselorEscaped(std::uintptr_t a){return pawns.count(a)&&pawns[a].escaped;}
    bool ReadableController(std::uintptr_t c){return controllers.count(c)!=0;}
    bool ReadControllerPawn(std::uintptr_t c,std::uintptr_t& p){if(!ReadableController(c))return false;p=controllers[c];return true;}
    bool IsNativeCounselorDeathConfirmed(std::uintptr_t p){return pawns.count(p)&&(pawns[p].health<=0||pawns[p].dead);}
    void ResetFinalDeathSpectatorState(){++resets;}
    std::uintptr_t LastHeldCounselor(){return held;}
    void RetireDeadAICounselorFromSpectating(std::uintptr_t){++retired;}
    bool ReadWorldLevelCount(std::uintptr_t,unsigned& count){++levelReads;count=static_cast<unsigned>(levels.size());return true;}
    bool ReadWorldLevel(std::uintptr_t,unsigned i,std::uintptr_t& level){if(i>=levels.size())return false;level=i+1;return true;}
    bool ReadLevelActorCount(std::uintptr_t level,unsigned& count){if(!level||level>levels.size())return false;count=static_cast<unsigned>(levels[level-1].size());return true;}
    bool ReadLevelActor(std::uintptr_t level,unsigned i,std::uintptr_t& actor){if(!level||level>levels.size()||i>=levels[level-1].size())return false;actor=levels[level-1][i];return true;}
};
int main(){unsigned checks=0;auto check=[&](bool x){++checks;assert(x);};
    {CounselorRoster r;Host h;h.Add(10,100);h.Add(20,200);h.Add(30,300);
        for(auto a:{10,20,30})r.DonorRegisterJasonAITarget(h,a);
        r.DonorBlockJasonAITarget(20,1000);r.DonorBlockJasonAITarget(30,2000);r.DonorBlockJasonAITarget(30,3000);
        r.Target=10;r.TrapPriorityVictim=10;r.TrapPriorityUntil=5000;r.TrapPriorityMorphCompleted=true;
        r.DonorRemoveCounselorTarget(10);
        check(r.count==2&&r.actors[0]==20&&r.actors[1]==30&&r.actors[2]==0);
        check(r.blockedUntil[0]==91000&&r.blockedUntil[1]==603000&&r.blockStrikes[0]==1&&r.blockStrikes[1]==2);
        check(!r.Target&&!r.TrapPriorityVictim&&!r.TrapPriorityUntil&&!r.TrapPriorityMorphCompleted);
        check(r.DonorIsJasonAITargetBlocked(20,90999)&&!r.DonorIsJasonAITargetBlocked(20,91000));
        r.DonorRemoveCounselorTarget(999);check(r.count==2);}
    {CounselorRoster r;Host h;h.Add(10,100);r.LocalPlayerController=100;
        r.DonorRefreshLocalCounselorTarget(h,1000);check(r.count==1&&r.LocalCounselorTarget==10);
        h.pawns[10].health=0;h.controllers[100]=0;
        r.DonorRefreshLocalCounselorTarget(h,1249);check(r.count==1); //250ms possession cadence
        r.DonorRefreshLocalCounselorTarget(h,1250);check(!r.count&&!r.LocalCounselorTarget&&r.LocalCounselorDeathEvidence);
        h.Add(20,100);r.DonorRefreshLocalCounselorTarget(h,1500);
        check(r.count==1&&r.actors[0]==20&&r.LocalCounselorTarget==20&&!r.LocalCounselorDeathEvidence&&!r.NextKillTeamTickAt);}
    {CounselorRoster r;Host h;h.Add(10,100);r.DonorRegisterJasonAITarget(h,10);
        r.DonorPruneAndRefreshCounselorTargets(h,1,1000);check(r.controllerCount==1&&r.controllers[0]==100);
        h.pawns[10].dead=1;h.Add(20,100);r.DonorPruneAndRefreshCounselorTargets(h,1,2000);
        check(r.count==1&&r.actors[0]==20&&!r.FinalCounselorDeathObserved&&!h.resets&&r.pendingDeaths[0].Pawn==10);
        h.held=10;r.DonorPruneAndRefreshCounselorTargets(h,1,17000);check(!h.retired&&r.pendingDeaths[0].Pawn==10);
        h.held=0;r.DonorPruneAndRefreshCounselorTargets(h,1,18000);check(h.retired==1&&!r.pendingDeaths[0].Pawn);}
    {CounselorRoster r;Host h;h.Add(10,100);r.DonorRegisterJasonAITarget(h,10);
        h.pawns[10].health=0;r.DonorPruneAndRefreshCounselorTargets(h,1,1000);
        check(!r.count&&r.FinalCounselorDeathObserved&&h.resets==1&&!h.retired&&!r.pendingDeaths[0].Pawn);
        r.DonorPruneAndRefreshCounselorTargets(h,1,2000);check(h.resets==1&&!h.retired);}
    for(unsigned fault=0;fault<6;++fault){CounselorRoster r;Host h;h.Add(10,100);
        if(fault==0)h.pawns[10].typed=false;else if(fault==1)h.pawns[10].health=0;
        else if(fault==2)h.pawns[10].escaped=true;else if(fault==3)h.pawns[10].dead=1;
        else if(fault==4)h.controllers[100]=20;else h.pawns[10].readable=false;
        check(!r.DonorIsValidatedLiveCounselorPawn(h,10));}
    {CounselorRoster r;Host h;h.Add(10,100);h.Add(20,200);h.levels={{10},{20}};
        r.DonorRegisterLiveCounselorTargets(h,1);check(r.count==2&&r.actors[0]==10&&r.actors[1]==20);}
    {CounselorRoster r;Host h;h.levels={{999},{888}};
        r.DonorPruneAndRefreshCounselorTargets(h,1,1000);check(h.levelReads==1&&r.RosterRefreshLevel==1);
        r.DonorPruneAndRefreshCounselorTargets(h,1,60999);check(h.levelReads==1);
        r.DonorPruneAndRefreshCounselorTargets(h,1,61000);check(h.levelReads==2&&r.RosterRefreshLevel==2);}
    {CounselorRoster r;Host h;for(std::uintptr_t i=1;i<=70;++i){h.Add(i,1000+i);r.DonorRegisterJasonAITarget(h,i);r.DonorTrackCounselorController(h,i);}
        check(r.count==64&&r.controllerCount==64);r.DonorPruneAndRefreshCounselorTargets(h,1,1000);
        check(r.count==64&&r.controllerCount==64&&!h.levelReads);}
    std::printf("J1a donor roster: %u checks PASS; closest-target metadata,250ms local/Tommy rebind,known-controller respawn,1s prune/15s retirement/final-camera guard/60s fallback. Native host pending.\n",checks);
}
