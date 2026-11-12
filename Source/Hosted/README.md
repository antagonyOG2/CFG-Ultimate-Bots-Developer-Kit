# Hosted — hosted online lobby AI (host runs the mod; joiners are vanilla)

Paths relative to `Source/original-layout/`.

| File | Role / key symbols |
|---|---|
| `EventObserver/HostedH1Runtime.hpp` | Hosted state machine / sampler cadence (500 ms with a NetDriver, 1000 ms otherwise); detects a new hosted world, adoption, leftover-offline retirement. |
| `EventObserver/HostedAdoption.hpp`, `HostedJasonTakeover.hpp`, `HostedJasonState.hpp`, `HostedJasonMembers.hpp` | Taking over the host's Jason slot: `HostedSpawnAi`, `HostedPrepare`, class restore at end. |
| `EventObserver/HostedRoutineRuntime.hpp`, `HostedRoutineRuntimeMembers.hpp` | `InitializeHostedRoutine` and `HostedRoutineActive()` — the authority switch that distinguishes hosted from offline (see ARCHITECTURE.md). |
| `EventObserver/HostedCounselorMembers.hpp`, `HostedCounselorBirthBrainMembers.hpp`, `HostedCounselorBrainMembers.hpp`, `HostedFillRuntime.hpp`, `HostedDeferredFill.hpp` | Filling empty slots with counselor bots (RestartPlayer-style births). |
| `EventObserver/HostedLobby*.hpp`, `HostedSettingsSpecs.hpp` | Host lobby panel rows and placement. |
| `Core/HostedOnlinePolicy.hpp`, `HostedOptions.hpp`, `HostedFillPolicy.hpp`, `HostedRolePreference.hpp`, `HostedCounselorBirth.hpp`, `HostedCounselorCandidatePolicy.hpp`, `HostedKillerPresentation.hpp`, `HostedMovementFallbackPolicy.hpp`, `HostedMaskTrigger*.hpp`, `HostedSpectatorHintPolicy.hpp`, `HostedObservationBudget.hpp` | Pure hosted policies. |
| `Core/DonorAdditionalJasonHunt.hpp` + `Core/J22FeatureFlags.hpp` (`CFG_H26x`) | Hosted F1 uses the same donor hunt lane as offline (`CFG_H263_F1_DONOR_HUNT`), with roster-wide target fallback (`CFG_H266_TARGET_FALLBACK`). |
| `RuntimeProbe/HostedAuthority35.generated.hpp` *(withheld from the public kit)*, `HostedBirth35.generated.hpp` *(withheld from the public kit)*, `HostedFill35.generated.hpp` *(withheld from the public kit)*, `HostedLobby35.generated.hpp` *(withheld from the public kit)*, `HostLocalView35.generated.hpp` *(withheld from the public kit)* | CFG 3.5 address/offset pins for the hosted systems (would be deleted in a native port). |

Hosted authority: the host machine is the server; it owns the Jason pawn/controller and all bot controllers. Joiners need nothing installed.
