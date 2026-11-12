#define CFG_UNVERIFIED_RUNTIME_FIXTURE
#include "../Core/JasonApproachPolicy.hpp"
#include "PrimarySpawnObservation.hpp"
#include "CrowdObservation.hpp"
#define main CfgExistingAiResourceTestsMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main

struct PrimaryAccess {
    bool owner=true,resolve=false,uncertainWrite=false,budgetOkay=true; unsigned step=0,failAt=0,calls=0,writes=0,conversions=0,index=0xFFFFFFFF;
    unsigned lookupCount=1,nulls=0,spawnCalls=0;std::uint64_t now=1000;
    bool spawnCallOkay=true,nativeException=false;unsigned moveResult=2;
    std::uint64_t Now()const{return now;}
    bool Next() { return ++step!=failAt; }
    bool OwnerThread() const { return owner; }
    bool PrepareOwnerContext() { return Next(); }
    bool ContextMatches() { return Next(); }
    bool ResourcesMatch() { return Next(); }
    bool ResolveFunctions() { if (!Next()) return false; calls+=4; return true; }
    bool ResolvePawnClass(std::uintptr_t& value) {
        if (!Next()) return false;
        ++calls; conversions=lookupCount; calls+=lookupCount;
        if (!resolve) return false;
        value=0x40000000; index=lookupCount-1; return true;
    }
    bool Location(cfgspawn::Vec3& v) { if (!Next()) return false; ++calls; v={100,200,300}; return true; }
    bool StartTransform(cfgspawn::Vec3& v,cfgspawn::Vec3& rotation){
        if(!Location(v))return false;rotation={0,75,0};return true;
    }
    bool Spawn(cfgspawn::SpawnAIParameters& p) {
        if (!Next()) return false;
        CHECK(p.pawnClass==0x40000000 && p.behaviorTree==0 && p.noCollisionFail==0);
        CHECK(p.location.x==100&&p.location.y==200&&p.location.z==300&&p.rotation.y==75);
        ++calls;++spawnCalls;if(!spawnCallOkay)return false;
        p.result=spawnCalls<=nulls?0:0x90000000;return true;
    }
    bool VerifyInitial(std::uintptr_t pawn,std::uintptr_t& controller) {
        if (!Next() || pawn!=0x90000000) return false; controller=0x90010000; return true;
    }
    bool Unpossess(std::uintptr_t c) { if (!Next() || c!=0x90010000) return false; ++calls; return true; }
    bool VerifyUnpossessed() { return Next(); }
    bool WriteInstanceClass() { if (!Next()) return false; ++writes; return !uncertainWrite; }
    bool DefaultController() { if (!Next()) return false; ++calls; return true; }
    bool VerifyKiller(std::uintptr_t& c,std::uintptr_t& p) { if (!Next()) return false; c=0x90020000; p=0x90030000; return true; }
    bool ConfigureNavigation() { if (!Next()) return false; ++calls; return true; }
    void NavigationReport(cfgspawn::SpawnReport& report) const { report.crowdKind=3; report.crowdStateBefore=0; report.crowdStateAfter=2; }
    bool Move(cfgspawn::MoveParameters& p) { if (!Next()) return false; CHECK(p.goal==0x20000000); ++calls; p.result=static_cast<std::uint8_t>(moveResult); return true; }
    bool VerifyAfterMove() { return Next(); }
    unsigned Reads() const { return step; } unsigned Calls() const { return calls; } unsigned Writes() const { return writes; }
    unsigned ClassConversions() const { return conversions; } unsigned CatalogueIndex() const { return index; }
    bool CompletionBudget() { return Next() && budgetOkay; }
    bool BudgetRefused() const { return !budgetOkay; }
    bool PrimaryKiller(std::uintptr_t& value) const { value=calls>=11?0x90000000:0; return true; }
    unsigned Error() const { return 0; } bool NativeException() const { return nativeException; }
    void Publish(std::uint32_t& value,std::uint32_t next) { value=next; }
};
int main() {
    CHECK(CfgExistingAiResourceTestsMain()==0);
    using namespace cfgspawn;
    const Identity human{0x10000000,0x20000000,7,8};
    SpawnRequest request; request.identity=human;
    OneJasonApproach wrong; CHECK(!wrong.Configure(human,true,100,0x30000000,0));
    OneJasonApproach preset; CHECK(!preset.Configure(human,true,100,human.pawn,0x40000000));
    OneJasonApproach noClass; CHECK(noClass.Configure(human,true,100,human.pawn,0) && noClass.Submit(request));
    PrimaryAccess refused; refused.lookupCount=18; noClass.Service(refused);
    CHECK(noClass.report.state==static_cast<unsigned>(SpawnState::ClassResolutionFailed) && noClass.report.result==0 &&
        refused.writes==0 && refused.calls==23 && noClass.report.classConversions==18 && ValidSpawnReport(noClass.report));
    OneJasonApproach first; CHECK(first.Configure(human,true,100,human.pawn,0) && first.Submit(request));
    PrimaryAccess live; live.resolve=true; live.owner=false; first.Service(live); CHECK(first.Pending() && live.calls==0);
    live.owner=true; first.Service(live);
    CHECK(first.report.state==static_cast<unsigned>(SpawnState::MoveSubmitted) && first.report.catalogueIndex==0 &&
        first.report.sourcePawn==human.pawn && live.calls==12 && live.writes==1 && ValidSpawnReport(first.report));
    const auto count=live.step; first.Service(live); CHECK(live.step==count && !first.Submit(request));
    for(unsigned result : {0u,1u,2u,3u,255u}){
        OneJasonApproach primary;CHECK(primary.Configure(human,true,100,human.pawn,0)&&primary.Submit(request));
        PrimaryAccess access;access.resolve=true;access.moveResult=result;primary.Service(access);
        CHECK((primary.report.state==static_cast<unsigned>(SpawnState::MoveSubmitted))==(result<=2));
        CHECK(primary.report.moveResult==result&&primary.report.result==0x90000000&&access.writes==1&&ValidSpawnReport(primary.report));
    }
    for(unsigned nulls=1;nulls<=MaximumPrimaryPlacements;++nulls){
        OneJasonApproach retry;CHECK(retry.Configure(human,true,100,human.pawn,0)&&retry.Submit(request));
        PrimaryAccess access;access.resolve=true;access.nulls=nulls;
        for(unsigned attempt=0;attempt<MaximumPrimaryPlacements;++attempt){
            access.calls=0;access.step=0;retry.Service(access);
            CHECK(retry.report.attempts==attempt+1&&ValidSpawnReport(retry.report));
            if(retry.report.state==static_cast<unsigned>(SpawnState::MoveSubmitted))break;
            if(attempt+1<MaximumPrimaryPlacements){
                CHECK(retry.Pending()&&retry.report.retryPending&&!access.writes);
                const auto steps=access.step;access.now+=399;retry.Service(access);CHECK(access.step==steps);
                ++access.now;
            }
        }
        CHECK(retry.report.nullSpawns==nulls&&retry.report.attempts==(nulls<4?nulls+1:4));
        CHECK(!retry.Pending()&&!retry.report.retryPending&&!retry.Submit(request));
        CHECK((retry.report.state==static_cast<unsigned>(SpawnState::MoveSubmitted))==(nulls<4));
        CHECK(access.writes==(nulls<4?1u:0u));
    }
    {
        OneJasonApproach ambiguous;CHECK(ambiguous.Configure(human,true,100,human.pawn,0)&&ambiguous.Submit(request));
        PrimaryAccess access;access.resolve=true;access.spawnCallOkay=false;ambiguous.Service(access);
        CHECK(!ambiguous.Pending()&&!ambiguous.report.retryPending&&!ambiguous.report.nullSpawns&&!access.writes);
    }
    for (unsigned fail=1;fail<=count;++fail) {
        OneJasonApproach r; CHECK(r.Configure(human,true,100,human.pawn,0) && r.Submit(request));
        PrimaryAccess a; a.resolve=true; a.failAt=fail; r.Service(a);
        CHECK(a.step==fail && r.report.state!=static_cast<unsigned>(SpawnState::MoveSubmitted) && !r.Submit(request) && ValidSpawnReport(r.report));
    }
    {
        OneJasonApproach fault;CHECK(fault.Configure(human,true,100,human.pawn,0)&&fault.Submit(request));
        PrimaryAccess access;access.resolve=true;access.spawnCallOkay=false;access.nativeException=true;
        fault.Service(access);
        CHECK(fault.report.state==static_cast<unsigned>(SpawnState::NativeException)&&
            fault.report.exceptionStage==static_cast<unsigned>(SpawnState::SpawnReturnedNull)&&
            !fault.report.nullSpawns&&!fault.report.retryPending&&!fault.Pending()&&ValidSpawnReport(fault.report));
        const auto entries=access.spawnCalls;access.now+=1000;fault.Service(access);
        CHECK(access.spawnCalls==entries&&!fault.Submit(request));
    }
    OneJasonApproach last; CHECK(last.Configure(human,true,100,human.pawn,0) && last.Submit(request));
    PrimaryAccess all; all.resolve=true; all.lookupCount=18; last.Service(all);
    CHECK(all.calls==29 && last.report.catalogueIndex==17 && ValidSpawnReport(last.report));
    auto bad=last.report; bad.nativeCalls=32; CHECK(!ValidSpawnReport(bad)); bad=last.report; bad.classConversions=19; CHECK(!ValidSpawnReport(bad));
    bad=last.report; bad.magic=0x4346474A; CHECK(!ValidSpawnReport(bad));
    CHECK(cfgprimary::CanComplete(4692,8192) && !cfgprimary::CanComplete(4693,8192) && !cfgprimary::CanComplete(8193,8192));
    OneJasonApproach low; CHECK(low.Configure(human,true,100,human.pawn,0) && low.Submit(request));
    PrimaryAccess budget; budget.resolve=true; budget.budgetOkay=false; low.Service(budget);
    CHECK(low.report.state==static_cast<unsigned>(SpawnState::PreSpawnChanged) && !low.report.result && !budget.writes &&
        budget.calls==7 && low.report.budgetRefused==1 && ValidSpawnReport(low.report));
    cfgprimary::SoftClassBytes bytes{}; bytes[16]=1;
    cfgprimary::ConversionParameters packet{}; CHECK(cfgprimary::BuildConversion(bytes,packet) && packet.result==0 && packet.softClass==bytes);
    bytes[24]=1; CHECK(!cfgprimary::BuildConversion(bytes,packet)); bytes={}; CHECK(!cfgprimary::BuildConversion(bytes,packet));
    { Fixture f; f.Put(f.module+cfgprimary::SerializationFlag,std::uint8_t{0}); f.Put(f.module+cfgprimary::GarbageCollectionCounter,std::uint32_t{0});
        CHECK(cfgprimary::LookupSafe(f,f.module)); f.Put(f.module+cfgprimary::SerializationFlag,std::uint8_t{1}); CHECK(!cfgprimary::LookupSafe(f,f.module));
        f.Put(f.module+cfgprimary::SerializationFlag,std::uint8_t{0}); f.Put(f.module+cfgprimary::GarbageCollectionCounter,std::uint32_t{256}); CHECK(!cfgprimary::LookupSafe(f,f.module)); }
    { Fixture f; f.Put(f.defaults+PawnAIClass,f.classes[BaseAI]); cfgspawn::TemplateObservation observed{};
        CHECK(cfgprimary::ObserveResolvedTemplate(f,f.objects,f.pawnClass,f.killerBase,f.classes[BaseAI],observed));
        CHECK(observed.classSlot.serial==0 && observed.defaultSlot.serial==0);
        f.Put(f.defaults+cfgprobe::ObjectClass,f.botClass); CHECK(!cfgprimary::ObserveResolvedTemplate(f,f.objects,f.pawnClass,f.killerBase,f.classes[BaseAI],observed)); }
    { Fixture f; f.Put(f.defaults+PawnAIClass,f.classes[BaseAI]); f.trigger=f.defaults+PawnAIClass; f.mutation=f.trigger; f.newPointer=f.classes[CounselorAI];
        cfgspawn::TemplateObservation observed{}; CHECK(!cfgprimary::ObserveResolvedTemplate(f,f.objects,f.pawnClass,f.killerBase,f.classes[BaseAI],observed)); }
    { Fixture f; f.cap=8192; cfgprobe::lifetime::SlotObservation stateSlot{};
        CHECK(ObjectSlot(f,f.objects,f.state,stateSlot));
        const auto humanController=std::uintptr_t{0x90040000};
        CHECK(cfgprimary::RegistryMatches(f,f.objects,stateSlot,0,humanController,f.classes[PlayerState]));
        f.Put(f.state+cfgprimary::CurrentKiller,f.pawn);
        CHECK(!cfgprimary::RegistryMatches(f,f.objects,stateSlot,0,humanController,f.classes[PlayerState]));
        CHECK(cfgprimary::RegistryMatches(f,f.objects,stateSlot,f.pawn,humanController,f.classes[PlayerState]));
        f.Put(f.state+cfgprimary::CurrentKillerState,f.botState);
        f.Put(f.pawn+0x3A0,f.botController); f.Put(f.botController+0x370,f.pawn);
        CHECK(cfgprimary::RegistryMatches(f,f.objects,stateSlot,f.pawn,humanController,f.classes[PlayerState]));
        CHECK(!cfgprimary::RegistryMatches(f,f.objects,stateSlot,f.pawn,f.botController,f.classes[PlayerState]));
        f.Put(f.pawn+0x3A0,std::uintptr_t{0});
        CHECK(!cfgprimary::RegistryMatches(f,f.objects,stateSlot,f.pawn,humanController,f.classes[PlayerState])); }
    { Fixture f; f.cap=4096; const auto component=f.Object();
        f.Put(component,f.module+cfgcrowd::SCVtable); f.Put(component+0x20,f.botController); f.Put(component+0xE0,f.botController);
        f.Put(f.module+cfgcrowd::SCVtable+cfgcrowd::FollowSlot,f.module+cfgcrowd::CrowdFollow);
        f.Put(f.module+cfgcrowd::SCVtable+cfgcrowd::SetterSlot,f.module+cfgcrowd::Setter);
        f.Put(component+cfgcrowd::PathStatus,std::uint32_t{0}); f.Put(component+cfgcrowd::SimulationState,std::uint8_t{0});
        f.Put(component+cfgcrowd::RegistrationFlags,std::uint8_t{0x10});
        cfgprobe::lifetime::SlotObservation slot{}; CHECK(ObjectSlot(f,f.objects,component,slot)); cfgcrowd::Observation o{};
        CHECK(cfgcrowd::Observe(f,f.module,f.objects,slot,f.botController,o) && cfgcrowd::Idle(o) && !cfgcrowd::OrdinaryReady(o));
        f.Put(component+cfgcrowd::PathStatus,std::uint32_t{3});
        CHECK(cfgcrowd::Observe(f,f.module,f.objects,slot,f.botController,o) && !cfgcrowd::Idle(o));
        f.Put(component+cfgcrowd::SimulationState,std::uint8_t{2}); f.Put(component+cfgcrowd::RegistrationFlags,std::uint8_t{0});
        CHECK(cfgcrowd::Observe(f,f.module,f.objects,slot,f.botController,o) && cfgcrowd::OrdinaryReady(o));
        f.Put(component+0xE0,f.pawn); CHECK(!cfgcrowd::Observe(f,f.module,f.objects,slot,f.botController,o));
        f.Put(component+0xE0,f.botController);
        f.Put(f.module+cfgcrowd::SCVtable+cfgcrowd::SetterSlot,f.module+0x1234);
        CHECK(!cfgcrowd::Observe(f,f.module,f.objects,slot,f.botController,o));
        f.Put(f.module+cfgcrowd::SCVtable+cfgcrowd::SetterSlot,f.module+cfgcrowd::Setter);
        f.Put(component,f.module+0x1234);
        CHECK(!cfgcrowd::Observe(f,f.module,f.objects,slot,f.botController,o)); }
    { Fixture f; f.cap=4096; const auto type=f.Object(),component=f.Object(type),updated=f.Object();
        f.Put(f.module+cfgcrowd::MovementClassCache,type); f.Put(f.pawn+cfgcrowd::CharacterMovement,component);
        f.Put(component+0xE0,f.pawn); f.Put(component+0x20,f.pawn); f.Put(updated+0xE0,f.pawn);
        f.Put(component+cfgcrowd::UpdatedComponent,updated); f.Put(component+cfgcrowd::MovementMode,std::uint8_t{1});
        f.Put(component+cfgcrowd::MaxWalkSpeed,250.0f); f.Put(component+cfgcrowd::Velocity,Vec3{50,0,0});
        cfgcrowd::MovementSample m{}; CHECK(cfgcrowd::Movement(f,f.module,f.objects,f.pawn,m) && m.mode==1 && m.velocity.x==50);
        f.Put(component+0xE0,f.bot); CHECK(!cfgcrowd::Movement(f,f.module,f.objects,f.pawn,m));
        f.Put(component+0xE0,f.pawn); f.Put(component+cfgcrowd::MovementMode,std::uint8_t{255});
        CHECK(!cfgcrowd::Movement(f,f.module,f.objects,f.pawn,m));
        f.Put(component+cfgcrowd::MovementMode,std::uint8_t{1}); f.cap=f.reads;
        CHECK(!cfgcrowd::Movement(f,f.module,f.objects,f.pawn,m)); }
    std::printf("CFG primary Jason policy/conversion/owned-template/crowd guards: %s. No game access.\n",failures?"FAIL":"PASS");
    return failures?1:0;
}
