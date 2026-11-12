# Changelog (developer-facing)

Built from the project's git history (snapshot `0f84e0ea`) and `Documentation/VERIFIED-FEATURES.md`. "H2.xx" are the author's internal test
builds; each was exercised in real matches before the next. Only items marked LIVE in the verified sheet are confirmed working in matches.

## 1.0.0 update (H2.98)
* The gameplay DLL accepts the supported game on any drive and folder: it checks the game's folder layout plus the exact size,
  PE timestamp and SHA-256 (it previously also required the author's own install path).
* Player README: credits and optional donation link; version and supported game hash are built into the installer.

## 1.0.0 — player release (built from H2.97)
* Player package: `INSTALL.bat` / `UNINSTALL.bat`, windowless launcher `CFG Offline Bots.exe`, hidden loader, gameplay DLL, SHA-256 checks.
* Launcher verifies the supported game binary (CFG 3.5, SHA-256 `4FC4BB54...A90A6`), backs up saves (newest 10), never modifies game files or saves.
* Release build fixes: launcher/loader name clash, self-test resolves mod paths only, compile-script `cd` line.
* Status: package built; not yet live-tested as an install at the time of this snapshot.

## H2.97
* Player release plumbing: path-independent helper (`CFG35_RELEASE_PIN`), trace written beside the DLL off the dev machine.

## H2.96
* Audit fixes: the Challenges boundary wait applies only to a failed read with no detected difference and unchanged function liveness/schema; root-window skip off unless lean convergence.

## H2.95
* Burst lead cap relaxed to 1.5x the Jason-counselor gap (`CFG_H295_LEAD_CAP_RELAX`).
* Last running heading (<= 2 s old) is used when no fresh heading exists (`CFG_H295_HEADING_FALLBACK`). LIVE: 135/135 and 91/96 bursts aimed ahead.

## H2.94
* Challenges boundary: a Verify failure while the scope is proven and both native functions are unchanged keeps the edit and re-verifies (bounded ~4 s). Out-of-bounds no longer ends the match when new map areas stream in. LIVE.

## H2.93
* Offline -> online leftover reset also covers a *finished* offline match, once per hosted world (`CFG_H293_LEFTOVER_ANY`). LIVE: offline -> online -> offline -> Challenges in one session.
* Hosted routine start clears startup-trap progress and sources left by an offline match (`CFG_H293_HOSTED_TRAP_RESET`); hosted F1 class selection ignores a stale offline catalogue (`CFG_H293_HOSTED_STALE_CATALOGUE`).

## H2.92
* Callback root-window skip for claims proven since the last native entry (`ProvenSinceLastNative`).
* Hosted start clears offline leftovers (startup traps/sources, F1 class catalogue).

## H2.91
* Timing-only counters: callback GC root window (sleeps/begin/operation/end); held-lane native query / `GrabKillInput` call time.

## H2.90
* Audit fixes: exhausted unstick rings mark their counselor in every mode/step; primary chase recovery uses the distance proved at burst start (-1 means never near).

## H2.89 / H2.88
* Burst lead overshoot 3 ft (H2.88) then 6 ft / 180 cm (H2.89).
* Water/frozen escape tries another counselor when every landing near the first failed (`CFG_H288_UNSTICK_OTHER_TARGET`). LIVE.

## H2.87
* Reports why the decision frame picks no target (empty causes, per-candidate skips).

## H2.86
* Burst lead: during a speed burst aim 2 s ahead of the running counselor (capped at the gap), re-aim every 500 ms instead of a 6 s path lock (`CFG_H286_BURST_LEAD`). LIVE.
* AI Jason sound-blip suppression uses the read-only lookup veto. LIVE: sound hidden from the host online.
* F1 press in a new match clears the previous match's F1 state (`CFG_H286_F1_NEW_MATCH`). LIVE.

## H2.85
* Chase burst: within 30 m a 3 s recovery instead of 7 s; a pending knife probe no longer vetoes the burst; burst veto counters (`CFG_H285_CHASE_BURST`). LIVE.

## H2.84
* Pursuit Morph cut-off: land 7-18 m ahead of a running counselor, then the donor ring most-ahead first (`CFG_H284_MORPH_CUTOFF`). LIVE (LandedClose).

## H2.83
* Offline F1 Jason's PlayerState takes the Jason class (matches hosted `HostedPrepareAi`); kill-completion diagnostics. LIVE: grab-kill death applies for both Jasons.

## H2.82 - H2.72 (earlier 2.7x/2.8x line, from the git log)
* H2.82: offline F1 releases instead of sending a grab-kill input that never kills.
* H2.81: Jason escapes water and frozen spots via the existing ring teleport (`CFG_H281_UNSTICK`); F4 Jason view on its own read budget.
* H2.80: keep hands off a committed grab kill until the montage can finish.
* H2.79 / H2.78 / H2.77: Challenges wall proof tolerates level actor-list changes; offline knife reader budget; a failed bot spawn no longer stops the match.
* H2.76: Jasons walk to where a grab kill is possible (`CFG_H276_HELD_WALK`); continuous blip suppression.
* H2.75: retire a leftover offline match before a hosted match in the same session.
* H2.74 / H2.73: offline Jasons prove their owner on a separate read budget.
* H2.72: F1 knife aims and launches through the primary's native geometry window; F1 car off for release.
