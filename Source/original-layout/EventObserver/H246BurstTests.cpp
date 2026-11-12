#include "../Core/DonorJasonNavContinuation.hpp"
#include "../Core/OnFootBurstPolicy.hpp"
#include "../Core/CounselorPursuitPolicy.hpp"
#include "../Core/H246BurstPolicy.hpp"
#include <cstdio>
#include <vector>
using namespace cfgdonorhunt;
static unsigned failures=0,checks=0;
static void check(bool b,const char* what){++checks;if(!b){++failures;std::printf("FAIL: %s\n",what);}}
// Mimics the runtime host: the first Project is entered and True; the roof-guard probes after it are
// budget-starved ("NotEntered") for the first `starveFor` slices of every candidate.
struct RingHost {
    NavRingContinuation* nav=nullptr;unsigned starveFor=0,sliceOfCandidate=0,lastCandidate=99,teleports=0,projects=0;
    NavProof Current(const NavRingIdentity&){return NavProof::Current;}
    NativeOutcome Project(const NavRingIdentity&,Position p,Position,Position& out){
        if(nav->candidate!=lastCandidate){lastCandidate=nav->candidate;sliceOfCandidate=0;}
        ++projects;out=p;
        if(sliceOfCandidate++<starveFor)return nav->RoofRejected(true);
        return NativeOutcome::ReturnedTrue;
    }
    NativeOutcome Teleport(const NavRingIdentity&,Position){++teleports;return NativeOutcome::ReturnedTrue;}
};
static NavRingIdentity Key(){NavRingIdentity k;k.module=0x100000;k.thread=7;
    for(unsigned i=0;i<4;++i)k.slots[i]={0x200000+0x1000*i,static_cast<int>(i+1),static_cast<int>(i+8)};return k;}
