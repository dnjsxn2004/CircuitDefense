// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CDCore.generated.h"

class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnCoreHealthChanged,
	float,
	CurrentHealth,
	float,
	MaxHealth
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(
	FOnCoreDestroyed
);

UCLASS()
class CIRCUITDEFENSE_API ACDCore : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACDCore();

	UFUNCTION(BlueprintCallable, Category = "Core")
	void ApplyCoreDamage(float DamageAmount);

	UFUNCTION(BlueprintPure, Category = "Core")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Core")
	bool IsCoreDestroyed() const;

	UPROPERTY(BlueprintAssignable, Category = "Core")
	FOnCoreHealthChanged OnCoreHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Core")
	FOnCoreDestroyed OnCoreDestroyed;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Core")
	TObjectPtr<UStaticMeshComponent> CoreMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Core")
	float CurrentHealth = 100.0f;

private:
	bool bCoreDestroyed = false;

	void HandleCoreDestroyed();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
