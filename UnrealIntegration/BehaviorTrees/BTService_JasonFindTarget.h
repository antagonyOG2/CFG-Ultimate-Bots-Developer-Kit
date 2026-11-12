// EXAMPLE — not compiled or tested.
// Mirrors Core/DonorJasonTargeting.hpp (Targets::DonorFindNearestJasonAICounselorTarget, block list) and, for the burst case,
// Core/ChaseLead.hpp (cfgchaselead::Lead) via TargetLocation.
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTService_JasonFindTarget.generated.h"

UCLASS()
class UBTService_JasonFindTarget : public UBTService
{
    GENERATED_BODY()
public:
    UBTService_JasonFindTarget() { Interval = 0.5f; RandomDeviation = 0.f; NodeName = TEXT("Jason Find Target"); }

    UPROPERTY(EditAnywhere) FBlackboardKeySelector TargetActorKey;      // Object
    UPROPERTY(EditAnywhere) FBlackboardKeySelector TargetLocationKey;   // Vector
    UPROPERTY(EditAnywhere) FBlackboardKeySelector BurstingKey;         // Bool

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    // TickNode:
    //  1. candidates = all living, un-escaped counselor pawns that have a controller (cap 64 in the mod)
    //  2. skip blocked (90 s first strike, 600 s repeat) and the excluded one; pick min 3-D squared distance
    //  3. TargetLocation = target location; if bursting and the target's ground speed >= 150 cm/s:
    //       lead = min(speed * 2 s, 1.5 * gap) + 180 cm, point = target + heading * lead      (re-evaluated every 500 ms)
    //  4. write keys; if nothing chosen, clear TargetActor (the mod records why: empty roster vs per-candidate skip reasons)
};
