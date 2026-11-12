#include "../Core/JasonEscapeGuardPolicy.hpp"
#include <cstdio>
using namespace cfgescapeguard;
int main(){
    unsigned failures=0;
    const auto check=[&](bool okay,const char* label){if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    std::array<Exit,MaximumExits> exits{};std::array<Counselor,MaximumCounselors> people{};
    exits[0]={true,{0,0,0}};exits[1]={false,{20000,0,0}};
    people[0]={true,{2000,0,0}};people[1]={true,{9000,0,0}};
    Guard guard;std::uint64_t now=1000;
    // Jason far away (120 m), runner 20 m from a live exit, Morph ready -> Morph.
    auto d=guard.Decide({-12000,0,0},exits,people,true,now);
    check(d.action==Action::Morph&&d.exit==0&&d.counselor==0,"far Jason Morphs to the live exit ahead of the runner");
    check(std::fabs(d.goal.x-300.f)<1.f&&std::fabs(d.goal.y)<1.f,"lands 3 m in front of the exit on the runner's side");
    check(guard.Guarding(now),"guard engaged");
    // Same situation, Morph on cooldown -> hunt toward the guard point.
    d=guard.Decide({-12000,0,0},exits,people,false,now);
    check(d.action==Action::Hunt,"no Morph available: run to the guard point");
    // Jason already near the exit: no Morph.
    d=guard.Decide({-1000,0,0},exits,people,true,now);
    check(d.action==Action::Hunt,"close Jason hunts instead of teleporting");
    // Jason 45 m out while the runner is 20 m out: inside the 50 m floor, run.
    d=guard.Decide({4500,0,0},exits,people,true,now);
    check(d.action==Action::Hunt,"Jason within 50 m of the exit runs instead of teleporting");
    // Inactive exits never count, even with someone standing at them.
    people[0]={true,{19500,0,0}};people[1]={false,{}};
    d=guard.Decide({0,0,0},exits,people,true,now+20000);
    check(d.action==Action::None&&!guard.Guarding(now+20000),"inactive exit (no police) is ignored");
    // Threat radius: 35 m.
    people[0]={true,{3400,0,0}};check(guard.Decide({-9000,0,0},exits,people,false,now+30000).action==Action::Hunt,"within 35 m is a threat");
    guard.Clear();people[0]={true,{3600,0,0}};check(guard.Decide({-9000,0,0},exits,people,false,now+40000).action==Action::None,"beyond 35 m is not");
    // Closest-to-exit counselor wins; hysteresis keeps the current pair.
    people[0]={true,{2000,0,0}};people[1]={true,{1800,500,0}};
    d=guard.Decide({-9000,0,0},exits,people,false,now+50000);check(d.counselor==1,"runner nearest the exit is prioritized");
    people[0]={true,{1700,0,0}};
    d=guard.Decide({-9000,0,0},exits,people,false,now+50100);check(d.counselor==1,"hysteresis keeps the guarded runner on a small change");
    people[0]={true,{500,0,0}};
    d=guard.Decide({-9000,0,0},exits,people,false,now+50200);check(d.counselor==0,"a runner much closer to the exit takes over");
    check(guard.Decide({NAN,0,0},exits,people,true,now).action==Action::None,"non-finite Jason position refused");
    std::printf("JasonEscapeGuardPolicyTests: %u failures; pure exit-guard decisions, no game access\n",failures);
    return failures?1:0;
}
