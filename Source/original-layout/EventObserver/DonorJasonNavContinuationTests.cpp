#include "../Core/DonorJasonNavContinuation.hpp"
#include "../Core/ChaseLead.hpp"
#include "../Core/DonorUnstick.hpp"
#include <cassert>
#include <cstdio>
#include <vector>
using namespace cfgdonorhunt;
struct Host {
    unsigned budget=12,entered=0,candidate=0,successAt=15,proofs=0;
    bool changed=false,unavailable=false,uncertain=false,rejectProjection=false;
    std::vector<Position> projects,teleports;
    NavProof Current(const NavRingIdentity&){++proofs;return unavailable?NavProof::Unavailable:changed?NavProof::Changed:NavProof::Current;}
    NativeOutcome Project(const NavRingIdentity&,Position p,Position extent,Position& out){
        assert(extent.X==350&&extent.Y==350&&extent.Z==110);
        if(entered>=budget)return NativeOutcome::NotEntered;
        ++entered;projects.push_back(p);out=p;
        if(rejectProjection)return NativeOutcome::ReturnedFalse;
        return NativeOutcome::ReturnedTrue;
    }
    NativeOutcome Teleport(const NavRingIdentity&,Position p){
        if(entered>=budget)return NativeOutcome::NotEntered;
        ++entered;teleports.push_back(p);
        if(uncertain)return NativeOutcome::Uncertain;
        return candidate++==successAt?NativeOutcome::ReturnedTrue:NativeOutcome::ReturnedFalse;
    }
};
NavRingIdentity Key(){NavRingIdentity k;k.module=0x100000;k.thread=7;
    for(unsigned i=0;i<4;++i)k.slots[i]={0x200000+0x1000*i,static_cast<int>(i+1),static_cast<int>(i+8)};
    return k;}
