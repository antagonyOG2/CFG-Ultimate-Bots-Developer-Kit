#include "../Core/DonorNativeTurn.hpp"
#include "../Core/DonorUnstick.hpp"
#include <array>
#include <cstdio>
struct Host {
    bool releaseBlocked=false,held=false,base=false,death=false;unsigned huntingReturn=0,hunts=0;
    std::array<unsigned,5> order{};unsigned count=0,moves=0,convergence=0,emotes=0;
    bool DrainOwedRelease(){order[count++]=1;return releaseBlocked;}
    bool DriveHeld(){order[count++]=2;if(held)++moves;return held;}
    bool HuntExclusive(){return death;}
    void Hunt(){order[count++]=3;++hunts;if(base&&!huntingReturn)++moves;}
    void Convergence(){order[count++]=4;++convergence;}
    void Emotes(){order[count++]=5;++emotes;}
};
int main(){unsigned checks=0,failures=0;const auto check=[&](bool value){++checks;if(!value)++failures;};
    for(unsigned release=0;release<2;++release)for(unsigned held=0;held<2;++held)
        for(unsigned base=0;base<2;++base)for(unsigned early=0;early<4;++early){
            Host h;h.releaseBlocked=release!=0;h.held=held!=0;h.base=base!=0;h.huntingReturn=early;
            cfgdonorturn::Service(h);
            check(h.order[0]==1);check(h.convergence==1&&h.emotes==1);
            check(h.order[h.count-2]==4&&h.order[h.count-1]==5);
            check(h.moves<=1);check(!release||h.count==3);
            check(release||!held||h.count==4); // held suppresses hunt, B/C remain
        }
    for(unsigned bits=0;bits<16;++bits){const bool driver=(bits&1)!=0,held=(bits&2)!=0,owed=(bits&4)!=0,water=(bits&8)!=0;
        check(cfgdonorturn::UseBaseTick(driver,held,owed,water)==(!water&&(driver||held||owed)));}
    {Host h;h.base=true;h.death=true; // an owed held stop returned, so release is no longer blocked
        cfgdonorturn::Service(h);check(!h.hunts&&!h.moves&&h.convergence==1&&h.emotes==1);}
    {cfgdonorturn::WaterCounter counter;check(counter.Observe(true,true,true)&&counter.ticks==1);
        check(!counter.Observe(false,true,true)&&counter.ticks==1&&counter.active);
        for(unsigned i=2;i<128;++i)check(!counter.Observe(true,true,true));
        check(counter.Observe(true,true,true)&&counter.ticks==128);
        check(counter.Observe(true,false,false)&&!counter.active&&counter.ticks==128);
        check(!counter.Observe(true,false,false));}
    { // H2.81 unstick timing (pure): water 8 s / wet target 20 s / frozen 12 s + 150 cm / busy, retry and recharge
        using namespace cfgunstick;
        {State s;for(std::uint64_t t=1000;t<=8000;t+=1000)check(s.Water(t,true,-1)==Action::None); // 7000 ms in water
            check(s.Water(9000,true,-1)==Action::NeedTarget&&s.Water(9000,true,0)==Action::WaterEscape); // 8000 ms, dry target
            s.Refused(Action::WaterEscape,9000,Reason::Refused);check(s.rows.waterRefused==1);
            for(std::uint64_t t=10000;t<=11000;t+=1000)check(s.Water(t,true,0)==Action::None);
            check(s.Water(12000,true,0)==Action::WaterEscape); // 3 s retry after a refusal
            s.Accepted(Action::WaterEscape,12000);check(s.rows.waterEscapes==1);
            for(std::uint64_t t=13000;t<=14000;t+=1000)check(s.Water(t,true,0)==Action::None);
            check(s.Water(15000,true,0)==Action::WaterEscape);
            check(s.Water(15100,false,0)==Action::None&&s.waterSince==0); // left water resets the clock
            for(std::uint64_t t=16000;t<=23000;t+=1000)check(s.Water(t,true,0)==Action::None); // 8 s counts from re-entry
            check(s.Water(24000,true,0)==Action::WaterEscape);}
        {State s;s.Water(1000,true,-1);for(std::uint64_t t=2000;t<=8000;t+=1000)check(s.Water(t,true,-1)==Action::None);
            check(s.Water(9000,true,1)==Action::None&&s.rows.lastReason==static_cast<std::uint32_t>(Reason::WaterWetTarget)); // wet target: wait
            for(std::uint64_t t=10000;t<=20000;t+=1000)check(s.Water(t,true,1)==Action::None);
            check(s.Water(21000,true,-1)==Action::NeedTarget&&s.Water(21000,true,1)==Action::WaterEscape);} // 20 s with a wet target
        {State s;s.Water(1000,true,0);check(s.Water(5000,true,0)==Action::None&&s.waterSince==5000);} // a 4 s observation gap (busy, held) restarts the clock
        {State s;check(s.SampleDue(1000)&&s.Frozen(1000,100,100,0)==Action::None);
            check(!s.SampleDue(1499)&&s.SampleDue(1500)&&s.Frozen(1500,200,100,0)==Action::None); // 100 cm: still inside 150
            for(std::uint64_t t=2000;t<13000;t+=500){check(s.SampleDue(t));check(s.Frozen(t,100,100,0)==Action::None);}
            check(s.SampleDue(13000)&&s.Frozen(13000,160,100,0)==Action::FrozenEscape); // 60 cm from the anchor for 12 s
            s.Refused(Action::FrozenEscape,13000,Reason::Busy);check(s.rows.frozenRefused==1&&s.rows.lastReason==static_cast<std::uint32_t>(Reason::Busy));
            for(std::uint64_t t=13500;t<16000;t+=500){check(s.SampleDue(t));check(s.Frozen(t,160,100,0)==Action::None);}
            check(s.SampleDue(16000)&&s.Frozen(16000,160,100,0)==Action::FrozenEscape); // 3 s retry after a refusal
            s.Accepted(Action::FrozenEscape,16000);check(s.rows.frozenEscapes==1&&!s.haveAnchor);
            check(s.SampleDue(16500)&&s.Frozen(16500,5000,100,0)==Action::None); // teleported: new anchor
            for(std::uint64_t t=17000;t<36000;t+=500){check(s.SampleDue(t));check(s.Frozen(t,5000,100,0)==Action::None);} // 20 s recharge holds
            check(s.SampleDue(36000)&&s.Frozen(36000,5000,100,0)==Action::FrozenEscape);}
        {State s;s.SampleDue(1000);s.Frozen(1000,0,0,0);check(s.Frozen(12000,149,0,0)==Action::None);
            check(s.Frozen(13000,0,0,0)==Action::FrozenEscape);
            check(s.Frozen(14000,150,0,0)==Action::None&&s.haveAnchor&&s.anchorAt==14000);} // 150 cm = moved, re-anchor
        {State s;s.SampleDue(1000);s.Frozen(1000,0,0,0);check(s.SampleDue(5000)&&!s.haveAnchor); // a busy lane hid him for 4 s: measuring restarts
            s.Frozen(5000,0,0,0);check(s.SampleDue(20000)&&!s.haveAnchor&&s.Frozen(20000,0,0,0)==Action::None&&s.Frozen(20500,0,0,0)==Action::None);}
        {State s;check(s.Frozen(1000,static_cast<float>(NAN),0,0)==Action::None&&!s.haveAnchor);}
    }
    std::printf("Production native turn ordering/fallback/held ownership: %u checks,%u failures (Ready/Census/External/lapse,original/base,water original+bounded counters,death-exclusive after owed stop,release deferral).\n",checks,failures);
    return failures?1:0;
}
