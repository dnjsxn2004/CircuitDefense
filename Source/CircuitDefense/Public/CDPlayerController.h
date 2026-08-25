#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "CDGamePhase.h"
#include "CDPlayerController.generated.h"

class ACDPlaceableDevice;
class UCDHUDWidget;
class UInputAction;
class UInputMappingContext;
class ACDCircuitManager;

UCLASS()
class CIRCUITDEFENSE_API ACDPlayerController
    : public APlayerController
{
    GENERATED_BODY()

public:
    ACDPlayerController();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Input"
    )
    TObjectPtr<UInputMappingContext>
        CombatMappingContext;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Input"
    )
    TObjectPtr<UInputAction> AttackAction;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Input"
    )
    TSubclassOf<ACDPlaceableDevice> DefaultPlaceableDeviceClass;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Input"
    )
    TObjectPtr<UInputAction> CancelPlacementAction;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Input"
    )
    TObjectPtr<UInputAction> RemoveDeviceAction;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Combat",
        meta = (ClampMin = "100.0")
    )
    float AttackRange = 1000.0f;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Combat",
        meta = (ClampMin = "0.0")
    )
    float AttackDamage = 10.0f;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Placement",
        meta = (ClampMin = "100.0")
    )
    float PlacementTraceDistance = 5000.0f;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Placement"
    )
    FVector PlacementCheckExtent =
        FVector(50.0f, 50.0f, 50.0f);

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Placement",
        meta = (
            ClampMin = "0.0",
            ClampMax = "1.0"
            )
    )
    float MinimumPlacementNormalZ = 0.75f;

    UPROPERTY(
        Transient,
        BlueprintReadOnly,
        Category = "Placement"
    )
    TObjectPtr<ACDPlaceableDevice>
        PlacementPreview;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "UI"
    )
    TSubclassOf<UCDHUDWidget> HUDWidgetClass;

    UPROPERTY(Transient)
    TObjectPtr<UCDHUDWidget> HUDWidget;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Input"
    )
    TObjectPtr<UInputAction> SelectPowerSourceAction;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Input"
    )
    TObjectPtr<UInputAction> SelectRelayAction;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Input"
    )
    TObjectPtr<UInputAction> SelectAttackDeviceAction;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Placement|Device Classes"
    )
    TSubclassOf<ACDPlaceableDevice>
        PowerSourceDeviceClass;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Placement|Device Classes"
    )
    TSubclassOf<ACDPlaceableDevice>
        RelayDeviceClass;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Placement|Device Classes"
    )
    TSubclassOf<ACDPlaceableDevice>
        AttackDeviceClass;

    UPROPERTY(
    Transient
    )
    TObjectPtr<ACDCircuitManager> 
        CircuitManager;
private:
    void HandleAttack(
        const FInputActionValue& InputActionValue
    );

    void HandleSelectPowerSource(
        const FInputActionValue& InputActionValue
    );

    void HandleSelectRelay(
        const FInputActionValue& InputActionValue
    );

    void HandleSelectAttackDevice(
        const FInputActionValue& InputActionValue
    );

    void StartPlacement(
        TSubclassOf<ACDPlaceableDevice> DeviceClass
    );

    void HandleCancelPlacement(
        const FInputActionValue& InputActionValue
    );

    void HandleRemoveDevice(
        const FInputActionValue& InputActionValue
    );

    void PerformAttackTrace();
    void UpdatePlacementPreview();
    void ConfirmPlacement();
    void CancelPlacement();
    void TryRemoveDevice();

    bool IsPlacementLocationValid(
        const FHitResult& SurfaceHit
    ) const;

    bool bCanPlacePreview = false;

    UFUNCTION()
    void HandleGamePhaseChanged(
        ECDGamePhase NewPhase
    );
};