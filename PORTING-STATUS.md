# Porting Status

**Honest summary.** Nothing in this kit has been natively integrated into Camp Forest Green, compiled against CFG's UE4 source, or tested in a CFG
development environment. Every statement below is an assessment from reading the mod's source, not a measured result.

The mod is an external DLL. A large share of its source exists only to work around that (reading game memory at pinned addresses, proving object
identity before every use, retaining objects against the garbage collector, detouring engine functions, an injector and launcher). In a native
port that code is **deleted, not translated**. The transferable value is:

1. the **decision policies** in `Source/original-layout/Core/` (timers, thresholds, ordering, state machines), which are mostly written as engine-independent templates with fixture tests; and
2. the **gameplay sequences** (e.g. what the F1 birth must do in what order, how a held grab-kill picks a slot) documented in `INTEGRATION-GUIDE.md`.

Legend: **A** Ready for asset/Blueprint integration · **B** Requires native C++ · **C** Needs substantial adaptation · **D** Cannot yet transfer reliably.

| System | Class | Why |
|---|---|---|
| Difficulty / tuning numbers | A | Plain numbers. `UnrealIntegration/Configuration/UBBotConfig.h` collects them; values still need tuning in CFG. |
| Settings rows (offline + hosted) | A | UMG rows + saved settings; titles/values/limits are listed in `UnrealIntegration/UI/BotMenuRows.md`. The mod's row engine (patching live widgets by address) is irrelevant natively. |
| Challenges edge walls removal / out-of-bounds fix | A | In native data: do not spawn/disable collision on the boundary volumes, do not end the match when streaming. The mod needed runtime edits of native functions. |
| Counselor emote mirroring, urination replacement | A/B | Mostly existing emote Blueprints plus a staggered mirror queue; sync logic is small C++. Animation assets belong to the game. |
| Target selection (nearest counselor, block list) | B | Small pure C++; trivial in a controller component. Eligibility checks (alive, escaped, has controller) must use CFG's real state. |
| Primary Jason chase (path lock, stuck, door escalation) | B | `Core/DonorJasonChase.hpp` is directly portable logic; the Host seams map to AIController calls. Needs tuning against CFG's navmesh and stock tree. |
| Speed burst (x5, 3 s / 7 s / 3 s) | B | `OnFootBurstPolicy.hpp` is pure; the mod writes four movement speed fields with compare-only restore. Natively use movement component multiplier or a gameplay effect; interaction with CFG's existing sprint/ability systems is unverified. |
| Predictive chase lead | B | `ChaseLead.hpp` pure math; needs a reliable counselor velocity source. |
| Morph (pursuit teleport) + cut-off ring | B | Ordering logic is pure (`DonorJasonNavContinuation.hpp`); must be wired to navmesh projection and the game's own Morph ability/cooldowns. The 16-cell ring constants come from the donor mod. |
| Unstick (water / frozen) | B | Pure timing in `DonorUnstick.hpp`. |
| Knife throw | C | Depends on the stock throwing-knife ability, its aim geometry window and animation notifies; the mod's multi-turn sequence is bound to memory reads. Re-derive from CFG's ability class. |
| Held grab-kill (slot choice + walk-to-space) | C | The state machine is portable, but its inputs (7 kill slots, `CanGrabKill` timing, `ForceKillCharacter` authority, kill montage) come from CFG's context-kill component; verify slot semantics in source. |
| Traps / startup trap placement | C | Depends on map-specific trap sources and CFG's trap actors. |
| Doors / hiding-spot search | C | Interaction components differ by map; phases are reusable. |
| F1 additional Jason (birth, controller, possess, class on PlayerState) | B | Sequence is well documented; native version is `SpawnActor` + `SpawnDefaultController`/`Possess` + set player-state character class, server only. |
| Counselor convergence + sweater power | C | Donor feature carried over; relies on blackboard keys and weapon classes of the stock counselor AI; Jason kill in the stun window was not re-validated in 1.0.0. |
| Cars (primary Jason interception) | D | Heavily dependent on vehicle internals; F1 car is OFF (unproven). Treat as a separate project. |
| Escape guard (exits) | C | `JasonEscapeGuardPolicy.hpp` is pure but not listed in the verified feature sheet; unproven. |
| Hosted lobby AI (host adoption, fill, rows) | D | The mod adopts the host's Jason slot from outside the engine; the native design (GameMode choosing AI Jason / fill bots) is a different implementation. The policies (`Hosted*Policy.hpp`) are informative only. |
| Match init / transitions / cleanup | B | Natively handled by per-match objects; keep the behavioral rule "nothing from match N leaks into N+1". |
| GC retention (`CallbackRootLease`, root bank) | D (delete) | Does not apply natively; use `UPROPERTY` / weak pointers. |
| Address pins, detours, identity proofs, read budgets | D (delete) | Not transferable; not needed. |
| Loader / installer / launcher | D (delete) | Not needed with a native build. |
| Report struct / capture counters | C | Replace with UE4 logging or stat counters; the counter list is valuable for parity testing. |

## What would make parity checks credible
* Re-express the fixtures in `Tests/` as UE4 Automation tests.
* Compare the burst, cut-off, unstick, kill and target-choice counters from a reference run (names in `INTEGRATION-GUIDE.md` section 15) with a native build's logs.
* Treat LIVE items in `Documentation/VERIFIED-FEATURES.md` as the acceptance list; do not promise OFF/UNPROVEN items.
