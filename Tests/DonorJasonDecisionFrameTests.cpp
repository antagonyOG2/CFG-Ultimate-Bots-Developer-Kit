#include "DonorJasonDecisionFrame.hpp"
#include <cassert>
#include <cstdio>
using namespace cfgdonorframe;
struct Host {
    unsigned cap=4,used=0,samples=0;int dead=-1;bool current=true,exhausted=false,origin=true;
    std::array<unsigned,64> visits{};
    bool Current(const cfgjasontick::Owner&){return current;}
    bool BudgetExhausted(){return exhausted;}
    bool TargetBudgetAvailable(){return used<cap;}
    bool Observe(const Slot& slot,cfgdonorhunt::CounselorObservation& value,cfgdonorhunt::Position& p){
        const auto i=static_cast<unsigned>(slot.index-10);assert(i<64);
        if(exhausted)return false;
        ++used;++samples;++visits[i];value.pawn=slot;value.controller={0x500000+0x1000*i,100+static_cast<int>(i),200+static_cast<int>(i)};
        value.possessed=true;value.health=static_cast<int>(i)==dead?0.0f:100.0f;p={static_cast<float>(6400-100*i),0,0};return true;
    }
    bool Origin(const Slot&,cfgdonorhunt::Position& point){point={0,0,0};return origin;}
};
Source Input(unsigned n){Source s;s.count=n;s.revision=1;s.owner.source.module=0x100000;s.owner.source.threadId=7;
    s.owner.actor.pawn={0x200000,1,10};s.owner.actor.killerController={0x210000,2,11};
    for(unsigned i=0;i<n;++i)s.targets[i]={0x300000+0x1000*i,10+static_cast<int>(i),20+static_cast<int>(i)};
    return s;}
cfgdonorhunt::Targets Roster(const Source& s){cfgdonorhunt::Targets r;r.count=static_cast<int>(s.count);
    for(unsigned i=0;i<s.count;++i)r.actors[i]=s.targets[i].object;return r;}
int main(){unsigned checks=0;auto check=[&](bool b){++checks;assert(b);};
    for(unsigned cap=1;cap<=12;++cap){Host h;h.cap=cap;Frame frame;const auto source=Input(64);auto roster=Roster(source);
        unsigned callbacks=0;
        while(frame.Status()!=State::Ready&&callbacks<65){h.used=0;const auto state=frame.Step(h,source);++callbacks;
            check(h.used<=cap);if(state!=State::Ready)check(!frame.Nearest(roster,1000));}
        check(frame.Status()==State::Ready&&frame.Cursor()==64&&h.samples==64);
        check(frame.Nearest(roster,1000)==source.targets[63].object);
        check(frame.Nearest(roster,1000,source.targets[63].object)==source.targets[62].object);
        for(auto visits:h.visits)check(visits==1);
        roster.blockedUntil[63]=2000;check(frame.Nearest(roster,1000)==source.targets[62].object);
        frame.Consumed();check(frame.Status()==State::Empty&&!frame.Nearest(roster,1000));
    }
    {Host h;Frame frame;auto source=Input(10);auto roster=Roster(source);
        check(frame.Step(h,source)==State::Collecting&&frame.Cursor()==4);
        ++source.revision;std::swap(source.targets[0],source.targets[1]);h.used=0;
        check(frame.Step(h,source)==State::Collecting&&frame.Cursor()==4&&h.visits[0]==2&&h.visits[1]==2);
        while(frame.Status()!=State::Ready){h.used=0;frame.Step(h,source);}
        check(!frame.Nearest(roster,1000));roster=Roster(source);check(frame.Nearest(roster,1000)==source.targets[9].object);}
    {Host h;Frame frame;const auto source=Input(2);auto roster=Roster(source);
        check(frame.Step(h,source)==State::Ready);h.current=false;h.exhausted=true;
        check(frame.Step(h,source)==State::Collecting);h.exhausted=false;
        check(frame.Step(h,source)==State::Refused&&!frame.Nearest(roster,1000));
        h.current=true;h.used=0;check(frame.Step(h,source)==State::Ready);}
    {Host h;Frame frame;const auto source=Input(2);h.exhausted=true;
        check(frame.Step(h,source)==State::Collecting&&frame.Cursor()==0&&!h.samples);
        h.exhausted=false;h.used=0;check(frame.Step(h,source)==State::Ready&&h.samples==2);}
    {Host h;Frame frame;auto source=Input(2);source.owner.actor.pawn.serial=0;check(frame.Step(h,source)==State::Refused&&!h.samples);}
    {Host h;h.cap=1;Frame frame;const auto source=Input(3);check(frame.Step(h,source)==State::Collecting&&frame.Cursor()==1);
        frame.Invalidate();check(frame.Status()==State::Empty&&frame.Cursor()==0);h.used=0;frame.Step(h,source);check(h.visits[0]==2);}
    {Host h;h.cap=4;h.dead=3;Frame frame;const auto source=Input(4);auto roster=Roster(source);
        check(frame.Step(h,source)==State::Ready&&frame.Nearest(roster,1000)==source.targets[2].object);
        check(frame.ObservedLive(source.targets[0].object)&&frame.ObservedLive(source.targets[2].object));
        check(!frame.ObservedLive(source.targets[3].object)&&!frame.ObservedLive(0xDEAD000));
        frame.Consumed();check(!frame.ObservedLive(source.targets[0].object));}
    {Host h;Frame frame;auto source=Input(1);auto roster=Roster(source);
     check(frame.Step(h,source)==State::Ready);roster.DonorBlockJasonAITarget(source.targets[0].object,1000);
     check(!frame.Nearest(roster,1001));check(frame.Nearest(roster,1001,0,true)==source.targets[0].object);
     check(!frame.Nearest(roster,1001,source.targets[0].object,true));
     roster.blockedUntil[0]=3501;check(!frame.Nearest(roster,1001,0,true));
     h.dead=0;frame.Invalidate();h.used=0;check(frame.Step(h,source)==State::Ready&&!frame.Nearest(roster,1001,0,true));}
    std::printf("Actual native donor decision frame: %u checks PASS;64targets/caps1-12,no prefix selection,cursor retained,source/reorder invalidation,block/exclude/current refusal,re-admission.\n",checks);
}
