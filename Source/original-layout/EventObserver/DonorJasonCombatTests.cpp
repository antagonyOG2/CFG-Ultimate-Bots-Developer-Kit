#include "../Core/DonorJasonCombat.hpp"
#include <cassert>
#include <cstdio>
#include <string>
using namespace cfgdonorhunt;
struct Host {
    CombatState s;std::uint64_t now=100;Position at{},targetAt{250,0,0};
    bool actor=true,module=true,releaseReadable=true,knife=false,throwKnife=false,door=false,
        grabReadable=true,grabOK=true,pressReadable=true,pressOK=true;
    unsigned releases=0,presses=0,grabs=0,killInputs=0,lastDirection=9;std::string order;
    Host(){s.Jason=2;}
    CombatState& Combat(){return s;}bool ReadableActor(std::uintptr_t){return actor;}
    std::uint64_t Now(){return now;}bool ModuleReady(){return module;}
    bool AttackReleaseReadable(){return releaseReadable;}void ReleaseAttack(){++releases;order+='R';}
    bool TickJasonAIKnifeSequenceOnGameThread(std::uint64_t){order+='K';return knife;}
    void UseReflectedGrabKill(unsigned d){++killInputs;lastDirection=d;order+='G';}
    std::uintptr_t FindNearestJasonAICounselorTarget(std::uintptr_t){return 4;}
    bool GetJasonAIActorLocation(std::uintptr_t who,Position& p){p=who==2?at:targetAt;return true;}
    bool TryStartJasonAIKnifeThrowOnGameThread(std::uintptr_t,float,std::uint64_t){order+='T';return throwKnife;}
    bool SelectedInteractableIsSCDoor(){order+='D';return door;}
    bool GrabReadable(){return grabReadable;}bool TryGrab(){++grabs;return grabOK;}
    bool AttackPressReadable(){return pressReadable;}bool PressAttack(){++presses;return pressOK;}
    bool Tick(){return DonorRunJasonAICombatOnGameThread(*this);}
};
int main(){unsigned checks=0;auto check=[&](bool x){++checks;assert(x);};
    {Host h;h.s.AttackPressed=true;h.s.AttackReleaseAt=100;h.knife=true;
        check(h.Tick()&&h.order=="RK"&&h.releases==1&&!h.s.AttackPressed&&!h.presses);}
    {Host h;h.s.AttackPressed=true;h.s.AttackReleaseAt=101;h.knife=true;
        check(h.Tick()&&!h.releases&&h.s.AttackPressed);++h.now;check(h.Tick()&&h.releases==1);}
    {Host h;h.targetAt.X=260;check(h.Tick()&&h.presses==1&&h.s.AttackReleaseAt==220&&h.s.NextCombatAttemptAt==1150);
        h.now=219;check(h.Tick()&&!h.releases);h.now=220;check(h.Tick()&&h.releases==1&&!h.s.AttackPressed);}
    {Host h;h.targetAt.X=260.01f;check(h.Tick()&&!h.presses&&!h.grabs&&h.s.NextCombatAttemptAt==0);}
    {Host h;h.targetAt={0,0,261};check(h.Tick()&&!h.presses);}
    {Host h;h.targetAt.X=220;check(h.Tick()&&h.grabs==1&&h.s.GrabKillReadyAt==400&&h.s.NextCombatAttemptAt==950);
        h.now=399;check(h.Tick()&&!h.killInputs);h.now=400;check(h.Tick()&&h.killInputs==1&&h.lastDirection==0&&
            h.s.NextGrabKillDirection==1&&h.s.NextCombatAttemptAt==1100&&!h.s.GrabKillReadyAt);}
    {Host h;h.targetAt.X=220;h.grabOK=false;check(h.Tick()&&h.grabs==1&&!h.s.GrabKillReadyAt&&!h.presses&&h.s.NextCombatAttemptAt==950);}
    {Host h;h.targetAt.X=220;h.grabReadable=false;check(h.Tick()&&h.presses==1&&!h.grabs);}
    {Host h;h.door=true;check(h.Tick()&&!h.presses&&!h.grabs&&!h.s.CombatAttemptCounter);}
    {Host h;h.throwKnife=true;check(h.Tick()&&h.order=="KT"&&!h.presses&&!h.grabs);}
    {Host h;h.pressOK=false;check(h.Tick()&&h.presses==1&&!h.s.AttackPressed&&h.s.NextCombatAttemptAt==1150);}
    {Host h;h.targetAt.X=200;
        for(unsigned decision=0;decision<9;++decision){h.now+=2000;h.s.GrabKillReadyAt=0;check(h.Tick());}
        check(h.grabs==3&&h.presses==6&&h.s.CombatAttemptCounter==9);}
    {Host h;for(unsigned d=0;d<8;++d){h.now+=1000;h.s.GrabKillReadyAt=h.now;
        check(h.Tick()&&h.lastDirection==(d&3));}check(h.killInputs==8&&!h.presses&&!h.grabs);}
    // Callback cadence measures the ACTUAL body: a new 250 cm target swings
    // on first callback, <=16.7 ms after reaching range at 60 Hz; release owed
    // at120ms is serviced even while subsequent combat cooldown remains open.
    {Host h;h.targetAt.X=1000;std::uint64_t firstPress=0,firstRelease=0;
        for(unsigned frame=0;frame<3600;++frame){h.now=100+(frame*1000/60);if(frame>=100)h.targetAt.X=250;
            const auto p=h.presses,r=h.releases;check(h.Tick());
            if(!firstPress&&h.presses!=p)firstPress=h.now;
            if(!firstRelease&&h.releases!=r)firstRelease=h.now;}
        check(firstPress==1766&&firstRelease-firstPress>=120&&firstRelease-firstPress<=137&&h.presses>50);}
    std::printf("J1a donor combat: %u checks PASS; immediate range/door/knife order, 120ms release, 300/700/850/1050ms, grab rotation. Native host pending.\n",checks);
}
