// EXAMPLE — not compiled or tested.
// Mirrors: Source/original-layout/EventObserver/NativeJasonControllerTickRuntime.hpp (BundleNativeJasonControllerTick,
// ServiceNativeJasonControllerTick) and Core/DonorJasonControllerTick.hpp (DonorRunOfflineBotsControllerTickOnGameThread).
// In the mod this is a detour; natively it is the controller itself.
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "UBJasonAIController.generated.h"

class UUBJasonPursuitComponent;
class UUBBotConfig;

UCLASS()
class AUBJasonAIController : public AAIController
{
    GENERATED_BODY()
public:
    AUBJasonAIController();

    /** Data asset with every tunable (see Configuration/UBBotConfig.h). */
    UPROPERTY(EditDefaultsOnly, Category="UB") TObjectPtr<UUBBotConfig> Config;

    /** All pursuit/burst/unstick/combat sequencing. Replaces the Core/ "Host" templates. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UB") TObjectPtr<UUBJasonPursuitComponent> Pursuit;

    /** True for the F1 additional Jason (separate state, same logic). Mirrors AdditionalHuntState vs the primary's ChaseState. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UB") bool bIsAdditionalJason = false;

    virtual void Tick(float DeltaSeconds) override;
    virtual void OnPossess(APawn* InPawn) override;

protected:
    /** Authority + match-in-progress + not-dead + not-in-cinematic. Replaces ResolveNativeJasonTickOwner / ObserveOwner. */
    bool ShouldDrive() const;
};
