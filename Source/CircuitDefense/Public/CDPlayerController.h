// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "CDPlayerController.generated.h"

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> CombatMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "100.0"))
	float AttackRange = 1000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float AttackDamage = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UCDHUDWidget> HUDWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UCDHUDWidget> HUDWidget;

private:
	void HandleAttack(const FInputActionValue& InputActionValue);

	void PerformAttackTrace();

};


