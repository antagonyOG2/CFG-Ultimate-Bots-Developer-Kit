#include "../Core/CallbackObservationPolicy.hpp"
#include "PatchTransactionPolicy.hpp"
#include "PatchTransaction.hpp"
#include "ObserverProtocol.hpp"
#include "ReportObservation.hpp"
#include <cstdio>
#include <limits>
#include <atomic>
#include <thread>

static int failures=0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL %d: %s\n",__LINE__,#c); ++failures; } } while(false)
// Controlled operation failures test the SAME coordinator used by the real
// owned-child Windows adapter, not merely isolated reducer inputs.
enum class Failure { None, Preparation, PartialFreeze, Context, ThreadSet, Readback,
    ChangedPrefix, Identity, Deadline, Protect, WriteFailed, WritePartial, Restore, Flush, Resume };
struct SimulatedOperations {
    Failure failure=Failure::None;
    unsigned observations=0,held=0,resumeCalls=0,resumed=0,writes=0,restores=0,flushes=0;
    cfgobserver::patch::Bytes bytes=cfgobserver::patch::Bytes::Original;
    cfgobserver::patch::Bytes ObserveBytes() noexcept {
        ++observations;
        if (observations>1 && failure==Failure::Readback) return cfgobserver::patch::Bytes::Unknown;
        if (observations>1 && failure==Failure::ChangedPrefix) return cfgobserver::patch::Bytes::Other;
        return bytes;
    }
    bool ValidatePreparation() noexcept { return failure!=Failure::Preparation; }
    bool Freeze() noexcept { held=failure==Failure::PartialFreeze?1u:3u; return failure!=Failure::PartialFreeze; }
    bool SpanClear() noexcept { return failure!=Failure::Context; }
    bool ThreadSetStable() noexcept { return failure!=Failure::ThreadSet; }
    bool IdentityStable() noexcept { return failure!=Failure::Identity; }
    bool WithinDeadline() noexcept { return failure!=Failure::Deadline; }
    bool Protect() noexcept { return failure!=Failure::Protect; }
    bool Write() noexcept {
        ++writes;
        if (failure==Failure::WriteFailed) return false;
        bytes=failure==Failure::WritePartial?cfgobserver::patch::Bytes::Other:cfgobserver::patch::Bytes::Desired;
        return failure!=Failure::WritePartial;
    }
    bool Restore() noexcept { ++restores; return failure!=Failure::Restore; }
    bool Flush() noexcept { ++flushes; return failure!=Failure::Flush; }
    bool Resume() noexcept { ++resumeCalls; resumed+=held; held=0; return failure!=Failure::Resume; }
};
int main() {
    using namespace cfgport::callbacks;
    Capture capture;
    CHECK(!capture.Arm(0,3)); CHECK(!capture.Arm(1,0));
    CHECK(capture.Arm(1,3)); CHECK(!capture.Arm(1,3));
    CHECK(capture.Candidate(1,3,false));
    CHECK(!capture.Candidate(2,3,false)); CHECK(!capture.Candidate(1,4,false));
    CHECK(!capture.Candidate(1,3,true));
    CHECK(!capture.Record(7,8,4,0,true)); CHECK(capture.Count()==0);
    CHECK(capture.At(0).sequence==0); CHECK(capture.At(Capacity).sequence==0);
    CHECK(capture.Record(7,0,3,1000000,true)); // Pause before first event has no cost.
    CHECK(capture.At(0).parameters==0 && capture.At(0).elapsedMs==0);
    CHECK(capture.Record(9,10,3,1005000,true));
    CHECK(capture.At(1).sequence==2 && capture.At(1).elapsedMs==5000);
    CHECK(!capture.Record(9,10,3,1005001,true));
    CHECK(!capture.Armed() && capture.Reason()==Stop::Timeout && capture.Count()==2);
    CHECK(!capture.Arm(1,3)); CHECK(!capture.Record(9,10,3,1005001,true));
    Capture identity; CHECK(identity.Arm(1,3));
    CHECK(!identity.Record(1,2,3,0,false)); CHECK(identity.Reason()==Stop::IdentityChanged);
    CHECK(!identity.Armed() && identity.Count()==0);
    Capture clock; CHECK(clock.Arm(1,3)); CHECK(clock.Record(1,2,3,100,true));
    CHECK(!clock.Record(1,2,3,99,true)); CHECK(clock.Reason()==Stop::ClockChanged);
    Capture full; CHECK(full.Arm(1,3));
    for (std::size_t i=0;i<Capacity;++i) CHECK(full.Record(1,2,3,i*MinimumSpacingMs,true));
    CHECK(full.Count()==Capacity && full.Reason()==Stop::Budget && !full.Armed());
    CHECK(!full.Record(1,2,3,Capacity*MinimumSpacingMs,true)); CHECK(full.At(Capacity).sequence==0);
    Capture overflow; CHECK(overflow.Arm(1,3));
    const auto max=std::numeric_limits<std::uint64_t>::max();
    CHECK(overflow.Record(1,2,3,max-10,true)); CHECK(overflow.Record(1,2,3,max,true));
    CHECK(overflow.At(1).elapsedMs==10);
    CHECK(!overflow.Record(1,2,3,0,true)); CHECK(overflow.Reason()==Stop::ClockChanged);
    Capture concurrent; CHECK(concurrent.Arm(1,3));
    std::atomic<bool> ready{false}; std::atomic<unsigned> foreignAccepted{0};
    std::thread foreign([&] {
        ready.store(true,std::memory_order_release);
        for (unsigned i=0;i<100000;++i)
            if (concurrent.Candidate(1,4,false) || concurrent.Record(1,2,4,i,true)) ++foreignAccepted;
    });
    while (!ready.load(std::memory_order_acquire)) std::this_thread::yield();
    for (unsigned i=0;i<64;++i) CHECK(concurrent.Record(1,2,3,i*MinimumSpacingMs,true));
    foreign.join(); CHECK(foreignAccepted.load()==0 && concurrent.Count()==64);
    Capture paced; CHECK(paced.Arm(1,3)); CHECK(paced.Due(100)); CHECK(paced.Record(1,2,3,100,true));
    CHECK(!paced.Due(100) && !paced.Due(109) && !paced.Record(1,2,3,109,true));
    CHECK(paced.Count()==1 && paced.Armed()); CHECK(paced.Due(110) && paced.Record(1,2,3,110,true));
    using namespace cfgobserver::patch;
    Facts f; f.observed=Bytes::Original;
    CHECK(!MayWrite(f)); CHECK(Classify(f)==State::RefusedUnchanged);
    f.prepared=f.allSuspended=f.threadSetStable=f.spanClear=f.identityStable=f.withinDeadline=true;
    CHECK(MayWrite(f));
    for (unsigned i=0;i<6;++i) {
        auto refused=f;
        switch (i) {
        case 0: refused.prepared=false; break;
        case 1: refused.allSuspended=false; break;
        case 2: refused.threadSetStable=false; break;
        case 3: refused.spanClear=false; break;
        case 4: refused.identityStable=false; break;
        default: refused.withinDeadline=false; break;
        }
        CHECK(!MayWrite(refused)); CHECK(Classify(refused)==State::RefusedUnchanged);
    }
    f.protectionAttempted=true; CHECK(Classify(f)==State::Indeterminate);
    f.protectionChanged=true; CHECK(Classify(f)==State::Indeterminate);
    f.protectionRestored=true; CHECK(Classify(f)==State::RefusedUnchanged);
    f.writeAttempted=true; f.observed=Bytes::Other;
    CHECK(Classify(f)==State::Indeterminate); // Partial bytes, no silent rollback.
    f.observed=Bytes::Unknown; CHECK(Classify(f)==State::Indeterminate);
    f.observed=Bytes::Desired; CHECK(Classify(f)==State::Indeterminate);
    f.writeReportedComplete=true; CHECK(Classify(f)==State::Indeterminate);
    f.cacheFlushed=true; CHECK(Classify(f)==State::Installed);
    f.protectionRestored=false; CHECK(Classify(f)==State::Indeterminate);
    f.protectionRestored=true; f.resumeComplete=false; CHECK(Classify(f)==State::Indeterminate);
    f.resumeComplete=true; f.writeReportedComplete=false; f.observed=Bytes::Original;
    CHECK(Classify(f)==State::RefusedUnchanged);
    f.cacheFlushed=false; CHECK(Classify(f)==State::Indeterminate);
    for (unsigned n=0;n<=static_cast<unsigned>(Failure::Resume);++n) {
        SimulatedOperations ops; ops.failure=static_cast<Failure>(n);
        const auto actual=Run(ops); const auto state=Classify(actual);
        CHECK(ops.resumeCalls==1 && ops.held==0);
        CHECK(ops.resumed==(ops.failure==Failure::Preparation?0u:ops.failure==Failure::PartialFreeze?1u:3u));
        switch (ops.failure) {
        case Failure::None: CHECK(state==State::Installed); break;
        case Failure::Readback: case Failure::ChangedPrefix: case Failure::Protect:
        case Failure::WritePartial: case Failure::Restore: case Failure::Flush: case Failure::Resume:
            CHECK(state==State::Indeterminate); break;
        default: CHECK(state==State::RefusedUnchanged); break;
        }
        if (actual.writeAttempted) CHECK(ops.restores==1 && ops.flushes==1);
        else CHECK(ops.writes==0 && ops.restores==0 && ops.flushes==0);
        if (ops.failure==Failure::Readback || ops.failure==Failure::ChangedPrefix) CHECK(ops.writes==0);
    }
    cfgobserver::Report report; report.bytes=sizeof(report);
    report.status=static_cast<std::uint32_t>(cfgobserver::Status::Prepared);
    report.threadId=3; report.pawn=0x10000; report.target=0x20000;
    report.trampoline=0x30000; report.relay=0x30013; report.detour=0x40000;
    CHECK(cfgobserver::SamePublishedPrefix(report,report));
    report.status=static_cast<std::uint32_t>(cfgobserver::Status::Capturing);
    report.hookInstalled=1; report.publishedCount=1; report.events[0]={7,8,0,3,1};
    CHECK(cfgobserver::SamePublishedPrefix(report,report));
    auto lagged=report; lagged.stopReason=static_cast<std::uint32_t>(Stop::Budget);
    CHECK(!cfgobserver::SamePublishedPrefix(lagged,lagged));
    auto torn=report; torn.events[0].function=9;
    CHECK(!cfgobserver::SamePublishedPrefix(report,torn));
    torn=report; torn.publishedCount=2;
    CHECK(!cfgobserver::SamePublishedPrefix(report,torn));
    torn=report; torn.events[0].threadId=4;
    CHECK(!cfgobserver::SamePublishedPrefix(torn,torn));
    torn=report; torn.events[0].elapsedMs=1;
    CHECK(!cfgobserver::SamePublishedPrefix(torn,torn));
    report.status=static_cast<std::uint32_t>(cfgobserver::Status::Complete);
    report.stopReason=static_cast<std::uint32_t>(Stop::Budget); report.publishedCount=64;
    for (std::size_t i=0;i<64;++i) report.events[i]={7,8,i,3,static_cast<std::uint32_t>(i+1)};
    CHECK(cfgobserver::SamePublishedPrefix(report,report));
    report.status=static_cast<std::uint32_t>(cfgobserver::Status::Stopped);
    report.publishedCount=2; report.stopReason=static_cast<std::uint32_t>(Stop::IdentityChanged);
    CHECK(cfgobserver::SamePublishedPrefix(report,report));
    cfgobserver::Report refused; refused.bytes=sizeof(refused);
    refused.status=static_cast<std::uint32_t>(cfgobserver::Status::RefusedIdentity);
    refused.preparationStage=static_cast<std::uint32_t>(cfgobserver::PreparationStage::Thread);
    refused.preparationError=5; refused.preparationReads=3;
    CHECK(cfgobserver::SamePreparationFailure(refused,refused));
    CHECK(!cfgobserver::PublishedReport(refused) && !cfgobserver::SamePublishedPrefix(refused,refused));
    auto changed=refused; changed.preparationError=0;
    CHECK(!cfgobserver::SamePreparationFailure(refused,changed));
    changed=refused; changed.preparationStage=static_cast<std::uint32_t>(cfgobserver::PreparationStage::ContextBefore);
    changed.preparationContextFailure=14;
    CHECK(cfgobserver::SamePreparationFailure(changed,changed));
    auto differentContext=changed; differentContext.preparationContextFailure=15;
    CHECK(!cfgobserver::SamePreparationFailure(changed,differentContext));
    differentContext.preparationContextFailure=17;
    CHECK(!cfgobserver::PreparationFailure(differentContext));
    differentContext=refused; differentContext.preparationContextFailure=14;
    CHECK(!cfgobserver::PreparationFailure(differentContext));
    changed=refused; changed.preparationReads=257; CHECK(!cfgobserver::PreparationFailure(changed));
    changed=refused; changed.preparationStage=0xFFFFFFFF; CHECK(!cfgobserver::PreparationFailure(changed));
    changed=refused; changed.publishedCount=1; CHECK(!cfgobserver::PreparationFailure(changed));
    changed=refused; changed.trampoline=0x10000; CHECK(!cfgobserver::PreparationFailure(changed));
    changed=refused; changed.dllPinned=1; CHECK(!cfgobserver::PreparationFailure(changed));
    changed=refused; changed.status=static_cast<std::uint32_t>(cfgobserver::Status::Prepared);
    CHECK(!cfgobserver::PreparationFailure(changed) && !cfgobserver::PublishedReport(changed));
    changed=report; changed.preparationStage=static_cast<std::uint32_t>(cfgobserver::PreparationStage::Thread);
    CHECK(!cfgobserver::PublishedReport(changed));
    std::printf("CFG observation/transaction policy tests: %s (%d failures)\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
