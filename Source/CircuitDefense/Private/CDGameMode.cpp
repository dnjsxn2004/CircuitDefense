// Fill out your copyright notice in the Description page of Project Settings.


#include "CDGameMode.h"
#include "CDGameState.h"
#include "CDPlayerController.h"

ACDGameMode::ACDGameMode()
{
	GameStateClass = ACDGameState::StaticClass();
	PlayerControllerClass = ACDPlayerController::StaticClass();

	FCDWaveConfig Wave1;
	Wave1.PreparationTime = 3.0f;
	Wave1.CombatTime = 5.0f;
	Wave1.SpawnCount = 5;
	Wave1.SpawnInterval = 2.0f;
	WaveConfig.Add(Wave1);
	
	FCDWaveConfig Wave2;
	Wave2.PreparationTime = 3.0f;
	Wave2.CombatTime = 7.0f;
	Wave2.SpawnCount = 8;
	Wave2.SpawnInterval = 1.5f;
	WaveConfig.Add(Wave2);
	
	FCDWaveConfig Wave3;
	Wave3.PreparationTime = 3.0f;
	Wave3.CombatTime = 9.0f;
	Wave3.SpawnCount = 12;
	Wave3.SpawnInterval = 1.0f;
	WaveConfig.Add(Wave3);

}

void ACDGameMode::BeginPlay()
{
	Super::BeginPlay();

	ActiveWaveIndex = 0;
	StartPreparation();

}

void ACDGameMode::StartPreparation()
{
	if (!WaveConfig.IsValidIndex(ActiveWaveIndex))
	{
		CompleteAllWaves();
		return;
	}

	ACDGameState* CDGameState = GetGameState<ACDGameState>();

	if (!IsValid(CDGameState))
	{
		UE_LOG(LogTemp, Error, TEXT("CDGameState를 찾을 수 없습니다."));
		return;
	}

	const FCDWaveConfig& CurrentConfig = WaveConfig[ActiveWaveIndex];

	CDGameState->SetCurrentWave(ActiveWaveIndex + 1);
	CDGameState->SetGamePhase(ECDGamePhase::Preparation);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Wave %d Preparation"),
		ActiveWaveIndex + 1
	);

	StartPhaseTimer(CurrentConfig.PreparationTime);

}

void ACDGameMode::StartCombat()
{
	if (!WaveConfig.IsValidIndex(ActiveWaveIndex))
	{
		CompleteAllWaves();
		return;
	}

	ACDGameState* CDGameState = GetGameState<ACDGameState>();

	if (!IsValid(CDGameState))
	{
		return;
	}

	const FCDWaveConfig& CurrentConfig = WaveConfig[ActiveWaveIndex];

	CDGameState->SetGamePhase(ECDGamePhase::Combat);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Wave %d Start - Spawn Count: %d"),
		ActiveWaveIndex + 1,
		CurrentConfig.SpawnCount
	);

	StartPhaseTimer(CurrentConfig.CombatTime);
}

void ACDGameMode::FinishWave()
{
	ACDGameState* CDGameState = GetGameState<ACDGameState>();

	if (!IsValid(CDGameState))
	{
		return;
	}

	CDGameState->SetGamePhase(ECDGamePhase::WaveClear);
	CDGameState->SetRemainingTime(0.0f);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Wave %d Clear"),
		ActiveWaveIndex + 1
	);

	// 웨이브 종료 화면을 보여줄 시간
	GetWorldTimerManager().SetTimer(
		PhaseTimerHandle,
		this,
		&ACDGameMode::StartNextWave,
		2.0f,
		false
	);
}

void ACDGameMode::StartNextWave()
{
	++ActiveWaveIndex;

	if (WaveConfig.IsValidIndex(ActiveWaveIndex))
	{
		StartPreparation();
	}
	else
	{
		CompleteAllWaves();
	}
}

void ACDGameMode::CompleteAllWaves()
{
	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);

	ACDGameState* CDGameState = GetGameState<ACDGameState>();

	if (IsValid(CDGameState))
	{
		CDGameState->SetRemainingTime(0.0f);
	}

	UE_LOG(LogTemp, Display, TEXT("All Waves Clear!"));
}

void ACDGameMode::StartPhaseTimer(float Duration)
{
	ACDGameState* CDGameState = GetGameState<ACDGameState>();

	if (!IsValid(CDGameState))
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);

	CDGameState->SetRemainingTime(Duration);

	GetWorldTimerManager().SetTimer(
		PhaseTimerHandle,
		this,
		&ACDGameMode::UpdatePhaseTimer,
		1.0f,
		true
	);
}

void ACDGameMode::UpdatePhaseTimer()
{
	ACDGameState* CDGameState = GetGameState<ACDGameState>();

	if (!IsValid(CDGameState))
	{
		GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
		return;
	}

	const float NewRemainingTime =
		CDGameState->RemainingTime - 1.0f;

	CDGameState->SetRemainingTime(NewRemainingTime);

	UE_LOG(
		LogTemp,
		Verbose,
		TEXT("Wave %d - Remaining Time: %.0f"),
		CDGameState->CurrentWave,
		CDGameState->RemainingTime
	);

	if (CDGameState->RemainingTime > 0.0f)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);

	switch (CDGameState->CurrentPhase)
	{
	case ECDGamePhase::Preparation:
		StartCombat();
		break;

	case ECDGamePhase::Combat:
		FinishWave();
		break;

	default:
		break;
	}
}