#define CFG_SPECTATOR_MEMBER_FIXTURE_ONLY
#define CFG_H1_JASON
inline bool HostedRoutineActive(){return false;}
#include "SpectatorRuntimeMembersTests.cpp"
#include "../Core/SweaterLifecyclePolicy.hpp"
#include "../Core/SweaterOwnerRefreshPolicy.hpp"
#include "../Core/HostedMaskTriggerMemo.hpp"
#include "../Core/HostedSweaterClassProofMemo.hpp"
struct OwnerFixture:SpectatorFixture {
    cfgprobe::lifetime::SlotObservation bundleSweaterOwner{},bundleSweaterAbility{},bundleSweaterSource{},bundleSweaterRetiredAbility{};
    cfgsweater::Policy bundleSweaterPolicy;
    struct {struct {bool blocked=false;bool Exclusive(std::uint64_t)const{return blocked;}} policy;} bundleKill;
    unsigned bundleSweaterSourceIndex=2,bundleSweaterCursor=5,bundleSweaterSettingsCursor=7;
    bool bundleSweaterFreshPending=true,bundleSweaterFreshRequested=true,bundleSweaterFreshGrantAttempted=true;
    bool bundleSweaterClassAttempted=true,bundleSweaterClassPending=false,bundleLateMaskRetryUsed=false;
    bool hasMask=false;
    bool hosted=false;
    bool HasPamelaTrigger(bool& acquired){acquired=hasMask;return true;}
    std::uint64_t bundleSweaterFreshUse=100;
    struct {unsigned sweaterBound=1,sweaterStage=4,sweaterClassAttempts=8;} bundleReport;
    bool Context(){context=plan;return true;} // Context admission is supplied; native owner/slot reads below are real fixture reads.
    bool HostedRoutineActive()const{return hosted;}
    bool Known(unsigned i,std::uintptr_t& pawn){pawn=bundleHumanTarget.object;return i==0;}
    bool Alive(std::uintptr_t,bool& living){living=true;return true;}
#include "SweaterOwnerRuntimeMembers.hpp"
    OwnerFixture(){
        r_.reads=0;r_.cap=100000;
        CHECK(cfgspawn::ObjectSlot(r_,plan.objectArray,r_.bot,bundleHumanTarget));
        bundleSweaterOwner=plan.objects[3];bundleSweaterAbility=bundleSweaterOwner;bundleSweaterSource=bundleSweaterOwner;
        bundleSweaterPolicy.Refuse();
        r_.Put(plan.controller+cfgprobe::ControllerPawn,r_.bot);
        r_.Put(r_.bot+cfgescape::disk::Controller,plan.controller);
        r_.reads=0;r_.cap=1536;
    }
};
int main(){
    CHECK(!cfgsweaterowner::PollDue(false,false,false,false,false,1000,1000));
    CHECK(cfgsweaterowner::PollDue(false,true,false,false,false,1000,1000)); // late mask pickup wakes a Refused scan
    CHECK(!cfgsweaterowner::PollDue(false,true,false,true,false,1000,1000));
    CHECK(!cfgsweaterowner::PollDue(false,true,false,false,true,1000,1000));
    CHECK(!cfgsweaterowner::PollDue(false,true,false,false,false,1000,999)); // retry remains paced
    CHECK(cfgsweaterowner::PollDue(false,false,true,false,false,5000,1000)); // new owner bypasses old cadence
    CHECK(!cfgsweaterowner::PollDue(false,false,true,true,false,5000,1000));
    CHECK(!cfgsweaterowner::PollDue(false,false,true,false,true,5000,1000));
    CHECK(cfgsweaterowner::PollDue(true,false,false,true,true,1000,1000)); // preserve ordinary lifecycle scheduling
    OwnerFixture f;CHECK(f.RefreshSweaterOwner(200));
    CHECK(f.bundleSweaterOwner==f.bundleHumanTarget&&f.bundleSweaterPolicy.stage==cfgsweater::Stage::Dormant);
    CHECK(!f.bundleSweaterClassAttempted&&!f.bundleSweaterClassPending&&f.bundleReport.sweaterClassAttempts==0);
    CHECK(!f.bundleSweaterAbility.object&&!f.bundleSweaterSource.object&&!f.bundleSweaterFreshUse);
    CHECK(!f.bundleSweaterFreshPending&&!f.bundleSweaterFreshRequested&&!f.bundleSweaterFreshGrantAttempted);
    CHECK(!f.bundleSweaterSettingsCursor); // declaration discovery belongs to this wearer/epoch
    // Death/replacement creates a new owner lifecycle even while the old
    // hosted owner's recharge was owed. It must not grant Tommy mask power.
    OwnerFixture hostHandoff;hostHandoff.hosted=true;
    hostHandoff.bundleSweaterPolicy.stage=cfgsweater::Stage::Waiting;
    hostHandoff.bundleSweaterPolicy.lastUse=100;hostHandoff.bundleSweaterPolicy.rearmAt=11100;
    hostHandoff.bundleSweaterPolicy.nativeOwnershipEarned=true;
    CHECK(hostHandoff.RefreshSweaterOwner(500));
    CHECK(hostHandoff.bundleSweaterPolicy.stage==cfgsweater::Stage::Dormant&&
        !hostHandoff.bundleSweaterPolicy.lastUse&&!hostHandoff.bundleSweaterPolicy.rearmAt);
    CHECK(!hostHandoff.bundleSweaterAbility.object&&!hostHandoff.bundleSweaterSource.object);
    CHECK(hostHandoff.bundleSweaterPolicy.Decide(1000,false,false,false)==cfgsweater::Action::None);
    CHECK(hostHandoff.bundleSweaterPolicy.Decide(1000,true,false,false)==cfgsweater::Action::Discover);
    CHECK(f.RefreshSweaterOwner(300)&&f.r_.reads<=1536);
    f.hasMask=true;f.bundleSweaterPolicy.Refuse();f.bundleSweaterPolicy.searchExpired=true;
    CHECK(f.RetryLateMask(500)&&f.bundleSweaterPolicy.stage==cfgsweater::Stage::Dormant&&f.bundleLateMaskRetryUsed);
    f.bundleSweaterPolicy.Refuse();f.bundleSweaterPolicy.searchExpired=true;CHECK(!f.RetryLateMask(600));
    for(unsigned fault=0;fault<5;++fault){OwnerFixture denied;CHECK(denied.RefreshSweaterOwner(200));
        denied.hasMask=true;denied.bundleSweaterPolicy.Refuse();denied.bundleSweaterPolicy.searchExpired=true;
        if(fault==0)denied.hasMask=false;
        if(fault==1)denied.bundleSweaterFreshGrantAttempted=true;
        if(fault==2)denied.bundleSweaterPolicy.lastUse=100;
        if(fault==3)denied.bundleSweaterPolicy.publicationStarted=true;
        if(fault==4)denied.bundleKill.policy.blocked=true;
        CHECK(!denied.RetryLateMask(500));}
    for(unsigned fault=0;fault<4;++fault){OwnerFixture denied;const auto owner=denied.bundleSweaterOwner;
        if(fault==0)denied.bundleSweaterPolicy.publicationStarted=true;
        if(fault==1)denied.bundleKill.policy.blocked=true;
        if(fault==2)denied.r_.Put(denied.plan.controller+cfgprobe::ControllerPawn,denied.r_.pawn);
        if(fault==3)denied.Serial(denied.r_.bot,700);
        CHECK(!denied.RefreshSweaterOwner(200));CHECK(denied.bundleSweaterOwner==owner&&denied.bundleSweaterClassAttempted);
    }
    std::printf("Included sweater owner transition: %d failures; native allocation absent.\n",failures);return failures?1:0;
}
