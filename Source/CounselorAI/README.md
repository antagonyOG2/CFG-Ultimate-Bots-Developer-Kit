# CounselorAI — counselor bots, convergence, sweater, emotes

Paths relative to `Source/original-layout/`.

| File | Role / key symbols |
|---|---|
| `Core/DonorCounselorConvergence.hpp` | `cfgdonorconvergence::State`: the donor's "teaming" system. Counselors converge on Jason when the sweater/kill setup is active; picks strong counselors (`IsStrongConvergenceCounselor`: Jock, Tough, Biker, Tommy, Hero, Rob, Julius, Boxer, Mark), scans for `CounselorTwoHandedAxe_C` / `CounselorMachete_C` weapon classes, `GiveConvergenceMeleeWeapon`, travel/recovery bookkeeping. |
| `Core/CounselorConvergencePolicy.hpp`, `ConvergenceTravelPolicy.hpp`, `ConvergencePublication.hpp`, `ConvergenceArrivalReport.hpp`, `LeanConvergencePolicy.hpp` | Convergence travel/publication policies (the "lean" variant is the lighter-weight J2.1x implementation). |
| `Core/SweaterLifecyclePolicy.hpp`, `SweaterOwnerRefreshPolicy.hpp`, `SweaterPublication.hpp`, `SweaterRetention.hpp`, `H248SweaterTarget.hpp` | Sweater (mask pickup) power lifecycle for bots, owner refresh, report-only diagnostics. |
| `Core/CounselorPursuitPolicy.hpp`, `CounselorPracticePolicy.hpp`, `CounselorPracticeSettings.hpp` | Counselor pursuit and "practice" options (`PamelaMute=1, GrabMute=2, MinimalHud=4, HardMovement=8, BotSweater=16`). |
| `EventObserver/DonorCounselorObservation.hpp`, `DonorCounselorContextObservation.hpp` | Reads one counselor's state (alive, escaped, context-kill, controller) for target eligibility. |
| `Core/EmoteSyncPolicy.hpp`, `NativeEmoteMirrorQueue.hpp`, `NativeEmoteCompletionPolicy.hpp`, `EventObserver/DonorCounselorEmotes.hpp`, `NativeEmoteMirror.hpp`, `EmoteBrainRuntimeMembers.hpp`, `RBEmoteRuntime.hpp` | Bot emote mirroring (bots staggered, humans not forced) and RB emote-wheel input. |
| `Core/RandomCounselorPolicy.hpp`, `CounselorLoadPolicy.hpp`, `CounselorStatsPolicy.hpp`, `BotNamePolicy.hpp` | Counselor selection, loading, stats, bot names. |
| `EventObserver/DonorCounselorConvergenceNativeMembers.hpp`, `ConvergenceAxeRuntime.hpp`, `ConvergenceTravelRuntimeMembers.hpp`, `LeanConvergenceNativeMembers.hpp`, `Sweater*RuntimeMembers.hpp` | Native-side implementations of the above (blackboard fear writes, travel MoveTo, axe grant, sweater HUD/ability reads). |

Note: Jason kill by axe/machete during the sweater stun window is a carried-over donor feature and was not re-validated for
release 1.0.0 (see `../../Documentation/VERIFIED-FEATURES.md`).
