// Fill out your copyright notice in the Description page of Project Settings.


#include "CDGameState.h"

ACDGameState::ACDGameState()
{
	CurrentPhase = ECDGamePhase::Preparation;
	CurrentWave = 0;
}

void ACDGameState::SetGamePhase(ECDGamePhase NewPhase)
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
		TEXT("Game phase chaged: %d"),
		static_cast<int32>(CurrentPhase)
	);
}

void ACDGameState::SetCurrentWave(int32 NewWave)
{
}

void ACDGameState::SetRemainingTime(float NewTime)
{
}
