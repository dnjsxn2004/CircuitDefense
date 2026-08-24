// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "CDPlayerController.generated.h"

class ACDPlaceableDevice;
class UInputAction;
class UInputMappingContext;
class UCDHUDWidget;

/**
 * 
 */
UCLASS()
class CIRCUITDEFENSE_API ACDPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACDPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> CombatMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> StartPlacementAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> CancelPlacementAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "100.0"))
	float AttackRange = 1000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float AttackDamage = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Placement")
	TSubclassOf<ACDPlaceableDevice> DefaultPlaceableDeviceClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Placement", meta = (ClampMin = "100.0"))
	float PlacementTraceDistance = 5000.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Placement")
	TObjectPtr<ACDPlaceableDevice> PlacementPreview;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UCDHUDWidget> HUDWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UCDHUDWidget> HUDWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Placement")
	FVector PlacementCheckExtent = FVector(50.0f, 50.0f, 50.0f);

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Placement",meta = (ClampMin = "0.0",ClampMax = "1.0"))
	float MinimumPlacementNormalZ = 0.75f;


private:
	void HandleAttack(const FInputActionValue& InputActionValue);

	void HandleStartPlacement(const FInputActionValue& InputActionValue);

	void HandleCancelPlacement(const FInputActionValue& InputActionValue);

	void PerformAttackTrace();
	void UpdatePlacementPreview();
	void ConfirmPlacement();
	void CancelPlacement();

	bool bCanPlacePreview = false;

	bool IsPlacementLocationValid(const FHitResult& SurfaceHit) const;
};


