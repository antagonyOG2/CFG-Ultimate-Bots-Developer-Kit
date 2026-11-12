#pragma once
#include <cstdint>
#include "Compatibility/CfgBuildProfile.hpp"

namespace cfgprobe {
// Fixed to the inventoried CFG build. Evidence: native class registration data
// in Diagnostics/CFG-NativeFieldCatalogue.json. No stock/donor layout fallback.
inline constexpr auto& ExecutableSha256 = cfgcompat::cfg35::Sha256;
constexpr std::uint64_t ExecutableBytes = cfgcompat::cfg35::Identity.bytes;
constexpr std::uint32_t ExecutableTimestamp = cfgcompat::cfg35::Identity.peTimestamp;
constexpr std::uintptr_t ExecutableImageBytes = cfgcompat::cfg35::Identity.imageBytes;
constexpr std::uintptr_t EngineGlobalCandidate = cfgcompat::cfg35::engine::GlobalCandidateRva;
constexpr std::uintptr_t EngineGameViewport = cfgcompat::cfg35::engine::EngineGameViewport;
constexpr std::uintptr_t ViewportWorld = cfgcompat::cfg35::engine::ViewportWorld;
constexpr std::uintptr_t WorldNetDriver = cfgcompat::cfg35::engine::WorldNetDriver;
constexpr std::uintptr_t WorldGameMode = cfgcompat::cfg35::engine::WorldGameMode;
constexpr std::uintptr_t WorldGameState = cfgcompat::cfg35::engine::WorldGameState;
constexpr std::uintptr_t WorldGameInstance = cfgcompat::cfg35::engine::WorldGameInstance;
constexpr std::uintptr_t GameInstanceLocalPlayers = cfgcompat::cfg35::engine::GameInstanceLocalPlayers;
constexpr std::uintptr_t PlayerController = cfgcompat::cfg35::engine::PlayerController;
constexpr std::uintptr_t ControllerPlayer = cfgcompat::cfg35::engine::ControllerPlayer;
constexpr std::uintptr_t ControllerAcknowledgedPawn = cfgcompat::cfg35::engine::ControllerAcknowledgedPawn;
constexpr std::uintptr_t ControllerPawn = cfgcompat::cfg35::engine::ControllerPawn;
constexpr std::uintptr_t ControllerCameraManager = cfgcompat::cfg35::engine::ControllerCameraManager;
constexpr std::uintptr_t ControllerSpectatorPawn = cfgcompat::cfg35::engine::ControllerSpectatorPawn;
constexpr std::uintptr_t CounselorStats = cfgcompat::cfg35::engine::CounselorStats;
constexpr std::uintptr_t KillerTrapCount = cfgcompat::cfg35::engine::KillerTrapCount;
// Current CFG image disk evidence is in Diagnostics/CFG-IdentityBindings.json
// and Diagnostics/CFG-NativeFieldCatalogue.json. The engine global is still a
// candidate because GEngine3 and GWorld2 share one reference; native runtime
// identity/lifecycle remain unverified until the exact one-shot offline probe.
// This executable observes candidate pointers only; no feature is enabled.
} // namespace cfgprobe