struct BurstHost {
    std::array<float,4> v{600,600,600,600};unsigned exchanges=0;
    bool Observe(std::array<float,4>& o){o=v;return true;}
    bool Exchange(unsigned i,float b,float a){if(v[i]!=b)return false;v[i]=a;++exchanges;return true;}
    bool Restore(unsigned i,float b,float a){if(v[i]==b)v[i]=a;return true;}
};
int main(){
    // ---- Task 2: starved probes must not burn candidates ----
    {RingHost h;h.starveFor=3;NavRingContinuation s;h.nav=&s;check(s.Begin(Key(),{100,200,90},NavRingPurpose::Distance),"begin");
        unsigned n=0;while(s.Pending()&&n++<400)DonorContinueNavRing(h,s);
        if(CFG_H246_TELEPORT_CONFIRM){check(s.stage==NavRingStage::Succeeded&&h.teleports==1&&s.candidate==0,"starved x3 retried, candidate 0 teleports");
            check(s.probeStarvedRetried==3&&s.probeStarvedGaveUp==0&&s.roofRejects==0,"retry counters");}
        else check(s.stage==NavRingStage::Failed&&h.teleports==0,"legacy: every starved probe burns a candidate -> Failed, no Teleport");}
    {RingHost h;h.starveFor=1000;NavRingContinuation s;h.nav=&s;s.Begin(Key(),{100,200,90},NavRingPurpose::Distance);
        unsigned n=0;while(s.Pending()&&n++<4000)DonorContinueNavRing(h,s);
        check(s.stage==NavRingStage::Failed&&h.teleports==0&&n<4000,"permanently starved ring still terminates");
        if(CFG_H246_TELEPORT_CONFIRM)check(s.probeStarvedGaveUp==16&&s.probeStarvedRetried==16*NavRingContinuation::MaxStarvedRetries,"bounded retries per candidate");}
    {NavRingContinuation s;check(s.RoofRejected(false)==NativeOutcome::ReturnedFalse&&s.roofRejects==1&&s.probeStarved==0,"genuine roof rejection still rejects");}
    {struct H:RingHost{NativeOutcome Project(const NavRingIdentity&,Position p,Position,Position& o){++projects;o={p.X,p.Y,p.Z+500};return NativeOutcome::ReturnedTrue;}};
     H h;NavRingContinuation s;h.nav=&s;s.Begin(Key(),{0,0,90},NavRingPurpose::Initial);while(s.Pending())DonorContinueNavRing(h,s);
     check(s.stage==NavRingStage::Failed&&s.dzRejects==16&&s.horizontalRejects==0&&h.teleports==0,"dz>120 counted");}
    {struct H:RingHost{NativeOutcome Project(const NavRingIdentity&,Position p,Position,Position& o){++projects;o={0,0,p.Z};return NativeOutcome::ReturnedTrue;}};
     H h;NavRingContinuation s;h.nav=&s;s.Begin(Key(),{0,0,90},NavRingPurpose::Initial);while(s.Pending())DonorContinueNavRing(h,s);
     check(s.stage==NavRingStage::Failed&&s.horizontalRejects==16&&s.dzRejects==0,"horizontal<1700 counted");}
    // ---- Task 1: burst under the lease ----
    {cfgfootburst::Policy p;BurstHost h;
        check(p.Begin(h,1000,0x10,true)&&p.Owned()&&p.leaseMode&&!p.ExternalOwned()&&p.leaseStarts==1,"lease burst owned, no donor bit 9");
        for(std::uint64_t t=1000;p.Owned()&&t<9000;t+=50){p.Step(h,t,false);if(p.stage==cfgfootburst::Stage::Hold)check(!p.ExternalOwned(),"hold: bit9 clear");}
        check(!p.Owned()&&h.v[0]==600&&h.v[2]==600,"restored to original");
        check(p.readyAt>=1000+cfgfootburst::DurationMs+cfgfootburst::CooldownMs&&p.readyAt<=1000+cfgfootburst::DurationMs+cfgfootburst::CooldownMs+500,"3 s / 7 s cooldown unchanged (7 s after restore)");
        check(!p.Begin(h,p.readyAt-1,0x10,true)||!p.Owned(),"cooldown respected");}
    {cfgfootburst::Policy p;BurstHost h;p.Begin(h,1000,0x10,false);check(p.Owned()&&p.ExternalOwned(),"offline/no-lease burst keeps bit 9 behaviour");}
    {cfgfootburst::Policy p;BurstHost h;p.Begin(h,1000,0x10,true);BurstHost g=h;h.v[0]=777.f; // donor wrote Walk mid-burst
        while(p.Owned())p.Step(h,1000+4000,true);check(h.v[0]==777.f,"foreign donor value is never overwritten on restore");}
    check(cfgh246::BurstWanted(true,500.f*500.f,false,100,100)==(CFG_H246_BURST!=0),"burst wanted under lease follows flag");
    check(!cfgh246::BurstWanted(true,300.f*300.f,false,100,100)&&(cfgh246::BurstWanted(false,2100.f*2100.f,false,100,100)==(CFG_H267_BURST_ANY_RANGE!=0))&&
          !cfgh246::BurstWanted(false,500.f*500.f,true,100,100)&&!cfgh246::BurstWanted(false,500.f*500.f,false,99,100)&&
          cfgh246::BurstWanted(false,500.f*500.f,false,100,100),"4-20 m / owned / ready gates unchanged");
    // ---- Morph under the lease: shared 20 s clock, 7 s counselor deadline ----
    {cfgvehicle::Cooldown shared;
        shared.Attempted(1000); // donor distance/fifty Morph or counselor Morph both publish next=now+20000
        check(!cfgpursuit::MorphDue(shared,7999,3000.f*3000.f)&&cfgpursuit::MorphDue(shared,8000,3000.f*3000.f),"counselor Morph due 7 s after ANY Morph (shared clock)");
        check(shared.next==21000,"donor-visible shared clock is 20 s");
        cfgh246::MorphQueueInputs in;in.leasePublished=true;in.optionalProbe=in.probeMorphFirst=true;in.morphDueSuperset=in.doorGraceOver=in.retryOver=in.moveOver=in.cabinsReady=true;
        check(cfgh246::MorphQueue(in)==(CFG_H246_BURST!=0),"lease: queued when due");
        auto x=in;x.donorNavPending=true;check(!cfgh246::MorphQueue(x),"never overlaps a donor nav ring in flight");
        x=in;x.donorTrapIntent=true;check(!cfgh246::MorphQueue(x),"never overlaps donor trap-victim Morph");
        x=in;x.morphDueSuperset=false;check(!cfgh246::MorphQueue(x),"not due: bit 10 is not raised (donor keeps driving)");
        x=in;x.leasePublished=false;x.morphDueSuperset=false;check(cfgh246::MorphQueue(x),"no lease: J2.19 offline admission unchanged");}
    std::printf("H246BurstTests checks=%u failures=%u %s\n",checks,failures,failures?"FAIL":"PASS");
    return failures?1:0;
}
