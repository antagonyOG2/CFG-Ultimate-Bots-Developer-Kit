#ifndef CFG_J21C_HELD_RECOVERY
#define CFG_J21C_HELD_RECOVERY 0
#endif
#include "../Core/DonorHeldGrabKill.hpp"
#include "../Core/H247CombatPolicy.hpp"
#include <cstdio>
using namespace cfgdonorgrab;
static unsigned failures=0,checks=0;
static void check(bool ok,const char* what){++checks;if(!ok){++failures;std::printf("FAIL: %s\n",what);}}
static cfgh247::LegacyBusyView Idle(){cfgh247::LegacyBusyView v;v.jason=0x1000;v.actorReadable=true;v.specialReadable=true;
    v.flagsReadable=true;v.stunReadable=true;v.heldReadable=true;return v;}
static bool Busy(cfgh247::LegacyBusyView v,std::uint32_t* causes=nullptr){return cfgh247::LegacyCombatBusy(v,5000,causes);}
// Minimal held host: Observe returns the configured sample.
struct Host {
    Observation sample{};bool observed=true;unsigned probed=0,preserved=0;
    bool Observe(Observation& o){o=sample;return observed;}
    void HoldReleased(const Hold&,std::uint64_t){}void ClearSpeculativeCombatTimer(){}void LogSlots(const Hold&){}
    void PreserveExtraction(const Observation&,std::uint64_t){++preserved;}
    Outcome ReleaseStale(const Hold&,std::uint64_t){return Outcome::Returned;}
    Outcome ProbeSlot(const Hold&,unsigned,bool& v){++probed;v=false;return Outcome::Returned;}
    Outcome ProbeOpenDirection(const Hold&,const Position&,unsigned,float,float& f,Position& p,bool& s){f=0;p={};s=false;return Outcome::Returned;}
    Outcome MoveHolding(const Hold&,const Position&){return Outcome::Returned;}
    Outcome StopHolding(const Hold&){return Outcome::Returned;}
    Outcome Kill(const Hold&,unsigned){return Outcome::Returned;}
    void LogAttempt(const Hold&,unsigned,unsigned,Outcome){}void CombatDelayed(std::uintptr_t,std::uint64_t){}
    std::uint32_t Random(){return 1;}
};
static Observation Sample(bool seat,bool extracting){Observation o{};o.hold={0x100000,0x200000,0x300000,2,3,1,1};o.allowed=true;o.seat=seat;o.extracting=extracting;return o;}
int main(){
    // ---- Part 1: combat-busy veto for the lease-mode legacy Morph / burst ----
    {std::uint32_t causes=0;check(!Busy(Idle(),&causes)&&!causes,"idle Jason: not busy, no cause");
     check(cfgh247::MorphBusyVeto(true,false)==false,"lease + idle: Morph allowed");}
    struct Case{const char* name;void(*set)(cfgh247::LegacyBusyView&);cfgdonorhunt::BusyCause cause;};
    const Case cases[]={
        {"knife sequence",[](cfgh247::LegacyBusyView& v){v.busy.KnifeSequenceActive=true;},cfgdonorhunt::BusyCause::KnifeSequence},
        {"attack pressed",[](cfgh247::LegacyBusyView& v){v.combat.AttackPressed=true;},cfgdonorhunt::BusyCause::AttackPressed},
        {"grab-kill ready",[](cfgh247::LegacyBusyView& v){v.combat.GrabKillReadyAt=123;},cfgdonorhunt::BusyCause::GrabKillReady},
        {"door break",[](cfgh247::LegacyBusyView& v){v.busy.DoorBreakActive=true;},cfgdonorhunt::BusyCause::DoorBreak},
        {"native special move (grab/kill montage)",[](cfgh247::LegacyBusyView& v){v.special=true;},cfgdonorhunt::BusyCause::NativeSpecialMove},
        {"stunned",[](cfgh247::LegacyBusyView& v){v.stunned=true;},cfgdonorhunt::BusyCause::Stunned},
        {"grabbed counselor",[](cfgh247::LegacyBusyView& v){v.held=0x55;},cfgdonorhunt::BusyCause::GrabbedCounselor},
        {"driver busy flag 0",[](cfgh247::LegacyBusyView& v){v.flags[0]=1;},cfgdonorhunt::BusyCause::BusyFlag0},
        {"unreadable special move is a conservative veto",[](cfgh247::LegacyBusyView& v){v.specialReadable=false;},cfgdonorhunt::BusyCause::SpecialMoveReadFailure},
        {"unreadable stun is a conservative veto",[](cfgh247::LegacyBusyView& v){v.stunReadable=false;},cfgdonorhunt::BusyCause::StunnedReadFailure},
        {"unreadable actor is a conservative veto",[](cfgh247::LegacyBusyView& v){v.actorReadable=false;},cfgdonorhunt::BusyCause::ActorUnreadable},
    };
    for(const auto& c:cases){auto v=Idle();c.set(v);std::uint32_t causes=0;const bool busy=Busy(v,&causes);
        char what[160];std::snprintf(what,sizeof what,"busy: %s -> busy with its cause bit",c.name);
        check(busy&&(causes&cfgdonorhunt::BusyCauseBit(c.cause)),what);
        std::snprintf(what,sizeof what,"lease Morph vetoed: %s",c.name);check(cfgh247::MorphBusyVeto(true,busy)==(CFG_H247_MORPH_BUSY!=0),what);
        std::snprintf(what,sizeof what,"offline (no lease) never vetoed: %s",c.name);check(!cfgh247::MorphBusyVeto(false,busy),what);}
    // ---- Part 2: held grab-kill wait split; a stale (non-vehicle) seat no longer holds the grab ----
    {State s;Host h;h.sample=Sample(true,false);
     for(std::uint64_t t=1000;t<1000+2500;t+=70)s.Drive(h,t);
     check(s.seatWaitsSeat>0&&s.seatWaitsExtracting==0&&!h.probed,"proven seat: waits counted as SEAT only, no probing");}
    {State s;Host h;h.sample=Sample(false,true);
     for(std::uint64_t t=1000;t<1000+2500;t+=70)s.Drive(h,t);
     check(s.seatWaitsExtracting>0&&s.seatWaitsSeat==0&&!h.probed,"active extraction: waits counted as EXTRACTING only");}
    {State s;Host h;h.sample=Sample(false,false); // stale CurrentSeat resolved by DonorHeldSeatState -> seat=false
     for(std::uint64_t t=1000;t<1000+600;t+=70)s.Drive(h,t);
     check(s.seatWaitsSeat==0&&s.seatWaitsExtracting==0&&h.probed>0,"stale seat cleared: no wait, normal Probing stage runs immediately");}
    {State s;Host h;h.sample=Sample(true,false);for(std::uint64_t t=1000;t<=1000+ExtractionHoldWaitMs+100;t+=70)s.Drive(h,t);
     check(CFG_H244_EXTRACTION_WAIT_BOUND?h.probed>0:true,"3 s bound still releases a genuine seat wait (H2.44 unchanged)");}
    std::printf("%s checks=%u failures=%u\n",failures?"FAIL":"PASS",checks,failures);
    return failures?1:0;
}
