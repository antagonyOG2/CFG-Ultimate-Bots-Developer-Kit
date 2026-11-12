// Exercise the real menu -> immutable submission -> world handoff, not a
// parallel option model. The fixture never opens or calls into CFG.
#define main ExistingMenuHandoffTests
#include "SetupMenuGameplayHandoffTests.cpp"
#undef main
int main(){
    unsigned failures=static_cast<unsigned>(ExistingMenuHandoffTests()!=0),checks=0;
    const auto check=[&](bool okay){++checks;if(!okay)++failures;};
    const std::array<RowId,3> ids{{RowId::PamelaMute,RowId::GrabMute,RowId::MinimalHud}};
    check(cfgpractice::SupportedMask()==7);
    for(unsigned mask=0;mask<8;++mask){
        Fixture f;f.session.Reset();Capabilities caps;caps.verified.fill(true);caps.automaticJason=true;
        check(f.session.Open(101,Mode::Counselor,caps,10));const auto gen=f.session.Generation();
        check(f.session.Pick(101,gen,11,22,33));check(f.session.SetCount(gen,5));
        for(unsigned i=0;i<ids.size();++i){
            const auto options=BaseOptions(ids[i],Mode::Counselor);check(options.count==2&&!options.initial);
            check(f.session.SetToggle(gen,ids[i],(mask&(1u<<i))!=0));
        }
        check(f.session.Commit(101,gen));f.menu.generation=f.world.generation=gen;
        Selection selection;check(f.session.InspectCommitted(101,gen,selection,f.world.submission));
        cfgbundle::Configuration out;check(f.handoff.Arm(f.menu));
        check(f.handoff.Consume(f.world,out)&&out.counselorPractice==mask&&cfgbundle::ValidConfiguration(out,9));
        auto invalid=out;invalid.counselorPractice=8;check(!cfgbundle::ValidConfiguration(invalid,9));
        invalid=out;invalid.version=6;check(!cfgbundle::ValidConfiguration(invalid,9));
        check(f.handoff.RearmForRestart(f.world.identity.world,f.world.identity.worldSerial,0x60000,3));
        f.world.identity={0x60000,0x70000,3,4};
        check(f.handoff.Consume(f.world,out)&&out.counselorPractice==mask);
        check(!f.handoff.Consume(f.world,out)&&!out.submission);
    }
    for(auto mode:{Mode::Jason,Mode::Challenges}){
        Session session;Capabilities caps;caps.verified.fill(true);caps.automaticJason=true;
        check(session.Open(101,mode,caps,10));const auto gen=session.Generation();check(session.Pick(101,gen,11,22,0));
        for(auto id:ids)check(!session.SetToggle(gen,id,true));
    }
    Fixture fresh;cfgbundle::Configuration out;check(fresh.handoff.Arm(fresh.menu)&&fresh.handoff.Consume(fresh.world,out)&&!out.counselorPractice);
    std::printf("J216 owned Counselor choices/default-OFF/isolation/restart: %s (%u checks, %u failures). No game access.\n",failures?"FAIL":"PASS",checks,failures);
    return failures?1:0;
}
