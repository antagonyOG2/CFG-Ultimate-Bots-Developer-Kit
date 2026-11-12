#include "../Core/OneCounselorSpawnPolicy.hpp"
#include <cstdio>
#include <cstring>
namespace {
int failures=0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL %d: %s\n",__LINE__,#c); ++failures; } } while(false)
struct Access {
    bool owner=true,context=true,resources=true,location=true,spawn=true,verify=true,exception=false;
    bool retain=true,retained=true,admitted=false,loseAfterLocation=false,loseAfterSpawn=false;
    cfgspawn::OneCounselorSpawn* reenter=nullptr;
    unsigned calls=0,contexts=0,resourcesCalls=0,publishCalls=0;
    unsigned contextFailAt=0,resourceFailAt=0;
    std::uintptr_t result=0xA0000;
    bool OwnerThread() { return owner; }
    bool ContextMatches() { return context && ++contexts!=contextFailAt; }
    bool ResourcesMatch() { return resources && ++resourcesCalls!=resourceFailAt; }
    bool RetainInputs() { admitted=retain;return retain; }
    bool InputsRetained() { return admitted&&retained; }
    bool Location(cfgspawn::Vec3& out) { CHECK(admitted&&retained);++calls; out={10,20,30};if(loseAfterLocation)retained=false;return location; }
    bool Spawn(cfgspawn::SpawnAIParameters& packet) {
        CHECK(admitted&&retained);
        ++calls; CHECK(!packet.noCollisionFail && packet.world==0x10000 && packet.pawnClass==0x40000 && packet.behaviorTree==0x50000);
        CHECK(packet.location.x==190 && packet.location.y==20 && packet.location.z==30);
        if(reenter)reenter->Service(*this); // allocating native call re-enters the adapter
        if(loseAfterSpawn)retained=false;
        packet.result=result; return spawn;
    }
    bool VerifyResult(std::uintptr_t pawn,std::uintptr_t& controller) { CHECK(pawn==result); controller=0xB0000; return verify; }
    unsigned Calls() const { return calls; }
    unsigned Reads() const { return 10; }
    unsigned Error() const { return 0; }
    bool NativeException() const { return exception; }
    void Publish(std::uint32_t& generation,unsigned value) { generation=value; ++publishCalls; }
};
cfgspawn::SpawnRequest Request() { cfgspawn::SpawnRequest r; r.identity={0x10000,0x20000,1,2}; return r; }
void Configure(cfgspawn::OneCounselorSpawn& policy) {
    CHECK(policy.Configure(Request().identity,true,100,0x30000,0x40000,0x50000));
}
}
int main() {
    using namespace cfgspawn;
    CHECK(SpawnServiceCandidate(true,false,true,true));
    CHECK(!SpawnServiceCandidate(false,false,true,true));
    CHECK(!SpawnServiceCandidate(true,true,true,true));
    CHECK(!SpawnServiceCandidate(true,false,false,true));
    CHECK(!SpawnServiceCandidate(true,false,true,false));
    CHECK(ActorReceiveTickIdentityMatches(0x10000,0x10000,4));
    CHECK(!ActorReceiveTickIdentityMatches(0x10008,0x10000,4));
    CHECK(!ActorReceiveTickIdentityMatches(0x10000,0x10000,16));
    CHECK(!ActorReceiveTickIdentityMatches(0,0,4));
    CHECK(SpawnServiceWindow(true,false,true,true,true));
    CHECK(!SpawnServiceWindow(false,false,true,true,true));
    CHECK(!SpawnServiceWindow(true,true,true,true,true));
    CHECK(!SpawnServiceWindow(true,false,false,true,true));
    CHECK(!SpawnServiceWindow(true,false,true,false,true));
    CHECK(!SpawnServiceWindow(true,false,true,true,false));
    OneCounselorSpawn policy; Configure(policy); Access access;
    policy.Service(access); CHECK(!access.calls && !policy.Pending());
    auto request=Request(); request.reserved=1; CHECK(!policy.Submit(request)); request=Request();
    request.identity.pawnSerial=3; CHECK(!policy.Submit(request));
    CHECK(policy.Submit(Request()) && !policy.Submit(Request()));
    access.owner=false; policy.Service(access); CHECK(policy.Pending() && !access.calls);
    access.owner=true;access.reenter=&policy;policy.Service(access);
    CHECK(policy.report.state==static_cast<unsigned>(SpawnState::LinkedNativeCounselor) && policy.report.attempts==1 && access.calls==2);
    CHECK(policy.report.generation==2 && access.publishCalls==2 && ValidSpawnReport(policy.report));
    policy.Service(access); CHECK(access.calls==2 && !policy.Submit(Request()));
    for (unsigned failure=0;failure<11;++failure) {
        OneCounselorSpawn failed; Configure(failed); CHECK(failed.Submit(Request())); Access bad;
        if (failure==0) bad.context=false;
        if (failure==1) bad.resources=false;
        if (failure==2) bad.location=false;
        if (failure==3) bad.contextFailAt=2;
        if (failure==4) bad.resourceFailAt=2;
        if (failure==5) bad.result=0;
        if (failure==6) bad.verify=false;
        if (failure==7) { bad.spawn=false; bad.exception=true; }
        if (failure==8) bad.retain=false;
        if (failure==9) bad.loseAfterLocation=true;
        if (failure==10) bad.loseAfterSpawn=true;
        failed.Service(bad);
        CHECK(failed.report.state!=static_cast<unsigned>(SpawnState::LinkedNativeCounselor) && ValidSpawnReport(failed.report));
        const auto before=bad.calls; failed.Service(bad); CHECK(bad.calls==before && !failed.Submit(Request()));
        if (failure<2) CHECK(!bad.calls);
        if (failure==3 || failure==4) CHECK(bad.calls==1);
        if (failure==7) CHECK(failed.report.state==static_cast<unsigned>(SpawnState::NativeException));
        if (failure==8) CHECK(!bad.calls);
        if (failure==9) CHECK(bad.calls==1);
        if (failure==10) CHECK(bad.calls==2&&failed.report.state==static_cast<unsigned>(SpawnState::ResultUnverified));
    }
    OneCounselorSpawn refused;
    CHECK(!refused.Configure(Request().identity,false,100,0x30000,0x40000,0x50000) && !refused.Submit(Request()));
    OneCounselorSpawn shared;
    CHECK(!shared.Configure(Request().identity,true,100,0x20000,0x40000,0x50000)); // never use the human pawn as source
    auto malformed=policy.report; malformed.generation=1; CHECK(!ValidSpawnReport(malformed));
    std::printf("CFG one-counselor spawn policy %s (%d failures). Mock native calls only; no game access.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
