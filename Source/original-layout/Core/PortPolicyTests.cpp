#include "CfgPortPolicy.hpp"
#include "ModeSettingsPolicy.hpp"
#include "ObjectiveTrapPolicy.hpp"
#include "CounselorStatsPolicy.hpp"
#include "EmoteSyncPolicy.hpp"
#include "../SourceBaseline/gameplay-backend/port/src/Game/Setup/GamepadReconnectPolicy.hpp"
#include "../SourceBaseline/gameplay-backend/port/src/Game/Setup/JasonDoorApproachPolicy.hpp"
#include "../SourceBaseline/gameplay-backend/port/src/Game/Setup/JasonChaseRetryPolicy.hpp"
#include "../SourceBaseline/challenge-addon/ChallengeBoundaryGeometry.hpp"
#include <cstdio>
#include <limits>

static int failures = 0;
#define CHECK(condition) do { if (!(condition)) { std::printf("FAIL line %d: %s\n", __LINE__, #condition); ++failures; } } while (false)
int main() {
    using namespace cfgport;
    Compatibility actualCfg{};
    actualCfg.recognizedForReporting = true;
    for (std::size_t i = 0; i < actualCfg.capabilities.size(); ++i)
        CHECK(!CanEnable(actualCfg, Session::OfflineBots, static_cast<Feature>(i)));
    Compatibility synthetic{}; // Test fixture only; not a shipped runtime profile.
    synthetic.recognizedForReporting = true;
    synthetic.permittedOfflineInterface = true;
    synthetic.targetBindingsVerified = true;
    synthetic.capabilities.fill(true);
    CHECK(CanEnable(synthetic, Session::OfflineBots, Feature::AiJason));
    CHECK(!CanEnable(synthetic, Session::OfflineBots, Feature::ChallengeBoundary));
    CHECK(CanEnable(synthetic, Session::Challenge, Feature::ChallengeBoundary));
    CHECK(!CanEnable(synthetic, Session::Online, Feature::AiJason));
    CHECK(!CanEnable(synthetic, Session::Unknown, Feature::AiJason));
    CHECK(!CanEnable(synthetic, Session::Challenge, Feature::Count));
    synthetic.permittedOfflineInterface = false;
    CHECK(!CanEnable(synthetic, Session::OfflineBots, Feature::AiJason));
    synthetic.permittedOfflineInterface = true;
    synthetic.recognizedForReporting = false;
    CHECK(!CanEnable(synthetic, Session::OfflineBots, Feature::AiJason));
    synthetic.recognizedForReporting = true;
    synthetic.capabilities[static_cast<std::size_t>(Feature::AiJason)] = false;
    CHECK(!CanEnable(synthetic, Session::OfflineBots, Feature::AiJason));
    synthetic.capabilities[static_cast<std::size_t>(Feature::AiJason)] = true;
    synthetic.targetBindingsVerified = false;
    CHECK(!CanEnable(synthetic, Session::Challenge, Feature::ChallengeKnives));
    // Synthetic names verify unlimited catalogue membership, not actual CFG assets.
    static constexpr std::string_view counselors[]{"counselor-a", "counselor-b"};
    static constexpr std::string_view jasons[]{"native-jason-a", "native-jason-b"};
    static constexpr std::string_view maps[]{"native-map-a", "native-extra-map"};
    static constexpr std::string_view weather[]{"native-rain", "native-snow"};
    Catalogue catalogue{1, {counselors,2}, {jasons,2}, {maps,2}, {weather,2}, 10};
    Settings settings;
    settings.mode = Mode::OfflineCounselor; settings.counselor = counselors[0];
    settings.jason = jasons[1]; settings.map = maps[1]; settings.weather = weather[1];
    settings.totalCounselors = 10; settings.weapon = Weapon::Shotgun;
    settings.inventory = Inventory::FirstAidSpray;
    CHECK(Validate(settings, catalogue));
    settings.totalCounselors = 11; CHECK(!Validate(settings, catalogue));
    settings.totalCounselors = 10;
    // Playable roster size is not concurrent pawn capacity. Synthetic evidence
    // permits a larger count without importing a fixed stock cap; not a claim
    // that the CFG runtime has passed a 16-counselor test.
    catalogue.maxCounselors = 16;
    settings.totalCounselors = 16; CHECK(Validate(settings,catalogue));
    settings.totalCounselors = 17; CHECK(!Validate(settings,catalogue));
    catalogue.maxCounselors = 0; CHECK(!Validate(settings,catalogue));
    catalogue.maxCounselors = 10; settings.totalCounselors = 10;
    settings.challenges[static_cast<std::size_t>(ChallengeOption::PreventEscapes)] = true;
    CHECK(!Validate(settings, catalogue));
    settings.challenges.fill(false);
    settings.minutes = 0; CHECK(!Validate(settings, catalogue)); settings.minutes = 90;
    settings.map = "unregistered-map"; CHECK(!Validate(settings, catalogue)); settings.map = maps[1];
    MatchSettings lifecycle;
    lifecycle.OnMenuOpened(10);
    CHECK(!lifecycle.Commit(actualCfg, catalogue, settings));
    synthetic.targetBindingsVerified = true;
    synthetic.capabilities[static_cast<std::size_t>(Feature::Loadout)] = false;
    CHECK(!lifecycle.Commit(synthetic, catalogue, settings));
    synthetic.capabilities[static_cast<std::size_t>(Feature::Loadout)] = true;
    CHECK(lifecycle.Commit(synthetic, catalogue, settings));
    CHECK(!lifecycle.OnMatchStarted(10,1));
    CHECK(!lifecycle.OnMatchStarted(20,2));
    CHECK(lifecycle.OnMatchStarted(20,1));
    CHECK(!lifecycle.ChallengeOptionEnabled(20,ChallengeOption::PreventEscapes));
    lifecycle.OnMenuOpened(30); CHECK(!lifecycle.IsCurrentMatch(20));
    settings.mode = Mode::OfflineJason; settings.weapon = Weapon::None;
    settings.inventory = Inventory::None; settings.minutes = 0;
    settings.unlimitedKnives = settings.fastRecharge = settings.fastMovement = true;
    CHECK(Validate(settings, catalogue));
    settings.mode = Mode::Online; CHECK(!Validate(settings, catalogue));
    settings.mode = Mode::Challenge;
    settings.unlimitedKnives = settings.fastRecharge = settings.fastMovement = false;
    settings.challenges[static_cast<std::size_t>(ChallengeOption::PreventEscapes)] = true;
    synthetic.capabilities[static_cast<std::size_t>(Feature::ChallengeEscapes)] = false;
    CHECK(!lifecycle.Commit(synthetic, catalogue, settings));
    synthetic.capabilities[static_cast<std::size_t>(Feature::ChallengeEscapes)] = true;
    CHECK(lifecycle.Commit(synthetic, catalogue, settings));
    CHECK(lifecycle.OnMatchStarted(40,1));
    CHECK(lifecycle.ChallengeOptionEnabled(40,ChallengeOption::PreventEscapes));
    CHECK(!lifecycle.ChallengeOptionEnabled(20,ChallengeOption::PreventEscapes));
    lifecycle.Reset(); CHECK(!lifecycle.ChallengeOptionEnabled(40,ChallengeOption::PreventEscapes));
    f13::input::ReconnectGate reconnect;
    CHECK(reconnect.Due(0)); reconnect.Missing(50);
    CHECK(!reconnect.Due(1049)); CHECK(reconnect.Due(1050));
    reconnect.Found(); CHECK(reconnect.Due(0));
    CHECK(JasonDoorApproachPolicy::IsOnApproach(100,0,0,1000,0,0));
    CHECK(!JasonDoorApproachPolicy::IsOnApproach(-100,0,0,1000,0,0));
    CHECK(!JasonDoorApproachPolicy::IsOnApproach(100,150,0,1000,0,0));
    CHECK(!JasonDoorApproachPolicy::IsOnApproach(100,0,0,
        std::numeric_limits<float>::quiet_NaN(),0,0));
    CHECK(JasonChaseRetryPolicy::IsSuspiciousAlreadyAtGoal(1,400.0f*400.0f,true));
    CHECK(!JasonChaseRetryPolicy::IsSuspiciousAlreadyAtGoal(1,100.0f*100.0f,true));
    CHECK(!JasonChaseRetryPolicy::IsSuspiciousAlreadyAtGoal(1,400.0f*400.0f,false));
    CHECK(JasonChaseRetryPolicy::AdvanceRepeatedResult(1,true) == 2);
    CHECK(JasonChaseRetryPolicy::AdvanceRepeatedResult(1,false) == 1);
    using f13::challenges::boundary::SegmentTouchesSphere;
    CHECK(SegmentTouchesSphere({-10,0,0},{10,0,0},{0,0,0},1));
    CHECK(!SegmentTouchesSphere({-10,3,0},{10,3,0},{0,0,0},1));
    CHECK(!SegmentTouchesSphere({-10,0,0},{10,0,0},{0,0,0},-1));
    using namespace cfgport::traps;
    // Synthetic placement anchors, NOT Crystal Lake Small coordinates.
    Anchor anchors[]{
        {Target::PhoneBoxFront,1,1,{0,0,0},true},
        {Target::CarDoorSide,2,1,{1,0,0},true},
        {Target::CarDoorSide,2,2,{-1,0,0},true},
        {Target::CarHoodFront,2,3,{0,2,0},true},
        {Target::CarDoorSide,3,1,{11,0,0},true},
        {Target::CarDoorSide,3,2,{9,0,0},true},
        {Target::CarHoodFront,3,3,{10,2,0},true}
    };
    Anchor planned[7]{}; std::size_t plannedCount = 0;
    CHECK(BuildExactPlan(anchors,7,7,planned,7,plannedCount));
    CHECK(plannedCount == 7 && planned[0].target == Target::PhoneBoxFront);
    const MapScope ground{100,200,MapFamily::Ground},grendel{100,200,MapFamily::Grendel};
    CHECK(BuildScopedPlan(ground,100,200,anchors,7,7,planned,7,plannedCount));
    CHECK(!BuildScopedPlan(grendel,100,200,anchors,7,7,planned,7,plannedCount)&&plannedCount==0);
    CHECK(!BuildScopedPlan(ground,101,200,anchors,7,7,planned,7,plannedCount));
    CHECK(!BuildScopedPlan(ground,100,201,anchors,7,7,planned,7,plannedCount));
    CHECK(!BuildScopedPlan({},100,200,anchors,7,7,planned,7,plannedCount));
    CHECK(!BuildExactPlan(anchors,7,5,planned,7,plannedCount)); CHECK(plannedCount == 0);
    anchors[6].nativePlacementVerified = false;
    CHECK(!BuildExactPlan(anchors,7,7,planned,7,plannedCount));
    anchors[6].nativePlacementVerified = true; anchors[6] = anchors[5];
    CHECK(!BuildExactPlan(anchors,7,7,planned,7,plannedCount));
    {
        using namespace cfgport::stats;
        Transaction stats;
        const Identity pawn{1,2,3};
        const Values native{1,2,3,4,5,6,7};
        Evidence unverified;
        CHECK(stats.PrepareEnable(unverified,pawn,native).action == Action::Reject);
        Evidence proof{true,true,true,Session::OfflineBots}; // Synthetic proof only.
        CHECK(stats.PrepareEnable(proof,{},native).action == Action::Reject);
        CHECK(stats.PrepareEnable(proof,pawn,{0,2,3,4,5,6,7}).action == Action::Reject);
        CHECK(stats.PrepareEnable(proof,pawn,{11,2,3,4,5,6,7}).action == Action::Reject);
        proof.session = Session::Online;
        CHECK(stats.PrepareEnable(proof,pawn,native).action == Action::Reject);
        proof.session = Session::OfflineBots;
        auto apply = stats.PrepareEnable(proof,pawn,native);
        CHECK(apply.action == Action::Apply && apply.expected == native);
        CHECK(apply.desired == Maximum() && !stats.Active());
        CHECK(!stats.Confirm(apply.revision,pawn,native)); // Failed write not captured.
        CHECK(stats.Confirm(apply.revision,pawn,Maximum()) && stats.Active());
        CHECK(!stats.Confirm(apply.revision,pawn,Maximum())); // Duplicate ack.
        CHECK(stats.PrepareEnable(proof,pawn,Maximum()).action == Action::None);
        auto restore = stats.PrepareDisable(proof,pawn,Maximum());
        CHECK(restore.action == Action::Restore && restore.desired == native);
        CHECK(!stats.Confirm(restore.revision,pawn,Maximum()) && stats.Active());
        CHECK(stats.Confirm(restore.revision,pawn,native) && !stats.Active());
        CHECK(stats.PrepareEnable(proof,pawn,Maximum()).action == Action::None);
        CHECK(!stats.Active()); // Native maximum isn't owned by this override.
        apply = stats.PrepareEnable(proof,pawn,native);
        CHECK(stats.PrepareEnable(unverified,pawn,native).action == Action::Reject);
        CHECK(!stats.Confirm(apply.revision,pawn,Maximum()));
        apply = stats.PrepareEnable(proof,pawn,native);
        CHECK(stats.PrepareEnable(proof,pawn,Maximum()).action == Action::None);
        CHECK(!stats.Confirm(apply.revision,pawn,Maximum()));
        apply = stats.PrepareEnable(proof,pawn,native);
        stats.Invalidate();
        CHECK(!stats.Confirm(apply.revision,pawn,Maximum()));
        auto newer = stats.PrepareEnable(proof,pawn,native);
        CHECK(newer.revision != apply.revision);
        CHECK(stats.Confirm(newer.revision,pawn,Maximum()));
        CHECK(stats.PrepareDisable(proof,{1,2,4},Maximum()).action == Action::Reject);
        CHECK(!stats.Active()); // Same reused address, different object serial.
        newer = stats.PrepareEnable(proof,pawn,native);
        CHECK(stats.Confirm(newer.revision,pawn,Maximum()));
        CHECK(stats.PrepareDisable(proof,pawn,{9,10,10,10,10,10,10}).action == Action::Conflict);
        CHECK(!stats.Active()); // Do not overwrite external/native stat changes.
    }
    {
        using namespace cfgport::emotes;
        SyncQueue queue;
        Compatibility verified = synthetic; // Synthetic proof, never live CFG.
        verified.capabilities.fill(true);
        CHECK(!queue.OnScope(actualCfg,Session::OfflineBots,1,2));
        CHECK(!queue.OnScope(verified,Session::Online,1,2));
        CHECK(!queue.OnScope(verified,Session::OfflineBots,0,2));
        CHECK(queue.OnScope(verified,Session::OfflineBots,1,2));
        const PawnId source{1,1,1};
        SourceEvent event{source,2,1,42,true,true,false};
        Recipient bots[]{
            {{1,2,1},true,true,true,true},
            {{1,3,1},true,true,true,true},
            {{1,2,1},true,true,true,true}, // Duplicate ignored.
            {{1,4,1},true,false,true,true}, // Human excluded.
            {{1,5,1},true,true,false,true}, // Dead excluded.
            {{1,6,1},false,true,true,true}, // Jason excluded.
            {{1,7,1},true,true,true,false}, // Native refusal excluded.
            {{2,8,1},true,true,true,true}, // Other world excluded.
            {source,true,true,true,true} // Source never mirrored to itself.
        };
        event.nativeStarted = false; CHECK(!queue.OnNativeStart(event,bots,9,100));
        event.nativeStarted = true; event.generatedByMirror = true;
        CHECK(!queue.OnNativeStart(event,bots,9,100)); event.generatedByMirror = false;
        CHECK(!queue.OnNativeStart(event,nullptr,9,100));
        CHECK(!queue.OnNativeStart(event,bots,SyncQueue::Capacity+1,100));
        CHECK(queue.OnNativeStart(event,bots,9,100));
        CHECK(!queue.OnNativeStart(event,bots,9,100)); // Dedup same native event.
        auto command = queue.TakeDue(verified,Session::OfflineBots,1,2,100);
        CHECK(command.Valid() && command.target == bots[0].pawn);
        CHECK(CanDispatch(command,bots[0],1,2));
        auto fresh = bots[0]; fresh.bot = false; CHECK(!CanDispatch(command,fresh,1,2));
        fresh = bots[0]; fresh.pawn.objectSerial = 2;
        CHECK(!CanDispatch(command,fresh,1,2));
        CHECK(!CanDispatch(command,bots[0],2,2));
        CHECK(!CanDispatch(command,bots[0],1,3));
        CHECK(queue.Pending() && queue.NextWakeMs() == 125);
        CHECK(!queue.TakeDue(verified,Session::OfflineBots,1,2,124).Valid());
        command = queue.TakeDue(verified,Session::OfflineBots,1,2,125);
        CHECK(command.Valid() && command.target == bots[1].pawn);
        CHECK(!queue.Pending() && queue.NextWakeMs() == 0);
        CHECK(!queue.TakeDue(verified,Session::OfflineBots,1,2,200).Valid());
        event.eventSerial = 2; CHECK(queue.OnNativeStart(event,bots,9,200));
        CHECK(queue.TakeDue(verified,Session::OfflineBots,1,2,1000).Valid());
        CHECK(!queue.TakeDue(verified,Session::OfflineBots,1,2,1000).Valid());
        CHECK(queue.NextWakeMs() == 1025); // Late wake does not catch up in burst.
        queue.OnNativeCancel(source,1); CHECK(queue.Pending()); // Old cancel ignored.
        queue.OnNativeCancel(source,2); CHECK(!queue.Pending());
        event.eventSerial = 3; CHECK(queue.OnNativeStart(event,bots,9,2000));
        CHECK(!queue.TakeDue(verified,Session::OfflineBots,1,2,1999).Valid());
        CHECK(!queue.Pending()); // Backward clock discards work.
        event.eventSerial = 4; CHECK(queue.OnNativeStart(event,bots,9,3000));
        CHECK(!queue.TakeDue(verified,Session::OfflineBots,1,2,6001).Valid());
        CHECK(!queue.Pending()); // Stale commands never replay.
        event.eventSerial = 5; CHECK(queue.OnNativeStart(event,bots,9,7000));
        event.eventSerial = 6; event.nativeEmoteKey = 43;
        CHECK(queue.OnNativeStart(event,bots,9,7010));
        command = queue.TakeDue(verified,Session::OfflineBots,1,2,7010);
        CHECK(command.eventSerial == 6 && command.nativeEmoteKey == 43);
        CHECK(!queue.TakeDue(verified,Session::OfflineBots,2,2,7040).Valid());
        CHECK(!queue.Pending()); // World transition clears everything.
        CHECK(queue.OnScope(verified,Session::OfflineBots,1,2));
        CHECK(queue.OnNativeStart(event,bots,9,8000));
        verified.capabilities[static_cast<std::size_t>(Feature::EmoteMirror)] = false;
        CHECK(!queue.TakeDue(verified,Session::OfflineBots,1,2,8000).Valid());
        CHECK(!queue.Pending());
        verified.capabilities.fill(true);
        CHECK(queue.OnScope(verified,Session::OfflineBots,1,2));
        bots[2].pawn.objectSerial = 2; // Ambiguous reused address in one snapshot.
        CHECK(!queue.OnNativeStart(event,bots,9,9000)); CHECK(!queue.Pending());
        bots[2].pawn.objectSerial = 1; event.eventSerial = 7;
        const auto nearMax = std::numeric_limits<std::uint64_t>::max()-10;
        CHECK(queue.OnNativeStart(event,bots,9,nearMax));
        CHECK(queue.TakeDue(verified,Session::OfflineBots,1,2,nearMax).Valid());
        CHECK(!queue.Pending()); // Timer arithmetic cannot wrap.
        CHECK(queue.OnScope(verified,Session::OfflineBots,1,2));
        Recipient fullBatch[SyncQueue::Capacity]{};
        for (std::size_t i=0; i<SyncQueue::Capacity; ++i)
            fullBatch[i] = {{1,100+i,1},true,true,true,true};
        event.eventSerial = 8;
        CHECK(queue.OnNativeStart(event,fullBatch,SyncQueue::Capacity,0));
        for (std::size_t i=0; i<SyncQueue::Capacity; ++i) {
            const auto dispatchAt = static_cast<std::uint64_t>(i)*SyncQueue::SpacingMs;
            command = queue.TakeDue(verified,Session::OfflineBots,1,2,dispatchAt);
            CHECK(command.Valid() && command.target == fullBatch[i].pawn);
            CHECK(!queue.TakeDue(verified,Session::OfflineBots,1,2,dispatchAt).Valid());
        }
        CHECK(!queue.Pending());
        event.eventSerial = 9;
        CHECK(!queue.OnNativeStart(event,nullptr,0,2000));
        CHECK(!queue.OnNativeStart(event,fullBatch,SyncQueue::Capacity,2000));
        CHECK(!queue.Pending()); // Empty accepted event isn't retried repeatedly.
    }
    std::printf("CFG portable policy tests: %s (%d failures)\n", failures ? "FAIL" : "PASS", failures);
    return failures ? 1 : 0;
}
