// EXAMPLE — not compiled or tested.
// Data asset collecting the tunables found in the mod's Core/ policies. Defaults are the values in the 1.0.0 source (cited per field).
// Rename UB* to your project's conventions. CFG's real settings classes should replace/extend this.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UBBotConfig.generated.h"

UENUM(BlueprintType)
enum class EUBJasonDifficulty : uint8 { Easy, Normal, Hard };

UCLASS(BlueprintType)
class UUBBotConfig : public UDataAsset
{
    GENERATED_BODY()
public:
    // ---- Chase / pursuit (Core/DonorJasonChase.hpp) ----
    UPROPERTY(EditAnywhere, Category="Chase") float MoveAcceptanceRadiusCm = 85.f;        // DonorIssueStockReflectedJasonMoveToLocation
    UPROPERTY(EditAnywhere, Category="Chase") float PathLockSeconds = 3.f;                 // ChaseState::PathLockDurationSeconds
    UPROPERTY(EditAnywhere, Category="Chase") int32 SamePathReuseMs = 6107;                // LastAcceptedMoveAt + 6107
    UPROPERTY(EditAnywhere, Category="Chase") int32 StuckCheckIntervalMs = 1375;           // NextStuckCheckAt
    UPROPERTY(EditAnywhere, Category="Chase") float StuckThresholdCm = 30.f;               // ChaseState::StuckThreshold
    UPROPERTY(EditAnywhere, Category="Chase") float RecoveryTeleportMinTargetDistCm = 3000.f; // targetDistance gate in stuck recovery
    UPROPERTY(EditAnywhere, Category="Chase") int32 MorphCooldownMs = 20000;               // ChaseState::MorphCooldownMs

    // ---- Chase lead during a burst (Core/ChaseLead.hpp) ----
    UPROPERTY(EditAnywhere, Category="Lead") float LeadSeconds = 2.f;
    UPROPERTY(EditAnywhere, Category="Lead") float LeadCapGapMultiplier = 1.5f;            // CFG_H295_LEAD_CAP_RELAX
    UPROPERTY(EditAnywhere, Category="Lead") float LeadOvershootCm = 180.f;
    UPROPERTY(EditAnywhere, Category="Lead") float LeadMinTargetSpeedCmS = 150.f;
    UPROPERTY(EditAnywhere, Category="Lead") int32 BurstReaimMs = 500;

    // ---- Cut-off Morph ring (Core/DonorJasonNavContinuation.hpp) ----
    UPROPERTY(EditAnywhere, Category="Morph") TArray<float> CutOffLandingDistancesCm = {700.f, 1000.f, 1400.f, 1800.f};
    UPROPERTY(EditAnywhere, Category="Morph") float MaxLandingDzCm = 120.f;
    UPROPERTY(EditAnywhere, Category="Morph") float RingMinDistanceCm = 1700.f;
    UPROPERTY(EditAnywhere, Category="Morph") float CutOffMinDistanceCm = 500.f;

    // ---- Speed burst (Core/OnFootBurstPolicy.hpp) ----
    UPROPERTY(EditAnywhere, Category="Burst") float BurstMultiplier = 5.f;
    UPROPERTY(EditAnywhere, Category="Burst") int32 BurstDurationMs = 3000;
    UPROPERTY(EditAnywhere, Category="Burst") int32 BurstCooldownMs = 7000;
    UPROPERTY(EditAnywhere, Category="Burst") int32 ChaseBurstCooldownMs = 3000;           // target within ChaseRangeCm at start
    UPROPERTY(EditAnywhere, Category="Burst") float ChaseRangeCm = 3000.f;
    UPROPERTY(EditAnywhere, Category="Burst") float BurstMinTargetDistanceCm = 400.f;      // H246BurstPolicy::BurstWanted

    // ---- Unstick (Core/DonorUnstick.hpp) ----
    UPROPERTY(EditAnywhere, Category="Unstick") int32 WaterEscapeMs = 8000;
    UPROPERTY(EditAnywhere, Category="Unstick") int32 FrozenMs = 12000;
    UPROPERTY(EditAnywhere, Category="Unstick") float FrozenRadiusCm = 150.f;
    UPROPERTY(EditAnywhere, Category="Unstick") int32 WaterRetryMs = 3000;
    UPROPERTY(EditAnywhere, Category="Unstick") int32 FrozenRechargeMs = 20000;
    UPROPERTY(EditAnywhere, Category="Unstick") int32 FailedTargetSkipMs = 30000;

    // ---- Target selection (Core/DonorJasonTargeting.hpp) ----
    UPROPERTY(EditAnywhere, Category="Target") int32 MaxTargets = 64;
    UPROPERTY(EditAnywhere, Category="Target") int32 FirstBlockMs = 90000;
    UPROPERTY(EditAnywhere, Category="Target") int32 RepeatBlockMs = 600000;

    // ---- Held grab kill (Core/DonorHeldGrabKill.hpp) ----
    UPROPERTY(EditAnywhere, Category="GrabKill") int32 KillSlots = 7;
    UPROPERTY(EditAnywhere, Category="GrabKill") float OpenDirectionProbeCm = 400.f;
    UPROPERTY(EditAnywhere, Category="GrabKill") int32 ExtractionWaitMs = 3000;
    UPROPERTY(EditAnywhere, Category="GrabKill") int32 LegacyFallbackMs = 9000;            // CFG_H276_HELD_WALK

    // ---- Settings defaults (Core/ModeSettingsPolicy.hpp) ----
    UPROPERTY(EditAnywhere, Category="Settings") int32 MaxCounselors = 10;
    UPROPERTY(EditAnywhere, Category="Settings") EUBJasonDifficulty Difficulty = EUBJasonDifficulty::Normal;
    UPROPERTY(EditAnywhere, Category="Settings") TArray<int32> MatchMinutes = {30, 60, 90};

    // ---- Feature switches (Core/J22FeatureFlags.hpp), 1.0.0 defaults ----
    UPROPERTY(EditAnywhere, Category="Features") bool bF1Knife = true;                     // CFG_F1_KNIFE
    UPROPERTY(EditAnywhere, Category="Features") bool bF1Car = false;                      // CFG_F1_CAR  (OFF for 1.0.0)
    UPROPERTY(EditAnywhere, Category="Features") bool bBotNearFollow = false;              // CFG_J23_BOT_NEAR_FOLLOW (OFF)
    UPROPERTY(EditAnywhere, Category="Features") bool bBotMeleeAssist = false;             // CFG_J23_BOT_MELEE_ASSIST (OFF)
};
