#pragma once
#include "CfgPortPolicy.hpp"
#include <array>
#include <cstdint>

namespace cfgport::stats {
using Values = std::array<std::uint8_t, 7>;
struct Identity {
    std::uint64_t worldGeneration = 0;
    std::uint64_t objectId = 0;
    std::uint64_t objectSerial = 0;
    bool Valid() const noexcept { return worldGeneration && objectId && objectSerial; }
    bool operator==(const Identity& other) const noexcept {
        return worldGeneration == other.worldGeneration && objectId == other.objectId &&
            objectSerial == other.objectSerial;
    }
};
inline bool Valid(const Values& values) noexcept {
    for (const auto value : values) if (value < 1 || value > 10) return false;
    return true;
}
inline Values Maximum() noexcept { return {10,10,10,10,10,10,10}; }
struct Evidence {
    bool targetBindingsVerified = false;
    bool nativeStatSchemaVerified = false;
    bool localCounselor = false;
    Session session = Session::Unknown;
    bool Allows(const Identity& identity) const noexcept {
        return identity.Valid() && targetBindingsVerified && nativeStatSchemaVerified &&
            localCounselor && (session == Session::OfflineBots || session == Session::Challenge || session == Session::Sandbox);
    }
};
enum class Action { None, Apply, Restore, Reject, Conflict };
struct Plan {
    Action action = Action::None;
    std::uint64_t revision = 0;
    Identity identity{};
    Values expected{};
    Values desired{};
};

// Event-driven two-phase policy, not a Tick or native-memory implementation.
// Future adapter must compare identity + expected values before native write,
// then confirm a fresh read. A failed write never captures an active override.
class Transaction {
public:
    Plan PrepareEnable(const Evidence& evidence, Identity identity, const Values& current) noexcept {
        pending_ = {}; // Any new request supersedes an unacknowledged old request.
        if (!evidence.Allows(identity) || !Valid(current)) {
            Invalidate();
            return {Action::Reject};
        }
        if (active_ && !(identity == identity_)) Invalidate();
        if (active_) {
            if (current == Maximum()) return {};
            Invalidate(); // Native stats changed; don't overwrite a different owner's data.
            return {Action::Conflict};
        }
        if (current == Maximum()) return {}; // Already native maximum: no restore ownership.
        return Stage(Action::Apply, identity, current, Maximum());
    }
    Plan PrepareDisable(const Evidence& evidence, Identity identity, const Values& current) noexcept {
        if (!active_) { pending_ = {}; return {}; }
        if (!evidence.Allows(identity) || !(identity == identity_)) {
            Invalidate(); // Old objects/worlds must never be dereferenced to restore.
            return {Action::Reject};
        }
        if (current != Maximum()) { Invalidate(); return {Action::Conflict}; }
        return Stage(Action::Restore, identity, current, original_);
    }
    bool Confirm(std::uint64_t revision, Identity identity, const Values& observed) noexcept {
        if (revision == 0 || pending_.revision != revision ||
            !(identity == pending_.identity) || observed != pending_.desired) return false;
        if (pending_.action == Action::Apply) {
            original_ = pending_.expected;
            identity_ = identity;
            active_ = true;
            pending_ = {};
            return true;
        }
        if (pending_.action == Action::Restore) {
            Invalidate();
            return true;
        }
        return false;
    }
    void Invalidate() noexcept {
        active_ = false; identity_ = {}; original_ = {}; pending_ = {};
        // revision_ isn't reset: acknowledgements from a previous world stay stale.
    }
    bool Active() const noexcept { return active_; }
private:
    Plan Stage(Action action, Identity identity, Values expected, Values desired) noexcept {
        ++revision_;
        if (revision_ == 0) ++revision_;
        pending_ = {action, revision_, identity, expected, desired};
        return pending_;
    }
    bool active_ = false;
    Identity identity_{};
    Values original_{};
    Plan pending_{};
    std::uint64_t revision_ = 0;
};
} // namespace cfgport::stats
