#pragma once
#ifndef CFG_J22_CHALLENGES_RETENTION
#define CFG_J22_CHALLENGES_RETENTION 1 // owned retention/queue/tail + full-source gates pass; live unverified
#endif
#ifndef CFG_J23_BOT_NEAR_FOLLOW
#define CFG_J23_BOT_NEAR_FOLLOW 0 // donor keeps brain suspended to 220 cm; near-follow returned it at 600 cm where the stock tree flees
#endif
#ifndef CFG_H256_HOSTED_AXE
#define CFG_H256_HOSTED_AXE 1
#endif
#ifndef CFG_H256_HOSTED_F1_RESTART
#define CFG_H256_HOSTED_F1_RESTART 1
#endif
#ifndef CFG_H256_EMOTE_PRIORITY
#define CFG_H256_EMOTE_PRIORITY 1
#endif
#ifndef CFG_J23_BOT_MELEE_ASSIST
#define CFG_J23_BOT_MELEE_ASSIST 0 // native melee-task/equipped/facing evidence required first
#endif
#ifndef CFG_J23_EMOTE_READ_YIELD
#define CFG_J23_EMOTE_READ_YIELD 1
#endif
#ifndef CFG_J23_RADIO_STREAMING_RETRY
#define CFG_J23_RADIO_STREAMING_RETRY 1
#endif
#ifndef CFG_J23_GRAB_TIMING
#define CFG_J23_GRAB_TIMING 1
#endif
#ifndef CFG_J22_ALL_SOUND_BLIPS
#define CFG_J22_ALL_SOUND_BLIPS 1 // J23: native F1 presentation + replacement-human scope, owned fixtures pass
#endif
#ifndef CFG_J22_F1_CLOSE_HUMAN
#define CFG_J22_F1_CLOSE_HUMAN 1 // owned 10m policy fixture pass; effect boundaries still reprove actor
#endif
#ifndef CFG_J22_END_SUMMARY
#define CFG_J22_END_SUMMARY 0
#endif
#ifndef CFG_J22_CHALLENGE_REPLAY
#define CFG_J22_CHALLENGE_REPLAY 0 // gameplay intro retention provider is not yet verified
#endif
#ifndef CFG_H269_CHAT_MOVE
#define CFG_H269_CHAT_MOVE 0 // host lobby chat box moves down clear of the added option rows
#endif
#ifndef CFG_F1_KNIFE
#define CFG_F1_KNIFE 1 // F1 Jason stock knife throw, donor transcription
#endif
#ifndef CFG_F1_CAR
#define CFG_F1_CAR 0 // H2.72: off for release; live H2.71: 4 acquisitions, 0 hood/morph/extraction, took over hunting. Revisit after release.
#endif
#ifndef CFG_H275_OFFLINE_THEN_HOSTED
#define CFG_H275_OFFLINE_THEN_HOSTED 1 // offline match then hosted match in one process: retire the leftover offline bundle epoch before hosted takeover
#endif
#ifndef CFG_H274_OFFLINE_OWNER_BUDGET
#define CFG_H274_OFFLINE_OWNER_BUDGET 1 // offline, both Jasons prove their owner on a separate read budget; same checks, hosted paths untouched
#endif
#ifndef CFG_H278_OFFLINE_KNIFE_CAP
#define CFG_H278_OFFLINE_KNIFE_CAP 1 // offline knife tick reader cap 4096: H2.77 first abort still reads==1536 on a fresh 1536 reader
#endif
#ifndef CFG_H281_SPECTATOR_BUDGET
#define CFG_H281_SPECTATOR_BUDGET 1 // F4 Jason spectator step on its own 4096-read reader: live H2.80, 9 bots, 15 publish refusals at SpectatorCurrent with reads 1472/1536
#endif
#ifndef CFG_H281_UNSTICK
#define CFG_H281_UNSTICK 1 // Jason is never frozen: water >=8 s or still >=12 s/150 cm -> existing nav-ring teleport to a counselor (primary and F1)
#endif
#ifndef CFG_H268_F1_DOOR_PRESS
#define CFG_H268_F1_DOOR_PRESS 1 // F1 locked-door press: fresh proof, then his own press if the first never entered
#endif
#ifndef CFG_H267_F1_BURST
#define CFG_H267_F1_BURST 1 // F1 Jason gets his own speed burst, same timing as the primary
#endif
#ifndef CFG_H266_TARGET_FALLBACK
#define CFG_H266_TARGET_FALLBACK 1 // hosted primary lane uses the donor target when its own list is empty
#endif
#ifndef CFG_H266_HIDER_APPROACH
#define CFG_H266_HIDER_APPROACH 1 // hosted Jasons walk to the hiding spot of the counselor they hunt, at any distance
#endif
#ifndef CFG_H266_F1_SWIM
#define CFG_H266_F1_SWIM 1 // F1 Jason uses the native underwater kill within 6 m
#endif
#ifndef CFG_H265_F1_RETRY
#define CFG_H265_F1_RETRY 1 // hosted F1: early spawn gates retry briefly; a later press works after a clean failure
#endif
#ifndef CFG_H264_F1_INTERACTIONS
#define CFG_H264_F1_INTERACTIONS 1 // F1 Jason opens or breaks doors and searches occupied hiding spots, as the donor F1 does
#endif
#ifndef CFG_H264_HIDING_ROSTER
#define CFG_H264_HIDING_ROSTER 1 // hosted primary searches the nearest occupied hiding spot from the full roster, as the donor does
#endif
#ifndef CFG_H263_F1_DONOR_HUNT
#define CFG_H263_F1_DONOR_HUNT 1 // hosted F1 Jason runs the same donor native hunting lane as offline
#endif
#ifndef CFG_H263_F1_ROSTER_TARGETS
#define CFG_H263_F1_ROSTER_TARGETS 1 // when that lane lapses, the hosted fallback drive uses the same target rule over the full roster
#endif
#ifndef CFG_H262_F1_SOUND_BLIPS
#define CFG_H262_F1_SOUND_BLIPS 1 // hosted F1 Jason clears its sound blips like the primary
#endif
#ifndef CFG_H261_F1_NEAR_PLAYER
#define CFG_H261_F1_NEAR_PLAYER 1 // hosted F1 spawns near the player on the offline ring instead of at the Jason cabin
#endif
#ifndef CFG_H259_HOSTED_GRANT_SUBMISSION
#define CFG_H259_HOSTED_GRANT_SUBMISSION 1
#endif
#ifndef CFG_H259_EMOTE_KEEP_QUEUE
#define CFG_H259_EMOTE_KEEP_QUEUE 1 // donor never clears the mirrored queue on a human cancel
#endif
#ifndef CFG_H258_HOSTED_F1_SPAWN
#define CFG_H258_HOSTED_F1_SPAWN 1 // hosted F1: donor SpawnActor of the Jason pawn at a native SCKillerPlayerStart + Possess, never RestartPlayer
#endif
#ifndef CFG_H261_HOSTED_RANDOM_JASON
#define CFG_H261_HOSTED_RANDOM_JASON 1 // hosted F1: random resident Jason class from the mode's KillerCharacterClasses that is not the primary's
#endif
#ifndef CFG_H261_CHAT_MEASURE
#define CFG_H261_CHAT_MEASURE 0 // strictly read-only host lobby chat/option layout report
#endif
#ifndef CFG_H276_HELD_WALK
#define CFG_H276_HELD_WALK 1 // held grab: legacy fallback only after 9 s, struggling no longer skips the walk, one second walk (not back) before release
#endif
#ifndef CFG_H276_SOUND_EMITTER
#define CFG_H276_SOUND_EMITTER 1 // donor continuous UpdateSoundBlips suppression: install the detour in the primary gameplay build; suppress the adopted/F1 AI Jason emitter online (hosted routine only)
#endif
#ifndef CFG_H277_OFFLINE_KNIFE
#define CFG_H277_OFFLINE_KNIFE 1 // offline primary knife Tick on its own read budget (first abort: Windup, owner proved, observe read failed at reads==cap)
#endif
#ifndef CFG_H277_HELPER_SPAWN_SAFE
#define CFG_H277_HELPER_SPAWN_SAFE 1 // helper SpawnActor that returned and published its result under proof, then met a post-call lookup veto: keep the root bank healthy, continue the birth; veto diagnostics
#endif
#ifndef CFG_H279_WALL_LIST_REFRESH
#define CFG_H279_WALL_LIST_REFRESH 1 // Challenges edge walls: level actor TArray growth/relocation between sampling and applying no longer fails membership (bounded unique rescan); first failing Member clause recorded
#endif
#ifndef CFG_H280_KILL_MONTAGE_GUARD
#define CFG_H280_KILL_MONTAGE_GUARD 1 // held grab: after a returned kill input the lane stays hands-off (no stale release / walk / second kill) for 12 s while the victim is still held and not dead; the 5 s window cut a 6.7 s kill montage
#endif
#ifndef CFG_H280_KILL_MONTAGE_MS
#define CFG_H280_KILL_MONTAGE_MS 12000
#endif
#ifndef CFG_H282_F1_OFFLINE_NO_HELD_KILL
#define CFG_H282_F1_OFFLINE_NO_HELD_KILL 1 // held grab: the offline F1 (additional) Jason never sends the grab-kill input (it left the victim half-dead, match broken); it releases the victim alive instead, like the no-slot cap path
#endif
#ifndef CFG_H295_LEAD_CAP_RELAX
#define CFG_H295_LEAD_CAP_RELAX 1 // burst lead may reach 1.5x the Jason-counselor gap (was 1x), plus the 6 ft overshoot
#endif
#ifndef CFG_H295_HEADING_FALLBACK
#define CFG_H295_HEADING_FALLBACK 1 // no fresh running heading: use the last measured running heading if at most 2 s old (live H2.94 offline: 67 of 79 burst moves unled)
#endif
#ifndef CFG_H293_LEFTOVER_ANY
#define CFG_H293_LEFTOVER_ANY 1 // H2.75 leftover-epoch retire also covers a finished offline match (was: only one still in progress), so offline->online gets the same full reset online->offline does
#endif
#ifndef CFG_H293_HOSTED_TRAP_RESET
#define CFG_H293_HOSTED_TRAP_RESET 1 // hosted routine start clears startup-trap progress and trap sources left by a finished offline match (live: no trap placements online after offline)
#endif
#ifndef CFG_H293_HOSTED_STALE_CATALOGUE
#define CFG_H293_HOSTED_STALE_CATALOGUE 1 // hosted F1 class selection ignores an offline match's leftover class catalogue (offline then online: F1 refused, no compatible resident class)
#endif
#ifndef CFG_H292_ROOT_PROVEN_SKIP
#define CFG_H292_ROOT_PROVEN_SKIP 1 // callback retention: skip the locked GC root window when every value is already a claim of this lease proved after the last native entry (live H2.91: ~8 windows/tick, ~0.83 ms each)
#endif
#ifndef CFG_H288_UNSTICK_OTHER_TARGET
#define CFG_H288_UNSTICK_OTHER_TARGET 1 // water/frozen escape: if every landing around the chosen counselor failed, the next try lands near a different counselor (live: primary in water, 16/16 cells unwalkable, retried every 3 s)
#endif
#ifndef CFG_H286_F1_NEW_MATCH
#define CFG_H286_F1_NEW_MATCH 1 // F1 press in a new world clears the previous match's F1 state (it stayed Active and refused every press)
#endif
#ifndef CFG_H286_BLIP_VETO
#define CFG_H286_BLIP_VETO 1 // AI Jason sound-blip suppression uses the read-only lookup veto: the game's UpdateSoundBlips runs outside any mod lookup scope, so LookupSafe refused every call (live H2.85: 857 entries, Gate=3)
#endif
#ifndef CFG_H284_MORPH_CUTOFF
#define CFG_H284_MORPH_CUTOFF 1 // pursuit Morph (distance/50 m) with a running counselor: first try 7-18 m straight ahead of the runner, then the donor ring most-ahead first (cut-off, not 22 m behind/aside)
#endif
#ifndef CFG_H283_F1_KILLER_CLASS
#define CFG_H283_F1_KILLER_CLASS 1 // offline F1 PlayerState gets the primary Jason's ActiveCharacterClass through the verified consuming setter, as the hosted F1 does (HostedPrepareAi); live: offline F1 listed as a counselor, kills scored as betrayal, held kills left victims half-dead. H2.82 release stays only as the fallback when the class was not applied
#endif
#ifndef CFG_H283_F1_KILL_DIAG
#define CFG_H283_F1_KILL_DIAG 1 // report only, both Jasons: counselor TakeDamage calls with this Jason as causer (kill-sized = native ForceKillCharacter) and the native EndAction kill gates read once per committed hold; no behaviour change, H2.82 untouched
#endif
#ifndef CFG_H276_CHALLENGE_WALLS
#define CFG_H276_CHALLENGE_WALLS 1 // Challenges edge walls: record which frontend gate refused (diagnostic)
#endif
