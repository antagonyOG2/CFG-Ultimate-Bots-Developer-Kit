// EXAMPLE — not compiled or tested.
// Component form of the mod's chase/burst/unstick policies.
// Mirrors: Core/DonorJasonChase.hpp (ChaseState, DonorRunJasonAIChaseUpdateOnGameThread), Core/DonorJasonNavContinuation.hpp
// (NavRingContinuation, cfgcutoff::Tracker), Core/ChaseLead.hpp, Core/OnFootBurstPolicy.hpp, Core/DonorUnstick.hpp,
// Core/DonorJasonTargeting.hpp, Core/DonorHeldGrabKill.hpp (via UBGrabKillComponent, not shown).
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UBJasonPursuitComponent.generated.h"

class UUBBotConfig;

UENUM()
enum class EUBBurstStage : uint8 { Idle, Apply, Hold, Restore };   // cfgfootburst::Stage
UENUM()
enum class EUBUnstickAction : uint8 { None, NeedTarget, WaterEscape, FrozenEscape }; // cfgunstick::Action

/** Per-controller runtime state; all times are world-time milliseconds. */
USTRUCT()
struct FUBChaseState                                              // cfgdonorhunt::ChaseState
{
    GENERATED_BODY()
    bool bActive = false, bInitialTeleportDone = false, bPathLocked = false, bHaveLastLocation = false;
    uint64 InitialTeleportAt = 0, NextStuckCheckAt = 0, LastAcceptedMoveAt = 0, PathLockUntil = 0,
           DoorGraceUntil = 0, LastMorphAt = 0, NextDistanceTeleportAt = 0;
    int32 ConsecutiveStuckChecks = 0, DoorAttemptsWithoutProgress = 0;
    FVector LastLocation = FVector::ZeroVector;
    TWeakObjectPtr<AActor> Target;
};

UCLASS(ClassGroup=(UltimateBots), meta=(BlueprintSpawnableComponent))
class UUBJasonPursuitComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    /** One slice of work per controller tick: traps -> door break -> knife -> chase -> combat (order from DonorRunOfflineBotsControllerTickOnGameThread). */
    void TickOnce(uint64 NowMs);

    /** Nearest eligible counselor by 3-D squared distance, skipping blocked ones. Mirrors DonorFindNearestJasonAICounselorTarget. */
    AActor* FindNearestCounselor() const;
    void BlockTarget(AActor* Target, uint64 NowMs);               // 90 s first strike, 600 s repeat

    /** Chase update: initial teleport, distance Morph, stuck detection, path lock, MoveToLocation(acceptance 85 cm). */
    bool UpdateChase(uint64 NowMs);

    /** Predictive aim point while bursting (cfgchaselead::Lead). Returns false below walking pace. */
    bool ComputeLeadPoint(const FVector& GoalXY, const FVector& JasonXY, const FVector2D& GoalVelocity, FVector& OutPoint) const;

    /** Morph = navmesh-projected teleport near the target; cut-off order when the runner's heading is known. */
    bool TryMorphNear(AActor* Target, const FVector2D& HeadingOrZero);

    /** Speed burst: x5 on Walk/Sprint/Run/SlowRun for 3 s, restore compare-only, 7 s (3 s if target < 30 m) cooldown. */
    bool BeginBurst(uint64 NowMs, float TargetDistSq);
    void StepBurst(uint64 NowMs, bool bInterrupted);

    /** Water >= 8 s or stationary >= 12 s within 150 cm -> Morph near a counselor (cfgunstick). */
    EUBUnstickAction EvaluateUnstick(uint64 NowMs, bool bInWater);

protected:
    UPROPERTY() TObjectPtr<UUBBotConfig> Config;
    FUBChaseState Chase;
    EUBBurstStage BurstStage = EUBBurstStage::Idle;
    uint64 BurstUntil = 0, BurstReadyAt = 0;
    float OriginalSpeeds[4] = {0, 0, 0, 0};

    /** Weak registry of counselors (64 max in the mod). Strong refs are unnecessary in native code. */
    TArray<TWeakObjectPtr<AActor>> Candidates;
    TMap<TWeakObjectPtr<AActor>, TPair<uint64, uint8>> Blocked;   // until, strikes
};
