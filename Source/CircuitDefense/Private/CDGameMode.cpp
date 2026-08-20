// Fill out your copyright notice in the Description page of Project Settings.


#include "CDGameMode.h"
#include "CDGameState.h"
#include "CDPlayerController.h"

ACDGameMode::ACDGameMode()
{
	GameStateClass = ACDGameState::StaticClass();
	PlayerControllerClass = ACDPlayerController::StaticClass();
}

void ACDGameMode::StartPlay()
{
	Super::StartPlay();

	ACDGameState* CDGameState = GetGameState<ACDGameState>();

	if (IsValid(CDGameState))
	{
		CDGameState->SetGamePhase(ECDGamePhase::Preparation);

		UE_LOG(
			LogTemp,
			Log,
			TEXT("Circuit Defense prototype started")
		);
	}
}
