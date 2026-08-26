#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CDCircuitManager.generated.h"

class ACDPlaceableDevice;

struct FCDPowerConnection
{
    TWeakObjectPtr<ACDPlaceableDevice> SourceDevice;
    TWeakObjectPtr<ACDPlaceableDevice> TargetDevice;
};

UCLASS()
class CIRCUITDEFENSE_API ACDCircuitManager
    : public AActor
{
    GENERATED_BODY()

public:
    ACDCircuitManager();

    virtual void Tick(
        float DeltaSeconds
    ) override;

    UFUNCTION(
        BlueprintCallable,
        Category = "Circuit"
    )
    void RefreshCircuit();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Circuit|Debug"
    )
    bool bDrawDebugConnections = true;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Circuit|Debug"
    )
    FColor ConnectionLineColor =
        FColor::Cyan;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Circuit|Debug",
        meta = (ClampMin = "0.0")
    )
    float ConnectionLineHeight = 50.0f;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Circuit|Debug",
        meta = (ClampMin = "0.1")
    )
    float ConnectionLineThickness = 5.0f;

private:
    void GatherInstalledDevices(
        TArray<ACDPlaceableDevice*>& OutDevices
    ) const;

    void DrawPowerConnections() const;

    TArray<FCDPowerConnection> ActiveConnections;
};