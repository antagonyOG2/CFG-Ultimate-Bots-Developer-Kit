#include "../Core/JasonApproachPolicy.hpp"
#include "JasonInstanceObservation.hpp"
// Exercise the existing production post-spawn reader as a regression too.
#define main CounselorResultRegressionEntry
#include "SpawnResultTests.cpp"
#undef main
namespace {
struct FakeAccess {
    bool owner=true,failAfterWrite=false; unsigned step=0,failAt=0,calls=0,writes=0,publishes=0;
    unsigned moveResult=2;
    bool Next() { return ++step!=failAt; }
    bool OwnerThread() const { return owner; }
    bool ContextMatches() { return Next(); }
    bool ResourcesMatch() { return Next(); }
    bool ResolveFunctions() { if (!Next()) return false; calls+=4; return true; }
    bool Location(cfgspawn::Vec3& out) { if (!Next()) return false; ++calls; out={100,200,300}; return true; }
    bool Spawn(cfgspawn::SpawnAIParameters& p) { if (!Next()) return false; ++calls; p.result=0x90000000; return true; }
    bool VerifyInitial(std::uintptr_t pawn,std::uintptr_t& controller) {
        if (!Next() || pawn!=0x90000000) return false; controller=0x90010000; return true;
    }
    bool Unpossess(std::uintptr_t c) { if (!Next() || c!=0x90010000) return false; ++calls; return true; }
    bool VerifyUnpossessed() { return Next(); }
    bool WriteInstanceClass() { if (!Next()) return false; ++writes; return !failAfterWrite; }
    bool DefaultController() { if (!Next()) return false; ++calls; return true; }
    bool VerifyKiller(std::uintptr_t& c,std::uintptr_t& p) { if (!Next()) return false; c=0x90020000; p=0x90030000; return true; }
    bool Move(cfgspawn::MoveParameters& p) { if (!Next()) return false; ++calls; p.result=static_cast<std::uint8_t>(moveResult); return true; }
    bool VerifyAfterMove() { return Next(); }
    unsigned Reads() const { return step; } unsigned Calls() const { return calls; } unsigned Writes() const { return writes; }
    unsigned Error() const { return 0; } bool NativeException() const { return false; }
    void Publish(std::uint32_t& value,std::uint32_t next) { value=next; ++publishes; }
};
cfgspawn::Identity Identity() { return {0x10000000,0x20000000,7,8}; }
void PolicyTests() {
    using namespace cfgspawn;
    SpawnAIParameters packet;
    CHECK(BuildJasonPacket(0x10000000,0x20000000,{10,20,30},packet));
    CHECK(packet.behaviorTree==0 && packet.noCollisionFail==0 && packet.result==0 && packet.padding[6]==0);
    CHECK(!BuildParameters(0x10000000,0x20000000,0,{10,20,30},{},packet));
    CHECK(!BuildJasonPacket(0x10000000,0x20000000,{NAN,20,30},packet));
    MoveParameters move; CHECK(move.goal==0 && move.radius==125 && move.stopOnOverlap==1 && move.usePathfinding==1 &&
        move.canStrafe==0 && move.filterClass==0 && move.allowPartial==1 && move.result==255 && move.padding[5]==0);
    SpawnRequest request; request.identity=Identity();
    OneJasonApproach runtime; CHECK(runtime.Configure(Identity(),true,100,0x30000000,0x40000000));
    auto wrong=request; wrong.identity.pawnSerial++; CHECK(!runtime.Submit(wrong));
    wrong=request; wrong.magic=0x43464753; CHECK(!runtime.Submit(wrong));
    wrong=request; wrong.reserved=1; CHECK(!runtime.Submit(wrong));
    CHECK(runtime.Submit(request) && !runtime.Submit(request));
    FakeAccess a; a.owner=false; runtime.Service(a); CHECK(runtime.Pending() && !a.step && !a.calls);
    a.owner=true; runtime.Service(a);
    CHECK(runtime.report.state==static_cast<unsigned>(SpawnState::MoveSubmitted) && runtime.report.resultController==0x90020000);
    CHECK(runtime.report.location.x==500 && runtime.report.tree==0 && a.calls==9 && a.writes==1 && a.publishes==2 && ValidSpawnReport(runtime.report));
    const auto steps=a.step; runtime.Service(a); CHECK(a.step==steps && !runtime.Pending() && !runtime.Submit(request));
    for (unsigned failure=1;failure<=steps;++failure) {
        OneJasonApproach r; CHECK(r.Configure(Identity(),true,100,0x30000000,0x40000000) && r.Submit(request));
        FakeAccess denied; denied.failAt=failure; r.Service(denied);
        CHECK(denied.step==failure && r.report.state!=static_cast<unsigned>(SpawnState::MoveSubmitted) &&
            !r.Pending() && !r.Submit(request) && denied.calls<=9 && denied.writes<=1 && ValidSpawnReport(r.report));
    }
    for (unsigned result: {0u,1u,2u,3u,255u}) {
        OneJasonApproach r; CHECK(r.Configure(Identity(),true,100,0x30000000,0x40000000) && r.Submit(request));
        FakeAccess resultAccess; resultAccess.moveResult=result; r.Service(resultAccess);
        CHECK((r.report.state==static_cast<unsigned>(SpawnState::MoveSubmitted))==(result==1 || result==2));
        CHECK(r.report.moveResult==result);
    }
    OneJasonApproach partial; CHECK(partial.Configure(Identity(),true,100,0x30000000,0x40000000) && partial.Submit(request));
    FakeAccess uncertain; uncertain.failAfterWrite=true; partial.Service(uncertain);
    CHECK(partial.report.state==static_cast<unsigned>(SpawnState::InstanceWriteIndeterminate) && partial.report.instanceWrites==1 &&
        !partial.Submit(request) && ValidSpawnReport(partial.report));
    OneJasonApproach invalid; CHECK(!invalid.Configure(Identity(),true,4097,0x30000000,0x40000000));
    OneJasonApproach human; CHECK(!human.Configure(Identity(),true,100,Identity().pawn,0x40000000));
}
void OwnedInstanceTests() {
    Fixture f; f.cap=2048;
    cfgprobe::lifetime::ArrayHeader array{f.array,64};
    cfgjason::NewActorEvidence e;
    CHECK(cfgspawn::ObjectSlot(f,array,f.pawn,e.pawn) && cfgspawn::ObjectSlot(f,array,f.controller,e.initialController));
    const auto target=std::uintptr_t{0x40100000},defaults=std::uintptr_t{0x50100000},targetController=std::uintptr_t{0x40110000};
    f.Put(f.controller+0x370,std::uintptr_t{0}); f.Put(f.pawn+0x3A0,std::uintptr_t{0});
    f.Put(f.pawn+cfgspawn::PawnAIClass,f.aiClass);
    const auto detached=[&](Fixture& reader,const cfgjason::NewActorEvidence& evidence) {
        reader.reads=0;
        return cfgjason::Detached(reader,array,evidence,f.plan,target,f.pawnClass,defaults,f.counselorClass,f.aiClass,targetController);
    };
    CHECK(detached(f,e));
    for (unsigned n=0;n<8;++n) {
        auto bad=f; auto altered=e;
        if (n==0) altered.pawn.object=f.plan.pawn;
        if (n==1) altered.pawn.object=defaults;
        if (n==2) altered.initialController.object=f.plan.controller;
        if (n==3) bad.Put(f.controller+0x370,f.plan.pawn);
        if (n==4) bad.Put(f.pawn+0x3A0,f.controller);
        if (n==5) bad.Put(f.pawn+cfgspawn::PawnAIClass,f.pawnClass);
        if (n==6) bad.Put(f.array+40*24+16,std::int32_t{0});
        if (n==7) bad.Put(f.array+40*24+8,std::uint32_t{0x20000000});
        CHECK(!detached(bad,altered));
    }
    auto changed=f; changed.trigger=f.array+40*24+16; changed.mutation=changed.trigger; changed.replacement=999;
    CHECK(!detached(changed,e));
    const auto killer=std::uintptr_t{0x40040000},killerClass=std::uintptr_t{0x50040000};
    f.Slot(killer,42,503); f.Slot(f.brain,43,504);
    CHECK(cfgspawn::ObjectSlot(f,array,killer,e.killerController) && cfgspawn::ObjectSlot(f,array,f.brain,e.path));
    f.Put(killer+cfgprobe::ObjectClass,killerClass); f.Put(killerClass+cfgprobe::ClassSuper,f.aiClass);
    f.Put(f.pawn+cfgspawn::PawnAIClass,killerClass); f.Put(f.pawn+0x3A0,killer); f.Put(killer+0x370,f.pawn);
    f.Put(killer+0x410,f.brain); f.Put(f.brain+0x20,killer);
    f.Put(f.brain+cfgjason::disk::ComponentOwner,killer);
    const auto linked=[&](Fixture& reader,const cfgjason::NewActorEvidence& evidence) {
        reader.reads=0;
        return cfgjason::LinkedKiller(reader,array,evidence,f.plan,target,f.pawnClass,defaults,f.counselorClass,killerClass,f.brainClass,targetController);
    };
    CHECK(linked(f,e));
    auto noWeakSerial=f; auto owned=e;
    noWeakSerial.Put(f.array+43*24+16,std::int32_t{0}); owned.path.serial=0;
    CHECK(linked(noWeakSerial,owned)); // owned subobject, not a reused weak identity
    noWeakSerial.Put(f.brain+0x20,f.plan.controller);
    CHECK(!linked(noWeakSerial,owned));
    auto wrongComponentOwner=f; wrongComponentOwner.Put(f.brain+cfgjason::disk::ComponentOwner,f.plan.controller);
    CHECK(!linked(wrongComponentOwner,e));
    auto ownerChanged=f; ownerChanged.trigger=f.brain+cfgjason::disk::ComponentOwner;
    ownerChanged.mutation=ownerChanged.trigger; ownerChanged.replacement=f.plan.controller;
    CHECK(!linked(ownerChanged,e));
    for (unsigned n=0;n<7;++n) {
        auto bad=f; auto altered=e;
        if (n==0) altered.killerController.object=f.plan.controller;
        if (n==1) altered.killerController.object=e.initialController.object;
        if (n==2) bad.Put(f.brain+0x20,f.plan.controller);
        if (n==3) bad.Put(killer+0x370,f.plan.pawn);
        if (n==4) bad.Put(killer+0x410,f.pawn);
        if (n==5) bad.Put(f.pawn+0x3A0,f.controller);
        if (n==6) bad.Put(f.brain+cfgprobe::ObjectClass,f.aiClass);
        CHECK(!linked(bad,altered));
    }
    auto limited=f; limited.cap=3; CHECK(!linked(limited,e));
}
}
int main() {
    const auto regression=CounselorResultRegressionEntry();
    PolicyTests(); OwnedInstanceTests();
    std::printf("CFG Jason approach policy, native packets and actual new-instance guards %s (%d failures). No game access.\n",failures?"FAIL":"PASS",failures);
    return regression || failures?1:0;
}
