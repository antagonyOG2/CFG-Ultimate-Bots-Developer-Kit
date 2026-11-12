#pragma once
#include "PatchTransactionPolicy.hpp"
namespace cfgobserver::patch {
// All work runs in the external harness. Operations must not invoke target-side
// helpers while threads are frozen. No retries or automatic byte rollback.
// Resume is mandatory even when Freeze reports partial failure. Adapter records
// each successful suspension separately and owns all cleanup handles.
template<class Operations> Facts Run(Operations& ops) noexcept {
    Facts f;
    f.observed=ops.ObserveBytes();
    f.prepared=f.observed==Bytes::Original && ops.ValidatePreparation();
    if (f.prepared) {
        f.allSuspended=ops.Freeze();
        f.spanClear=f.allSuspended && ops.SpanClear();
        f.threadSetStable=f.spanClear && ops.ThreadSetStable();
        if (f.threadSetStable) {
            // Never retain the earlier Original evidence after a failed read.
            f.observed=ops.ObserveBytes();
            f.identityStable=f.observed==Bytes::Original && ops.IdentityStable();
        }
        f.withinDeadline=ops.WithinDeadline();
        if (MayWrite(f)) {
            f.protectionAttempted=true;
            f.protectionChanged=ops.Protect();
            if (f.protectionChanged) {
                f.writeAttempted=true;
                f.writeReportedComplete=ops.Write();
                f.observed=ops.ObserveBytes();
                f.protectionRestored=ops.Restore();
                f.cacheFlushed=ops.Flush();
            }
        }
    }
    if (!f.writeAttempted) f.observed=ops.ObserveBytes();
    f.resumeComplete=ops.Resume();
    return f;
}
} // namespace cfgobserver::patch
