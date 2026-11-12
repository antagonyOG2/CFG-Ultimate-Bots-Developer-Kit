# Navigation — chase, path lock, Morph cut-off ring, unstick

Paths relative to `Source/original-layout/`.

| File | Role / key symbols |
|---|---|
| `Core/DonorJasonChase.hpp` | Path lock (`PathLockDurationSeconds=3`, 6107 ms re-use window), stuck checks every 1375 ms (`StuckThreshold=30` cm), door-attempt escalation (H2.50d), recovery teleport eligibility (target >= 30 m away, or no target). |
| `Core/DonorJasonNavContinuation.hpp` | `NavRingContinuation` + `DonorContinueNavRing`: bounded continuation of the donor's 16-cell teleport ring (projection then teleport). `Lead(vx,vy)` implements the H2.84 cut-off: four close landings straight ahead of a running counselor (7/10/14/18 m) then the ring sorted most-ahead first. `cfgcutoff::Tracker` derives a counselor heading from two position samples. Landing rules: |dz| <= 120 cm; ring cells >= 17 m, cut-off cells >= 5 m from the target. |
| `Core/ChaseLead.hpp` | `cfgchaselead::Lead`: aim `speed*2 s` ahead of the runner, lead capped at 1.5x the gap, plus 180 cm overshoot; minimum counselor speed 150 cm/s; re-aim every 500 ms while bursting. |
| `Core/DonorUnstick.hpp` | `cfgunstick::State`: "Jason is never frozen" — water >= 8 s or still >= 12 s within 150 cm triggers the nav-ring teleport; retries 3 s / 20 s; a counselor whose every landing failed is skipped for 30 s. |
| `Core/ChaseRecoveryPolicy.hpp`, `DriverNavigationPolicy.hpp`, `DonorMoveRetentionPolicy.hpp`, `DonorNavigationYield.hpp` | Chase recovery, driver navigation, MoveTo retention, yield. |
| `Core/DoorRecoveryPolicy.hpp`, `CabinBoundsPolicy.hpp`, `EventObserver/CabinBoundsRuntime.hpp` | Blocked-door detection/approach and cabin bounds. |
| `Core/H246BurstPolicy.hpp` | Morph queue admission (`MorphQueue`) and burst admission shared with the movement lease. |
| `EventObserver/ChaseRecoveryRuntimeMembers.hpp`, `DriverNavigationRuntimeMembers.hpp` | Native-side implementations. |

Port note: UE4 already has `UNavigationSystemV1::ProjectPointToNavigation` and `ACharacter::TeleportTo`; the ring is a deterministic
candidate list over those two calls.
