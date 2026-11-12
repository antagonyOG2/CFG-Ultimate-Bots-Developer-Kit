# Challenges — Challenges-mode changes (edge walls, escape boundary, intro)

Paths relative to `Source/original-layout/`.

| File | Role / key symbols |
|---|---|
| `Core/ChallengeEdgeWallsPolicy.hpp` | `cfgj36walls`: classification of `BlockingVolume*` actors in the Challenges map package (capacity 176, owner capacity 32). The invisible edge walls are intentionally disabled in 1.0.0 (walk through after about 1 s). |
| `Core/ChallengeEscapeBoundaryPolicy.hpp`, `ChallengeEscapeStartPolicy.hpp`, `ChallengeBoundaryWallCensusPolicy.hpp`, `ChallengeNativeScopePolicy.hpp`, `ChallengeStrongChildrenPolicy.hpp` | Escape/out-of-bounds boundary logic. H2.94: a Verify failure while the scope is proven and both functions are unchanged keeps the edit and re-verifies (bounded about 4 s) so newly streamed map areas no longer end the match. |
| `Core/ChallengeIntroDispatchPolicy.hpp`, `ChallengeIntroSkipPolicy.hpp` | Challenge intro (replay/skip flags OFF in 1.0.0). |
| `EventObserver/NativeChallengeBoundary*.hpp`, `NativeChallengeEdgeWalls*.hpp`, `NativeChallengeEscape*.hpp`, `NativeChallengeSettingsService.hpp`, `NativeChallengeOptions.hpp` | Runtime that edits the native boundary/escape functions and adds Challenges options. |
| `RuntimeProbe/ChallengeEdgeWallsBindings.generated.hpp` *(withheld from the public kit)*, `ChallengeEscapeRecovery.generated.hpp` *(withheld from the public kit)*, `ChallengeIntro.generated.hpp` *(withheld from the public kit)* | 3.5 address pins. |

Native port: simply do not spawn / disable collision on the boundary `BlockingVolume`s and do not end the match on out-of-bounds while streaming.
