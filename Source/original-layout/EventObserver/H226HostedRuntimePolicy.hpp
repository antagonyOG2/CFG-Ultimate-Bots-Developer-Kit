#pragma once

namespace cfgh226 {
// A failed compatibility projection may block the offline primary driver, but
// an adopted hosted Jason has a separate, frame-validated donor roster.
inline constexpr bool ContinueDonorAfterTargetProjectionFailure(bool adoptedHosted)noexcept{
    return adoptedHosted;
}

// Only the outer hosted aim gets a fresh bounded reader. Offline aims and the
// sibling itself stay on the current reader, preventing recursion/replay.
inline constexpr bool UseHostedAimSibling(bool hosted,bool alreadySibling)noexcept{
    return hosted&&!alreadySibling;
}
}
