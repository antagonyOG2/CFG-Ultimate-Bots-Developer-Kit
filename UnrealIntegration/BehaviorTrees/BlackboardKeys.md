# Blackboard keys

**EXAMPLE — not compiled or tested.**

## Counselor blackboard keys the mod reads/writes (REAL names)
From `Source/original-layout/Core/CounselorConvergencePolicy.hpp` (`cfgconvergence::Key` / `KeyText`). These already exist in the stock counselor
blackboard asset; the mod publishes values into them to make counselors fight back or converge when the sweater/kill setup is active.

| Key (enum) | Blackboard name | Type used by the mod | Meaning when published |
|---|---|---|---|
| Weapon | `SCWeapon` | Object | weapon the counselor should use (axe/machete) |
| Jason | `JasonCharacter` | Object | Jason to converge on |
| Fight | `ShouldFightBack` | Bool | fight back |
| Armed | `ShouldArmedFightBack` | Bool | fight back using the weapon |
| Melee | `ShouldMeleeFightBack` | Bool | melee fight back |
| SeekWeapon | `SeekWeaponWhileFleeing` | Bool | look for a weapon while fleeing |
| Flee | `ShouldFleeKiller` | Bool | (cleared while converging) |
| Hide | `ShouldHide` | Bool | (cleared while converging) |
| Orient | `ShouldOrientTowardKiller` | Bool | face Jason |

(The mod's schema reader, `EventObserver/ConvergenceBlackboardSchema.hpp`, proves each key exists with the right type before writing; natively you
use `UBlackboardComponent::SetValueAsBool/Object` and the keys' existence is compile/asset time.)

## Suggested Jason blackboard (EXAMPLE design, not from the mod)
The mod does not use a behavior tree for Jason; it runs the state machines in code. If CFG prefers a BT, this is a mapping:

| Key | Type | Written by | Mirrors |
|---|---|---|---|
| `TargetCounselor` | Object (Actor) | `BTService_JasonFindTarget` | `DonorFindNearestJasonAICounselorTarget` |
| `TargetLocation` | Vector | service (with lead when bursting) | `DonorIssueStockReflectedJasonMoveOnGameThread` + `cfgchaselead::Lead` |
| `bBursting` | Bool | burst task | `cfgfootburst::Policy::Owned` |
| `BurstReadyAt` | Float (world seconds) | burst task | `Policy::readyAt` |
| `bBusyCombat` | Bool | combat tasks | `IsJasonBridgeCombatBusy` |
| `HeldVictim` | Object | grab-kill task | `cfgdonorgrab::Hold` |
| `bInWater` | Bool | service | `cfgunstick::State::Water` input |
| `MorphReadyAt` | Float | morph task | `JasonAIMorphReadyAt` |
