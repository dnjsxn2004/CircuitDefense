// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CDPlaceableDevice.generated.h"

class USceneComponent;
class UStaticMeshComponent;

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

protected:
	// Called when the game starts or when spawned
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

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
