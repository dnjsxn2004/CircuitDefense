#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CDCircuitManager.generated.h"

class ACDPlaceableDevice;

UCLASS()
class CIRCUITDEFENSE_API ACDCircuitManager
    : public AActor
{
    GENERATED_BODY()

public:
    ACDCircuitManager();

    UFUNCTION(
        BlueprintCallable,
        Category = "Circuit"
    )
    void RefreshCircuit();

protected:
    virtual void BeginPlay() override;

private:
    void GatherInstalledDevices(
        TArray<ACDPlaceableDevice*>& OutDevices
    ) const;
};