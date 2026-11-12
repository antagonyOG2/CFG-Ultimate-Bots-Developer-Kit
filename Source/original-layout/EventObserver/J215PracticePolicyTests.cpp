#define CFG_J215_PAMELA_MUTE 1
#define CFG_J215_GRAB_MUTE 1
#define CFG_J215_MINIMAL_HUD 1
#define CFG_J215_HARD_MOVEMENT 1
#define CFG_J215_BOT_SWEATER 1
#include "../Core/CounselorPracticePolicy.hpp"
#include <cstdio>
struct Host {
    bool current=true,readable=true,apply=false;float value=1;unsigned writes=0;
    cfgpractice::Write outcome=cfgpractice::Write::Returned;
    bool Current(){return current;}bool Read(float& out){out=value;return readable;}
    cfgpractice::Write Set(float wanted){++writes;if(outcome==cfgpractice::Write::Returned||apply)value=wanted;return outcome;}
};
int main(){unsigned checks=0,failures=0;const auto check=[&](bool okay,const char* text){++checks;if(!okay){++failures;std::printf("FAIL %s\n",text);}};
    for(auto line:{u"PamelaSweaterActivate",u"PamelaSweaterActive",u"PamelaJasonStunned"})check(!cfgpractice::MutePamela(line)&&cfgpractice::KeepPamela(line),"sweater names always audible");
    check(cfgpractice::MutePamela(u"PamelaCounselorDeath"),"known Pamela taunt muted");
    for(auto line:{u"",u"CounselorDeath",u"OtherVoice",u"pamelaUnknown"})check(!cfgpractice::MutePamela(line),"unrecognized voice passes through");
    {Host host;cfgpractice::Override<float> state;
        check(state.Apply(host,0)&&state.owned&&host.value==0&&host.writes==1,"one confirmed mute");
        check(!state.Apply(host,0)&&host.writes==1,"no repeated mute");
        check(state.Restore(host)&&host.value==1&&!state.owned&&host.writes==2,"exact original volume restored");}
    {Host host;host.value=0;cfgpractice::Override<float> state;
        check(!state.Apply(host,0)&&!state.owned&&!host.writes,"preexisting muted component not owned");
        check(!state.Restore(host)&&!host.writes,"no invented restore");}
    {Host host;cfgpractice::Override<float> state;check(state.Apply(host,0),"foreign change setup");host.value=.5f;
        check(!state.Restore(host)&&host.value==.5f&&host.writes==1&&!state.owned,"external volume preserved");}
    {Host host;cfgpractice::Override<float> state;host.outcome=cfgpractice::Write::NotEntered;
        check(!state.Apply(host,0)&&!state.attempted,"pre-entry denial can retry");host.outcome=cfgpractice::Write::Returned;
        check(state.Apply(host,0)&&host.writes==2,"one entered mutation after denial");}
    for(bool changed:{false,true}){Host host;cfgpractice::Override<float> state;host.outcome=cfgpractice::Write::Uncertain;host.apply=changed;
        check(!state.Apply(host,0)&&state.uncertain&&state.attempted,"ambiguous native call quarantined");
        host.outcome=cfgpractice::Write::Returned;check(!state.Apply(host,0)&&!state.Restore(host)&&host.writes==1,"no ambiguous replay/restore");}
    {Host host;cfgpractice::Override<float> state;check(state.Apply(host,0),"world departure setup");host.current=false;
        check(!state.Restore(host)&&host.writes==1,"no old-world mutation");}
    std::printf("J215 Pamela keep-list/owned override/no-replay policy: %u checks, %u failures; no game access.\n",checks,failures);return failures?1:0;
}
