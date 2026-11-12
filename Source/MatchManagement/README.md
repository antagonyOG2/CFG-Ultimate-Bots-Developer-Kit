# MatchManagement — match init, transitions, cleanup

Paths relative to `Source/original-layout/`.

| File | Role / key symbols |
|---|---|
| `EventObserver/GameplayBundleRuntime.hpp` | `ResetBundleWorldState(now)` (about line 4801): per-world reset of all donor lane state, knife, traps, convergence, burst, emotes, summary; also the F1 new-match reset (`CFG_H286_F1_NEW_MATCH`, about line 4016). |
| `EventObserver/GameplayEpochReset.hpp` | `BundleEpochRetirable()`, `BundleEpochRetireMask()`, `BundleResetEpoch()`: retire a whole match "epoch" (clear every per-match member via `CFG_EPOCH_CLEAR`) without touching resident hooks. |
| `EventObserver/HostedH1Runtime.hpp` | H2.75/H2.93 leftover reset: before hosted takeover, `cfghostedoffline::LeftoverEpochDecision(...)` retires a leftover offline epoch (also a finished offline match) once per hosted world. |
| `Core/HostedOfflineEpoch.hpp` | `LeftoverEpochDecision` (Retire / Blocked / none). |
| `EventObserver/HostedRoutineRuntime.hpp` | `InitializeHostedRoutine`: runs `ResetBundleWorldState`, nine numbered clauses, clears leftover traps/sources (H2.93 `CFG_H293_HOSTED_TRAP_RESET`). |
| `EventObserver/WorldRootRetirementRuntime.hpp`, `Core/WorldRootRetirement.hpp` | Retiring GC roots of a finished world. |
| `Core/NativeOfflineLifecycle.hpp`, `FrontendEpochService.hpp`, `FrontendMapArrivalPolicy.hpp`, `FrontendArrivalWindow.hpp`, `EventObserver/NativeOfflineLifecycleHooks.hpp`, `NativeFrontendEpochSource.hpp` | Menu -> map -> match transitions. |
| `EventObserver/PostMatchRecoveryObservation.hpp`, `Core/HostedEndFreezePolicy.hpp`, `HostedClassRestorePolicy.hpp`, `JasonDeathCompletion.hpp` | Post-match recovery, end freeze, class restore. |

A native port gets all of this from the engine: per-match state lives on `AGameState` / `AGameMode` / controllers and dies with the world.
