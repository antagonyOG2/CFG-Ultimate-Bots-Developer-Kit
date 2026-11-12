# Integration Guide for the Camp Forest Green team

Goal: help CFG port the behaviors of *Camp Forest Green OFFLINE and ONLINE Bots MAXIMIZED* into the CFG UE4 C++ source / Blueprints so no
external DLL is needed.

Ground rules for reading this guide:

* **Only behaviors listed as LIVE in `Documentation/VERIFIED-FEATURES.md` are claimed to work.** Disabled/unproven items are marked.
* Paths are relative to `Source/original-layout/` unless they start with `UnrealIntegration/` or `Documentation/`.
* Function and file names were verified by reading the snapshot (commit `0f84e0ea`, release 1.0.0 = H2.97). Numbers (timers, ranges,
  caps) are copied from that code; tune them in your own build.
* Blocks marked **EXAMPLE** are illustrative pseudocode written for this guide. They are not compiled or tested and are not code from the
  mod. The real logic is in the cited files.
* Nothing in this kit has been natively integrated or tested in a CFG development environment (`PORTING-STATUS.md`).
* Using this material inside CFG beyond the `LICENSE` terms requires the author's written permission (`THIRD-PARTY-NOTICES.md`).

Table of contents: 1 Runtime architecture, 2 Primary Jason AI, 3 Secondary (F1) Jason, 4 Counselor AI, 5 Target selection, 6 Navigation and
pursuit, 7 Abilities and combat, 8 Counselor escape and Jason kill, 9 Custom menus and settings, 10 Spawning and controller ownership,
11 Offline vs hosted authority, 12 Match init, transitions, cleanup, 13 What must be native C++ vs Blueprint/BT/data, 14 What to delete,
15 Parity verification.

---

## 1. Runtime architecture

See `ARCHITECTURE.md` for the full picture. Summary for porters:

* The **driver** is a per-tick call on each AI Jason's `AIController`. In the mod that is a detour on the game's KillerAIController Tick
  (`EventObserver/NativeJasonControllerTickRuntime.hpp`: `BundleNativeJasonControllerTick` -> `ServiceNativeJasonControllerTick`).
  In CFG source this is simply an override of `AController::Tick` / a tick component / a BT service on the Jason controller.
* The **decisions** live in header-only templates in `Core/` taking a `Host&`. A `Host` method such as `GetJasonAIActorLocation`,
  `SafeMoveToLocation`, `ReadablePawnController` is the seam between "what to do" and "how the engine is touched".
* The policies are **time-sliced state machines** (`now` in ms, `Step` functions). That shape matches BT services/tasks or a component Tick.

**EXAMPLE — not compiled or tested.** Shape of the native driver:

```cpp
// EXAMPLE
void AUBJasonAIController::Tick(float Dt)
{
    Super::Tick(Dt);                       // stock behavior first, as the mod does (base Tick, then donor work)
    if (!IsJasonAIActive()) return;        // replaces ResolveNativeJasonTickOwner + ObserveOwner
    Pursuit->TickOnce(NowMs());            // replaces ServiceNativeJasonControllerTick -> DonorRunOfflineBotsControllerTickOnGameThread
}
```

Real per-tick order (`Core/DonorJasonControllerTick.hpp`): startup traps -> door break -> knife sequence -> chase -> combat -> driver call.

---

## 2. Primary Jason AI

Files: `Core/DonorJasonChase.hpp`, `Core/DonorJasonControllerTick.hpp`, `Core/DonorJasonMovement.hpp`, `Core/DonorJasonPursuit.hpp`,
`Core/DonorJasonMorph.hpp`, `EventObserver/DonorJasonHuntingService.hpp`, `DonorJasonHuntingHostMembers.hpp`.

What it does each tick (LIVE: pursuit, Morph, speed bursts, knives, traps, door open/break, hiding search, grab kills, car interception):

1. **Opening.** One initial teleport near the counselors after a start delay (`ChaseState::InitialTeleportAttempted`, `InitialTeleportAt`;
   hosted seeds `LastMorphTeleportAt`, `NextDistanceTeleportAt`, `InitialTeleportAt = now + MorphCooldownMs (20000)` in
   `DonorInitializeHostedPrimaryChase`). Never issues MoveTo in the teleport frame.
