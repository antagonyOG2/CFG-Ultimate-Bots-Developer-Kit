// EXAMPLE — not compiled or tested.
// Mirrors Core/OnFootBurstPolicy.hpp (cfgfootburst::Policy) and Core/H246BurstPolicy.hpp (BurstWanted).
// A latent task: Begin -> hold 3 s -> restore. Interrupted bursts still restore (compare-only restore).
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_JasonSpeedBurst.generated.h"

UCLASS()
class UBTTask_JasonSpeedBurst : public UBTTaskNode
{
    GENERATED_BODY()
public:
    UBTTask_JasonSpeedBurst() { bNotifyTick = true; bNotifyTaskFinished = true; NodeName = TEXT("Jason Speed Burst"); }

    // Defaults copied from the mod; prefer reading them from UUBBotConfig.
    UPROPERTY(EditAnywhere) float Multiplier = 5.f;
    UPROPERTY(EditAnywhere) float DurationSeconds = 3.f;
    UPROPERTY(EditAnywhere) float CooldownSeconds = 7.f;
    UPROPERTY(EditAnywhere) float ChaseCooldownSeconds = 3.f;      // when target within ChaseRangeCm at start
    UPROPERTY(EditAnywhere) float ChaseRangeCm = 3000.f;
    UPROPERTY(EditAnywhere) float MinTargetDistanceCm = 400.f;

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;   // must restore speeds

private:
    // ExecuteTask: refuse if not ready or any original speed is non-finite / outside 25..2500 (the mod refuses and retries after 1 s),
    //              save Walk/Sprint/Run/SlowRun, write value*Multiplier, return InProgress.
    // TickTask:    after DurationSeconds (or on interruption) restore ONLY values still equal to our boosted value (a value the
    //              game changed meanwhile is left alone), set ready-at = now + cooldown, FinishLatentTask(Succeeded).
};
