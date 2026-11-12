#pragma once
#include "CfgPortPolicy.hpp"
#include <array>
#include <cstdint>
#include <limits>

namespace cfgport::emotes {
struct PawnId {
    std::uint64_t worldGeneration = 0, objectId = 0, objectSerial = 0;
    bool Valid() const noexcept { return worldGeneration && objectId && objectSerial; }
    bool operator==(const PawnId& other) const noexcept {
        return worldGeneration == other.worldGeneration && objectId == other.objectId &&
            objectSerial == other.objectSerial;
    }
};
struct Recipient {
    PawnId pawn{};
    bool counselor = false, bot = false, alive = false, nativeCanPlay = false;
};
struct SourceEvent {
    PawnId pawn{};
    std::uint64_t catalogueGeneration = 0, eventSerial = 0, nativeEmoteKey = 0;
    // A confirmed native local-human start, NOT a wheel/button request. This
    // avoids treating a failed selection or sweater action as an emote.
    bool localHumanCounselor = false, nativeStarted = false, generatedByMirror = false;
};
struct Command {
    PawnId target{};
    std::uint64_t catalogueGeneration = 0, eventSerial = 0, nativeEmoteKey = 0;
    bool Valid() const noexcept {
        return target.Valid() && catalogueGeneration && eventSerial && nativeEmoteKey;
    }
};
inline bool CanDispatch(const Command& command, const Recipient& fresh,
    std::uint64_t world, std::uint64_t catalogue) noexcept {
    return command.Valid() && command.target.worldGeneration == world &&
        command.catalogueGeneration == catalogue && command.target == fresh.pawn &&
        fresh.counselor && fresh.bot && fresh.alive && fresh.nativeCanPlay;
}

// Portable event/timer policy ONLY. No game calls, allocations, process reads,
// asset loads or standing timer. Future adapter arms a one-shot wake only while
// Pending(), verifies fresh object serials and uses the native validated route.
class SyncQueue {
public:
    static constexpr std::size_t Capacity = 64; // Queue bound, NOT CFG bot capacity.
    static constexpr std::uint64_t SpacingMs = 25; // Hypothesis pending FPS tests.
    static constexpr std::uint64_t LifetimeMs = 3000;

    bool OnScope(const Compatibility& profile, Session session, std::uint64_t world,
        std::uint64_t catalogue) noexcept {
        Reset();
        if (!world || !catalogue || !CanEnable(profile,session,Feature::EmoteMirror))
            return false;
        world_ = world; catalogue_ = catalogue; session_ = session;
        return true;
    }
    bool OnNativeStart(const SourceEvent& event, const Recipient* recipients,
        std::size_t count, std::uint64_t now) noexcept {
        if (!world_ || !event.pawn.Valid() || event.pawn.worldGeneration != world_ ||
            event.catalogueGeneration != catalogue_ || !event.localHumanCounselor ||
            !event.nativeStarted || event.generatedByMirror || !event.nativeEmoteKey ||
            !event.eventSerial || count > Capacity || (count && !recipients)) return false;
        if (source_.Valid() && !(source_ == event.pawn)) return false;
        if (event.eventSerial <= lastEvent_) return false;
        source_ = event.pawn; lastEvent_ = event.eventSerial;
        // A newer native start supersedes pending work. Never retry a start if
        // no eligible bots exist; duplicate notifications cannot replay it.
        ClearPending();
        for (std::size_t i = 0; i < count; ++i) {
            const auto& item = recipients[i];
            if (!item.pawn.Valid() || item.pawn.worldGeneration != world_ ||
                item.pawn.objectId == source_.objectId || !item.counselor ||
                !item.bot || !item.alive || !item.nativeCanPlay) continue;
            bool duplicate = false;
            for (std::size_t j = 0; j < queued_; ++j) {
                if (targets_[j].objectId != item.pawn.objectId) continue;
                if (!(targets_[j] == item.pawn)) { ClearPending(); return false; }
                duplicate = true; break;
            }
            if (!duplicate) targets_[queued_++] = item.pawn;
        }
        emote_ = event.nativeEmoteKey; firstAt_ = lastAt_ = nextAt_ = now;
        return Pending();
    }
    Command TakeDue(const Compatibility& profile, Session session,
        std::uint64_t world, std::uint64_t catalogue, std::uint64_t now) noexcept {
        if (!CanEnable(profile,session,Feature::EmoteMirror) || session != session_ ||
            world != world_ || catalogue != catalogue_) { Reset(); return {}; }
        if (!Pending()) return {};
        if (now < lastAt_ || now - firstAt_ > LifetimeMs) { ClearPending(); return {}; }
        lastAt_ = now;
        if (now < nextAt_) return {};
        const Command result{targets_[cursor_++],catalogue_,lastEvent_,emote_};
        if (Pending()) {
            if (now > std::numeric_limits<std::uint64_t>::max() - SpacingMs)
                ClearPending();
            else nextAt_ = now + SpacingMs;
        }
        return result; // At most ONE command; late timers never drain a burst.
    }
    void OnNativeCancel(PawnId source, std::uint64_t eventSerial) noexcept {
        if (source == source_ && eventSerial == lastEvent_) ClearPending();
    }
    bool Pending() const noexcept { return cursor_ < queued_; }
    std::uint64_t NextWakeMs() const noexcept { return Pending() ? nextAt_ : 0; }
    void Reset() noexcept {
        ClearPending(); world_ = catalogue_ = lastEvent_ = 0;
        session_ = Session::Unknown; source_ = {};
    }
private:
    void ClearPending() noexcept {
        queued_ = cursor_ = 0; emote_ = firstAt_ = lastAt_ = nextAt_ = 0;
    }
    std::array<PawnId,Capacity> targets_{};
    std::size_t queued_ = 0, cursor_ = 0;
    PawnId source_{};
    Session session_ = Session::Unknown;
    std::uint64_t world_ = 0, catalogue_ = 0, lastEvent_ = 0, emote_ = 0;
    std::uint64_t firstAt_ = 0, lastAt_ = 0, nextAt_ = 0;
};
} // namespace cfgport::emotes
