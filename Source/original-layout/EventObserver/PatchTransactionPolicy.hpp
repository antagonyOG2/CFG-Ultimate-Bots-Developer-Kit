#pragma once
#include <cstdint>

namespace cfgobserver::patch {
// A state reducer, not an installer. Every fact must come from checked operations.
// A partial/failed write is never assumed to mean that no bytes changed.
enum class Bytes : std::uint32_t { Unknown, Original, Desired, Other };
enum class State : std::uint32_t { RefusedUnchanged, Installed, Indeterminate };
struct Facts {
    bool prepared=false, allSuspended=false, threadSetStable=false;
    bool spanClear=false, identityStable=false, withinDeadline=false;
    bool protectionAttempted=false, protectionChanged=false, protectionRestored=false;
    bool writeAttempted=false, writeReportedComplete=false, cacheFlushed=false;
    bool resumeComplete=true;
    Bytes observed=Bytes::Unknown;
};
constexpr bool MayWrite(const Facts& f) noexcept {
    return f.prepared && f.allSuspended && f.threadSetStable && f.spanClear &&
        f.identityStable && f.withinDeadline && f.observed==Bytes::Original;
}
constexpr State Classify(const Facts& f) noexcept {
    if (!f.resumeComplete) return State::Indeterminate;
    if (f.protectionAttempted && (!f.protectionChanged || !f.protectionRestored))
        return State::Indeterminate;
    if (!f.writeAttempted)
        return f.observed==Bytes::Original ? State::RefusedUnchanged : State::Indeterminate;
    if (f.writeReportedComplete && f.observed==Bytes::Desired && f.cacheFlushed &&
        f.protectionRestored) return State::Installed;
    // Even an original byte readback after a attempted write needs cache coherence.
    if (f.observed==Bytes::Original && f.cacheFlushed && f.protectionRestored)
        return State::RefusedUnchanged;
    return State::Indeterminate;
}
} // namespace cfgobserver::patch
