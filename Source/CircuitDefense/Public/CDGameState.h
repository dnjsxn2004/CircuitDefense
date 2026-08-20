// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CDGamePhase.h"
#include "CDGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnGamePhaseChanged,
	ECDGamePhase,
	NewPhase
);

UCLASS()
class CIRCUITDEFENSE_API ACDGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	ACDGameState();

	UPROPERTY(BlueprintReadOnly, Category = "Game Flow")
	ECDGamePhase CurrentPhase = ECDGamePhase::Preparation;

	UPROPERTY(BlueprintReadOnly, Category = "Wave")
	int32 CurrentWave = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Game Flow")
	FOnGamePhaseChanged OnGamePhaseChanged;

	UFUNCTION(BlueprintCallable, Category = "Game State")
	void SetGamePhase(ECDGamePhase NewPhase);

	UPROPERTY(BlueprintReadOnly, Category = "Game State")
	float RemainingTime = 0.0f;

	void SetCurrentWave(int32 NewWave);
	void SetRemainingTime(float NewTime);


};
