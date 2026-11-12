#pragma once
#include "EmoteSyncPolicy.hpp"

namespace cfgport::emotes {
// Portable completion-to-queue adapter. Does not resolve or retain game pointers,
// call the engine, load assets, scan bots, or turn diagnostic evidence into proof.
// A runtime provider must supply fresh owner-thread identity/catalogue proofs.
enum class NativeOutcome { Pending, Started, Cleared, Unchanged, ReadRefused,
    OriginalException, OtherTransition };
enum class NativeBinding { MulticastCancel, MulticastPlay, ServerCancel, ServerPlay };
struct NativeCompletion {
    std::uint64_t sequence = 0;
    NativeBinding binding = NativeBinding::ServerPlay;
    NativeOutcome outcome = NativeOutcome::Pending;
    bool hadMontage = false, hasMontage = false, montageChanged = false;
};
struct FreshSourceProof {
    PawnId pawn{};
    std::uint64_t catalogueGeneration = 0, nativeEmoteKey = 0;
    bool ownerThread = false, localHumanCounselor = false;
    bool nativeBindingsVerified = false, catalogueVerified = false;
    // The native selected class and resulting gender-specific montage must
    // belong to this catalogue key. Raw FName words are not catalogue keys.
    bool selectedClassMontageMatch = false, generatedByMirror = false;
};
class CompletionRouter {
public:
    bool OnScope(const Compatibility& profile, Session session,
        std::uint64_t world, std::uint64_t catalogue) noexcept {
        Reset();
        if (!queue_.OnScope(profile,session,world,catalogue)) return false;
        world_ = world; catalogue_ = catalogue;
        return true;
    }
    bool OnCompletion(const NativeCompletion& event, const FreshSourceProof& proof,
        const Recipient* recipients, std::size_t count, std::uint64_t now) noexcept {
        // Nested/mirror/foreign-thread notifications must not modify local work.
        if (!proof.ownerThread || !proof.localHumanCounselor || proof.generatedByMirror)
            return false;
        if (!world_ || !proof.pawn.Valid() || proof.pawn.worldGeneration != world_ ||
            proof.catalogueGeneration != catalogue_ || !proof.nativeBindingsVerified ||
            !proof.catalogueVerified || (source_.Valid() && !(source_ == proof.pawn))) {
            Reset(); return false;
        }
        if (!event.sequence || event.sequence <= lastSequence_) return false;
        if (static_cast<unsigned>(event.binding) > 3 ||
            static_cast<unsigned>(event.outcome) > 6) { Reset(); return false; }
        // Pending is not a completed notification; its later completion with
        // the same sequence remains eligible. Failed reads/exceptions invalidate.
        if (event.outcome == NativeOutcome::Pending) return false;
        if (event.outcome == NativeOutcome::ReadRefused ||
            event.outcome == NativeOutcome::OriginalException ||
            event.outcome == NativeOutcome::OtherTransition) {
            Reset(); return false;
        }
        const bool startedShape = event.hasMontage && event.montageChanged &&
            (event.binding == NativeBinding::ServerPlay ||
                event.binding == NativeBinding::MulticastPlay);
        const bool clearedShape = event.hadMontage && !event.hasMontage && event.montageChanged;
        const bool unchangedShape = !event.montageChanged && event.hadMontage == event.hasMontage;
        if ((event.outcome == NativeOutcome::Started && !startedShape) ||
            (event.outcome == NativeOutcome::Cleared && !clearedShape) ||
            (event.outcome == NativeOutcome::Unchanged && !unchangedShape)) {
            Reset(); return false;
        }
        source_ = proof.pawn; lastSequence_ = event.sequence;
        if (event.outcome == NativeOutcome::Cleared ||
            (event.outcome == NativeOutcome::Unchanged && !event.hasMontage)) {
            // A failed replacement play can clear the old montage too. Any
            // corroborated absence of the source animation cancels stale work.
            queue_.OnNativeCancel(source_,activeStart_);
            activeStart_ = 0;
            return false; // Cancellation never queues another emote.
        }
        if (event.binding != NativeBinding::ServerPlay ||
            event.outcome != NativeOutcome::Started || !event.hasMontage ||
            !event.montageChanged) return false;
        if (!proof.selectedClassMontageMatch || !proof.nativeEmoteKey) {
            Reset(); return false;
        }
        if (count > SyncQueue::Capacity || (count && !recipients)) {
            Reset(); return false; // Never retain older work after a bad snapshot.
        }
        SourceEvent source{proof.pawn,catalogue_,event.sequence,proof.nativeEmoteKey,
            true,true,false};
        const bool queued = queue_.OnNativeStart(source,recipients,count,now);
        activeStart_ = event.sequence;
        return queued;
    }
    Command TakeDue(const Compatibility& profile, Session session,
        std::uint64_t world, std::uint64_t catalogue, std::uint64_t now) noexcept {
        return queue_.TakeDue(profile,session,world,catalogue,now);
    }
    bool Pending() const noexcept { return queue_.Pending(); }
    std::uint64_t NextWakeMs() const noexcept { return queue_.NextWakeMs(); }
    void Reset() noexcept {
        queue_.Reset(); source_ = {}; world_ = catalogue_ = lastSequence_ = activeStart_ = 0;
    }
private:
    SyncQueue queue_{};
    PawnId source_{};
    std::uint64_t world_ = 0, catalogue_ = 0, lastSequence_ = 0, activeStart_ = 0;
};
} // namespace cfgport::emotes