2. **Distance teleport** ("Morph" toward the counselors) when far (`RunJasonAIDistanceTeleportOnGameThread`).
3. **Stuck check** every 1375 ms: moved less than 30 cm with a recently accepted move -> door interaction first; after two door
   "successes" at the same spot (150 cm) with no progress, escalate to the failsafe teleport (only if the target is >= 3000 cm away, or
   after two near-target deferrals, or there is no target).
4. **Target choice** then **path lock** (3 s; a repeat of the same target within 6107 ms of the last accepted move reuses the lock).
5. **MoveToLocation** to the counselor's location (acceptance 85 cm; pathfinding + project-to-navigation + allow-partial).

Native mapping: UE4 `AAIController::MoveToLocation/MoveToActor` (a BT `MoveTo` task with acceptance radius 85 does the same),
`UNavigationSystemV1::ProjectPointToNavigation`, `ACharacter::TeleportTo`.

---

## 3. Secondary (F1) Jason

Files: `Core/DonorAdditionalJasonHunt.hpp` (`AdditionalHuntState`, `DonorFrozenBridgeDriveAdditionalJason`), `Core/AdditionalJasonPolicy.hpp`,
`EventObserver/AdditionalJason*Members.hpp`.

* F1 spawns a second AI Jason (offline and hosted; LIVE). The F1 Jason runs the same donor hunt lane as the primary with **separate state**
  (the primary's globals are never swapped to the reinforcement).
* Capabilities: pursuit, Morph, bursts (`CFG_H267_F1_BURST`), knife (`CFG_F1_KNIFE`), traps, doors/hiding (`CFG_H264_F1_INTERACTIONS`),
  grab kills, swim kill within 6 m (`CFG_H266_F1_SWIM`: "follow into water and use the game's underwater kill"; no new animations).
* **F1 car is OFF** (`CFG_F1_CAR 0`). Offline F1 never sends the grab-kill input from a hold when `CFG_H282_F1_OFFLINE_NO_HELD_KILL`
  applies; it releases the victim alive (`Kill()` in `DonorHeldGrabKill.hpp`, host method `HeldKillAllowed`). Live status per VERIFIED-FEATURES:
  grab kills apply death correctly for both Jasons because the F1 PlayerState takes the Jason class (H2.83).
* Birth sequence: see section 10. Request handling: `cfgadditional::Policy::Request` — rising edge of F1 while the window is foreground, 700 ms debounce.
* One F1 per match in the first milestone ("one reinforcement (two Jasons)") — `AdditionalJasonPolicy.hpp` comment; hosted "Number of Jasons" row offers 1 or 2.

Native mapping: a second `AUBJasonAIController` possessing a second Jason character spawned by the GameMode; all state in the controller component.

---

## 4. Counselor AI

Files: `Core/CounselorPursuitPolicy.hpp`, `Core/DonorCounselorConvergence.hpp`, `Core/CounselorConvergencePolicy.hpp`,
`Core/ConvergenceTravelPolicy.hpp`, `EventObserver/DonorCounselorEmotes.hpp`, `NativeEmoteMirror.hpp`, `Core/RandomCounselorPolicy.hpp`,
`Core/CounselorLoadPolicy.hpp`, `EventObserver/OfflineCounselorSpawn.cpp`.

* The stock game already drives counselor bots with its own behavior tree; the mod mostly **adds**: more bots than stock (up to ten),
  F3 additional bots, skill levels, convergence on Jason, emotes (Wave replaced by the urination animation/effect; synchronized bot
  emotes, staggered), sweater (mask) power for bots, bot names (`BotNamePolicy.hpp`).
* Convergence writes blackboard keys (fear) and issues travel MoveTo to bots (`ConvergenceBlackboardSchema.hpp`, `ConvergenceFearObservation.hpp`,
  `ConvergenceTravelRuntimeMembers.hpp`) — see `UnrealIntegration/BehaviorTrees/CounselorConvergence.md` for a key list.
* **Not enabled in 1.0.0:** bot near-follow (`CFG_J23_BOT_NEAR_FOLLOW 0`) and bot melee assist (`CFG_J23_BOT_MELEE_ASSIST 0`).

---

## 5. Target selection

Files: `Core/DonorJasonTargeting.hpp`, `EventObserver/DonorJasonDecisionFrame.hpp`, `Core/J22TargetPolicy.hpp`, `Core/JasonHuntPolicy.hpp`.

`cfgdonorhunt::Targets::DonorFindNearestJasonAICounselorTarget(host, jason, excluded)`:

1. `jason` must be readable and have a location.
2. For each registered candidate (capacity 64): skip null, the Jason, the excluded actor, **blocked** targets
   (`DonorBlockJasonAITarget`: 90 s on the first strike, 600 s from the second), unreadable actors, and actors without a Pawn controller
   (`ReadablePawnController`).
3. Choose the smallest **3-D squared distance**; non-finite distances are skipped.

The mod's refinement: the *decision frame* (`DonorJasonDecisionFrame.hpp`) takes a read-only snapshot of up to 64 candidates over several callbacks, then calls the same
function; it also records why nothing was chosen (H2.87 counters `h286NearestEmpty`, `h286NearestSkips`, `h286NearestChosen`).
Hosted fallback: when the lane's own list is empty, the full roster is used with the same rule (`CFG_H266_TARGET_FALLBACK`, `CFG_H263_F1_ROSTER_TARGETS`).

**EXAMPLE — not compiled or tested.** UE4 equivalent:

```cpp
// EXAMPLE
AActor* UUBJasonPursuitComponent::FindNearestCounselor(const TSet<TWeakObjectPtr<AActor>>& Candidates) const
{
    AActor* Best = nullptr; float BestDistSq = FLT_MAX;
    for (const TWeakObjectPtr<AActor>& C : Candidates)
    {
        AActor* A = C.Get();
        if (!IsValid(A) || A == GetOwner() || IsTargetBlocked(A)) continue;
        if (!Cast<APawn>(A) || !Cast<APawn>(A)->GetController()) continue;   // "has a controller"
        if (!IsAliveAndNotEscaped(A)) continue;                              // game-specific eligibility
        const float D = FVector::DistSquared(A->GetActorLocation(), GetOwner()->GetActorLocation());
        if (D < BestDistSq) { BestDistSq = D; Best = A; }
    }
    return Best;
}
```

---

## 6. Navigation and pursuit

Files: `Core/DonorJasonChase.hpp`, `Core/ChaseLead.hpp`, `Core/DonorJasonNavContinuation.hpp`, `Core/DonorUnstick.hpp`,
`Core/H246BurstPolicy.hpp`.

* **Path lock** (section 2). While a speed burst is active the 3 s lock is replaced by a 500 ms re-aim (`cfgchaselead::BurstRefreshMs`).
* **Chase lead** (`cfgchaselead::Lead`, LIVE H2.86/H2.95): during a burst aim at `goal + heading * (min(speed * 2 s, 1.5 * gap) + 180 cm)`.
  Needs the counselor's ground speed >= 150 cm/s. Heading comes from `cfgcutoff::Tracker` (position samples >= 250 ms apart, newest <= 1.5 s old;
  if no fresh running heading, the last running heading <= 2 s old is used — `CFG_H295_HEADING_FALLBACK`).
