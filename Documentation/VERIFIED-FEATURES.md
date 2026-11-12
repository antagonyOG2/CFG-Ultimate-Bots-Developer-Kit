# Verified feature sheet — CFG 3.5 Ultimate Bots MAXIMIZED, release 1.0.0 (built from H2.97, commit 0f84e0ea)

Source of truth for every public claim in this kit. "LIVE" = confirmed in real matches by the author's in-game captures. Anything not listed
here, or listed under OFF/UNPROVEN, must not be described as a working feature.

## Release package (player install) — built, NOT yet live-tested as an install
- CFG-Offline-Bots-1.0.0.zip: folder CFG-Offline-Bots with INSTALL.bat, UNINSTALL.bat, "CFG Offline Bots.exe" (windowless launcher),
  CFG-Offline-Bots-Loader.exe (hidden helper), Cfg35PrimaryGameplay.dll (gameplay), README.txt,
  CHECKSUMS-SHA256.txt, Install.ps1, Uninstall.ps1 (version + supported game hash are built into Install.ps1).
- INSTALL.bat: finds CFG (parent folder, running game, Steam libraries on ANY drive via registry + libraryfolders.vdf, any fixed drive
  <drive>\*\CampForestGreenLauncher\Games\*), asks if several, accepts a pasted folder; copies the mod into <CFG>\CFG-Offline-Bots,
  verifies checksums, unblocks downloaded files, warns on unsupported game version, creates Desktop + Start-menu "CFG Offline Bots".
- Launcher: checks layout/files/game SHA-256; backs up %LOCALAPPDATA%\CampForestGreen\Saved\{SaveGames,Config} to
  CFG-Offline-Bots\Backups (newest 10); starts Start_CampForestGreen.exe; loader waits read-only and installs the mod ONCE when the
  player opens Offline Play (required once per game launch, also before hosting online).
- Never modifies game files or saves. UNINSTALL removes shortcuts + mod folder (optional save-backup copy to Desktop).
- Supported game: CFG 3.5, SummerCamp-Win64-Shipping.exe SHA-256 4FC4BB544747FFF85EA03DCB83DE0367AC2F5AF1AE5355614608C737E01A90A6
  (53,778,432 bytes); Steam app id in steam_appid.txt 1782210. Other builds: launcher refuses to load the mod (can start game without it).
- Windows only (x64). No extra runtimes (static CRT).

## Modes (LIVE)
- Offline counselor vs AI Jason; offline Jason vs counselor bots (donor menus); hosted online lobbies (host runs the mod; joiners vanilla);
  Challenges mode. Offline -> online -> offline -> Challenges in ONE game session without restart (LIVE H2.93/H2.94).
- F1: spawn additional AI Jason (offline and hosted). F3: add counselor bots. (Hosted F8 arm / F9 disarm exist in code from donor;
  current flow arms automatically — do NOT document F8/F9 unless the author confirms.)

## Jason AI (LIVE unless noted)
- Primary + F1 Jason both hunt with the donor native lane: pursuit, Morph, Shift-style speed bursts, throwing knives, traps (startup trap
  placement), door open/break, hiding-spot search, grab kills, car interception (primary; F1 car OFF).
- Grab kills apply death correctly for both Jasons offline and online ("You died"); F1 offline fixed by giving its PlayerState the Jason
  class (it was treated as a counselor: betrayal/half-dead victims) — H2.83 LIVE.
- Speed burst: 5x on-foot burst 3 s; within 30 m recovery 3 s (was 7 s); pending knife probe no longer blocks it — H2.85 LIVE.
- Predictive chase: during a burst Jason aims 2 s ahead of the running counselor (cap 1.5x the gap) + 6 ft overshoot, re-aims every 500 ms
  (donor held one point ~6 s); last running heading used up to 2 s when fresh one missing — H2.86/H2.95 LIVE (135/135, 91/96 bursts aimed ahead).
- Morph interception: pursuit Morph lands 7-18 m AHEAD of a running counselor first, then the ring most-ahead first — H2.84 LIVE (LandedClose).
- Never-frozen recovery: water >=8 s or stuck >=12 s within 150 cm -> nav-ring teleport near a counselor; if every landing near that
  counselor is unwalkable, next try uses another counselor (H2.88/H2.90) — LIVE (frozen escapes reached and killed Tommy; water escape LIVE).
- F1 swim: F1 Jason uses the game's native underwater (water) kill within 6 m (CFG_H266_F1_SWIM). Primary: donor water handling.
  Describe only as "Jason can follow into water and use the game's underwater kill"; no new kill animations were added.
- Jason spectating (F4 / spectator view of AI Jason) — LIVE (works best when most counselors are dead).
- AI Jason sound blips hidden from the host online (H2.86 LIVE).
- F1 spawns reliably after mode switches (H2.86/H2.93 LIVE).

## Counselor bots (donor systems carried over, LIVE in matches)
- Counselor bots, F3 additional bots, convergence/teaming on Jason when the sweater/kill setup is active, emotes (Wave/urination mirror),
  escape attempts, sweater power. Jason kill (axe/machete in stun window) is a DONOR feature carried over; not re-validated today —
  describe as carried over from the original mod.

## Challenges (LIVE)
- Invisible edge walls intentionally removed (walk through after ~1 s) — author's wish.
- Out-of-bounds no longer ends the match when new map areas stream in (H2.94 LIVE).

## Hosted (LIVE)
- Host as counselor with human joiners + AI Jason + bots; Tommy (second-PC player) hunted; traps, knives, car pursuit, grab kills online.

## OFF / UNPROVEN — do not advertise as working
- F1 Jason driving/occupied car (CFG_F1_CAR 0, off for release). Hosted chat move (0). Bot near-follow/melee assist (0). End summary/challenge
  replay flags (0). Myers add-on, Centered Camera add-on (future). Online summary screen may list bots late (known). Primary may stall
  ~12 s at an unreachable spot before the frozen escape (known). Joiners do not need the mod; remote Jason-spectating extensions need it.

## Credits facts
- Author: AntagonyOG (Nick Vasallo). Donor: "F13 Ultimate Bots Mode Offline and Online Expanded" / F13 Offline Bots Complete Edition 1.0.0-rc1,
  made by Antagony (same author) — the original mod's release README. Third-party: MinHook (BSD-2), used by the gameplay DLL.
- Donation link: https://www.paypal.com/donate/?hosted_button_id=RA3XBB6VDUTAC . Nexus profile: https://www.nexusmods.com/profile/antagonyOG
