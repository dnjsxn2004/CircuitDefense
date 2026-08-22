// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "CDPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> CombatMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "100.0"))
	float AttackRange = 1000.0f;

private:
	void HandleAttack(const FInputActionValue& InputActionValue);

	void PerformAttackTrace();

};


