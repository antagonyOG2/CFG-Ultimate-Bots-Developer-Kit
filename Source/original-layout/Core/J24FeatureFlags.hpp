#pragma once
// J2.4 fix package. Each item is isolated behind its own flag; nothing here
// relaxes a proof. Mailbox #351 (J2.3 live PID11008) is the evidence source.
#ifndef CFG_J24_SPAWN_RESET
// Per-world spawn-asset reset at preparation plus sub-stage codes for every
// ResourcesMatch/ResolveFunctions refusal (state 5 RefusedResources).
#define CFG_J24_SPAWN_RESET 1
#endif
#ifndef CFG_J24_FAILED_START_RETIRE
// A world that took mod GC roots but whose AI never started is recorded for the
// same world-root retirement as a normal match before the next LoadMap.
#define CFG_J24_FAILED_START_RETIRE 1
#endif
#ifndef CFG_J24_GRAB_FALLBACK_2500
// J2.1c held-recovery fallback deadline (constant only; every safety
// condition is unchanged). Measured first CanGrabKill ~2.3 s, max 2.34 s.
#define CFG_J24_GRAB_FALLBACK_2500 1
#endif
#ifndef CFG_J25_SWEATER_REBIND
// Current native sweater ability may replace a positively retired instance.
// OFF until the CFG 3.5 full regression and live tests pass.
#define CFG_J25_SWEATER_REBIND 1
#endif
#ifndef CFG_J25_ROOF_GUARD
// Extra native nav projections await full CFG 3.5 safety and budget gates.
#define CFG_J25_ROOF_GUARD 1
#endif
#ifndef CFG_J27_ESCAPE_OPTIONAL
// CFG 3.5 integration builds never install the five escape hooks
// (CFG35_MENU_RETENTION_TEST), so arming them always failed and refused
// every Challenges Start. When the hooks are absent, start without the
// escape guard instead of refusing. No hook, read or write is added.
#define CFG_J27_ESCAPE_OPTIONAL 1
#endif
#ifndef CFG_J25_CHALLENGE_PUSH_OUTPUTS
// Challenges-only push/pop post-transition retention gate. Disabled until
// CFG 3.5 validation gates have independently passed.
#define CFG_J25_CHALLENGE_PUSH_OUTPUTS 1
#endif
#ifndef CFG_J28_CHALLENGE_35_BINDINGS
// Challenges in-match features (options, boundary, intro skip) were refused in
// CFG 3.5 because offlineowner::LookupSafe needs an active scoped-lookup bundle
// that these standalone runtimes never enter (3.5 RuntimeLookupEligibilityVerified
// is false). When 1 they use the read-only GC/serialization veto and reflected
// property offsets. 0 restores the previous behavior exactly.
#define CFG_J28_CHALLENGE_35_BINDINGS 1
#endif
#ifndef CFG_J29_CHALLENGE_INTRO_SCOPE
// Challenges intro skip requires a current, owner-validated native lookup
// scope. Keep OFF until the Windows fixture and full CFG 3.5 gates pass.
#define CFG_J29_CHALLENGE_INTRO_SCOPE 1
#endif
#ifndef CFG_J29_CHALLENGE_ESCAPE_HOOKS
// Install the proven five-hook transaction after the native offline pair.
// Non-fatal failure preserves that already installed pair.
#define CFG_J29_CHALLENGE_ESCAPE_HOOKS 1
#endif
#ifndef CFG_J29_CHALLENGE_ESCAPE_PHYSICAL
// No retained ScopedLookupEligibility root is furnished to escape callbacks in
// this handoff. Keep all escape native queries, factories and recovery OFF.
// Only enable with separately reviewed/entered scoped lookup session proof.
#define CFG_J29_CHALLENGE_ESCAPE_PHYSICAL 0
#endif
#ifndef CFG_J30_INTRO_SKIP_35_VALIDATE
// Match the proven CFG 3.5 post-match SkipLevelIntroOutro function proof.
// OFF preserves J29's exact CFG 3.4 validation until full CFG 3.5 gates pass.
#define CFG_J30_INTRO_SKIP_35_VALIDATE 1
#endif
#ifndef CFG_J32_CHALLENGE_NATIVE_SCOPE
// Task008: Challenges-only native callback/retention candidate. OFF until the
// synchronized root provider and callback return-PC witness pass Windows gates.
#define CFG_J32_CHALLENGE_NATIVE_SCOPE 1
#endif
#ifndef CFG_J31_INTRO_SKIP_GAMEPLAY_DISPATCH
// J31 request handoff only. This remains OFF until a Challenges-mode retained
// gameplay service is wired and its Windows GC/ownership gates pass.
#define CFG_J31_INTRO_SKIP_GAMEPLAY_DISPATCH 1
#endif
#ifndef CFG_J29_CHALLENGE_BOUNDARY_FULL
// Read-only per-actor wall-class census. Does not modify native collision.
#define CFG_J29_CHALLENGE_BOUNDARY_FULL 1
#endif
#ifndef CFG_J33_INTRO_STRONG_CHILDREN
// Serial-unassigned PersistentLevel/WorldSettings may be borrowed ONLY by
// proving exact retained-world strong links, stable registry index and GC veto.
#define CFG_J33_INTRO_STRONG_CHILDREN 1
#endif
#ifndef CFG_J34_JASON_OPTIONS_FIX
// Re-enable independently owned native human-Jason Start under the production
// counselor-startup test branch, without changing the counselor path.
#define CFG_J34_JASON_OPTIONS_FIX 1
#endif
#ifndef CFG_J34_JASON_TEN_COUNSELORS
// NO-GO: human-Jason route has no deferred-bot gameplay birth service.
#define CFG_J34_JASON_TEN_COUNSELORS 0
#endif
#include "JasonStartRoutePolicy.hpp"
#ifndef CFG_J35_MENU_REENTRY_FIX
#define CFG_J35_MENU_REENTRY_FIX 1
#endif
#include "FrontendMapArrivalPolicy.hpp"
namespace cfgj24 {
inline constexpr unsigned long long HeldFallbackMs=CFG_J24_GRAB_FALLBACK_2500?2500ull:1500ull;
}

#include "J214FeatureFlags.hpp"