* **Morph ring / cut-off** (`NavRingContinuation`, LIVE H2.84): a Morph = project a candidate point near the counselor onto the navmesh, accept
  if |dz| <= 120 cm and far enough, then teleport. Candidate order: with a running counselor, four landings straight ahead at 7, 10, 14,
  18 m, then the 16-cell donor ring (radii 22 m, 15.56 m diagonals, 30 m, 21.2 m) sorted most-ahead first; otherwise the donor ring order.
  A candidate whose roof-guard probe could not be run is retried up to 8 times (`MaxStarvedRetries`; an external-DLL budget artifact).
* **Unstick** (`cfgunstick`, LIVE H2.81/H2.88/H2.90): water for >= 8 s, or still for >= 12 s within 150 cm, triggers the ring teleport near a counselor;
  water retry every 3 s, frozen recharge 20 s; if every landing near a counselor failed, the next try uses another counselor (30 s skip).
  Known limitation: the primary may stall ~12 s at an unreachable spot before the frozen escape.

**EXAMPLE — not compiled or tested.** Ring selection in native code:

```cpp
// EXAMPLE
bool UUBJasonPursuitComponent::TryCutOffMorph(AActor* Target, const FVector2D& Heading /*unit, valid if runner*/)
{
    TArray<FVector> Offsets;                                   // order matters
    if (Heading.SizeSquared() > 0.f)
        for (float D : {700.f, 1000.f, 1400.f, 1800.f}) Offsets.Add(FVector(Heading * D, 0.f));
    AppendRingMostAheadFirst(Offsets, Heading);                // 16 cells, see DonorJasonNavContinuation.hpp
    for (const FVector& O : Offsets)
    {
        FNavLocation Proj;
        if (!Nav->ProjectPointToNavigation(Target->GetActorLocation() + O - FVector(0,0,65.f), Proj, FVector(350,350,110))) continue;
        if (FMath::Abs(Proj.Location.Z - (Target->GetActorLocation().Z - 90.f)) > 120.f) continue;
        if (FVector::Dist2D(Proj.Location, Target->GetActorLocation()) < MinDistFor(O)) continue;
        return GetPawn()->TeleportTo(Proj.Location, GetPawn()->GetActorRotation());
    }
    return false;
}
```

