// Fill out your copyright notice in the Description page of Project Settings.

#include "CDGameState.h"

ACDGameState::ACDGameState()
{
	CurrentPhase = ECDGamePhase::Preparation;
	CurrentWave = 0;
	RemainingTime = 0.0f;
}

void ACDGameState::SetGamePhase(
	ECDGamePhase NewPhase
)
{
	if (CurrentPhase == NewPhase)
	{
		return;
	}

	CurrentPhase = NewPhase;

	OnGamePhaseChanged.Broadcast(CurrentPhase);

	UE_LOG(
		LogTemp,
		Log,
		TEXT("Game phase changed: %d"),
		static_cast<int32>(CurrentPhase)
	);
}

void ACDGameState::SetCurrentWave(
	int32 NewWave
)
{
	CurrentWave = FMath::Max(NewWave, 0);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Current wave changed: %d"),
		CurrentWave
	);
}

void ACDGameState::SetRemainingTime(
	float NewTime
)
{
	RemainingTime = FMath::Max(NewTime, 0.0f);
}