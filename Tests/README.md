# Tests — policy fixtures from the original project

These are the author's own standalone fixture programs (each has its own `main`, prints `FAIL <n>` on a failed check and exits non-zero).
They test the **pure decision policies** in `Source/original-layout/Core/` (and a few runtime classes) against fake `Host` objects, with
no game, no engine, and no injection. They are byte-identical copies of files in `Source/original-layout/EventObserver/`.

**Status of this kit:** the files were copied here and every `#include "..."` was statically checked to resolve inside
`Source/original-layout/`. They were **not compiled or run while assembling this kit**. In the original repository they are built and
run as part of the author's gates with MSVC (Visual Studio 2022/2026 toolset), C++17.

## Building one fixture (MSVC, from a VS x64 developer prompt, at the kit root)

```bat
cl /nologo /std:c++17 /EHsc /O2 /W4 /WX /MT /I Source\original-layout\EventObserver Tests\OnFootBurstTests.cpp /FeOnFootBurstTests.exe
OnFootBurstTests.exe
```

`/I Source\original-layout\EventObserver` is required: the tests include `"../Core/...hpp"` and `"../RuntimeProbe/...hpp"`, which MSVC
resolves relative to that include directory. (The original repo also uses `/sdl /guard:cf`; see
`Documentation/BuildScripts-Reference/Build-Cfg35-J2HeldKillTests.cmd`.) The copied fixtures do not include `<windows.h>` directly,
but runtime headers they pull in may; use MSVC on Windows.

## What each fixture verifies

| File | Verifies |
|---|---|
| `OnFootBurstTests.cpp` | `cfgfootburst::Policy`: 3 s burst / 7 s cooldown timing, 4-value 5x write, interruption at every step, ambiguous/failed compare-exchange at each of the 4 writes still restores, a foreign value written by the game during the burst is preserved on restore, non-finite or out-of-range inputs refused. |
| `H246BurstTests.cpp` | `cfgh246` burst/Morph admission while the movement lease is published (`BurstWanted`, `MorphQueue`) together with `CounselorPursuitPolicy`. |
| `DonorJasonNavContinuationTests.cpp` | Nav ring `DonorContinueNavRing` (projection then teleport, candidate advance, starved-probe retries), the H2.84 cut-off ordering (`Lead`), `cfgchaselead::Lead` and `cfgunstick::State` (water escape 8 s, frozen 12 s / 150 cm, failed-ring avoidance). |
| `DonorHeldGrabKillTests.cpp` | `cfgdonorgrab::State::Drive`: slot probing, direction scan and walk, uniform random slot selection (never the previous one), commit/montage guard, stale release, offline-F1 release-instead-of-kill, uncertain-outcome handling. |
| `J23GrabTimingTests.cpp` | `cfggrabtiming::Timing` validity and the report-version/layout check (`cfgbundle::ReportVersion`). |
| `DonorJasonChaseTests.cpp` | `DonorRunJasonAIChaseUpdateOnGameThread` against a fake host. |
| `DonorJasonTargetingTests.cpp` | `DonorFindNearestJasonAICounselorTarget`, `DonorRegisterJasonAITarget`, `DonorHasAlternativeUsableJasonAITarget`. |
| `DonorJasonDecisionFrameTests.cpp` | Decision-frame target selection and "why no target" statistics. |
| `DonorJasonControllerTickTests.cpp` | Per-tick ordering in `DonorRunOfflineBotsControllerTickOnGameThread`. |
| `DonorAdditionalJasonHuntTests.cpp` | F1 (additional) Jason hunt state. |
| `AdditionalJasonPolicyTests.cpp` | Additional-Jason lane enablement and presentation dispatch (`cfgbundle::AdditionalJasonLaneEnabled`, `PresentationDispatch`, `HostedPresentationDispatch`) and the spawn packet (`cfgspawn::SpawnAIParameters`). |
| `H245HostJasonTests.cpp` | Hosted host-as-Jason role layout and role/decision writes (`cfghostedrole`). |
| `ChaseRecoveryTests.cpp`, `DoorRecoveryTests.cpp`, `HidingInteractionTests.cpp` | Small chase-recovery policy check; door recovery (`cfgdoor::Recovery`, selected-door admission); hiding interaction. |
| `DonorCounselorConvergenceTests.cpp` | Counselor convergence state (`cfgdonorconvergence`) and convergence-lane enablement. |
| `CallbackRootLeaseTests.cpp` | `cfgretain::CallbackRootLease` state machine (prepare/enter/complete/close, deferred retirement, poisoning). Specific to the external-DLL GC-retention layer. |
| `ChallengeEscapeBoundaryPolicyTests.cpp`, `NativeChallengeBoundaryTests.cpp` | Challenges boundary/escape policy and the native boundary writer. `NativeChallengeBoundaryTests.cpp` includes `NativeChallengeEscapeActorsTests.cpp` and runtime headers and is Windows/native-layer, not a pure-policy fixture. |

Many more fixtures exist in `Source/original-layout/EventObserver/*Tests.cpp` and `RuntimeProbe/*Tests.cpp` (about 500 files in total); this folder
holds a curated set that exercises the transferable gameplay policies. For parity work in a native port, re-express the same cases as
UE4 Automation tests (`IMPLEMENT_SIMPLE_AUTOMATION_TEST`) and compare outcomes.
