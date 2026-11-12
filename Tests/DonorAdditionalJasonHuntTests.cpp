#include "../Core/DonorAdditionalJasonHunt.hpp"
#include "../Core/DonorJasonPursuit.hpp"
#include "../Core/DonorJasonStartup.hpp"
#include "../Core/DonorNativeMorphCompletion.hpp"
#include "DonorJasonRuntimeState.hpp"
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>
using namespace cfgdonorhunt;
struct Host {
    DonorNativeJasonRuntime runtime;AdditionalHuntState& s=runtime.additional;
    std::uint64_t now=1000;unsigned index=2;
    std::uintptr_t world=1,engineWorld=1,controller=3,local=4,alternative=5,all=4;
    Position here{},localAt{250,0,0},otherAt{300,0,0};
    MatchPhase phase=MatchPhase::InProgress;
    bool actor=true,controllerValid=true,hosted=false,authority=true,death=false,exclusive=false,
        sweater=false,held=false,knife=false,car=false,hiding=false,hideAttempt=true,doorEarly=false,
        doorRegular=false,morph=false,grab=false,grabReadable=false,press=true,module=true,pending=false;
    unsigned bases=0,stops=0,binds=0,moves=0,presses=0,releases=0,grabs=0,morphs=0,runs=0;
    float acceptance=0;std::vector<std::uintptr_t> exclusions;
    std::vector<std::string> events;
    Host(){s.World=1;s.Jason=2;s.Controller=3;}
    auto& Additional(){return s;}auto Controller(){return controller;}auto World(){return world;}
    auto Now(){return now;}bool ReadableActor(std::uintptr_t){return actor;}bool ReadableController(std::uintptr_t){return controllerValid;}
    bool Hosted(){return hosted;}auto EngineWorld(){return engineWorld;}auto ReadCounselorRouteMatchPhase(){return phase;}
    void StopMovement(){++stops;events.push_back("stop");}bool NativeJasonDeathCinematicObserved(){return death;}
    void BindNativeCurrentPrimaryKiller(){++binds;}void RunBaseAIControllerTick(){++bases;events.push_back("base");}
    bool IsHostedAuthorityWorld(){return authority;}bool IsPamelaExclusiveKillLaneActive(std::uint64_t){return exclusive;}
    bool PermanentHumanSweaterActivationActive(std::uint64_t){return sweater;}
    bool DriveAdditionalJasonHeldGrab(std::uint64_t){events.push_back("held");return held;}
    bool DriveAdditionalJasonKnife(std::uintptr_t,float,std::uint64_t){events.push_back("knife");return knife;}
    bool DriveAdditionalJasonOccupiedCar(std::uint64_t){events.push_back("car");return car;}
    bool DriveAdditionalJasonSwim(std::uintptr_t,std::uint64_t){return false;}
    bool FindNearbyOccupiedHidingSpotForJason(std::uintptr_t,std::uintptr_t& spot,std::uintptr_t& hidden,std::uintptr_t& component){
        events.push_back("hide");spot=6;hidden=7;component=8;return hiding;}
    bool GetJasonAIActorLocation(std::uintptr_t who,Position& out){out=who==2?here:who==4?localAt:otherAt;return true;}
    void IssueMove(Position,float radius){++moves;acceptance=radius;events.push_back("move");}
    bool AttemptJasonHidingSpotComponentForJason(std::uintptr_t,std::uintptr_t){return hideAttempt;}
    bool ModuleReady(){return module;}bool AttackReleaseReadable(){return true;}
    void ReleaseAttack(){++releases;events.push_back("release");}
    auto LocalCounselor(){return local;}
    auto FindNearestJasonAICounselorTarget(std::uintptr_t,std::uintptr_t excluded){exclusions.push_back(excluded);return excluded?alternative:all;}
    void EnableAdditionalJasonAuthoredRunSpeed(std::uint64_t){++runs;}
    bool TryAdditionalJasonNativeDoor(std::uintptr_t,Position,Position,std::uint64_t,bool early){
        events.push_back(early?"earlyDoor":"door");return early?doorEarly:doorRegular;}
    bool RunJasonAINavRingTeleportOnGameThread(std::uintptr_t,std::uintptr_t){++morphs;return morph;}
    bool NavigationPending(){return pending;}
    auto StateIndex(){return index;}bool GrabReadable(){return grabReadable;}bool TryGrab(){++grabs;return grab;}
    bool AttackPressReadable(){return true;}bool PressAttack(){++presses;events.push_back("press");return press;}
};
int main(){unsigned checks=0;auto check=[&](bool b){++checks;assert(b);};
    {Host h;h.controller=99;check(!DonorFrozenBridgeDriveAdditionalJason(h)&&h.events.empty());}
    {Host h;h.world=99;check(!DonorFrozenBridgeDriveAdditionalJason(h)&&!h.s.Jason&&!h.bases);}
    {Host h;h.hosted=true;h.engineWorld=9;check(DonorFrozenBridgeDriveAdditionalJason(h)&&h.events.empty());}
    {Host h;h.phase=MatchPhase::NotInProgress;
        for(unsigned i=0;i<100;++i)DonorFrozenBridgeDriveAdditionalJason(h);
        check(h.stops==1&&!h.bases&&!h.presses&&h.binds==100);}
    {Host h;h.exclusive=true;for(unsigned i=0;i<100;++i)DonorFrozenBridgeDriveAdditionalJason(h);
        check(h.stops==1&&h.bases==100&&!h.presses&&!h.moves);}
    for(unsigned lane=0;lane<3;++lane){Host h;h.held=lane==0;h.knife=lane==1;h.s.KnifeActive=h.knife;h.car=lane==2;
        check(DonorFrozenBridgeDriveAdditionalJason(h)&&!h.moves&&!h.presses);
        check(h.events.front()=="base"&&h.events.back()==(lane==0?"held":lane==1?"knife":"car"));}
    {Host h;h.alternative=0;check(DonorFrozenBridgeDriveAdditionalJason(h)&&h.s.Target==h.local&&h.presses==1);
        check(h.exclusions.size()==2&&h.exclusions[0]==h.local&&!h.exclusions[1]);}
    {Host h;h.otherAt={8001,0,0};DonorFrozenBridgeDriveAdditionalJason(h);check(h.s.Target==h.local&&h.exclusions.size()==2);}
    {Host h;h.otherAt={8000,0,9999};DonorFrozenBridgeDriveAdditionalJason(h);check(h.s.Target==h.alternative&&h.exclusions.size()==1);}
    {Host h;h.otherAt={275,0,0};DonorFrozenBridgeDriveAdditionalJason(h);
        check(h.presses==1&&h.s.AttackReleaseAt==1120&&h.s.NextCombatAt==2137&&h.s.NextMoveAt==2542&&h.acceptance==150);
        h.now=1119;DonorFrozenBridgeDriveAdditionalJason(h);check(!h.releases);
        h.now=1120;DonorFrozenBridgeDriveAdditionalJason(h);check(h.releases==1&&!h.s.AttackPressed);}
    {Host h;h.otherAt={175,0,0};h.grabReadable=true;h.grab=true;DonorFrozenBridgeDriveAdditionalJason(h);
        check(h.grabs==1&&!h.presses&&h.s.NextGrabAt==5000&&h.s.NextCombatAt==1700);}
    {Host h;h.otherAt={6000,0,0};h.morph=true;DonorFrozenBridgeDriveAdditionalJason(h);
        check(h.morphs==1&&h.s.NextMorphAt==21000&&h.s.NextMoveAt==2000&&!h.moves);
        h.now=20999;DonorFrozenBridgeDriveAdditionalJason(h);check(h.morphs==1);
        h.now=21000;DonorFrozenBridgeDriveAdditionalJason(h);check(h.morphs==2);}
    {Host h;h.otherAt={6000,0,0};DonorFrozenBridgeDriveAdditionalJason(h);check(h.morphs==1&&h.s.NextMorphAt==7000&&h.moves==1);
        h.now=6999;DonorFrozenBridgeDriveAdditionalJason(h);check(h.morphs==1);
        h.now=7000;DonorFrozenBridgeDriveAdditionalJason(h);check(h.morphs==2);}
    {Host h;h.alternative=0;h.all=0;h.s.Target=99;DonorFrozenBridgeDriveAdditionalJason(h);
        check(h.stops==1&&!h.s.Target&&!h.presses&&!h.moves);}
    {Host h;h.hiding=true;h.otherAt={200,0,0};DonorFrozenBridgeDriveAdditionalJason(h);
        check(h.moves==1&&h.acceptance==110&&!h.presses&&!h.exclusions.size());}
    {Host h;h.doorEarly=true;h.s.DoorInteractionUntil=0;DonorFrozenBridgeDriveAdditionalJason(h);
        check(h.s.DoorNextEarlyProbeAt==1250&&!h.moves&&!h.presses);}
    {Host h;h.pending=true;h.otherAt={6000,0,0};DonorFrozenBridgeDriveAdditionalJason(h);
        check(h.morphs==1&&!h.moves&&!h.presses);}
    {Host h;h.runtime.isAdditional=true;
        check(DonorFrozenBridgeDriveAdditionalJason(h)&&h.moves==1);
        h.runtime.MarkUncertain(UncertainAction::Move,h.now);
        h.now=1999;check(!h.runtime.ClearSettledUncertainty(h.now)&&h.moves==1);
        h.now=2000;check(h.runtime.ClearSettledUncertainty(h.now)&&!h.s.Target&&h.s.NextMoveAt==2000);
        check(DonorFrozenBridgeDriveAdditionalJason(h)&&h.moves==2&&h.runtime.health.uncertainEvents==1);
    }
    std::printf("J1a donor F1 root: %u checks PASS; independent state,ordered action ownership,local fallback,80m XY,120ms release,175/275cm inputs,6s/20s Morph,per-index pacing. Native host pending.\n",checks);
}
