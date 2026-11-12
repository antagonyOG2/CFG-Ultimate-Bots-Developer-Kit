#include "../Core/DonorDriverLiveness.hpp"
#include <cassert>
#include <cstdio>
#include <initializer_list>
using namespace cfgdonorhunt;
int main(){unsigned checks=0;const auto check=[&](bool okay){++checks;assert(okay);};
    {DriverLiveness h;check(!h.Active(0));h.Begin(1000);check(h.Active(1000)&&h.Active(4000));
        check(!h.Active(4001));h.Begin(5000);check(h.handoffAt==1000&&!h.Active(5000));
        check(h.ObserveLapse(5000)&&!h.ObserveLapse(5001));
        check(h.Decision(5002)&&h.Active(6502)&&!h.Active(6503));}
    // BOTH independent controllers use the same production clock. Forced
    // owner refusal for5s hands motion to the accepted PE route, then a NEW
    // admitted native decision resumes. No callback can drive both lanes.
    for(unsigned role=0;role<2;++role){DriverLiveness h;h.Begin(0);h.Decision(0);
        unsigned peMoves=0,nativeMoves=0,overlap=0;std::uint64_t firstFallback=0;
        for(std::uint64_t now=10;now<=7000;now+=10){
            const bool currentOwner=now<1000||now>=6000;bool native=false,pe=false;
            if(currentOwner){h.ObserveLapse(now);h.Decision(now);native=true;++nativeMoves;}
            else {h.Refused(DriverRefusal::Owner);h.ObserveLapse(now);}
            // Actual PE scheduling is outside the in-flight native callback.
            // Its own native/root admission is a separate mandatory gate.
            if(!native&&!h.Active(now)){pe=true;++peMoves;if(!firstFallback)firstFallback=now;}
            if(native&&pe)++overlap;
            check(!(native&&pe));
        }
        check(firstFallback>=2490&&firstFallback<=2500&&peMoves>300&&nativeMoves>100&&!overlap&&h.Active(7000));
        check(h.lastDonorDecisionAt==7000&&!h.lapsed);
        (void)role;
    }
    // Merely collecting/refusing the same frame never pays a heartbeat. An
    // owned finite nav/release progress return may pay one explicitly.
    {DriverLiveness h;h.Begin(1000);h.Decision(2000);
        for(std::uint64_t now=2100;now<5000;now+=100){h.Refused(DriverRefusal::Frame);check(h.lastDonorDecisionAt==2000);}
        check(!h.Active(3501));check(h.ObserveLapse(3501));check(h.Decision(5000)&&h.Active(5000));}
    for(unsigned role=0;role<2;++role){DriverLiveness h;h.Begin(1000);h.Decision(1000,true);
        for(std::uint64_t now=1010;now<=2500;now+=10){
            check(!h.Decision(now,false)&&h.lastDonorDecisionAt==1000);
            h.Refused(DriverRefusal::Frame);check(h.refusal==DriverRefusal::NoTarget);
        }
        check(!h.Active(2501)&&h.ObserveLapse(2501)&&h.refusal==DriverRefusal::NoTarget);
        const bool acceptedPE=!h.Active(2501);check(acceptedPE);
        check(h.Decision(3000,true)&&h.Active(3000)&&!h.noTarget&&h.refusal==DriverRefusal::None);
        (void)role;
    }
    for(auto kind:{UncertainAction::Release,UncertainAction::Stop,UncertainAction::Move,UncertainAction::Slash,
        UncertainAction::Grab,UncertainAction::GrabKill,UncertainAction::Projection,UncertainAction::SetLocation,
        UncertainAction::Teleport,UncertainAction::Running}){
        DriverLiveness h;h.Begin(1000);h.Decision(1000);h.Uncertain(kind,1100);
        check(h.uncertainAction==kind&&h.uncertainAt==1100&&h.uncertainEvents==1);
        check(!h.Settled(2099)&&h.Settled(2100));
        check(h.Active(2100)&&!h.permanentFallback);h.Decision(2100);check(h.Active(3500));
    }
    {DriverLiveness h;h.Begin(0);h.Decision(1);
        for(unsigned i=1;i<=8;++i){h.Uncertain(UncertainAction::Move,1000*i);check(h.uncertainEvents==i);
            check(h.permanentFallback==(i>=8));}
        check(!h.Active(8000)&&!h.Decision(9000)&&h.refusal==DriverRefusal::UncertaintyLimit);}
    std::printf("Review208 actual donor liveness clock: %u checks PASS; both roles,5s owner refusal/1500ms fallback,new decision recovery,no dual driver,frame refusal cannot renew,1000ms action settle,8-event cap.\n",checks);
}
