# UMG row mapping

**EXAMPLE — not compiled or tested.** The row titles, value lists and limits below are the real ones from the mod
(`Source/original-layout/EventObserver/FrontendBundleRuntime.hpp` ~line 691, `HostedSettingsSpecs.hpp`, `Core/ModeSettingsPolicy.hpp`,
`Core/CounselorPracticeSettings.hpp`, `Core/GameplayLimits.hpp`). Widget class names are placeholders for CFG's real settings widgets.

## Offline setup menu (counselor and Jason modes)
| Row title in mod | Widget suggestion | Backing value | Notes |
|---|---|---|---|
| Counselor skill | Combo (Low/Medium/High) | `Skill` | |
| Counselors | Combo 1..10 | `TotalCounselors` | stock limit is 7; mod allows 10 |
| Jason difficulty | Combo (Easy/Normal/Hard) | `Difficulty` | |
| Match length | Combo (30/60/90 min) | `Minutes` | unlimited (0) only when playing as Jason offline |
| Starting weapon | Combo (None/Random/Machete/Axe/Shotgun) | `Weapon` | counselor mode |
| Starting inventory | Combo (None/Random/Pocket Knife/First Aid Spray/Firecrackers/Map) | `Inventory` | counselor mode |
| Mute Pamela (keep sweater voice) | Toggle | bit `PamelaMute=1` | practice option |
| Mute Distant Bot Grab Sounds | Toggle | bit `GrabMute=2` | |
| Minimal HUD (keep objectives) | Toggle | bit `MinimalHud=4` | |
| Hard AI Jason Movement (2x) | Toggle | bit `HardMovement=8` | present in code; hidden in the 1.0.0 finishing build (`!CFG_J216_FINISHING`) |
| Bot Sweater Starts Convergence | Toggle | bit `BotSweater=16` | present in code; hidden in the 1.0.0 finishing build |

## Hosted lobby rows (host only)
| Row title in mod | Values | Backing field (`cfghostedoptions::Values`) |
|---|---|---|
| Online: Jason Bot | Off / On | `jason` |
| Online: Jason Bot Difficulty | Off / Easy / Normal / Hard | `difficulty` (0 = off) |
| Online: Fill Empty Slots with Counselor Bots | Off / On | `fill` |
| Online: Number of Jasons | 1 / 2 | `jasonCount` |

Behavior contract worth keeping: choices are process-lifetime preferences, **snapshotted once per match** (`cfghostedoptions::Match::Capture`),
so editing a row cannot change a birth in flight. `cfgport::Validate` shows the legal combinations (e.g. Challenge mode forbids loadout/boost rows).

Challenges options named in `ModeSettingsPolicy.hpp`: Knives, Recharge, Movement, Stealth, Untimed, SkipReplayIntro, PreventEscapes, DisableBoundary.
(Replay-intro skip and end-summary replay flags are OFF in 1.0.0.)