int main(){unsigned checks=0;const auto check=[&](bool b){++checks;assert(b);};
    for(unsigned cap=1;cap<=12;++cap){Host h;h.budget=cap;NavRingContinuation s;
        check(s.Begin(Key(),{100,200,90},NavRingPurpose::Initial));unsigned slices=0;
        while(s.Pending()&&slices<40){h.entered=0;DonorContinueNavRing(h,s);check(h.entered<=cap);++slices;}
        check(s.stage==NavRingStage::Succeeded&&s.candidate==15&&h.projects.size()==16&&h.teleports.size()==16);
        check(h.projects.front().X==2300&&h.projects[8].X==3100&&h.projects.back().X==-2021&&h.projects.back().Y==-1921);
        for(unsigned i=0;i<16;++i)check(h.projects[i].X==h.teleports[i].X&&h.projects[i].Y==h.teleports[i].Y&&h.projects[i].Z==25);
    }
    {Host h;h.budget=1;NavRingContinuation s;check(s.Begin(Key(),{0,0,90},NavRingPurpose::Distance));
        check(DonorContinueNavRing(h,s)==NavRingStage::Teleport&&h.projects.size()==1&&h.teleports.empty());
        h.changed=true;h.entered=0;check(DonorContinueNavRing(h,s)==NavRingStage::Invalid&&h.teleports.empty());}
    {Host h;h.uncertain=true;NavRingContinuation s;check(s.Begin(Key(),{0,0,90},NavRingPurpose::Stuck));
        check(DonorContinueNavRing(h,s)==NavRingStage::Uncertain&&h.teleports.size()==1&&s.uncertainTeleport);
        h.entered=0;check(DonorContinueNavRing(h,s)==NavRingStage::Uncertain&&h.teleports.size()==1);
        check(!s.Begin(Key(),{0,0,90},NavRingPurpose::Initial));}
    {Host h;h.unavailable=true;NavRingContinuation s;check(s.Begin(Key(),{0,0,90},NavRingPurpose::SharedFifty));
        check(DonorContinueNavRing(h,s)==NavRingStage::Project&&s.candidate==0&&h.projects.empty());
        h.unavailable=false;h.rejectProjection=true;h.budget=12;DonorContinueNavRing(h,s);
        check(s.candidate==12&&h.projects.size()==12&&h.teleports.empty());h.entered=0;
        check(DonorContinueNavRing(h,s)==NavRingStage::Failed&&h.projects.size()==16&&h.teleports.empty());}
    {auto k=Key();k.slots[3].serial=0;NavRingContinuation s;check(!s.Begin(k,{0,0,90},NavRingPurpose::Initial));}
#if CFG_H284_MORPH_CUTOFF
    // H2.84 cut-off: runner heading +Y at 600 cm/s. First 4 landings straight ahead at 7/10/14/18 m (500 cm floor), then the ring most-ahead first.
    {Host h;h.successAt=1;NavRingContinuation s;check(s.Begin(Key(),{0,0,90},NavRingPurpose::SharedFifty));
        check(!s.Lead(50,50)&&!s.lead); // walking pace keeps donor order
        check(s.Lead(0,600)&&s.count==20&&!s.Lead(0,600));
        for(unsigned i=0;i<6&&s.Pending();++i){h.entered=0;DonorContinueNavRing(h,s);}
        check(s.stage==NavRingStage::Succeeded&&h.projects.size()==2&&s.Index()==17);
        check(h.projects[0].X==0&&h.projects[0].Y==700&&h.projects[1].Y==1000);
        check(s.order[4]==2&&s.order[5]==10&&s.order.back()==11);} // 22 m ahead, 30 m ahead ... 30 m behind last
    {Host h;h.rejectProjection=true;NavRingContinuation s;check(s.Begin(Key(),{0,0,90},NavRingPurpose::Distance)&&s.Lead(-400,0));
        for(unsigned i=0;i<10&&s.Pending();++i){h.entered=0;DonorContinueNavRing(h,s);}
        check(s.stage==NavRingStage::Failed&&h.projects.size()==20&&h.projects[0].X==-700&&h.projects[4].X==-2200);}
    {NavRingContinuation s;check(s.Begin(Key(),{0,0,90},NavRingPurpose::Stuck));check(s.Index()==0&&s.count==16);}
    {cfgcutoff::Tracker t;float vx=0,vy=0;t.Note(0x10,0,0,1000);check(!t.Heading(0x10,1000,vx,vy));
        t.Note(0x10,50,0,1100);t.Note(0x10,300,0,1500);check(t.Heading(0x10,1600,vx,vy)&&vx>599&&vx<601&&vy==0);
#if CFG_H295_HEADING_FALLBACK
        // H2.95: fresh samples stale (now-at>1.5 s) -> last running heading while it is at most 2 s old; then nothing.
        vx=vy=0;check(t.Heading(0x10,3100,vx,vy)&&vx>599&&vx<601&&!t.Heading(0x10,3600,vx,vy)&&!t.Heading(0x20,1600,vx,vy));
        // A stop: fresh samples show ~0 cm/s, fallback keeps the last running direction (within 2 s of it).
        t.Note(0x10,302,0,3700);t.Note(0x10,303,0,3960);vx=vy=0;check(!t.Heading(0x10,4000,vx,vy)); // last run sample at 1500 is >2 s old
        {cfgcutoff::Tracker u;u.Note(0x30,0,0,1000);u.Note(0x30,300,0,1500);u.Note(0x30,302,0,1800);vx=vy=0;
         check(u.Heading(0x30,1900,vx,vy)&&vx>599&&vx<601);} // stopped 300 ms ago: still the running heading
#else
        check(!t.Heading(0x10,3100,vx,vy)&&!t.Heading(0x20,1600,vx,vy));
#endif
        }
#endif
#if CFG_H286_BURST_LEAD
    // H2.86 burst lead: counselor at (0,0) running +X 600 cm/s; Jason 3000 cm behind -> aim 1200 cm ahead; Jason 500 cm behind -> cap 500.
    {float x=0,y=0;check(cfgchaselead::Lead(0,0,-3000,0,600,0,x,y)&&x>1379&&x<1381&&y==0); // 1200 + 180 cm overshoot
#if CFG_H295_LEAD_CAP_RELAX
     check(cfgchaselead::Lead(0,0,-500,0,600,0,x,y)&&x>929&&x<931); // capped 1.5x500=750 + 180
#else
     check(cfgchaselead::Lead(0,0,-500,0,600,0,x,y)&&x>679&&x<681); // capped 500 + 180
#endif
     x=y=7;check(!cfgchaselead::Lead(0,0,-3000,0,100,0,x,y)&&x==7&&y==7);} // walking pace: unchanged
#endif
#if CFG_H288_UNSTICK_OTHER_TARGET
    // H2.88/H2.90: an exhausted unstick ring avoids that counselor for 30 s (any mode); null target is ignored.
    {cfgunstick::State u;check(!u.Avoid(1000));u.RingFailed(0,1000);check(!u.Avoid(1000)&&!u.failedRings);
     u.RingFailed(0xA0,1000);check(u.Avoid(1000)==0xA0&&u.Avoid(30999)==0xA0&&!u.Avoid(31000)&&u.failedRings==1);
     u.RingFailed(0xB0,40000);check(u.Avoid(40001)==0xB0&&u.failedRings==2);}
#endif
    std::printf("J1a donor navigation continuation: %u checks PASS; call caps1-12, all16 candidates reachable, retained projection, identity change and uncertain-entry no replay.\n",checks);
}
