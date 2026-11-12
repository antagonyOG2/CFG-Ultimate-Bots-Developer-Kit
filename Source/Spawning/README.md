# Spawning — F1 additional Jason, bot births, controller ownership

Paths relative to `Source/original-layout/`.

| File | Role / key symbols |
|---|---|
| `Core/AdditionalJasonPolicy.hpp` | `cfgadditional::Stage` (Idle, Select, Project, Pawn, Detach, ControllerClass, Controller, Possess, Navigation, RetireGeneric, Active, Refused, Indeterminate, Retired), `Policy::Request` (F1 edge + 700 ms debounce), `BuildPacket` (donor reinforcement spawn packet: no behavior tree, `noCollisionFail=1`). |
| `EventObserver/AdditionalJasonBirthMembers.hpp` | `AdditionalBirth()` runs one stage per turn. Offline: project a ring point near the human, `AdditionalPawnSpawn` (SpawnActor of the Jason pawn), detach any default controller, set `PawnAIClass` to the KillerAI controller class, `AdditionalNativeSpawn` the controller, `AdditionalKillerClass` (PlayerState takes the Jason `ActiveCharacterClass`, H2.83), `AdditionalPossessCall`, then navigation and retire-generic. Hosted: `HostedF1Birth` / `HostedF1BirthTx`. |
| `EventObserver/HostedJasonMembers.hpp` | `HostedSpawnAi(birth)`: hosted SpawnActor of the KillerAI controller via the native spawn-parameters constructor + SpawnActor. |
| `EventObserver/AdditionalJasonSelectionMembers.hpp`, `AdditionalJasonLifecycleMembers.hpp`, `AdditionalJasonRegistryMembers.hpp`, `AdditionalJasonRuntimeMembers.hpp` | Class selection (random resident Jason class that is not the primary's), lifecycle proof, registry/delegate admission, runtime plumbing. |
| `Core/AdditionalJasonJournal.hpp` | Per-extra immutable birth journal (identity pairs captured around each stage). |
| `Core/DonorAdditionalJasonHunt.hpp` | `AdditionalHuntState`, `DonorFrozenBridgeDriveAdditionalJason(Host&)`: the F1 Jason's hunt (door, hiding, knife, grab, car, swim), separate from the primary's state. |
| `Core/NativeSpawnParameters.hpp`, `NativeDeferredSpawnParameters.hpp`, `OneCounselorSpawnPolicy.hpp`, `HelperSpawnPolicy.hpp`, `UnifiedAISpawnCommands.hpp`, `J24SpawnRefusal.hpp`, `KillerStartBootstrap.hpp` | Spawn parameter structs/policies for counselors, helpers and Jason. |
| `EventObserver/SpawnRuntime.hpp`, `SpawnBindings.hpp`, `HelperBirthRuntimeMembers.hpp`, `NativeOfflineCounselorBirth.hpp`, `NativePreparedCounselorConstruct.hpp`, `PrimarySpawnObservation.hpp`, `OfflineCounselorSpawn.cpp` | Native spawn plumbing (offline counselor / F3 bot births). |

Key facts: offline F1 = SpawnActor(Jason pawn) + SpawnActor(KillerAI controller) + Possess + `ActiveCharacterClass`;
hosted F1 = donor SpawnActor at a native `SCKillerPlayerStart` + Possess, never `RestartPlayer` (`CFG_H258_HOSTED_F1_SPAWN`).
