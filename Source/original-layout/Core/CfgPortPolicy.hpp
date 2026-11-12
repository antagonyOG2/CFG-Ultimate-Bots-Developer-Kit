#pragma once
#include <array>
#include <cstddef>

namespace cfgport {
enum class Session { Unknown, OfflineBots, Challenge, Online, Sandbox };
enum class Feature { OfflineSetup, AiJason, BotSpawn, Loadout, EmoteMirror, Spectator,
    MaskSweater, JasonKnives, JasonRecharge, JasonMovement, ChallengeKnives,
    ChallengeRecharge, ChallengeMovement, ChallengeStealth, ChallengeTimeout,
    ChallengeReplay, ChallengeEscapes, ChallengeBoundary, Count };
struct Compatibility {
    bool recognizedForReporting = false;
    bool permittedOfflineInterface = false;
    bool targetBindingsVerified = false;
    std::array<bool, static_cast<std::size_t>(Feature::Count)> capabilities{};
};
// Recognition alone never permits execution. No base-game offsets are inherited.
inline bool CanEnable(const Compatibility& profile, Session session, Feature feature) noexcept {
    const auto index = static_cast<std::size_t>(feature);
    if (index >= profile.capabilities.size() || !profile.recognizedForReporting ||
        !profile.permittedOfflineInterface || !profile.targetBindingsVerified ||
        !profile.capabilities[index]) return false;
    // Online support is a separate future integration, not enabled by an offline port.
    if (session != Session::OfflineBots && session != Session::Challenge) return false;
    const bool challengeOnly = index >= static_cast<std::size_t>(Feature::ChallengeKnives);
    return !challengeOnly || session == Session::Challenge;
}
} // namespace cfgport
