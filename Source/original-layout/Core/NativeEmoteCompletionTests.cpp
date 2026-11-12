#include "NativeEmoteCompletionPolicy.hpp"
#include <cstdio>

static int failures = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n",__LINE__,#c); ++failures; } } while(false)
int main() {
    using namespace cfgport;
    using namespace cfgport::emotes;
    Compatibility synthetic{};
    synthetic.recognizedForReporting = synthetic.permittedOfflineInterface =
        synthetic.targetBindingsVerified = true;
    synthetic.capabilities.fill(true); // Owned fixture, NOT a live CFG profile.
    CompletionRouter router;
    FreshSourceProof proof{{1,1,1},2,42,true,true,true,true,true,false};
    Recipient bots[]{ {{1,2,1},true,true,true,true}, {{1,3,1},true,true,true,true},
        {{1,4,1},true,false,true,true}, {{1,5,1},true,true,false,true} };
    NativeCompletion start{1,NativeBinding::ServerPlay,NativeOutcome::Started,false,true,true};
    NativeCompletion cancel{2,NativeBinding::ServerCancel,NativeOutcome::Cleared,true,false,true};
    CHECK(!router.OnScope(synthetic,Session::Sandbox,1,2));
    CHECK(!router.OnScope(synthetic,Session::Online,1,2));
    for (const auto mode : {Session::OfflineBots,Session::Challenge}) {
        CHECK(router.OnScope(synthetic,mode,1,2));
        auto pending = start; pending.outcome = NativeOutcome::Pending;
        CHECK(!router.OnCompletion(pending,proof,bots,4,100));
        CHECK(router.OnCompletion(start,proof,bots,4,100));
        CHECK(!router.OnCompletion(start,proof,bots,4,100));
        auto command = router.TakeDue(synthetic,mode,1,2,100);
        CHECK(command.Valid() && command.target == bots[0].pawn);
        CHECK(router.NextWakeMs() == 125);
        CHECK(!router.TakeDue(synthetic,mode,1,2,124).Valid());
        CHECK(!router.OnCompletion(cancel,proof,nullptr,0,124));
        CHECK(!router.Pending()); // Cancel sequence2 cancels START1, not START2.
        CHECK(!router.TakeDue(synthetic,mode,1,2,125).Valid());
    }
    CHECK(router.OnScope(synthetic,Session::OfflineBots,1,2));
    CHECK(router.OnCompletion(start,proof,bots,4,100));
    auto ignored = proof; ignored.generatedByMirror = true;
    CHECK(!router.OnCompletion(cancel,ignored,nullptr,0,101) && router.Pending());
    ignored = proof; ignored.ownerThread = false;
    CHECK(!router.OnCompletion(cancel,ignored,nullptr,0,101) && router.Pending());
    ignored = proof; ignored.localHumanCounselor = false;
    CHECK(!router.OnCompletion(cancel,ignored,nullptr,0,101) && router.Pending());
    CHECK(router.TakeDue(synthetic,Session::OfflineBots,1,2,1000).Valid());
    CHECK(!router.TakeDue(synthetic,Session::OfflineBots,1,2,1000).Valid());
    CHECK(router.NextWakeMs() == 1025); // Late wake never bursts.
    CHECK(router.TakeDue(synthetic,Session::OfflineBots,1,2,1025).Valid());
    CHECK(!router.Pending()); // Human/dead counselors never enter queue.
    for (unsigned flaw=0; flaw<6; ++flaw) {
        CHECK(router.OnScope(synthetic,Session::OfflineBots,1,2));
        auto bad = proof;
        switch (flaw) {
        case 0: bad.pawn.objectSerial=0; break; // Live observed zero is NOT proof.
        case 1: bad.nativeBindingsVerified=false; break;
        case 2: bad.catalogueVerified=false; break;
        case 3: bad.selectedClassMontageMatch=false; break;
        case 4: bad.nativeEmoteKey=0; break;
        case 5: bad.catalogueGeneration=3; break;
        }
        CHECK(!router.OnCompletion(start,bad,bots,4,100));
        CHECK(!router.Pending());
    }
    for (const auto outcome : {NativeOutcome::ReadRefused,NativeOutcome::OriginalException,
        NativeOutcome::OtherTransition}) {
        CHECK(router.OnScope(synthetic,Session::OfflineBots,1,2));
        CHECK(router.OnCompletion(start,proof,bots,4,100));
        auto bad = cancel; bad.outcome=outcome;
        CHECK(!router.OnCompletion(bad,proof,nullptr,0,101) && !router.Pending());
    }
    for (const auto binding : {NativeBinding::ServerCancel,NativeBinding::MulticastPlay,
        NativeBinding::MulticastCancel}) {
        CHECK(router.OnScope(synthetic,Session::OfflineBots,1,2));
        auto bad = start; bad.binding=binding;
        CHECK(!router.OnCompletion(bad,proof,bots,4,100) && !router.Pending());
    }
    CHECK(router.OnScope(synthetic,Session::OfflineBots,1,2));
    auto unchanged = start; unchanged.outcome=NativeOutcome::Unchanged;
    unchanged.hasMontage=unchanged.montageChanged=false;
    CHECK(!router.OnCompletion(unchanged,proof,bots,4,100));
    unchanged.sequence=2;
    CHECK(router.OnCompletion({3,NativeBinding::ServerPlay,NativeOutcome::Started,false,true,true},proof,bots,4,200));
    CHECK(!router.OnCompletion(cancel,proof,nullptr,0,201) && router.Pending()); // Old cancel ignored.
    auto newer= start; newer.sequence=4;
    auto key2=proof; key2.nativeEmoteKey=43;
    CHECK(router.OnCompletion(newer,key2,bots,4,210));
    CHECK(router.TakeDue(synthetic,Session::OfflineBots,1,2,210).nativeEmoteKey==43);
    CHECK(!router.TakeDue(synthetic,Session::Online,1,2,235).Valid() && !router.Pending());
    CHECK(router.OnScope(synthetic,Session::OfflineBots,1,2));
    CHECK(router.OnCompletion(start,proof,bots,4,100));
    auto reused=proof; reused.pawn.objectSerial=2;
    CHECK(!router.OnCompletion(cancel,reused,nullptr,0,101) && !router.Pending());
    for (unsigned flaw=0; flaw<2; ++flaw) {
        CHECK(router.OnScope(synthetic,Session::OfflineBots,1,2));
        CHECK(router.OnCompletion(start,proof,bots,4,100));
        newer.sequence=2;
        CHECK(!router.OnCompletion(newer,proof,flaw?bots:nullptr,
            flaw?SyncQueue::Capacity+1:4,101));
        CHECK(!router.Pending());
    }
    for (const auto binding : {NativeBinding::ServerPlay,NativeBinding::MulticastPlay,
        NativeBinding::MulticastCancel}) {
        CHECK(router.OnScope(synthetic,Session::OfflineBots,1,2));
        CHECK(router.OnCompletion(start,proof,bots,4,100));
        auto clear=cancel; clear.binding=binding;
        CHECK(!router.OnCompletion(clear,proof,nullptr,0,101) && !router.Pending());
    }
    for (unsigned flaw=0; flaw<6; ++flaw) {
        CHECK(router.OnScope(synthetic,Session::OfflineBots,1,2));
        CHECK(router.OnCompletion(start,proof,bots,4,100));
        auto bad=cancel;
        switch (flaw) {
        case 0: bad.binding=static_cast<NativeBinding>(99); break;
        case 1: bad.outcome=static_cast<NativeOutcome>(99); break;
        case 2: bad.hadMontage=false; break;
        case 3: bad.hasMontage=true; break;
        case 4: bad.montageChanged=false; break;
        case 5: bad.outcome=NativeOutcome::Started; break;
        }
        CHECK(!router.OnCompletion(bad,proof,nullptr,0,101) && !router.Pending());
    }
    CHECK(router.OnScope(synthetic,Session::OfflineBots,1,2));
    CHECK(router.OnCompletion(start,proof,bots,4,100));
    NativeCompletion same{2,NativeBinding::ServerCancel,NativeOutcome::Unchanged,true,true,false};
    CHECK(!router.OnCompletion(same,proof,nullptr,0,101) && router.Pending());
    same.sequence=3; same.hadMontage=same.hasMontage=false;
    CHECK(!router.OnCompletion(same,proof,nullptr,0,102) && !router.Pending());
    std::printf("CFG completion-to-emote-sync policy tests %s (%d failures); no game access.\n",
        failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
