# Counselor convergence — design notes

**EXAMPLE — not compiled or tested.** Derived from `Source/original-layout/Core/DonorCounselorConvergence.hpp`, `Core/CounselorConvergencePolicy.hpp`,
`EventObserver/DonorCounselorConvergenceNativeMembers.hpp`, `ConvergenceTravelRuntimeMembers.hpp`. This is a **donor feature carried over**; the
final Jason kill by axe/machete in the stun window was not re-validated for 1.0.0.

## Behavior (as read from the source)
1. Trigger: the kill setup is active (the sweater/mask power has been used; the practice option "Bot Sweater Starts Convergence" makes bots start it).
2. Choose participants: strong types first (`Jock, Tough, Biker, Tommy, Hero, Rob, Julius, Boxer, Mark`).
3. Arm them: find the `CounselorTwoHandedAxe_C` / `CounselorMachete_C` classes and grant one (`GiveConvergenceMeleeWeapon`), tracked in `ConvergenceArmedCounselors` (cap 64).
4. Publish Blackboard keys (see `BlackboardKeys.md`): `JasonCharacter`, `SCWeapon`, fight-back flags on; flee/hide off.
5. Travel: issue MoveTo toward Jason's position per counselor with per-counselor progress tracking and recovery windows
   (`ConvergenceTravelLastProgressAt`, `ConvergenceTravelRecoveryUntil`).
6. Clear everything when the setup ends or the counselor dies/escapes (`Action::Clear`).

## Native sketch
- `UUBConvergenceCoordinator` (GameState-owned component) holds `TArray<TWeakObjectPtr<ACounselor>>`, runs on a ~250 ms timer on the server.
- Per counselor: `Blackboard->SetValueAsObject("JasonCharacter", Jason)` etc.; a BT service re-issues MoveTo when progress stalls.
- Keep it behind a config bool; the mod keeps bot near-follow and melee assist OFF.
