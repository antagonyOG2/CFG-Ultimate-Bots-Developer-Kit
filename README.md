# Camp Forest Green OFFLINE and ONLINE Bots MAXIMIZED

**Expanded offline bots and hosted online AI functionality for Camp Forest Green 3.5.**

*Developer Integration Kit — source reference for porting these systems into Camp Forest Green natively.*

## Overview

*Camp Forest Green OFFLINE and ONLINE Bots MAXIMIZED* is a community mod by AntagonyOG for Camp Forest Green 3.5 (an unofficial
community build of *Friday the 13th: The Game*, Unreal Engine 4). It adds an AI Jason, counselor bots, a second AI Jason on demand,
custom setup menus, hosted-lobby AI and Challenges changes. It currently works as an **external DLL** that is loaded into the game
by a helper, using reverse-engineered native addresses, reflected `UFunction` calls (ProcessEvent) and engine detours, with extensive
garbage-collector and object-identity safety layers.

This repository is **not the mod download.** It is the developer kit: the original source, architecture notes and porting guides, so that
the Camp Forest Green team can implement the same systems natively in their UE4 C++ source and Blueprints, with no external DLL.

## Major Features

Only features confirmed in real matches (release 1.0.0, see `Documentation/VERIFIED-FEATURES.md`) are listed as working:

* Offline: play as a counselor against AI Jason, or as Jason against counselor bots; hosted online lobbies (host runs the mod, joiners need nothing); Challenges mode; mode switching in one game session.
* AI Jason: pursuit, Morph (teleport ability) with running-counselor interception, Shift-style speed bursts with predictive aim, throwing knives, traps, door open/break, hiding-spot search, grab kills, car interception (primary Jason), water/stuck recovery, using the game's native underwater kill.
* F1 spawns an additional AI Jason (offline and hosted); F3 adds counselor bots (up to ten counselors).
* Counselor bots: convergence on Jason when the sweater/kill setup is active, emotes, sweater power (carried over from the original mod).
* Custom menus: expanded offline setup rows and hosted-lobby rows (Jason on/off, difficulty, fill empty slots, number of Jasons).
* Jason spectating (F4), Challenges edge walls removed, out-of-bounds no longer ends the match while new map areas stream in.

## Camp Forest Green Compatibility

* Target: Camp Forest Green **3.5**, `SummerCamp-Win64-Shipping.exe` SHA-256
  `4FC4BB544747FFF85EA03DCB83DE0367AC2F5AF1AE5355614608C737E01A90A6` (53,778,432 bytes). The mod refuses to load on other builds.
* Windows x64 only.
* Does not modify game files or save files.
* Unofficial fan-made project; not affiliated with or endorsed by the rights holders of *Friday the 13th*.

## Project Status

* Mod release **1.0.0** (built from internal test line H2.97). The player installer was built but, at the time of this snapshot, not yet live-tested as an install.
* **Nothing in this kit has been natively integrated or tested inside a CFG development environment.** See `PORTING-STATUS.md` for an honest per-system assessment.
* Disabled or unproven in 1.0.0 (not claimed as working): F1 Jason in a car, hosted lobby chat move, bot near-follow, bot melee assist, end-of-match summary and Challenges replay, Myers and Centered-Camera add-ons.

## Source Repository Purpose

* Transparency, security review, education and non-commercial community use (see `LICENSE`).
* A reference for CFG developers: the decision logic is in header-only policy templates (`Source/original-layout/Core/`) with fixture tests (`Tests/`);
  the runtime that glues them to the game (`EventObserver/`, `RuntimeProbe/`) shows exactly which engine calls each behavior needs.
* Much of the source (address pins, GC retention, detours, the loader) exists only because the mod is an external DLL and would be deleted in a native port.

> **Withheld from this public kit:** the external loader/injector, the player launcher, and all CFG 3.5 address tables and code signatures (addresses redacted in the remaining source). Not needed for a native port; the full source is available to the CFG team on request. See [WITHHELD.md](WITHHELD.md).

## Developer Integration

Start here:

1. `INTEGRATION-GUIDE.md` — 15 sections, system by system, with file/function citations and labeled example pseudocode.
2. `ARCHITECTURE.md` — how the current external-DLL runtime works.
3. `PORTING-STATUS.md` — what is ready, what needs native C++, what cannot transfer.
4. `Source/<Subsystem>/README.md` — indexes into the source (JasonAI, CounselorAI, Spawning, Navigation, Combat, Gameplay, Menus, MatchManagement, Hosted, Challenges, Retention, Loader).
5. `UnrealIntegration/` — **example** UE4 designs (headers/markdown, not compiled or tested).

Using this material inside CFG beyond the `LICENSE` terms requires the author's prior written permission (`THIRD-PARTY-NOTICES.md`).

## Build and Documentation Links

* `BUILDING.md` — how the DLL is built today (MSVC, C++17); the kit itself is a reference and is not expected to build standalone.
* `Source/SOURCE-MAP.md` — where each subsystem lives; `Source/original-layout/` keeps the original folders so relative includes still resolve.
* `Tests/README.md` — the policy fixtures and how to compile one.
* `Documentation/` — verified feature sheet, feature-flag table, build-script references.
* `CHANGELOG.md`, `GITHUB-RELEASE-NOTES.md`, `THIRD-PARTY-NOTICES.md`.

## License and Attribution

Released under the **F13 Ultimate Bots Source License** (see `LICENSE`): view, study and modify for personal non-commercial use; no sale, no commercial
redistribution, no repackaging as your own work, no commercial use, no removing attribution. Third-party components keep their own licenses
(`THIRD-PARTY-NOTICES.md`). Friday the 13th and related names, characters and assets belong to their respective owners.

## Support Development

Optional donations: https://www.paypal.com/donate/?hosted_button_id=RA3XBB6VDUTAC

**The mod will remain free for everyone.** Donations do not grant additional rights, ownership, access or licensing privileges.

Made by AntagonyOG — https://www.nexusmods.com/profile/antagonyOG
