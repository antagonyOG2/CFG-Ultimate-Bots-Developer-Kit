# UnrealIntegration — example native designs

**EXAMPLE — not compiled or tested.** Everything in this folder was written for this kit as a starting point for CFG's UE4 C++ source
and Blueprints. It is derived from the real logic in `Source/original-layout/` (each file names the original it mirrors) but it is not
code from the mod, has never been built against CFG's UE4 headers, and class names such as `AUBJasonAIController` are placeholders
(`UB` = Ultimate Bots) that you should rename to your conventions. CFG's real class names (`ASCKillerPlayerStart`, `USCContextKillComponent`,
Jason/counselor character classes, interaction components) must be taken from CFG's own source.

| Folder | Contents |
|---|---|
| `AIControllers/` | `UBJasonAIController.h`, `UBJasonPursuitComponent.h`: controller + component that replace the Tick hook and the `Core/` chase/burst/unstick policies. |
| `BehaviorTrees/` | `BT_JasonHunt.md` (tree layout), `BlackboardKeys.md`, `BTTask_JasonSpeedBurst.h`, `BTService_JasonFindTarget.h`, `CounselorConvergence.md`. |
| `Blueprints/` | `BlueprintMap.md`: which behaviors suit Blueprint vs C++. |
| `UI/` | `BotMenuRows.md`: UMG row mapping for the offline and hosted-lobby options. |
| `Configuration/` | `UBBotConfig.h`: `UDataAsset` of every tunable number from the mod. |

Server-authoritative: all of it is intended to run only where `HasAuthority()` is true.
