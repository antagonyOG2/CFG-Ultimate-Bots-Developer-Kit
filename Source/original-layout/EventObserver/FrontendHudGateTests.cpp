#define main ExistingActiveMenuTests
#include "ActiveMenuObservationTests.cpp"
#undef main
#include "FrontendHudGate.hpp"
struct HudHost {
    Fixture fixture;unsigned matches=0,services=0;bool finished=false,refused=false;
    bool FrontendMatches(const cfgmenu::frontend::HudDescriptor& d){++matches;return cfgmenu::active::Current(fixture.reader,d.context,42);}
    bool Service(std::uint64_t){++services;return true;}
    bool SetupFinished()const{return finished;}bool SetupRefused()const{return refused;}
};
int main(){unsigned failures=0;const auto check=[&](bool value,const char* label){if(!value){++failures;std::printf("FAIL %s\n",label);}};
    using namespace cfgmenu::frontend;HudHost host;HudDescriptor descriptor;
    {HudHost scheduled;HudDescriptor d;
     cfgmenu::active::Observe(scheduled.fixture.reader,Fixture::Module,d.context);d.event=0x500000;
     HudGate g;check(g.Bind(d,true),"paced recurrence fixture bind");
     std::uint64_t next=0;
     // Additional native events precede ReceiveDrawHUD every frame. They used
     // to take the outer slot despite doing nothing in the Observing gate.
     for(unsigned frame=0;frame<8;++frame){const auto now=100ULL+32ULL*frame;
      if(now>=next&&MayClaimSetupPacing(g.Report().state,true)){
       next=now+25;g.CompletedHudOpportunity(scheduled,Fixture::HUD,42,false,true,now);}
      if(now>=next&&MayClaimSetupPacing(g.Report().state,false)){
       next=now+25;g.After(scheduled,Fixture::HUD,d.event,42,false,true,now);}}
     check(g.Report().samples==3&&g.Report().state==HudState::Armed&&scheduled.services==5,
       "auxiliary events cannot starve real recurrence, then accelerate armed work");
     check(!MayClaimSetupPacing(HudState::Observing,true)&&MayClaimSetupPacing(HudState::Observing,false),
       "only genuine bound HUD callback owns unarmed recurrence pacing");}
    check(cfgmenu::active::Observe(host.fixture.reader,Fixture::Module,descriptor.context),"owned HUD descriptor");descriptor.event=0x500000;
    HudGate gate;check(!gate.Bind(descriptor,false)&&gate.Bind(descriptor,true),"unverified descriptor refuses");
    const auto reads=host.fixture.reader.reads;
    for(unsigned i=0;i<1000;++i){
        check(!gate.After(host,Fixture::Widget,descriptor.event,42,false,true,i*25),"unrelated object ignored");
        check(!gate.After(host,Fixture::HUD,descriptor.event+16,42,false,true,i*25),"unrelated function ignored");
        check(!gate.After(host,Fixture::HUD,descriptor.event,41,false,true,i*25),"foreign thread ignored");
        check(!gate.After(host,Fixture::HUD,descriptor.event,42,true,true,i*25),"nested callback ignored");
    }
    check(host.fixture.reader.reads==reads&&!host.matches&&!host.services,"unrelated events perform zero reads/calls");
    check(!gate.Opportunity(host,Fixture::Widget,42,false,true,90)&&!host.services,
        "widget returns cannot replace the real three-sample HUD recurrence");
    check(!gate.CompletedHudOpportunity(host,Fixture::HUD,42,false,true,90)&&!host.services&&gate.Report().samples==0,
        "additional HUD callbacks cannot invent recurrence");
    for(unsigned i=0;i<3;++i)check(gate.After(host,Fixture::HUD,descriptor.event,42,false,true,100+i*25),"observation-only recurrence sample");
    check(gate.Report().state==HudState::Armed&&!host.services,"three separate samples precede native menu work");
    check(!gate.After(host,Fixture::HUD,descriptor.event,42,false,true,174)&&!gate.After(host,Fixture::HUD,descriptor.event,42,false,true,99),"no bunched or backward-clock callbacks");
    check(gate.After(host,Fixture::HUD,descriptor.event,42,false,true,175)&&host.services==1,"one owner-thread phase after normal original return");
    check(!gate.Opportunity(host,Fixture::Widget+8,42,false,true,200)&&
        !gate.Opportunity(host,Fixture::Widget,41,false,true,200)&&
        !gate.Opportunity(host,Fixture::Widget,42,true,true,200)&&
        !gate.Opportunity(host,Fixture::Widget,42,false,false,200)&&host.services==1,"foreign/nested/incomplete widget returns never service");
    check(gate.Opportunity(host,Fixture::Widget,42,false,true,200)&&host.services==2,
        "armed view uses real owned-widget return without waiting250ms HUD timer");
    host.finished=true;check(gate.After(host,Fixture::HUD,descriptor.event,42,false,true,225)&&gate.Report().state==HudState::Finished,"stop after setup Ready");
    const auto readyReads=host.fixture.reader.reads;for(unsigned i=0;i<1000;++i)gate.After(host,Fixture::HUD,descriptor.event,42,false,true,225+i*25);
    check(host.fixture.reader.reads==readyReads&&host.services==3,"no ongoing read/native work after Ready");
    gate.Retire();check(gate.Bind(descriptor,true),"explicit new binding only");
    host.fixture.reader.Set(Fixture::Registry+24*3+16,std::int32_t{999});
    check(!gate.After(host,Fixture::HUD,descriptor.event,42,false,true,250)&&gate.Report().state==HudState::Refused&&host.services==3,"replaced HUD at same address stops before service");
    HudHost other;HudDescriptor fresh;cfgmenu::active::Observe(other.fixture.reader,Fixture::Module,fresh.context);fresh.event=descriptor.event;
    check(gate.Bind(fresh,true),"refused descriptor may be explicitly rebound");
    check(!gate.After(other,Fixture::HUD,fresh.event,42,false,false,0)&&!other.matches&&!other.services&&gate.Report().state==HudState::Refused,"original exception permits no post-return work");
    check(gate.Bind(fresh,true)&&gate.After(other,Fixture::HUD,fresh.event,42,false,true,0)&&
        !gate.After(other,Fixture::HUD,fresh.event,42,false,true,30001)&&gate.Report().state==HudState::Refused,"recurrence timeout cannot silently arm");
    {HudHost accelerated;HudDescriptor d;cfgmenu::active::Observe(accelerated.fixture.reader,Fixture::Module,d.context);d.event=descriptor.event;
     HudGate g;check(g.Bind(d,true),"accelerated HUD bind");
     for(unsigned i=0;i<3;++i)g.After(accelerated,Fixture::HUD,d.event,42,false,true,100+25*i);
     const auto observations=g.Report().samples;
     check(!g.CompletedHudOpportunity(accelerated,Fixture::Widget,42,false,true,175)&&
        !g.CompletedHudOpportunity(accelerated,Fixture::HUD,41,false,true,175)&&
        !g.CompletedHudOpportunity(accelerated,Fixture::HUD,42,true,true,175)&&
        !g.CompletedHudOpportunity(accelerated,Fixture::HUD,42,false,false,175),"foreign/nested/incomplete HUD opportunity refuses");
     check(g.CompletedHudOpportunity(accelerated,Fixture::HUD,42,false,true,175)&&accelerated.services==1&&
        g.Report().samples==observations,"real current HUD callback advances setup, not recurrence count");
     accelerated.finished=true;check(g.CompletedHudOpportunity(accelerated,Fixture::HUD,42,false,true,200),"accelerated setup finishes");
     const auto readsAfter=accelerated.fixture.reader.reads;
     check(!g.CompletedHudOpportunity(accelerated,Fixture::HUD,42,false,true,225)&&accelerated.fixture.reader.reads==readsAfter,
        "finished HUD opportunity does no native work or metadata read");}
    std::printf("CFG frontend HUD recurrence/thread/lifetime/pacing gate: %s (%u failures). Owned fixtures only.\n",failures?"FAIL":"PASS",failures);return failures?1:0;
}