(The `- FVector(0,0,65.f)` is `target.Z - 90 + 25` from the real code.)

---

## 7. Abilities and combat

### 7.1 Speed burst (LIVE H2.85)
`Core/OnFootBurstPolicy.hpp`. Shift-style burst: multiply four movement values by 5 for 3 s, then restore. Cooldown 7 s, or 3 s when the
target was within 30 m when the burst started. Starts at any range over 4 m while in pursuit (`BurstWanted`). The restore is
compare-only (a value the game changed meanwhile is left alone). Native mapping: a gameplay effect / `CharacterMovementComponent::MaxWalkSpeed`
multiplier with a timed restore — the four values are the Jason movement component's Walk, Sprint, Run and SlowRun speed fields (`OnFootBurstRuntimeMembers.hpp`, `FootBurstField`).

### 7.2 Knife throw
`Core/KnifeAimGeometry.hpp`, `EventObserver/KnifeRuntime.hpp`, `KnifeRuntimeMembers.hpp`, `DonorKnifeContinuationMembers.hpp`. A multi-turn sequence
(press, release, launch observation, finalize) using the game's own throwing-knife ability; F1 aims and launches through the primary's native geometry
window (H2.72, `CFG_F1_KNIFE`). Native mapping: trigger the same ability (GameplayAbility/Blueprint event) from a BT task.

### 7.3 Held grab kill (LIVE)
`Core/DonorHeldGrabKill.hpp` state machine (see `Source/Combat/README.md`):

```
Waiting -> Probing (7 kill slots) -> [>=2 slots, or struggling+any] Kill()
                                  -> else Scanning (8 directions, 400 cm) -> Walking (<=2.5 s / 400 cm) -> Probing again
Kill(): choose uniformly random available slot != last used -> host.Kill(hold, slot) -> Committed (guard for the native montage)
```

Highlights: seats/extraction waits are bounded (3 s); water cancels our walking (the native swim kill proceeds); after a commit Jason keeps hands
off until the montage can finish (H2.80); the walk exists because the native "CanGrabKill" turns true ~2.4 s into a hold (H2.76).
The native kill chain (H2.83 diagnostic, `h283::ObserveKillGates`): `GrabKillInput` -> `GrabActors[slot]` -> `USCContextKillComponent::ActivateGrabKill`
-> server/multicast `ActivateContextKill` -> `EndAction` kills the victim via `ForceKillCharacter` only with authority. In source you call
the same ability; slot choice is the only logic worth porting.

