#include "../Core/JasonHuntPolicy.hpp"
#include "../Core/GameplayBundlePolicy.hpp"
#include <cstdio>
using namespace cfghunt;
int main(){
    unsigned failures=0;
    const auto check=[&](bool okay,const char* label){if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    const std::uintptr_t human=0x1000,bot=0x2000;
    // Counselor running +Y at 500 cm/s, sampled every 100 ms.
    TargetTracker tracker;std::uint64_t now=10000;
    for(unsigned i=0;i<6;++i){tracker.Observe(0,human,{2000.f,500.f*i/10.f,0.f},now);now+=100;}
    now-=100;
    const auto* t=tracker.Current(0,human,now);
    check(t&&t->moving&&std::fabs(TargetTracker::Speed(*t)-500.f)<5.f,"velocity measured from existing samples");
    check(!tracker.Current(0,bot,now)&&!tracker.Current(1,human,now),"track bound to its own slot and actor");
    check(!tracker.Current(0,human,now+601),"stale track is not used");
    // Jason at origin, target 20 m away running perpendicular: lead ahead of it.
    V3 goal{};
    check(t&&Intercept::Plan({0,0,0},*t,false,goal)&&goal.y>t->position.y+150.f&&std::fabs(goal.x-2000.f)<1.f,
        "mid-range perpendicular runner is cut off ahead");
    const float normalLead=goal.y-t->position.y;
    V3 stalledGoal{};
    check(t&&Intercept::Plan({0,0,0},*t,true,stalledGoal)&&stalledGoal.y-t->position.y>normalLead+300.f,
        "stalled loop chase leads further to cut across");
    // Close range keeps native actor follow (melee/grab own it); far keeps it too.
    check(t&&!Intercept::Plan({1700.f,t->position.y,0},*t,false,goal),"close range uses MoveToActor");
    check(t&&!Intercept::Plan({-3000.f,t->position.y,0},*t,false,goal),"beyond 40 m uses MoveToActor");
    // Target running straight away: leading only overshoots, keep actor follow.
    TargetTracker away;now=20000;
    for(unsigned i=0;i<6;++i){away.Observe(0,human,{2000.f+60.f*i,0.f,0.f},now);now+=100;}
    now-=100;const auto* a=away.Current(0,human,now);
    check(a&&!Intercept::Plan({0,0,0},*a,false,goal),"runner moving straight away is followed, not led");
    // Standing / slow target: no lead.
    TargetTracker idle;now=30000;
    for(unsigned i=0;i<6;++i){idle.Observe(0,human,{2000.f,5.f*i,0.f},now);now+=100;}
    now-=100;const auto* s=idle.Current(0,human,now);
    check(s&&!s->moving&&!Intercept::Plan({0,0,0},*s,false,goal),"standing target is followed directly");
    // Vehicle speed / teleport resets instead of extrapolating.
    TargetTracker car;now=40000;
    for(unsigned i=0;i<6;++i){car.Observe(0,human,{2000.f,2000.f*i/10.f,0.f},now);now+=100;}
    now-=100;check(!car.Current(0,human,now),"vehicle-speed samples never produce a foot intercept");
    // Lead offset is capped.
    TargetTracker fast;now=50000;
    for(unsigned i=0;i<6;++i){fast.Observe(0,human,{3500.f,1000.f*i/10.f,0.f},now);now+=100;}
    now-=100;const auto* f=fast.Current(0,human,now);
    check(f&&Intercept::Plan({0,0,0},*f,true,goal)&&goal.y-f->position.y<=Intercept::MaximumOffset+1.f,"lead offset capped at 14 m");
    // Progress: closing distance never stalls; holding distance for 5 s does.
    ChaseProgress progress;now=60000;
    for(unsigned i=0;i<60;++i){progress.Observe(human,(1500.f-10.f*i)*(1500.f-10.f*i),now);now+=100;}
    check(!progress.Stalled(human,now),"closing chase is not stalled");
    for(unsigned i=0;i<60;++i){progress.Observe(human,1000.f*1000.f,now);now+=100;}
    check(progress.Stalled(human,now)&&!progress.Stalled(bot,now),"kited chase is stalled for that target only");
    progress.Observe(bot,800.f*800.f,now);check(!progress.Stalled(bot,now),"target switch restarts the window");
    // Smarter target choice: isolation bonus, lock and chase fatigue.
    {
        using cfgbundle::TargetChoice;
        const float pa[2]{1500.f,0.f},pc[2]{1500.f,600.f},pb[2]{-1700.f,0.f};
        TargetChoice choice;choice.Begin();
        choice.Consider(0,1500.f*1500.f,true,pa);choice.Consider(1,1700.f*1700.f,false,pb);choice.Consider(2,1620.f*1620.f,false,pc);
        check(choice.Index(false)==1,"isolated counselor at 17 m beats a grouped one at 15 m");
        check(choice.Index(true)==0,"six-second lock still holds the current grouped target");
        check(choice.Index(true,0)==1,"a fatigued (kiting) target loses the lock and its priority");
        TargetChoice unknown;unknown.Begin();
        unknown.Consider(0,1500.f*1500.f,false,pa);unknown.Consider(1,1700.f*1700.f,false,pb);unknown.Consider(2,1620.f*1620.f);
        check(unknown.Index(false)==0,"no isolation bonus when a neighbour's position is unknown");
        TargetChoice legacy;legacy.Begin();legacy.Consider(2,100,true);legacy.Consider(0,46);
        check(legacy.Index(true)==2&&legacy.Index(false)==0,"legacy squared .45 lock semantics unchanged without positions");
        ChaseProgress kite;ChaseFatigue fatigue;std::uint64_t ft=100000;
        for(unsigned i=0;i<130;++i){kite.Observe(human,1000.f*1000.f,ft);fatigue.Observe(3,human,kite,ft);ft+=100;}
        check(fatigue.Slot(ft)==3,"12 s kite marks that counselor fatigued");
        check(fatigue.Slot(ft+ChaseFatigue::Hold)==~0u,"fatigue expires after 15 s");
        ChaseProgress closing;ChaseFatigue none;ft=200000;
        for(unsigned i=0;i<130;++i){closing.Observe(human,(2500.f-15.f*i)*(2500.f-15.f*i),ft);none.Observe(3,human,closing,ft);ft+=100;}
        check(none.Slot(ft)==~0u,"a closing chase never becomes fatigued");
    }
    // Non-finite input is ignored.
    tracker.Observe(2,human,{NAN,0,0},now);check(!tracker.Current(2,human,now),"non-finite sample ignored");
    std::printf("JasonHuntPolicyTests: %u failures; pure intercept/loop policy, no game access\n",failures);
    return failures?1:0;
}
