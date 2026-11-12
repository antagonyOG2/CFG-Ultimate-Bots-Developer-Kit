#pragma once
#include "CfgPortPolicy.hpp"
#include <array>
#include <cstdint>
#include <string_view>

namespace cfgport {
enum class Mode { OfflineCounselor, OfflineJason, Challenge, Online, Unknown };
enum class Skill { Low, Medium, High };
enum class Difficulty { Easy, Normal, Hard };
enum class Weapon { None, Random, Machete, Axe, Shotgun };
enum class Inventory { None, Random, PocketKnife, FirstAidSpray, Firecrackers, Map };
enum class ChallengeOption { Knives, Recharge, Movement, Stealth, Untimed,
    SkipReplayIntro, PreventEscapes, DisableBoundary, Count };
struct ChoiceList {
    // Owned by the future native catalogue provider; no class pointers/RVAs.
    const std::string_view* keys = nullptr;
    std::size_t count = 0;
    bool Contains(std::string_view key) const noexcept {
        if (!keys || key.empty()) return false;
        for (std::size_t i = 0; i < count; ++i) if (keys[i] == key) return true;
        return false;
    }
};
struct Catalogue {
    std::uint64_t generation = 0;
    ChoiceList counselors, jasons, maps, weathers;
    unsigned maxCounselors = 0; // Unknown CFG limits must not inherit stock limits.
};
struct Settings {
    Mode mode = Mode::Unknown;
    std::string_view counselor, jason, map, weather;
    unsigned totalCounselors = 1;
    Skill skill = Skill::High;
    Difficulty difficulty = Difficulty::Normal;
    unsigned minutes = 60; // Zero means unlimited, only in offline Jason mode.
    Weapon weapon = Weapon::None;
    Inventory inventory = Inventory::None;
    bool unlimitedKnives = false, fastRecharge = false, fastMovement = false;
    std::array<bool, static_cast<std::size_t>(ChallengeOption::Count)> challenges{};
};
inline bool Validate(const Settings& settings, const Catalogue& catalogue) noexcept {
    if (!catalogue.generation || !catalogue.maps.Contains(settings.map) ||
        !catalogue.jasons.Contains(settings.jason)) return false;
    if (settings.mode == Mode::Challenge) {
        // Challenge-native selections are separate from bots loadout/boost rows.
        return settings.weapon == Weapon::None && settings.inventory == Inventory::None &&
            !settings.unlimitedKnives && !settings.fastRecharge && !settings.fastMovement;
    }
    if (settings.mode != Mode::OfflineCounselor && settings.mode != Mode::OfflineJason)
        return false;
    if (catalogue.maxCounselors == 0 || settings.totalCounselors == 0 ||
        settings.totalCounselors > catalogue.maxCounselors ||
        !catalogue.weathers.Contains(settings.weather)) return false;
    if (settings.minutes != 30 && settings.minutes != 60 && settings.minutes != 90 &&
        !(settings.minutes == 0 && settings.mode == Mode::OfflineJason)) return false;
    if (static_cast<unsigned>(settings.skill) > static_cast<unsigned>(Skill::High) ||
        static_cast<unsigned>(settings.difficulty) > static_cast<unsigned>(Difficulty::Hard) ||
        static_cast<unsigned>(settings.weapon) > static_cast<unsigned>(Weapon::Shotgun) ||
        static_cast<unsigned>(settings.inventory) > static_cast<unsigned>(Inventory::Map)) return false;
    for (const bool enabled : settings.challenges) if (enabled) return false;
    if (settings.mode == Mode::OfflineCounselor)
        return catalogue.counselors.Contains(settings.counselor) &&
            !settings.unlimitedKnives && !settings.fastRecharge && !settings.fastMovement;
    return settings.weapon == Weapon::None && settings.inventory == Inventory::None;
}
enum class Lifecycle { Idle, Selecting, Committed, Playing };
class MatchSettings {
public:
    // Event-driven only: no tick/poll loop, game memory access or asset loading.
    void OnMenuOpened(std::uint64_t frontendIdentity) noexcept {
        Reset(); frontend_ = frontendIdentity;
        state_ = frontendIdentity ? Lifecycle::Selecting : Lifecycle::Idle;
    }
    bool Commit(const Compatibility& profile, const Catalogue& catalogue,
        const Settings& settings) noexcept {
        const Session session = settings.mode == Mode::Challenge ? Session::Challenge : Session::OfflineBots;
        if (state_ != Lifecycle::Selecting || !CanEnable(profile, session, Feature::OfflineSetup) ||
            !Validate(settings, catalogue)) return false;
        if ((settings.weapon != Weapon::None || settings.inventory != Inventory::None) &&
            !CanEnable(profile, session, Feature::Loadout)) return false;
        if ((settings.unlimitedKnives && !CanEnable(profile,session,Feature::JasonKnives)) ||
            (settings.fastRecharge && !CanEnable(profile,session,Feature::JasonRecharge)) ||
            (settings.fastMovement && !CanEnable(profile,session,Feature::JasonMovement))) return false;
        constexpr std::array<Feature, static_cast<std::size_t>(ChallengeOption::Count)> features{
            Feature::ChallengeKnives, Feature::ChallengeRecharge, Feature::ChallengeMovement,
            Feature::ChallengeStealth, Feature::ChallengeTimeout, Feature::ChallengeReplay,
            Feature::ChallengeEscapes, Feature::ChallengeBoundary};
        for (std::size_t i = 0; i < features.size(); ++i)
            if (settings.challenges[i] && !CanEnable(profile,session,features[i])) return false;
        selected_ = settings; generation_ = catalogue.generation;
        state_ = Lifecycle::Committed; return true;
    }
    bool OnMatchStarted(std::uint64_t worldIdentity, std::uint64_t catalogueGeneration) noexcept {
        if (state_ != Lifecycle::Committed || !worldIdentity || worldIdentity == frontend_ ||
            catalogueGeneration != generation_) return false;
        world_ = worldIdentity; state_ = Lifecycle::Playing; return true;
    }
    bool IsCurrentMatch(std::uint64_t worldIdentity) const noexcept {
        return state_ == Lifecycle::Playing && world_ == worldIdentity;
    }
    bool ChallengeOptionEnabled(std::uint64_t worldIdentity, ChallengeOption option) const noexcept {
        const auto index = static_cast<std::size_t>(option);
        return IsCurrentMatch(worldIdentity) && selected_.mode == Mode::Challenge &&
            index < selected_.challenges.size() && selected_.challenges[index];
    }
    void Reset() noexcept {
        selected_ = Settings{}; frontend_ = world_ = generation_ = 0; state_ = Lifecycle::Idle;
    }
    Lifecycle State() const noexcept { return state_; }
private:
    Settings selected_{};
    std::uint64_t frontend_ = 0, world_ = 0, generation_ = 0;
    Lifecycle state_ = Lifecycle::Idle;
};
} // namespace cfgport