### 7.4 Traps, doors, hiding
* Startup trap placement and trap-victim priority: `Core/ObjectiveTrapPolicy.hpp`, `TrapPriorityPolicy.hpp`, `ShackTrapPolicy.hpp`, `StartupTrap*`, `EventObserver/NativeStartupTraps.hpp`.
* Doors: `cfgdoor::Recovery` — a Jason that has not moved more than 80 cm between 2 s samples while a target is farther than 100 cm away is "blocked"; pick the nearest closed door in reach (<= 500 cm), approach to 100 cm, press the interaction (`EILLInteractMethod::Press`), or break it; waits 8 s between pulses and 30 s before the same door.
* Hiding: `cfghiding::State` phases (Approach, Stop, FaceActor, Face, Attempt, Confirm, Locked, Reposition, Cancel, Unlock, Recover, Retreat); 14 s / 6 attempt budget; ignore a spot 7 s (25 s if an alternative exists).

### 7.5 Cars
Primary Jason only (F1 car OFF): acquire an occupied car, hood/seat actions, extraction, driving (`Core/Vehicle*.hpp`, `EventObserver/Vehicle*RuntimeMembers.hpp`). Treat as a separate project.

---

## 8. Counselor escape and Jason kill

* **Escape guard** (`Core/JasonEscapeGuardPolicy.hpp`, `cfgescapeguard`): once an exit is live (police arrived, car/boat exit in use) a counselor within 35 m of it
  becomes Jason's top priority; Morph to 3 m from the exit toward the runner if Jason would arrive > 10 m later and the exit is > 50 m away; hold the guard
  10 s while the threat persists. Present in code; **not listed in VERIFIED-FEATURES.md**, so treat as unproven.
* **Convergence + sweater** (`Core/DonorCounselorConvergence.hpp`): when the kill setup (sweater/mask) is active counselors converge on Jason; strong
  types get priority; the scan finds the axe (`CounselorTwoHandedAxe_C`) and machete (`CounselorMachete_C`) classes and grants a melee weapon.
  Sweater power with recharge is carried over from the donor.
* **Jason kill** by axe/machete in the stun window is a **donor feature carried over; not re-validated** for 1.0.0.
* No counselor-escape rewrite is part of the verified feature list; the stock game's escape behavior is unchanged by this kit's documented systems.

---

## 9. Custom menus and settings

Files: `EventObserver/FrontendBundleRuntime.hpp` (row list ~line 691), `NativeSetupRows.hpp`, `HostedSettingsSpecs.hpp`, `Core/HostedOptions.hpp`,
`Core/ModeSettingsPolicy.hpp`, `Core/CounselorPracticeSettings.hpp`, `Core/GameplayLimits.hpp`.

Rows (real titles from source):

| Mode | Row | Values |
|---|---|---|
| Offline | Counselor skill | Low / Medium / High |
| Offline | Counselors | 1..10 (`cfglimits::MaximumCounselors`) |
| Offline | Jason difficulty | Easy / Normal / Hard |
| Offline | Match length | 30 / 60 / 90 min (unlimited only offline-as-Jason) |
| Offline | Starting weapon | None / Random / Machete / Axe / Shotgun |
| Offline | Starting inventory | None / Random / Pocket Knife / First Aid Spray / Firecrackers / Map |
| Offline | Mute Pamela (keep sweater voice); Mute Distant Bot Grab Sounds; Minimal HUD (keep objectives); Hard AI Jason Movement (2x); Bot Sweater Starts Convergence | Off/On (`CounselorPracticeSettings.hpp` bit flags) |
| Online (host lobby) | Online: Jason Bot | Off / On |
| Online | Online: Jason Bot Difficulty | Off / Easy / Normal / Hard |
| Online | Online: Fill Empty Slots with Counselor Bots | Off / On |
| Online | Online: Number of Jasons | 1 / 2 |

Port: add the rows to the existing UMG settings widgets and persist them in a `USaveGame` / config; see `UnrealIntegration/UI/` and
`UnrealIntegration/Configuration/UBBotConfig.h`. `cfgport::Validate` documents which combinations are legal. Hosted choices are snapshotted
once per match (an edit cannot affect a birth in flight).

