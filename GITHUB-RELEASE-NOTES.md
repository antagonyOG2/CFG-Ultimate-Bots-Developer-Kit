# Camp Forest Green OFFLINE and ONLINE Bots MAXIMIZED — Developer Integration Kit

*Expanded offline bots and hosted online AI functionality for Camp Forest Green 3.5.*

## Overview
A source-reference kit for the Camp Forest Green team. It documents and includes the original source of the Bots MAXIMIZED mod (release 1.0.0) so its
AI Jason, counselor bots, second-Jason (F1), custom menus, hosted-lobby AI and Challenges changes can be ported **natively** into CFG's UE4 C++ source
and Blueprints, removing the need for an injected DLL.

## Contents
* `INTEGRATION-GUIDE.md` (15 sections), `ARCHITECTURE.md`, `PORTING-STATUS.md`, `BUILDING.md`, `CHANGELOG.md`, `THIRD-PARTY-NOTICES.md`, `LICENSE`.
* `Source/` — subsystem indexes plus the unmodified source in `Source/original-layout/` (Core, EventObserver, RuntimeProbe).
* `Tests/` — 19 standalone policy fixtures with a README.
* `UnrealIntegration/` — example UE4 designs (controller, component, BT task/service, blackboard keys, data asset, UMG row mapping). **Examples only: not compiled or tested.**
* `Documentation/` — verified feature sheet, feature-flag table, build-script references.

## Key Jason AI systems
* Pursuit with path lock, stuck/door escalation, Morph interception ahead of a running counselor, predictive aim during speed bursts.
* Speed burst (5x for 3 s, 7 s or 3 s cooldown), throwing knives, traps, door open/break, hiding-spot search.
* Held grab-kill state machine (slot probe, walk to open space, random slot), native underwater kill, car interception (primary).
* Never-frozen recovery for water and stuck positions.
* F1 additional Jason (offline and hosted) with its own state and controller.

## Counselor AI systems
* Bot counts up to ten, F3 additional bots, skill settings.
* Convergence on Jason with axe/machete when the sweater/kill setup is active (carried over from the original mod), emotes mirrored and staggered, sweater power.

## Menu systems
* Offline setup rows: counselor skill, counselors, Jason difficulty, match length, starting weapon and inventory, practice options.
* Hosted-lobby rows: Jason bot, difficulty, fill empty slots with bots, number of Jasons.

## Enhancements beyond the original (donor) mod
Release-line improvements H2.72-H2.97: predictive chase lead, Morph cut-off interception, short-range burst recovery, never-frozen water/stuck recovery,
F1 Jason fixes (Jason class on PlayerState, knife, swim, new-match reset), hosted F1, offline <-> online <-> Challenges mode switching in one session,
Challenges edge-wall removal and out-of-bounds fix, hidden AI Jason sound blips online. Details in `CHANGELOG.md`.

## Technical integration requirements
Server-authoritative AI in UE4 C++ (controller/component, navmesh queries, movement component, ability triggers); access to CFG's Jason, counselor,
interaction, trap and context-kill classes; UMG settings widgets. Pure policy logic transfers directly; the DLL-specific layers do not (`PORTING-STATUS.md`).

## Native porting status
**This is not a native .pak or a ready-to-drop-in plugin.** It is a reference kit. Nothing has been natively integrated or tested in a CFG development environment.

## Known limitations
* Disabled/unproven in 1.0.0: F1 Jason in a car, hosted chat move, bot near-follow, bot melee assist, end-summary and Challenges replay, Myers and Centered-Camera add-ons.
* Online summary screen may list bots late; the primary Jason may stall ~12 s at an unreachable spot before the frozen escape.
* The player install package was built but not yet live-tested as an install at the time of the snapshot.
* Jason kill by axe/machete in the stun window is a carried-over original-mod feature, not re-validated.

## Documentation links
`README.md` · `INTEGRATION-GUIDE.md` · `ARCHITECTURE.md` · `PORTING-STATUS.md` · `BUILDING.md` · `Source/SOURCE-MAP.md` · `Tests/README.md` · `Documentation/VERIFIED-FEATURES.md`

> **Withheld from this public kit:** the external loader/injector, the player launcher, and all CFG 3.5 address tables and code signatures (addresses redacted in the remaining source). Not needed for a native port; the full source is available to the CFG team on request. See [WITHHELD.md](WITHHELD.md).

## License and attribution
F13 Ultimate Bots Source License (see `LICENSE`): view, study, personal non-commercial modification and educational reference; no sale, commercial redistribution,
repackaging as your own work, commercial use or removal of attribution. Integration into CFG beyond the license needs the author's written permission.
Third-party code (MinHook) is not included and keeps its own license (`THIRD-PARTY-NOTICES.md`).
Friday the 13th and related names and assets belong to their respective owners; this is an unofficial fan-made project.

## Support development
Optional donations: https://www.paypal.com/donate/?hosted_button_id=RA3XBB6VDUTAC — **the mod will remain free for everyone.**
Made by AntagonyOG — https://www.nexusmods.com/profile/antagonyOG
