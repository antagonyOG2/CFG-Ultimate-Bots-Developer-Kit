#include "HostedTakeoverFixture.hpp"
#include "HostedH1Sampler.hpp"
#include "HostedPlanBuilder.hpp"
using namespace cfgh1;
int main(){
    unsigned failures=0,checks=0;const auto check=[&](bool ok){++checks;if(!ok)++failures;};
    const auto sample=[&](HostedFixture& f,Report& rep,cfghosted::takeover35::Frame& frame,bool& valid){
        return Sample(f.r,f.a,99,rep,frame,valid);};
    // 1. Remote-selected Jason: full frame, every report field the plan asks for.
    {HostedFixture f;Report rep;cfghosted::takeover35::Frame frame;bool valid=false;
     const auto stage=sample(f,rep,frame,valid);
     check(stage==Stage::FramePassed&&valid&&frame.eligibility==cfghosted::takeover35::Eligibility::Ready);
     check(rep.roster==2&&rep.numBots==0&&rep.botFill==0&&rep.counselorTree==0&&rep.explicitPickPresent==1&&rep.explicitPickIsKiller==1);
     check(rep.killerKind==static_cast<std::uint32_t>(Kill::RemoteHuman)&&rep.matchInProgress==1);
     check(rep.killerPawn==HostedFixture::killer&&rep.killerController==HostedFixture::human&&rep.killerState==HostedFixture::humanState);
     check(rep.localController==Fixture::controller&&rep.localPawn==HostedFixture::localPawn&&rep.mode==Fixture::mode&&rep.netDriver==Fixture::driver);
     const auto all=ProbeWorld|ProbeNetDriver|ProbeDriverWorld|ProbeListenHost|ProbeModePresent|ProbeStatePresent|ProbeModeHunt|ProbeModeAuthority|ProbeLocalController|ProbeLocalAuthority;
     check((rep.probeBits&all)==all);
     check(rep.lastFrameReads>0&&rep.lastFrameReads<cfghosted::native::ReadLimit);
     // Samples are read-only: the fixture memory is byte-identical afterwards.
     HostedFixture before;check(before.r.bytes==f.r.bytes);}
    // Real hosted derived classes exhaust the historical 1376-read frame cap.
    {HostedFixture f;for(unsigned i=0;i<7;++i){const std::uintptr_t object=std::array<std::uintptr_t,7>{{f.a.world,Fixture::driver,Fixture::mode,Fixture::state,Fixture::instance,Fixture::player,Fixture::controller}}[i];
      const auto start=0xA00000+0x10000ULL*i;f.r.Put(object+cfgprobe::ObjectClass,start);
      for(unsigned j=0;j<7;++j)f.r.Put(start+0x100*j+cfgprobe::ClassSuper,j==6?f.types[i]:start+0x100*(j+1));}
     Report rep;cfghosted::takeover35::Frame frame;bool valid=false;
     check(sample(f,rep,frame,valid)==Stage::FramePassed&&valid&&rep.lastFrameReads>1376&&rep.lastFrameReads<8192);}
    // 2. Host selected as Jason: identity is distinct but uses the same donor transaction.
    {HostedFixture f;f.r.Put(HostedFixture::killer+cfgescape::disk::Controller,Fixture::controller);
     f.r.Put(Fixture::controller+cfgprobe::ControllerPawn,HostedFixture::killer);f.r.Put(Fixture::state+b::CurrentKillerState,HostedFixture::hostState);
     Report rep;cfghosted::takeover35::Frame frame;bool valid=false;
     check(sample(f,rep,frame,valid)==Stage::FramePassed&&rep.killerKind==static_cast<std::uint32_t>(Kill::LocalHost)&&
        rep.eligibility==static_cast<std::uint32_t>(cfghosted::takeover35::Eligibility::HostSelected));}
    // 3. No killer yet.
    {HostedFixture f;f.r.Put(Fixture::state+b::CurrentKiller,std::uintptr_t{});f.r.Put(Fixture::state+b::CurrentKillerState,std::uintptr_t{});
     Report rep;cfghosted::takeover35::Frame frame;bool valid=false;
     check(sample(f,rep,frame,valid)==Stage::FramePassed&&rep.killerKind==static_cast<std::uint32_t>(Kill::None));}
    // 4. Lobby/offline world without NetDriver.
    {HostedFixture f;f.r.Put(f.a.world+cfgprobe::WorldNetDriver,std::uintptr_t{});
     Report rep;cfghosted::takeover35::Frame frame;bool valid=false;
     check(sample(f,rep,frame,valid)==Stage::NoNetDriver&&!valid&&(rep.probeBits&ProbeWorld)&&!(rep.probeBits&ProbeNetDriver));}
    // 5. Client connection / foreign driver -> not a listen host.
    {HostedFixture f;f.r.Put(Fixture::driver+cfghosted::disk35::ServerConnection,std::uintptr_t{0x123450});
     Report rep;cfghosted::takeover35::Frame frame;bool valid=false;
     check(sample(f,rep,frame,valid)==Stage::ClientOrForeign&&!valid&&!(rep.probeBits&ProbeListenHost));
     HostedFixture g;g.r.Put(Fixture::driver+cfghosted::disk35::DriverWorld,std::uintptr_t{0x9990});
     check(sample(g,rep,frame,valid)==Stage::ClientOrForeign);}
    // 6. Match not InProgress (lobby/pre-start): context refused, with diagnostic probes still set.
    {HostedFixture f;f.r.Put(0x520010,std::array<char,16>{'W','a','i','t','i','n','g'});
     Report rep;cfghosted::takeover35::Frame frame;bool valid=false;
     check(sample(f,rep,frame,valid)==Stage::ContextRefused&&!valid&&(rep.probeBits&ProbeListenHost)&&(rep.probeBits&ProbeModeHunt)&&!rep.matchInProgress);}
    // 7. Mode that is not Hunt -> refused, ProbeModeHunt clear (names the real reason).
    {HostedFixture f;f.r.Put(Fixture::mode+cfgprobe::ObjectClass,std::uintptr_t{0x4F0000});f.r.Put(0x4F0000+cfgprobe::ClassSuper,std::uintptr_t{});
     Report rep;cfghosted::takeover35::Frame frame;bool valid=false;
     check(sample(f,rep,frame,valid)==Stage::ContextRefused&&!(rep.probeBits&ProbeModeHunt)&&(rep.probeBits&ProbeModeAuthority));}
    // 8. Unreadable world.
    {HostedFixture f;f.r.bytes.erase(f.a.world+cfgprobe::WorldNetDriver);
     Report rep;cfghosted::takeover35::Frame frame;bool valid=false;check(sample(f,rep,frame,valid)==Stage::WorldUnreadable);}
    // 9. Local counselor not ready / ownership unproven are CAPTURED (not READY) states: frame passes, killer unproven.
    {HostedFixture f;f.r.Put(Fixture::controller+cfgprobe::ControllerPawn,std::uintptr_t{});
     Report rep;cfghosted::takeover35::Frame frame;bool valid=false;
     check(sample(f,rep,frame,valid)==Stage::FramePassed&&rep.killerKind==static_cast<std::uint32_t>(Kill::Unproven)&&
        rep.eligibility!=static_cast<std::uint32_t>(cfghosted::takeover35::Eligibility::Ready));}
    // 10. Every single read failure while sampling never yields a READY frame from stale data.
    {HostedFixture probe;Report r0;cfghosted::takeover35::Frame f0;bool v0=false;(void)sample(probe,r0,f0,v0);const auto total=probe.r.reads;
     for(unsigned at=1;at<=total;++at){HostedFixture f;f.r.failAt=at;Report rep;cfghosted::takeover35::Frame frame;bool valid=false;
        const auto stage=sample(f,rep,frame,valid);
        check(!valid||stage==Stage::FramePassed);                    // a passed frame is a complete re-verified capture
        if(stage==Stage::FramePassed&&valid)check(rep.roster>=1);}}
    // 11. Stability gate: identical READY frames over time and count; any change resets.
    {Report rep;rep.stage=static_cast<std::uint32_t>(Stage::FramePassed);
     rep.eligibility=static_cast<std::uint32_t>(cfghosted::takeover35::Eligibility::Ready);
     rep.world=1;rep.mode=2;rep.killerPawn=3;rep.killerController=4;rep.killerState=5;rep.localController=6;rep.localPawn=7;rep.roster=2;rep.numBots=0;
     Stability s;check(!s.Observe(rep,1000,5000,3));check(!s.Observe(rep,3000,5000,3));check(!s.Observe(rep,4999,5000,3));
     check(!s.Observe(rep,5999,5000,3));check(s.Observe(rep,6000,5000,3));
     auto changed=rep;changed.killerPawn=99;check(!s.Observe(changed,7000,5000,3)&&s.frames==1);
     auto notReady=rep;notReady.eligibility=static_cast<std::uint32_t>(cfghosted::takeover35::Eligibility::RemoteOwnership);
     check(!s.Observe(notReady,8000,5000,3)&&s.frames==0);
     check(!s.Observe(rep,9000,5000,3)&&!s.Observe(rep,13000,5000,3)&&s.Observe(rep,14100,5000,3));
     auto bots=rep;bots.numBots=1;check(!s.Observe(bots,20000,5000,3));}
    // 13. Hosted plan + binding are built once from a READY frame and pass the driver-side proof; every other state refuses.
    {HostedFixture f;Report rep;cfghosted::takeover35::Frame frame;bool valid=false;(void)sample(f,rep,frame,valid);
     const std::uintptr_t table=0x800000;f.r.Put(HostedFixture::localPawn,table);
     f.r.Put(table+cfgprobe::dispatch::EventSlotBytes,f.a.module+cfgprobe::dispatch::NativeActorDispatcherRva);
     cfgobserver::Plan plan;cfgprimary::HostedBinding binding;
     check(valid&&cfgprimary::BuildHostedPlan(f.r,frame,plan,binding)&&binding.Matches(plan)&&binding.KindOf(plan)==cfgprimary::PlanKind::Hosted);
     check(plan.world==f.a.world&&plan.mode==Fixture::mode&&plan.controller==Fixture::controller&&plan.pawn==HostedFixture::localPawn&&
           plan.objects[3].serial>0&&binding.netDriver==Fixture::driver);
     cfgobserver::Plan again;check(cfgprimary::RetainedWorld(f.r,plan,again,binding)&&!cfgprimary::RetainedOfflineWorld(f.r,plan,again));
     auto notReady=frame;notReady.eligibility=cfghosted::takeover35::Eligibility::RemoteOwnership;
     check(!cfgprimary::BuildHostedPlan(f.r,notReady,plan,binding)&&!binding.active);
     auto hostPick=frame;hostPick.eligibility=cfghosted::takeover35::Eligibility::HostSelected;check(!cfgprimary::BuildHostedPlan(f.r,hostPick,plan,binding));
     HostedFixture noDispatch;(void)sample(noDispatch,rep,frame,valid);check(!cfgprimary::BuildHostedPlan(noDispatch.r,frame,plan,binding));}
    {HostedFixture f;f.r.Put(HostedFixture::killer+cfgescape::disk::Controller,Fixture::controller);
     f.r.Put(Fixture::controller+cfgprobe::ControllerPawn,HostedFixture::killer);f.r.Put(Fixture::state+b::CurrentKillerState,HostedFixture::hostState);
     Report rep;cfghosted::takeover35::Frame frame;bool valid=false;check(sample(f,rep,frame,valid)==Stage::FramePassed&&valid);
     const std::uintptr_t table=0x800000;f.r.Put(HostedFixture::killer,table);f.r.Put(table+cfgprobe::dispatch::EventSlotBytes,f.a.module+cfgprobe::dispatch::NativeActorDispatcherRva);
     cfgobserver::Plan plan;cfgprimary::HostedBinding binding;check(cfgprimary::BuildHostedPlan(f.r,frame,plan,binding)&&plan.pawn==HostedFixture::killer);
     rep.stage=static_cast<unsigned>(Stage::FramePassed);Stability stable;check(!stable.Observe(rep,100,3000,3)&&!stable.Observe(rep,2000,3000,3)&&stable.Observe(rep,3200,3000,3));}
    // 12. Report protocol.
    {Report a,b;check(Accept(a,b));b.generation=2;check(!Accept(a,b));a.generation=3;b.generation=3;check(!Accept(a,b));
     a.generation=0;b=a;a.magic=0;check(!Accept(a,b));}
    std::printf("H1 hosted sampler/report: %u checks, %u failures. Read-only; no native calls.\n",checks,failures);
    return failures?1:0;
}
