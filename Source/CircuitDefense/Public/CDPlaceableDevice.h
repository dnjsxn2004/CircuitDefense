// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CDDeviceType.h"
#include "CDPlaceableDevice.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class USceneComponent;
class UStaticMeshComponent;
class UWidgetComponent;
class UCDDeviceStatusWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnDevicePowerChanged,
	bool,
	bNewPowered
);

UCLASS()
class CIRCUITDEFENSE_API ACDPlaceableDevice : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACDPlaceableDevice();

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void CompletePlacement();

	UFUNCTION(BlueprintPure, Category = "Placement")
	bool IsInstalled() const;
	
	UFUNCTION(BlueprintPure, Category = "Placement")
	int32 GetInstallationCost() const;

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void SetPlacementPreview(bool bIsPreview);

	UFUNCTION(BlueprintPure, Category = "Placement")
	bool IsPlacementPreview() const;

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void SetPlacementValid(bool bIsValid);

	UFUNCTION(
		BlueprintPure,
		Category = "Placement"
	)
	int32 GetRefundAmount() const;

	UFUNCTION(BlueprintPure, Category = "Circuit")
	ECDDeviceType GetDeviceType() const;

	UFUNCTION(BlueprintPure, Category = "Circuit")
	bool IsPowered() const;

	UFUNCTION(BlueprintCallable, Category = "Circuit")
	void SetPowered(bool bNewPowered);

	UFUNCTION(BlueprintPure, Category = "Circuit")
	float GetConnectionRange() const;

	UPROPERTY(
		BlueprintAssignable,
		Category = "Circuit"
	)
	FOnDevicePowerChanged OnDevicePowerChanged;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Device")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Device")
	TObjectPtr<UStaticMeshComponent> DeviceMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Placement", meta = (ClampMin = "0"))
	int32 InstallationCost = 10;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Placement")
	bool bInstalled = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Placement")
	bool bPlacementPreview = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category ="Placement|Preview")
	TObjectPtr<UMaterialInterface> PreviewMaterialBase;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Placement|Preview")
	TObjectPtr<UMaterialInstanceDynamic> PreviewDynamicMaterial;

	UPROPERTY(Transient)
	TArray <TObjectPtr< UMaterialInterface>> OriginalMaterials;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Placement",
		meta = (
			ClampMin = "0.0",
			ClampMax = "1.0"
			)
	)
	float RefundRate = 0.5f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Circuit"
	)
	ECDDeviceType DeviceType =
		ECDDeviceType::Relay;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Circuit",
		meta = (ClampMin = "0.0")
	)
	float ConnectionRange = 500.0f;

	UPROPERTY(
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Category = "Circuit"
	)
	bool bPowered = false;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Device|UI"
	)
	TObjectPtr<UWidgetComponent>
		PowerStatusWidgetComponent;

private:
	void ApplyPreviewMaterial();
	void RestoreOriginalMaterials();

	UPROPERTY()
	TObjectPtr<UCDDeviceStatusWidget>
		PowerStatusWidget;

	void UpdatePowerStatusWidget();


};
