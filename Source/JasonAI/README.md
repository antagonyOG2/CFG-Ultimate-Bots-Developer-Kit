# JasonAI — primary AI Jason driver and decision policy

All paths are relative to `Source/original-layout/` (the unmodified source layout, so relative `#include`s still resolve).
Nothing here is compiled or runnable on its own; see `../../BUILDING.md`.

## What this subsystem is
The "donor native lane": the AI Jason hunting loop that runs once per Jason `AIController` tick. The pure decision code is in
`Core/` (header-only templates parameterized on a `Host` that supplies reads and native calls). The runtime that hooks the game
and feeds those templates is in `EventObserver/`.

## Entry points (verified by reading)
| File | Role / key symbols |
|---|---|
| `EventObserver/NativeJasonControllerTickRuntime.hpp` | The main driver. `BundleNativeJasonControllerTick` (detour body) runs the original/base `Tick`, then `ServiceNativeJasonControllerTick(owner)` (donor work). `ResolveNativeJasonTickOwner` routes a controller to the primary or F1 Jason. `NativeJasonControllerTickSession` is the returned-call lifetime/proof scope. |
| `Core/DonorJasonControllerTick.hpp` | `DonorRunOfflineBotsControllerTickOnGameThread(Host&)`: per-tick order = startup traps, door break, knife sequence, chase update, combat, driver call. |
| `Core/DonorJasonChase.hpp` | `ChaseState`, `DonorRunJasonAIChaseUpdateOnGameThread(Host&)`: initial teleport, distance teleport, stuck detection (1375 ms sampling), door interaction, path lock, MoveTo. |
| `Core/DonorJasonTargeting.hpp` | `Targets::DonorFindNearestJasonAICounselorTarget`, `DonorRegisterJasonAITarget`, block list (90 s, 600 s after two strikes). |
| `EventObserver/DonorJasonDecisionFrame.hpp` | Per-callback "decision frame": a read-only snapshot of up to 64 counselor candidates and the chosen target; reports why no target was chosen (H2.87). |
| `Core/ChaseLead.hpp` | `cfgchaselead::Lead`: predictive aim point during a speed burst (see `../Navigation/README.md`). |
| `Core/DonorJasonPursuit.hpp`, `DonorJasonMorph.hpp`, `DonorNativeMorphCompletion.hpp` | Pursuit Morph (teleport-style ability) admission and completion. |
| `Core/DonorJasonMovement.hpp`, `DonorJasonCombat.hpp`, `DonorOwedAttackRelease.hpp` | Movement lease, combat slice, owed attack-button release. |
| `Core/DonorJasonRoster.hpp`, `DonorJasonStartup.hpp`, `DonorJasonBusyCause.hpp`, `DonorNavigationYield.hpp` | Roster, opening sequence, "busy" predicates, navigation yield. |
| `Core/J22TargetPolicy.hpp`, `JasonHuntPolicy.hpp`, `JasonRunPolicy.hpp`, `JasonApproachPolicy.hpp`, `JasonRageDifficultyPolicy.hpp`, `PursuitSpeedPolicy.hpp`, `JasonEscapeGuardPolicy.hpp` | Earlier-generation and supporting policies (targeting variants, run speed, rage/difficulty scaling, escape guard). Some are superseded by the donor lane; read before relying. |
| `EventObserver/DonorJasonHuntingService.hpp`, `DonorJasonHuntingHostMembers.hpp`, `DonorJasonHuntingNativeMembers.hpp` | The `Host` implementation (native reads, reflected calls) the Core templates run against. |
| `EventObserver/DonorJasonRuntimeState.hpp`, `DonorJasonActionRuntimeMembers.hpp`, `DonorJasonMovementNativeMembers.hpp` | Runtime state and native action seams. |

## For a native port
The `Host` interface is the seam: in UE4 C++ each `host.X()` becomes a direct call on `AController` / `APawn` /
`UPathFollowingComponent`. All `ReadableActor` / identity-serial proofs disappear (see `../Retention/README.md`).
Pure timing logic in `Core/` transfers as-is.
