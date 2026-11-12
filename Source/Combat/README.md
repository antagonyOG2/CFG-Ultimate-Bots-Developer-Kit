# Combat — speed burst, knife, grab kill, traps, doors, hiding, cars

Paths relative to `Source/original-layout/`.

| File | Role / key symbols |
|---|---|
| `Core/OnFootBurstPolicy.hpp` | `cfgfootburst::Policy`: Idle -> Apply -> Hold -> Restore. 5x multiplier on 4 movement values (each must be finite and within 25..2500), 3 s duration, 7 s cooldown, or 3 s cooldown when the target is within 30 m at burst start (H2.85). |
| `Core/H246BurstPolicy.hpp` | `BurstWanted`: 4 m or farther, not already owned, ready (`CFG_H267_BURST_ANY_RANGE`). |
| `Core/DonorHeldGrabKill.hpp` | `cfgdonorgrab::State::Drive(Host&, now)`: held grab-kill state machine (Idle, Waiting, Probing, Scanning, Walking, Committed, Uncertain). Probes seven kill slots, scans eight directions for open space (400 cm), walks to a spot where a kill is possible (max 2 rounds), `Kill()` picks a uniformly random available slot different from the last, native montage guard after commit, legacy fallback after 9 s. `h283::ObserveKillGates` reads the native kill-completion gates (report only). |
| `Core/J23GrabTiming.hpp`, `GrabCombatPolicy.hpp`, `HeldRecoveryPolicy.hpp`, `KillPolicy.hpp`, `H247CombatPolicy.hpp`, `JasonDeathCompletion.hpp` | Grab timing/diagnostics, held recovery, kill admission, combat policy, Jason death completion. |
| `EventObserver/DonorHeldGrabKillNativeMembers.hpp`, `HeldRecoveryRuntimeMembers.hpp`, `GrabRuntimeMembers.hpp` | Native `Host` for the held-kill machine. |
| `Core/KnifeAimGeometry.hpp`, `EventObserver/KnifeRuntime.hpp`, `KnifeRuntimeMembers.hpp`, `DonorKnifeContinuationMembers.hpp`, `KnifeNotifyCompletion.hpp` | Throwing-knife aim geometry and the multi-turn knife sequence (press, release, launch observation). |
| `Core/ObjectiveTrapPolicy.hpp`, `TrapPriorityPolicy.hpp`, `ShackTrapPolicy.hpp`, `DonorTrapPriorityBridge.hpp`, `StartupTrapRuntime.hpp`, `StartupTrapMapScope.hpp`, `EventObserver/NativeStartupTraps.hpp`, `TrapEventObservationMembers.hpp`, `StartupTrapIntegration.txt` | Startup trap placement and trap-victim priority. |
| `Core/DoorRecoveryPolicy.hpp`, `EventObserver/DoorReadOnlySnapshot.cpp`, `SelectedDoorObservation.hpp` | Door open/break (`cfgdoor::Recovery`, `cfgdoor::Approach`). |
| `Core/HidingInteractionPolicy.hpp`, `DonorHidingTargetPolicy.hpp`, `EventObserver/HidingRuntimeMembers.hpp` | Hiding-spot search: phases Approach, Stop, FaceActor, Face, Attempt, Confirm, Locked, RepositionStop, Reposition, Cancel, Unlock, Recover, Retreat; 14 s / 6 attempt budget. |
| `Core/Vehicle*.hpp`, `StuckOccupiedCarPolicy.hpp`, `OnFootCarObstaclePolicy.hpp`, `EventObserver/Vehicle*RuntimeMembers.hpp`, `CarPendingActionRuntimeMembers.hpp` | Car interception (primary Jason only; the F1 car is OFF for 1.0.0). |