---

## 10. Spawning and controller ownership

Files: `EventObserver/AdditionalJasonBirthMembers.hpp`, `Core/AdditionalJasonPolicy.hpp`, `EventObserver/HostedJasonMembers.hpp`.

**Offline F1** (`AdditionalBirth`, one stage per turn): `Select` (class) -> `Project` (ring point near the human on the navmesh, 12 candidates) ->
`Pawn` (SpawnActor the Jason pawn at that point; the new pawn must have no controller) -> `Detach` (unpossess any default controller) ->
`ControllerClass` (set the pawn's AI controller class to KillerAI) -> `Controller` (SpawnActor the KillerAI controller; check it is a new object, unpossessed;
PlayerState takes the primary Jason's `ActiveCharacterClass` — H2.83) -> `Possess` -> `Navigation` -> `RetireGeneric` (destroy the generic default controller, if any) -> `Active`.

**Hosted F1** (`HostedF1Birth`/`HostedF1BirthTx`): donor SpawnActor of the Jason pawn at a native `SCKillerPlayerStart` + Possess, never `RestartPlayer`;
`HostedSpawnAi` spawns the KillerAI controller via the spawn-parameters constructor; random resident Jason class not equal to the primary's
(`CFG_H261_HOSTED_RANDOM_JASON`); spawn near the player on the offline ring (`CFG_H261_F1_NEAR_PLAYER`).

**EXAMPLE — not compiled or tested.**

```cpp
// EXAMPLE (server only)
AJasonCharacter* AUBJasonSpawner::SpawnExtraJason(const FVector& Near)
{
    FVector Loc; if (!FindNavmeshRingPoint(Near, Loc)) return nullptr;
    FActorSpawnParameters P; P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn; // donor: noCollisionFail
    AJasonCharacter* Pawn = GetWorld()->SpawnActor<AJasonCharacter>(PickJasonClass(), Loc, FRotator::ZeroRotator, P);
    if (!Pawn) return nullptr;
    Pawn->AIControllerClass = AUBJasonAIController::StaticClass();
    Pawn->SpawnDefaultController();                          // replaces SpawnActor(ai) + Possess
    SetJasonCharacterClassOnPlayerState(Pawn);               // so kills/ending treat it as Jason, not a counselor
    return Pawn;
}
```

Ownership rule from the mod: the primary Jason's globals are never swapped to the extra; each Jason has its own controller + state.

---

## 11. Offline vs hosted authority

Offline: the local process is both server and client. Hosted: the host machine is the server; all AI runs on the host; joiners are vanilla.
In the mod the distinguishing predicate is `HostedRoutineActive()`. Differences you must reproduce on the server:

* Where the Jason comes from (adopted slot vs spawned bot) and when it is "ours" (`HostedAdoptionOwnsDriver`).
* Counselor fill births use the hunt `RestartPlayer` path hosted, a helper pipeline offline.
* Hosted selects F1's class excluding the primary's, ignores a leftover offline class catalogue (`CFG_H293_HOSTED_STALE_CATALOGUE`).
* AI Jason sound blips are hidden from the host online (H2.86, LIVE).
* Hosted end: class restore at match end (`HostedClassRestorePolicy`), end freeze (`HostedEndFreezePolicy`).

In native code run all AI strictly with `HasAuthority()`; replication of the existing Jason/counselor classes is unchanged.

---

## 12. Match init, transitions, cleanup

* New world: `ResetBundleWorldState(now)` (`GameplayBundleRuntime.hpp:4801`) clears donor lane, knife, traps, emotes, convergence, burst, summary, etc.
* Epoch reset: `GameplayEpochReset.hpp` `BundleResetEpoch()` clears every per-match member (`CFG_EPOCH_CLEAR`).
* H2.75/H2.93: when a hosted world starts, a leftover offline epoch (also a finished match) is retired once per world (`HostedH1Runtime.hpp`, `LeftoverEpochDecision`);
  hosted start also clears startup-trap progress and sources (`HostedRoutineRuntime.hpp`).
* H2.86: pressing F1 in a new world discards the previous match's F1 state (`GameplayBundleRuntime.hpp:4016`).
* Verified live (H2.93/H2.94): offline -> online -> offline -> Challenges in one session without restart.

In UE4, per-match state belongs to GameMode/GameState/controller instances, which are destroyed with the world, so most of this disappears. The
requirement to carry over is the *behavioral* one: nothing from match N may influence match N+1 (timers, F1 state, trap sources, class catalogue).

---

## 13. Native C++ vs Blueprint / BT / data

| Concern | Recommended home |
|---|---|
| Per-tick Jason decision loop, timers, Morph ring, unstick, burst policy | **Native C++** (component on the Jason controller). Pure logic is transferable from `Core/`. |
| Target selection, block list | C++ (or BT service writing `TargetActor` to the Blackboard). |
| Move to target | BT `MoveTo` task (acceptance 85) or C++ `MoveToLocation`. |
| Speed burst, knife, grab kill triggers | C++ ability triggers; BT tasks that call them. Tuning values in data. |
| Held grab-kill slot choice | C++ (small state machine) or BT sub-tree. |
| Spawning, controller ownership, GameMode hooks | C++ (GameMode / spawner subsystem). |
| Menu rows and settings | UMG Blueprints + `USaveGame`/`UDataAsset`. |
| Difficulty and numbers (cooldowns, ranges) | Data asset (`UnrealIntegration/Configuration/UBBotConfig.h`). |
| Counselor convergence | BT/Blackboard additions + a C++ coordinator. |

---

## 14. What to delete in a native port

Everything whose only purpose is working around being outside the engine:

* Identity proofs (`SlotObservation`, `SameSlot`, `ObserveOwner`), read/call budgets (`Reader.cap`, `J21Timing`, `CallCap`), "starved probe" retries.
* GC retention (`Retention/`), root leases, resident root bank, poison diagnostics.
* Address/offset pins (`RuntimeProbe/*.generated.hpp`), code-byte checks, hook install/uninstall, injector/launcher.
* Report-struct publishing (replace with logs/stat counters).

---

## 15. Parity verification

Use the same checks the author used:

1. **Re-express the fixtures.** `Tests/` fixtures encode exact expected timings (burst 3 s / 7 s / 3 s chase; unstick 8 s / 12 s / 150 cm; held-kill
   slot rules; nav ring ordering). Turn them into UE4 Automation tests against your native classes.
2. **Compare counters** between a recorded reference run of the mod and your native build (names from `cfgbundle::Report`):
   * Bursts: `h285Starts`, `h285ChaseStarts`, `h285BurstRequests`, `h285BeginBlocked`, `h285Veto`.
   * Burst lead: `h286Lead` (led / unled) — released build measured 135/135 and 91/96 bursts aimed ahead.
   * Cut-off Morph: `h284Cutoff` (pursuitMorphs, led, noHeading, landedClose, landedRing).
   * Unstick: `h281Unstick` (waterEscapes/Refused, frozenEscapes/Refused), `h288FailedRings`, `h288OtherTargets`.
   * Target choice: `h286NearestChosen`, `h286NearestEmpty`, `h286NearestSkips`.
   * Kills: `h283Kill` (hits, kills, killsApplied, releasedAlive/Dead), `h283ClassApplied` for the F1 class.
   * F1 lifecycle: `h286F1NewMatchResets`.
   * Knife: `knifePresses`, `knifeReleases`, `knifeLaunches`, `knifeRefusals`; cars: `carChecks`, `carMoves`, `morphAttempts`.
3. **Scenario checklist** (all LIVE in VERIFIED-FEATURES): offline counselor vs AI Jason; offline Jason vs bots; hosted with a human joiner; F1 mid-match; F3 bots;
   mode switches in one session; Challenges edge walk-through and no out-of-bounds end; Jason spectating (F4).
4. **Known gaps to expect**: online summary screen may list bots late; primary may stall ~12 s at unreachable spots; do not claim F1 car, chat move, near-follow, melee assist,
   end-summary replay, Myers/Centered camera add-ons.
