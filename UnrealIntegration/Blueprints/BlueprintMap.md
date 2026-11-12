# Blueprint vs C++ map

**EXAMPLE — not compiled or tested.** Recommendations for CFG's team; not requirements.

| System | Blueprint-friendly? | Recommendation |
|---|---|---|
| Settings rows (UMG) | Yes | UMG widgets bound to a settings object. See `../UI/BotMenuRows.md`. |
| Difficulty and tuning numbers | Yes | `UUBBotConfig` data asset (`../Configuration/UBBotConfig.h`) editable in editor. |
| Behavior tree layout | Yes | BT/Blackboard assets (`../BehaviorTrees/BT_JasonHunt.md`). |
| BT task/service bodies (target scan, burst, morph) | Partially | C++ nodes with BP-exposed properties; heavy loops stay in C++. |
| Speed burst timing and restore | No | C++ (compare-only restore, interruption handling). |
| Nav ring / Morph cut-off candidate ordering | No | C++ (deterministic ordering, navmesh queries). |
| Unstick (water/frozen) | No | C++ (position sampling). |
| Held grab-kill slot state machine | No | C++ (or one BT sub-tree); slot choice must be uniform-random, not the previous slot. |
| F1 spawn (Jason pawn + controller + possess + PlayerState class) | No | C++ in GameMode/spawner; server only. |
| Hosted-lobby option rows | Yes | UMG + replicated lobby settings. |
| Convergence | Partially | C++ coordinator + BT hooks. |
| Emote mirroring / RB wheel | Yes | Existing emote Blueprints; stagger bot responses. |
| Challenges edge walls | Yes | Disable collision / remove the boundary BlockingVolumes in the Challenges level data. |
| Status label ("AI JASON", gameplay DLL) | Yes | HUD/UMG text; no D3D hook. |
| Match start/end resets | No | Per-match state in GameState/GameMode components. |
