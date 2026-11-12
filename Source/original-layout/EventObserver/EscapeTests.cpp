#include "CounselorTerminalObservation.hpp"
#include "../Core/GrabCombatPolicy.hpp"
#include "../Core/CounselorConvergencePolicy.hpp"
#include "../Core/SweaterLifecyclePolicy.hpp"
#define main CfgFixtureResourceTestMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
int main(){
    using namespace cfgescape;
    CHECK(OfflineCompletionPhase(u"InProgress"));
    CHECK(OfflineCompletionPhase(u"WaitingPostMatchOutro"));
    CHECK(!OfflineCompletionPhase(u"PostMatchOutro"));
    CHECK(!OfflineCompletionPhase(u"WaitingPostMatch"));
    CHECK(!OfflineCompletionPhase(u"EnteringMap"));
    CHECK(!OfflineCompletionPhase(u""));
    {PostMatchWatch watch;CHECK(!watch.Due(100));watch.Begin(100);
     CHECK(watch.Due(100));CHECK(!watch.Due(1099));CHECK(watch.Due(1100));
     watch.Observe(u"PostMatchOutro");CHECK(watch.active);CHECK(watch.Due(45101));CHECK(!watch.Due(75101)&&!watch.active);
     watch.Begin(50000);watch.Observe(u"LeavingMap");CHECK(!watch.Due(51000));
     watch.Begin(60000);watch.Lost();CHECK(!watch.Due(61000));
     watch.Begin(UINT64_MAX);CHECK(!watch.active);}
    Fixture f;f.Put(f.bot+disk::bHasEscaped,std::uint8_t{0});
    // Regression: a pointer-width equality helper read adjacent flag bytes.
    f.Put(f.bot+0x110,std::uint8_t{3});f.Put(f.bot+0x111,std::uint8_t{0xA5});
    CHECK(Same(f,f.bot+0x110,std::uint8_t{3}));
    CHECK(!cfgobserver::Same(f,f.bot+0x110,std::uintptr_t{3}));
    f.Put(f.bot+cfghunter::disk::Health,100.f);f.Put(f.bot+disk::PlayerState,std::uintptr_t{0});
    f.Put(f.bot+disk::Controller,f.pawn);f.Put(f.pawn+disk::Pawn,f.bot);
    auto observe=[&](Fixture& value,State expected){value.reads=0;State state;
        return Counselor(value,value.objects,value.module,value.bot,value.counselorBase,state)&&state==expected;};
    CHECK(observe(f,State::Living));
    auto escaped=f;escaped.Put(f.bot+disk::bHasEscaped,std::uint8_t{1});CHECK(observe(escaped,State::Escaped));
    auto dead=f;dead.Put(f.bot+cfghunter::disk::Health,0.f);CHECK(observe(dead,State::Dead));
    auto unpossessed=f;unpossessed.Put(f.bot+disk::Controller,std::uintptr_t{0});CHECK(observe(unpossessed,State::Unpossessed));
    // Vehicle possession may leave a healthy driver's controller pointing at
    // the car. Such a source-bound driver remains eligible for pursuit, while
    // walking target selection and terminal completion retain strict rules.
    auto driving=f;driving.Put(f.pawn+disk::Pawn,f.Object());CHECK(observe(driving,State::Unpossessed));
    CHECK(Driving(State::Living)&&Driving(State::Unpossessed));
    CHECK(!Driving(State::Dead)&&!Driving(State::Escaped)&&!Driving(State::Unknown));
    auto bad=f;bad.Put(f.bot+disk::bHasEscaped,std::uint8_t{2});State state;bad.reads=0;
    CHECK(!Counselor(bad,bad.objects,bad.module,bad.bot,bad.counselorBase,state));
    const auto stateType=f.Object(),playerState=f.Object(stateType);
    f.Put(f.module+disk::SCPlayerStateReflection,stateType);f.Put(f.module+disk::SCPlayerStateConstructor,stateType);
    f.Put(f.bot+disk::PlayerState,playerState);f.Put(playerState+disk::bEscaped,std::uint8_t{1});f.Put(playerState+disk::bDead,std::uint8_t{0});
    CHECK(observe(f,State::Escaped)); // PlayerState leads pawn during vehicle escape.
    f.Put(playerState+disk::bEscaped,std::uint8_t{0});f.Put(playerState+disk::bDead,std::uint8_t{1});CHECK(observe(f,State::Dead));
    f.Put(playerState+disk::bDead,std::uint8_t{0});CHECK(observe(f,State::Living));
    auto changed=f;changed.trigger=f.bot+disk::PlayerState;changed.triggerAt=2;changed.mutation=changed.trigger;changed.newPointer=0;
    changed.reads=0;CHECK(!Counselor(changed,changed.objects,changed.module,changed.bot,changed.counselorBase,state));
    auto unreadable=f;unreadable.cap=4;unreadable.reads=0;
    CHECK(!Counselor(unreadable,unreadable.objects,unreadable.module,unreadable.bot,unreadable.counselorBase,state));
    Completion completion;completion.Observe(0,State::Escaped);completion.Observe(1,State::Dead);
    CHECK(!completion.Ready(1,true,true,100));CHECK(!completion.Ready(1,true,true,3099));CHECK(completion.Ready(1,true,true,3100));
    completion.Observe(0,State::Living);CHECK(!completion.Ready(1,true,true,5000)); // Native Tommy resets terminal evidence.
    completion.Observe(0,State::Escaped);CHECK(!completion.Ready(1,false,true,6000));CHECK(!completion.Ready(1,true,false,7000));
    CHECK(!completion.Ready(1,true,true,8000));CHECK(completion.Ready(1,true,true,11000));completion.dispatched=true;
    CHECK(!completion.Ready(1,true,true,15000)); // No repeat after successful OR ambiguous native dispatch.
    {Completion pending;CHECK(!pending.CheckDue(1,true,0));
        pending.Observe(0,State::Escaped);pending.Observe(1,State::Dead);
        CHECK(pending.CheckDue(1,true,1000));CHECK(!pending.Ready(1,true,true,1000));
        CHECK(!pending.CheckDue(1,true,1999));
        CHECK(pending.CheckDue(1,true,2000));CHECK(!pending.Ready(1,true,true,2000));
        CHECK(pending.CheckDue(1,true,4000));CHECK(pending.Ready(1,true,true,4000));
        pending.Observe(0,State::Living);CHECK(!pending.CheckDue(1,true,5000));
        pending.Observe(0,State::Escaped);CHECK(!pending.CheckDue(1,false,6000));
        pending.dispatched=true;CHECK(!pending.CheckDue(1,true,9000));}
    Completion full;
    for(unsigned i=0;i<cfglimits::MaximumCounselors;++i)full.Observe(i,State::Escaped);
    CHECK(full.terminalMask==cfglimits::TerminalMaskLimit-1);
    CHECK(!full.Ready(cfglimits::MaximumHelpers,true,true,100));
    CHECK(full.Ready(cfglimits::MaximumHelpers,true,true,3100));
    full.Observe(cfglimits::MaximumHelpers,State::Living);
    CHECK(!full.Ready(cfglimits::MaximumHelpers,true,true,4100));
    CHECK(!full.Ready(cfglimits::MaximumHelpers+1,true,true,5100));
    const auto mode=f.Object(),gameState=f.Object();const std::uintptr_t entries=0x850000;
    struct List {std::uintptr_t data;std::int32_t count,capacity;};
    f.Put(mode+disk::HunterToSpawn,std::uintptr_t{0});f.Put(gameState+disk::HunterTimeRemaining,std::int32_t{0});
    f.Put(gameState+disk::PlayerCharacterList,List{entries,1,1});f.Put(entries,f.bot);
    const auto native=[&](Fixture value){value.reads=0;return NativeRosterTerminal(value,value.objects,value.module,mode,gameState,value.counselorBase,value.pawn);};
    CHECK(!native(f));f.Put(playerState+disk::bEscaped,std::uint8_t{1});CHECK(native(f));
    bad=f;bad.Put(mode+disk::HunterToSpawn,f.bot);CHECK(!native(bad));
    {TerminalWitness proof;bad.reads=0;CHECK(!NativeRosterTerminal(bad,bad.objects,bad.module,mode,gameState,bad.counselorBase,bad.pawn,&proof));CHECK(proof.failure==1&&proof.pending==f.bot);}
    bad=f;bad.Put(gameState+disk::HunterTimeRemaining,std::int32_t{1});CHECK(!native(bad));
    {TerminalWitness proof;bad.reads=0;CHECK(!NativeRosterTerminal(bad,bad.objects,bad.module,mode,gameState,bad.counselorBase,bad.pawn,&proof));CHECK(proof.failure==2&&proof.hunter==1);}
    bad=f;bad.Put(gameState+disk::PlayerCharacterList,List{entries,17,17});CHECK(!native(bad));
    bad=f;bad.Put(entries,std::uintptr_t{0});CHECK(!native(bad));
    cfggrab::Policy grab;
    CHECK(!grab.ChooseGrab(100.f*100.f));CHECK(grab.ChooseGrab(220.f*220.f));
    CHECK(!grab.ChooseGrab(100.f*100.f));CHECK(!grab.ChooseGrab(221.f*221.f));
    CHECK(!grab.ChooseGrab(-1));CHECK(grab.decisions==4);
    grab.Observe(0x123400,100);CHECK(!grab.Due(399));CHECK(grab.Due(400));
    grab.Refused(400);CHECK(grab.slot==1&&!grab.Due(649)&&grab.Due(650));
    grab.Refused(650);grab.Refused(900);grab.Refused(1150);CHECK(grab.slot==0&&!grab.Due(3149)&&grab.Due(3150));
    grab.Commit();CHECK(!grab.Due(10000));grab.Observe(0x123400,10000);CHECK(!grab.Due(10001));
    grab.Observe(0,11000);CHECK(!grab.Due(12000));grab.Observe(0x123500,12000);CHECK(grab.Due(12300)&&!grab.committed&&grab.slot==1);
    for(unsigned i=0;i<8;++i){const auto before=grab.slot;grab.Commit();grab.Observe(0,13000+i*1000);
        grab.Observe(0x123500+i,13300+i*1000);CHECK(grab.slot==(before+1)%4);}
    cfgconvergence::FeatureTurns turns;cfgsweater::Policy oldSweater,newSweater;
    oldSweater.ConfirmPublish();newSweater.ConfirmPublish();unsigned oldProgress=0,newProgress=0,sweaterPolls=0;
    for(std::uint64_t now=1000;now<61000;now+=600){
        if(!oldSweater.Due(now))++oldProgress;
        if(turns.Sweater()&&newSweater.Due(now))++sweaterPolls;else ++newProgress;
    }
    CHECK(oldProgress==0&&newProgress==50&&sweaterPolls==50);
    std::printf("CFG escape/terminal observations and native ending gate: %s (%d failures). No game access.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
