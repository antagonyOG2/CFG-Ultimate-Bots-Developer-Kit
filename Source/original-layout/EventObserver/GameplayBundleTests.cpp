#include "../Core/GameplayBundlePolicy.hpp"
#include "../Core/SweaterLifecyclePolicy.hpp"
#include "../Core/GameplayConfiguration.hpp"
#include "../Core/NativeFunctionFrame.hpp"
#include <cstdio>
int main(){
    using namespace cfgbundle;
    unsigned failures=0;
    const auto checkAt=[&](bool valid,int line){if(!valid){++failures;std::printf("FAIL GameplayBundleTests.cpp:%d\n",line);}};
#define check(...) checkAt((__VA_ARGS__),__LINE__)
    {Schedule melee;check(melee.Start(100,0));melee.helpersActive=false;
     melee.selectAt=melee.vehicleAt=melee.convergenceAt=10000;
     melee.CloseCombat(200,200.f*200.f);check(melee.Due(300)==Lane::Hunt);
     melee.Pressed(300);check(!melee.AttackDue(301,1));check(melee.Due(500)==Lane::Release);
     melee.Released();check(melee.AttackDue(500,1));check(melee.Due(600)==Lane::Hunt);
     melee.CloseCombat(600,300.f*300.f);check(!melee.meleeUntil);}
    {Report r;r.footBurstStage=3;r.footBurstMask=15;r.footBurstStarts=2;r.footBurstRestores=1;check(Valid(r));
     r.footBurstStage=4;check(!Valid(r));r.footBurstStage=3;r.footBurstMask=16;check(!Valid(r));
     r.footBurstMask=0;r.footBurstRestores=3;check(!Valid(r));}
    {Report r;r.initializationStage=static_cast<unsigned>(InitializationStep::Ready);check(Valid(r));
     ++r.initializationStage;check(!Valid(r));r={};r.version=56;check(!Valid(r));}
    {Schedule close;check(close.Start(0,0));close.helpersActive=false;
     close.CloseCombat(100,200.f*200.f);close.next=125;
     close.vehicleAt=close.selectAt=close.huntAt=125;
     check(close.Due(125)==Lane::Hunt);check(close.next==150);
     check(close.Due(149)==Lane::None);check(close.Due(150)==Lane::Vehicle);
     check(close.Due(175)==Lane::Select);check(close.Due(200)==Lane::Hunt);
     close.Pressed(200);check(close.Due(350)==Lane::Release);
     close.Released();close.CloseCombat(400,300.f*300.f);close.next=400;
     close.vehicleAt=400;check(close.Due(400)==Lane::Vehicle&&close.next==500);}
    {Schedule setup;check(setup.Start(0,0));setup.helpersActive=false;
     setup.next=1000;setup.vehicleAt=1000;setup.huntAt=1000;setup.selectAt=1000;setup.convergenceAt=1000;
     const auto unused=SelectActionTurn(setup,100,false,false,false,true);
     check(!unused.camera&&unused.gameplay==Lane::Hunt); // due trap gets unused callback
     setup.pressed=true;setup.releaseAt=100;
     check(SelectActionTurn(setup,100,false,false,false,true).gameplay==Lane::None);
     setup.next=100;check(SelectActionTurn(setup,100,false,false,false,true).gameplay==Lane::Release);
     setup.pressed=false;setup.next=200;setup.vehicleAt=200;
     check(SelectActionTurn(setup,200,false,false,false,true).gameplay==Lane::Vehicle);
     setup.next=1000;check(SelectActionTurn(setup,201,false,false,true,true).gameplay==Lane::Hunt);
    }
    {
        std::array<Bot,MaximumBots> roster{};roster[0].pawn=0x30000;
        unsigned observations=0;std::uintptr_t human=0x20000;bool living=true;
        const auto observe=[&](unsigned index,std::uintptr_t& pawn){
            ++observations;pawn=index?roster[index-1].pawn:human;return living;};
        check(TrackedDriver(human,1,roster,observe)&&observations==1);
        human=0x40000;observations=0; // Native Tommy replaces the starting counselor.
        check(TrackedDriver(human,1,roster,observe)&&observations==1);
        observations=0;check(!TrackedDriver(0x20000,1,roster,observe)&&observations==1);
        observations=0;check(TrackedDriver(roster[0].pawn,1,roster,observe)&&observations==2);
        living=false;check(!TrackedDriver(roster[0].pawn,1,roster,observe));
        observations=0;check(!TrackedDriver(0,1,roster,observe)&&!observations);
        check(!TrackedDriver(human,MaximumBots+1,roster,observe)&&!observations);
    }
    check(OwnedCallback(0x10000,0x10000,0,false));
    check(!OwnedCallback(0,0x10000,0,false));check(!OwnedCallback(0x20000,0x10000,0x20000,false));
    check(OwnedCallback(0x20000,0x10000,0x20000,true));check(!OwnedCallback(0x30000,0x10000,0x20000,true));
    check(!OwnedCallback(0x30000,0x10000,0x20000,true,0x30000,false));
    check(!OwnedCallback(0x30000,0x10000,0x20000,false,0x30000,true));
    check(OwnedCallback(0x30000,0x10000,0x20000,true,0x30000,true));
    check(!OwnedCallback(0x40000,0x10000,0x20000,true,0x30000,true));
    check(NativeFrame(61,61,64)&&!NativeFrame(64,61,64)&&!NativeFrame(60,61,64));
    check(NativeFrame(13,13,16)&&!NativeFrame(16,13,16));
    check(NativeFrame(9,9,16)&&!NativeFrame(16,9,16)&&!NativeFrame(9,9,8));
    check(NativeFrame(21,21,24)&&!NativeFrame(24,21,24)&&!NativeFrame(20,21,24));
    check(ClassifyMove(0)==MoveOutcome::Refused&&ClassifyMove(1)==MoveOutcome::AlreadyAtGoal&&
        ClassifyMove(2)==MoveOutcome::Accepted&&ClassifyMove(3)==MoveOutcome::Invalid&&ClassifyMove(255)==MoveOutcome::Invalid);
    MoveCooldown movement;check(movement.Due(0));movement.Completed(1000,0);
    check(!movement.Due(2799)&&movement.Due(2800)&&movement.failures==1);
    movement.Completed(2800,0);check(!movement.Due(6399)&&movement.Due(6400));
    movement.Completed(6400,0);movement.Completed(11400,0);check(movement.failures==3&&movement.next==16400);
    movement.Completed(16400,2);check(!movement.failures&&movement.next==17600);
    movement.Completed(17000,255);check(!movement.failures&&movement.next==17600);
    movement.Clear();check(!movement.failures&&movement.Due(0));
    ActorFollowPath follow;check(!follow.Preserve(0,0x10000));
    follow.Completed(1000,0x10000,2);
    check(follow.Preserve(7106,0x10000)&&!follow.Preserve(7107,0x10000));
    check(!follow.Preserve(1000,0x20000)&&!follow.Preserve(1000,0));
    for(unsigned result : {0u,1u,3u,255u}){
        follow.Completed(1000,0x10000,2);follow.Completed(1500,0x10000,result);
        check(!follow.Preserve(1500,0x10000));
    }
    follow.Completed(1000,0,2);check(!follow.target);
    follow.Completed(1000,0x10000,2);follow.Clear();check(!follow.target&&!follow.until);
    {
        HelperStartup helper;check(helper.Active());
        const auto point=helper.Candidate({100,200,300},0);
        check(point.x==800&&point.y==200&&point.z==235);
        const cfgspawn::Vec3 floor{point.x,point.y,210};
        check(helper.Projected(point,floor)&&helper.stage==HelperStage::Spawn);
        check(helper.surface.z==210&&helper.location.z==300);
        helper.Next(HelperFailure::NullResult);check(helper.Active()&&helper.candidate==1);
        check(helper.Candidate({100,200,300},0).x==-600);
        check(!helper.Projected(point,{point.x+301,point.y,point.z}));
        check(!helper.Projected(point,{point.x,point.y,point.z+501}));
        check(!helper.Projected(point,{point.x,point.y,331})); // roof/floor delta121
        check(!helper.Projected(point,{point.x,point.y,89})); // lower layer delta121
        check(!helper.Projected(point,{NAN,0,0}));
        for(unsigned i=1;i<HelperCandidates;++i)helper.Next(HelperFailure::NullResult);
        check(!helper.Active()&&helper.stage==HelperStage::Unavailable);
        HelperStartup uncertain;uncertain.Refuse(HelperFailure::Result,true);
        check(!uncertain.Active()&&uncertain.stage==HelperStage::Indeterminate);
        check(!uncertain.Projected(point,point));
        uncertain.Next(HelperFailure::NullResult);uncertain.Confirm(1,4);check(!uncertain.Active());
        HelperStartup success;check(success.Projected(point,point));success.Confirm(1,4);check(success.Active()&&!success.candidate);
        check(success.Projected(point,point));
        success.Confirm(4,4);check(!success.Active()&&success.stage==HelperStage::Complete);
        HelperStartup boundary;check(boundary.Projected(point,{point.x+220,point.y,330}));
        check(boundary.location.z==420); // exact inclusive geometry bounds
        // A helper-only refusal keeps hunt/retarget/release work serviceable.
        Schedule continuing;check(continuing.Start(0,4));continuing.helpersActive=false;
        check(continuing.Due(1500)!=Lane::Spawn&&continuing.active);
        continuing.vehicleAt=continuing.selectAt=continuing.convergenceAt=10000;
        check(continuing.Due(1700)==Lane::Hunt);
        continuing.Pressed(1800);check(continuing.Due(2000)==Lane::Release);
    }
    Schedule s;check(s.Due(100)==Lane::None);s.Start(1000);
    check(s.expires==1000+MaximumElapsedMs&&MaximumPhases==60000);
    for(const unsigned seconds:{1800u,3600u,5400u}){
        Schedule duration;check(duration.Start(1000,4,seconds));
        check(duration.expires==1000+static_cast<std::uint64_t>(seconds)*1000);
    }
    {Schedule invalidDuration;check(!invalidDuration.Start(1000,4,600));check(!invalidDuration.active);}
    {
        Schedule preparation;check(preparation.Start(0,1));
        preparation.settingsActive=true;preparation.settingsAt=0;
        check(preparation.Due(1500)==Lane::Settings); // settings precede helper spawning
        preparation.Pressed(1600);check(preparation.Due(1800)==Lane::Release);
        preparation.Released();preparation.settingsActive=false;
        check(preparation.Due(1900)==Lane::Spawn);
        preparation.spawned=1;preparation.loadoutActive=true;preparation.loadoutAt=0;
        check(preparation.Due(2000)==Lane::Loadout);
    }
    check(s.Due(1099)==Lane::None);check(s.Due(1100)==Lane::None);
    check(s.Due(1300)==Lane::Select);check(s.Due(1700)==Lane::Hunt);
    check(s.Due(2500)==Lane::Spawn);++s.spawned;
    s.Pressed(3000);check(s.Due(3100)==Lane::None);
    check(s.Due(3200)==Lane::Release);check(s.pressed);s.Released();check(!s.pressed);
    check(s.AttackDue(3200,1));check(s.AttackDue(5600,260.f*260.f));
    check(!s.AttackDue(5600,261.f*261.f));check(!s.AttackDue(5600,-1));
    check(!s.AttackDue(5600,NAN));s.active=false;check(s.Due(9000)==Lane::None);
    s.Pressed(9000);check(s.Due(9200)==Lane::Release); // stopped new work still drains input
    s.Released();check(s.Due(10000)==Lane::None);
    Schedule convergence;check(convergence.Start(0,0));
    convergence.selectAt=convergence.huntAt=convergence.vehicleAt=10000;
    check(convergence.Due(500)==Lane::Convergence);check(convergence.Due(500)==Lane::None);
    convergence.Pressed(600);check(convergence.Due(800)==Lane::Release);
    convergence.Released();check(convergence.Due(900)==Lane::Convergence);
    {
        Schedule sweep;check(sweep.Start(0,0));
        sweep.huntAt=sweep.vehicleAt=sweep.convergenceAt=10000;
        check(sweep.Due(300)==Lane::Select);sweep.ContinueSelection(300,false);
        check(sweep.Due(400)==Lane::None&&sweep.Due(500)==Lane::Select);
        sweep.ContinueSelection(500,true);
        check(sweep.Due(2399)==Lane::None&&sweep.Due(2499)==Lane::Select);
        sweep.ContinueSelection(2499,false);sweep.Pressed(2499);
        check(sweep.Due(2699)==Lane::Release); // short sweep cannot starve release
    }
    Report r;check(Valid(r));r.maxReads=ReportReadCap+1;check(!Valid(r));r.maxReads=0;
    r.releasePending=2;check(!Valid(r));r.releasePending=0;r.generation=1;check(!Valid(r));
    r.generation=0;r.convergenceActive=2;check(!Valid(r));r.convergenceActive=1;r.convergenceKeys=10;check(!Valid(r));
    r.convergenceKeys=9;r.phase=static_cast<unsigned>(Lane::Convergence);check(Valid(r));
    r.phase=static_cast<unsigned>(Lane::Loadout);check(Valid(r));
    r.phase=static_cast<unsigned>(Lane::Loadout)+1;check(!Valid(r));
    r.phase=static_cast<unsigned>(Lane::None);
    r.helperProjected.z=NAN;check(!Valid(r));r.helperProjected={};
    r.helperRequested.z=INFINITY;check(!Valid(r));r.helperRequested={};
    r.helperActual.x=NAN;check(!Valid(r));r.helperActual={};check(Valid(r));
    r.knifeStage=9;check(Valid(r));r.knifeStage=10;check(!Valid(r));r.knifeStage=0;
    r.difficulty=3;check(!Valid(r));r.difficulty=1;r.difficultyHookReady=2;check(!Valid(r));r.difficultyHookReady=0;
    r.difficultyMaxReads=ReadCap+1;check(!Valid(r));r.difficultyMaxReads=0;
    r.soundEmitterReady=2;check(!Valid(r));r.soundEmitterReady=0;r.doorDecisionPhase=3;check(!Valid(r));r.doorDecisionPhase=0;
    r.knifeFailure=8;check(!Valid(r));r.knifeFailure=99;check(!Valid(r));
    for(unsigned reason=100;reason<=119;++reason){r.knifeFailure=reason;check(Valid(r));}
    r.knifeFailure=120;check(!Valid(r));r.knifeFailure=0;
    r.endingRepairStage=10;check(!Valid(r));r.endingRepairStage=0;
    r.endingRepairCalls=5;check(!Valid(r));r.endingRepairCalls=0;
    r.killFailure=10;check(!Valid(r));r.killFailure=0;
    r.killResourceFailure=16;check(!Valid(r));r.killResourceFailure=0;check(Valid(r));
    r.killInteractionAdmission=10;check(!Valid(r));r.killInteractionAdmission=0;
    r.sweaterRetentionFailure=10;check(!Valid(r));r.sweaterRetentionFailure=0;
    r.sweaterRetentionRestored=1;check(!Valid(r));r.sweaterRetentionChecks=1;check(Valid(r));
    r.sweaterRetentionRestored=r.sweaterRetentionChecks=0;
    check(MaximumBots==9&&MaximumHelperAttempts==36);
    r.terminalCounselorMask=cfglimits::TerminalMaskLimit;check(!Valid(r));
    r.terminalCounselorMask=cfglimits::TerminalMaskLimit-1;check(Valid(r));r.terminalCounselorMask=0;
    r.emotePrepared=3;check(!Valid(r));r.emotePrepared=0;
    r.emoteReturns=1;check(!Valid(r));r.emoteReturns=0;
    r.emoteFailure=7;check(!Valid(r));r.emoteFailure=0;
    r.emoteCancelReturns=1;check(!Valid(r));r.emoteCancelReturns=0;check(Valid(r));
    for(unsigned failure=111;failure<=114;++failure){r.sweaterFailure=failure;check(Valid(r));}
    r.sweaterFailure=110;check(!Valid(r));r.sweaterFailure=115;check(!Valid(r));r.sweaterFailure=0;
    {Report trace;RecordSweaterFirstRefusal(trace,4,103,4,0x1234,777);
        RecordSweaterFirstRefusal(trace,7,104,4,0x5678,888);
        check(trace.sweaterFirstRefusalRecorded==1&&trace.sweaterFirstRefusalCaller==4&&
            trace.sweaterFirstRefusalFailure==103&&trace.sweaterFirstRefusalStage==4&&
            trace.sweaterFirstRefusalOwner==0x1234&&trace.sweaterFirstRefusalTick==777);
        for(unsigned i=0;i<10;++i)RecordSweaterOwnerChange(trace,0x100+i,0x200+i,900+i);
        check(trace.sweaterOwnerChangeCount==8&&trace.sweaterOwnerChangeOverflow==2&&
            trace.sweaterOwnerChanges[0].previous==0x100&&trace.sweaterOwnerChanges[7].tick==907);}
    {cfgsweater::Policy fresh;fresh.stage=cfgsweater::Stage::Waiting;fresh.lastFailure=103;fresh.Refuse();
        Report trace;trace.sweaterFreshFailure=2;
        RecordSweaterFirstRefusal(trace,201,fresh.lastFailure,static_cast<unsigned>(fresh.stage),0xCAFE,1234,trace.sweaterFreshFailure);
        check(fresh.stage==cfgsweater::Stage::Refused&&trace.sweaterFirstRefusalRecorded==1&&
            trace.sweaterFirstRefusalCaller==201&&trace.sweaterFirstRefusalFailure==103&&
            trace.sweaterFirstRefusalFreshFailure==2&&trace.sweaterFirstRefusalOwner==0xCAFE&&
            trace.sweaterFirstRefusalTick==1234);}
    r.roadStage=5;check(!Valid(r));r.roadStage=0;
    r.roadLastReads=ReadCap+1;check(!Valid(r));r.roadLastReads=0;
    r.sweaterFreshCreated=1;check(!Valid(r));r.sweaterFreshCreated=0;check(Valid(r));
    TargetChoice choice;check(!choice.Found());choice.Consider(0,100);check(choice.Found()&&choice.Index()==0);
    choice.Consider(2,25);check(choice.Index()==2);choice.Begin();check(!choice.Found());
    choice.Consider(0,NAN);choice.Consider(1,-1);choice.Consider(MaximumBots+2,1);check(!choice.Found());
    choice.Begin();choice.Consider(2,100,true);choice.Consider(0,46);
    check(choice.Index(true)==2&&choice.Index(false)==0);
    choice.Consider(1,44);check(choice.Index(true)==1);
    choice.Begin();choice.Consider(0,100);check(choice.Index(true)==0); // stale/dead retained pawn absent
    choice.Begin();choice.Consider(MaximumBots+1,25);check(choice.Found()&&choice.Index()==MaximumBots+1);
    // Each maintenance frame chooses only ONE lane; spawn/retarget/Move cannot bunch.
    Schedule continuous;continuous.Start(0);unsigned spawns=0,selections=0,hunts=0;
    for(std::uint64_t now=0;now<60000;now+=10){
        switch(continuous.Due(now)){
        case Lane::Spawn:++spawns;++continuous.spawned;break;
        case Lane::Select:++selections;break;
        case Lane::Hunt:++hunts;break;
        default:break;
        }
    }
    check(spawns==InitialBots);check(selections<35);check(hunts<90);
    {
        Schedule paced;paced.Start(0,4);unsigned scanned=0,sweeps=0,moving=0,releasing=0;
        for(std::uint64_t now=0;now<60000;now+=25){
            switch(paced.Due(now)){
            case Lane::Spawn:++paced.spawned;break;
            case Lane::Select:
                ++scanned;paced.ContinueSelection(now,scanned%5==0);
                if(scanned%5==0)++sweeps;
                break;
            case Lane::Hunt:++moving;if(paced.AttackDue(now,100))paced.Pressed(now);break;
            case Lane::Release:++releasing;paced.Released();break;
            default:break;
            }
        }
        check(sweeps>=12&&moving>=40&&releasing>=15&&paced.spawned==4);
    }
    for(unsigned requested=0;requested<=MaximumBots;++requested){
        Schedule selected;check(selected.Start(0,requested));unsigned actual=0;
        for(std::uint64_t now=0;now<30000;now+=100){if(selected.Due(now)==Lane::Spawn){++actual;++selected.spawned;}}
        check(actual==requested);check(!selected.Start(40000,requested));
    }
    Schedule invalid;check(!invalid.Start(0,MaximumBots+1));check(!invalid.active);
    {
        Schedule extra;HelperStartup helper;check(!extra.AdditionalHelper(0));check(extra.Start(0,0));
        check(helper.Reinforcement(0,0,MaximumBots)&&extra.AdditionalHelper(100));
        check(extra.requestedBots==1&&extra.spawnAt==200);check(!extra.AdditionalHelper(101));
        check(!helper.Reinforcement(0,1,MaximumBots));
        helper.stage=HelperStage::Spawn;helper.Confirm(1,1);extra.spawned=1;
        check(helper.Reinforcement(1,1,MaximumBots)&&extra.AdditionalHelper(300));
        helper.Refuse(HelperFailure::Result,true);extra.spawned=2;
        check(!helper.Reinforcement(2,2,MaximumBots));
        extra.spawned=extra.requestedBots=MaximumBots;check(!extra.AdditionalHelper(400));
        extra.active=false;check(!extra.AdditionalHelper(500));
    }
    check(!invalid.Start(UINT64_MAX,1));
    Configuration c;c.identity={0x10000,0x20000,1,2};c.submission=5;c.counselorBots=7;c.jasonCatalogueIndex=2;c.selectedJason[0]=44;
    cfgprimary::Report resources;resources.catalogue={0x30000,3,3};resources.entries[2]=c.selectedJason;
    check(MatchesConfiguration(c,c.identity,resources,MaximumBots));
    c.jasonCatalogueIndex=1;check(!MatchesConfiguration(c,c.identity,resources,MaximumBots));
    c.jasonCatalogueIndex=2;c.identity.worldSerial=3;check(!MatchesConfiguration(c,{0x10000,0x20000,1,2},resources,MaximumBots));
    c.identity.worldSerial=1;c.counselorBots=MaximumBots+1;check(!ValidConfiguration(c,MaximumBots));
    c.counselorBots=7;c.submission=0;check(!ValidConfiguration(c,MaximumBots));
    c.submission=5;c.selectedJason={};check(!ValidConfiguration(c,MaximumBots));
    for(const auto step:{25u,50u,125u,250u}){
        Schedule watched;check(watched.Start(0,0));watched.helpersActive=false;bool previous=false;
        unsigned cameras=0,watchedHunts=0,selection=0,release=0;std::uint64_t nextCamera=0;
        for(std::uint64_t now=0;now<30000;now+=step){
            const auto turn=SelectCameraTurn(watched,now,!watched.pressed&&now>=nextCamera,previous);
            previous=turn.camera;
            if(turn.camera){++cameras;nextCamera=now+50;continue;}
            if(turn.gameplay==Lane::Hunt){++watchedHunts;watched.Pressed(now);}
            if(turn.gameplay==Lane::Select)++selection;
            if(turn.gameplay==Lane::Release){++release;watched.Released();}
        }
        check(cameras>20&&watchedHunts>10&&selection>5&&release==watchedHunts);
    }
    {Schedule actions;check(actions.Start(0,0));const auto next=actions.next;
        const auto knife=SelectActionTurn(actions,10,true,false,true);
        check(!knife.camera&&knife.gameplay==Lane::Hunt&&actions.next==next);
        actions.Pressed(20);const auto release=SelectActionTurn(actions,200,true,false,true);
        check(release.gameplay==Lane::Release&&!release.camera);
        actions.Released();actions.active=false;check(SelectActionTurn(actions,300,true,false,true).gameplay==Lane::None);}
    {Report diagnostic;check(Valid(diagnostic));diagnostic.trapShackSlices=514;check(!Valid(diagnostic));
        diagnostic={};diagnostic.trapShackFailure=10;check(Valid(diagnostic));
        diagnostic.trapShackFailure=11;check(!Valid(diagnostic));
        diagnostic={};diagnostic.sweaterArmPending=2;check(!Valid(diagnostic));
        diagnostic={};diagnostic.sweaterArmFailure=7;check(!Valid(diagnostic));
        diagnostic={};diagnostic.sweaterArms=1;check(!Valid(diagnostic));
        diagnostic.sweaterArmAttempts=1;check(Valid(diagnostic));
        diagnostic={};diagnostic.rbPhase=9;check(Valid(diagnostic));
        diagnostic.rbPhase=10;check(Valid(diagnostic));diagnostic.rbPhase=11;check(!Valid(diagnostic));
        diagnostic={};diagnostic.endingRepairStage=8;diagnostic.endingRepairCalls=4;diagnostic.endingRepairFailure=19;check(Valid(diagnostic));
        diagnostic.endingRepairFailure=27;check(Valid(diagnostic));
        diagnostic.endingRepairFailure=28;check(!Valid(diagnostic));
        diagnostic={};diagnostic.soundBlipApplied=2;check(!Valid(diagnostic));
        diagnostic={};diagnostic.soundBlipRestored=1;check(!Valid(diagnostic));
        diagnostic.soundBlipRestores=1;check(Valid(diagnostic));
        diagnostic={};diagnostic.killGateFields=4097;check(!Valid(diagnostic));
        diagnostic={};diagnostic.loadoutReady=2;check(!Valid(diagnostic));
        diagnostic={};diagnostic.carSpeedActive=2;check(!Valid(diagnostic));
        diagnostic={};diagnostic.carStage=14;check(Valid(diagnostic));diagnostic.carStage=15;check(Valid(diagnostic));diagnostic.carStage=16;check(!Valid(diagnostic));
        diagnostic={};diagnostic.knifeFlightActive=2;check(!Valid(diagnostic));
        diagnostic={};diagnostic.knifeFlightSpeed=-1;check(!Valid(diagnostic));
        diagnostic={};diagnostic.trapShackRestarts=12;check(Valid(diagnostic));
        diagnostic.trapShackRestarts=514;check(!Valid(diagnostic));
        diagnostic={};diagnostic.knifeLaunchObserved=2;check(!Valid(diagnostic));
        diagnostic={};diagnostic.knifeActualVelocity.x=NAN;check(!Valid(diagnostic));}
    {Report diagnostic;diagnostic.botNameFailure=9;diagnostic.endingRepairFailure=27;check(Valid(diagnostic));
     diagnostic.botNameFailure=10;check(!Valid(diagnostic));}
    {Report diagnostic;check(Valid(diagnostic));diagnostic.sweaterClassAttempts=9;check(!Valid(diagnostic));
        diagnostic={};diagnostic.sweaterClassFailure=6;check(!Valid(diagnostic));
        diagnostic={};diagnostic.travelMoveNativeResult=3;check(Valid(diagnostic)); // preserve anomalous native diagnostics
        diagnostic.travelMoveNativeResult=256;check(!Valid(diagnostic));
        diagnostic={};diagnostic.roadActual.x=NAN;check(!Valid(diagnostic));}
    {Report diagnostic;diagnostic.sweaterClassAnchor=9;check(Valid(diagnostic));
     for(const auto failure:{30u,31u,32u}){diagnostic.sweaterClassFailure=failure;check(Valid(diagnostic));}
     diagnostic.sweaterClassFailure=33;check(Valid(diagnostic));
     diagnostic.sweaterClassFailure=34;check(!Valid(diagnostic));
     diagnostic.sweaterClassFailure=0;diagnostic.sweaterClassAnchor=8;check(Valid(diagnostic));
     diagnostic.sweaterClassAnchor=10;check(!Valid(diagnostic));}
    {Report diagnostic;diagnostic.spectatorRefusals=1;diagnostic.spectatorResourceBinding=4;
        diagnostic.spectatorLastRefusal.Capture(100,1,9,8,0,901,3,4,4,2,25);
        check(Valid(diagnostic)); // owned Actor teleport refusal must not erase the whole report
        diagnostic.spectatorLastRefusal.binding=5;check(!Valid(diagnostic));
        diagnostic.spectatorLastRefusal.binding=4;diagnostic.spectatorLastRefusal.resource=6;check(!Valid(diagnostic));}
    {Report diagnostic;diagnostic.knifeAcquiredCollision=3;check(!Valid(diagnostic));
     diagnostic={};diagnostic.knifeReleaseCollisionAfter=3;check(!Valid(diagnostic));
     diagnostic={};diagnostic.knifeNotifyMaxReads=ReadCap+1;check(!Valid(diagnostic));
     diagnostic={};diagnostic.knifeStopStage=11;check(!Valid(diagnostic));
     diagnostic={};diagnostic.doorBreakIntent=2;check(!Valid(diagnostic));}
    std::printf("CFG bundle phased scheduling/range/release/report tests: %s (%u failures). No game access.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
