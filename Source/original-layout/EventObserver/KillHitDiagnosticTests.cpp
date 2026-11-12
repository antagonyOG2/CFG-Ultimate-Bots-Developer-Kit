#define main ExistingKillRuntimeSuiteMain
#include "KillRuntimeTests.cpp"
#undef main
#include <vector>
struct ObservedHost:Host {
    std::vector<cfgkilldiag::Hit> observed;
    void TraceKillHit(const cfgkilldiag::Hit& hit){observed.push_back(hit);}
};
int main(){
    if(ExistingKillRuntimeSuiteMain())return 1;
    unsigned checks=0;const auto check=[&](bool value){++checks;assert(value);};
    for(unsigned fault=0;fault<12;++fault){
        Host original;ObservedHost observed;
        auto before=Armed(original),after=Armed(observed);
        switch(fault){
        case 1:original.scope=observed.scope=false;break;
        case 2:original.event=observed.event=false;break;
        case 3:original.packet.damage=observed.packet.damage=0;break;
        case 4:original.packet.instigatedBy=observed.packet.instigatedBy=0;break;
        case 5:original.packet.damageCauser=observed.packet.damageCauser=6;break;
        case 6:original.living=observed.living=false;break;
        case 7:original.held=observed.held=0;break;
        case 8:original.kind=observed.kind=Weapon::Other;break;
        case 9:original.packet.damageType=observed.packet.damageType=99;break;
        case 10:original.mask=observed.mask=false;break;
        case 11:original.squared=observed.squared=201.f*201.f;break;
        }
        const auto acceptedBefore=HitNow(before,original),acceptedAfter=HitNow(after,observed);
        check(acceptedBefore==acceptedAfter&&before.policy.stage==after.policy.stage&&before.accepted==after.accepted);
        check(observed.observed.size()==1);
        check((observed.observed[0].clause==0)==acceptedAfter);
        check(original.bridges==observed.bridges&&original.stances==observed.stances&&original.interactions==observed.interactions);
    }
    {ObservedHost h;auto r=Armed(h);h.context={10,11,12,false};
        // A native kneel/context without a proved hit must not gain new kill
        // authority merely because diagnostics are enabled.
        check(r.Tick(h,110)==Step::Idle&&r.policy.stage==Stage::AwaitHit&&!r.stanceDispatches&&!r.interactionDispatches);
        check(h.observed.empty()&&!h.stances&&!h.interactions);
        check(!r.ObserveDamage(h,2,8,9,111,false)&&h.observed.empty());
        check(!r.ObserveDamage(h,99,8,9,111,true)&&h.observed.empty());
    }
    {cfgkilldiag::Budget budget;check(budget.Take(0)&&!budget.Take(0));
        for(unsigned i=1;i<16;++i)check(budget.Take(i));
        for(unsigned i=16;i<200;++i)check(!budget.Take(i));
        check(budget.count==16&&!budget.Available());budget={};check(budget.Available()&&budget.Take(0));}
    std::printf("B1 kill diagnostics: %u checks PASS; same admission/no synthetic stance or interaction;16-mark cap.\n",checks);
    return 0;
}
