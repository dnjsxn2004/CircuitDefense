// Fill out your copyright notice in the Description page of Project Settings.


#include "CDGameMode.h"
#include "CDEnemy.h"
#include "CDGameState.h"
#include "CDPlayerController.h"
#include "CDPlayerState.h"
#include "CDWaveSpawner.h"
#include "CDCore.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ACDGameMode::ACDGameMode()
{
	GameStateClass =
		ACDGameState::StaticClass();

	PlayerControllerClass =
		ACDPlayerController::StaticClass();

	PlayerStateClass =
		ACDPlayerState::StaticClass();

}

void ACDGameMode::PostLogin(
	APlayerController* NewPlayer
)
{
	Super::PostLogin(NewPlayer);

	InitializePlayerRespawnSystem(
		Cast<ACDPlayerController>(NewPlayer)
	);
}

void ACDGameMode::EndPlay(
	const EEndPlayReason::Type EndPlayReason
)
{
	ClearPlayerRespawnTimers();

	if (IsValid(CachedPlayerState))
	{
		CachedPlayerState->OnPlayerDeath.RemoveDynamic(
			this,
			&ACDGameMode::HandlePlayerDeath
		);
	}

	CachedPlayerController = nullptr;
	CachedPlayerState = nullptr;

	Super::EndPlay(EndPlayReason);
}

void ACDGameMode::BeginPlay()
{
	Super::BeginPlay();

	ActiveWaveIndex = 0;

	if (WaveConfig.Num() == 0)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"WaveConfig is empty - "
				"Configure waves in BP_CDGameMode"
			)
		);

		return;
	}

	for (
		int32 WaveIndex = 0;
		WaveIndex < WaveConfig.Num();
		++WaveIndex
		)
	{
		const FCDWaveConfig& Config =
			WaveConfig[WaveIndex];

		if (
			Config.SpawnCount <= 0
			|| Config.SpawnInterval <= 0.0f
			|| Config.CombatTime <= 0.0f
			)
		{
			UE_LOG(
				LogTemp,
				Error,
				TEXT(
					"Invalid WaveConfig - "
					"Wave: %d, Count: %d, "
					"Interval: %.1f, CombatTime: %.1f"
				),
				WaveIndex + 1,
				Config.SpawnCount,
				Config.SpawnInterval,
				Config.CombatTime
			);

			return;
		}
	}

	FindWaveSpawner();
	FindCore();

	if (
		!IsValid(WaveSpawner)
		|| !IsValid(CoreActor)
		)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"Game initialization failed - "
				"WaveSpawner: %s, Core: %s"
			),
			IsValid(WaveSpawner)
			? TEXT("Valid")
			: TEXT("Invalid"),
			IsValid(CoreActor)
			? TEXT("Valid")
			: TEXT("Invalid")
		);

		return;
	}

	StartPreparation();
}

void ACDGameMode::HandleEnemySpawned(
	ACDEnemy* SpawnedEnemy
)
{
	if (!IsValid(SpawnedEnemy))
	{
		return;
	}

	ACDGameState* CDGameState =
		GetGameState<ACDGameState>();

	if (
		bGameOver
		|| bGameClear
		|| !IsValid(CDGameState)
		|| CDGameState->CurrentPhase
		!= ECDGamePhase::Combat
		)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"Late enemy spawn rejected - "
				"Enemy: %s, Phase: %d"
			),
			*SpawnedEnemy->GetName(),
			IsValid(CDGameState)
			? static_cast<int32>(
				CDGameState->CurrentPhase
				)
			: -1
		);

		SpawnedEnemy->Destroy();
		return;
	}

	if (!WaveConfig.IsValidIndex(ActiveWaveIndex))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"Enemy initialization failed - "
				"Invalid wave index: %d"
			),
			ActiveWaveIndex
		);

		SpawnedEnemy->Destroy();
		return;
	}

	const FCDWaveConfig& CurrentWaveConfig =
		WaveConfig[ActiveWaveIndex];

	SpawnedEnemy->InitializeForWave(
		CurrentWaveConfig.EnemyMaxHealth,
		CurrentWaveConfig.EnemyMoveSpeed,
		CurrentWaveConfig.EnemyCoreDamage,
		CurrentWaveConfig.EnemyResourceReward
	);

	++AliveEnemyCount;

	CDGameState->SetAliveEnemyCount(
		AliveEnemyCount
	);

	SpawnedEnemy->OnDestroyed.AddDynamic(
		this,
		&ACDGameMode::HandleEnemyDestroyed
	);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Enemy registered - Alive: %d"),
		AliveEnemyCount
	);
}

void ACDGameMode::HandleEnemyDestroyed(
	AActor* DestroyedActor
)
{
	ACDEnemy* DestroyedEnemy =
		Cast<ACDEnemy>(DestroyedActor);

	if (
		DestroyedEnemy != nullptr
		&& DestroyedEnemy->WasKilled()
		&& !bGameOver
		&& !bGameClear
		)
	{
		ACDGameState* CDGameState =
			GetGameState<ACDGameState>();

		if (IsValid(CDGameState))
		{
			const int32 Reward =
				DestroyedEnemy->GetResourceReward();

			CDGameState->AddResources(Reward);

			UE_LOG(
				LogTemp,
				Display,
				TEXT(
					"Enemy kill reward - "
					"Reward: %d, Resources: %d"
				),
				Reward,
				CDGameState->GetCurrentResources()
			);
		}
	}

	AliveEnemyCount = FMath::Max(
		AliveEnemyCount - 1,
		0
	);

	ACDGameState* UpdatedGameState =
		GetGameState<ACDGameState>();

	if (IsValid(UpdatedGameState))
	{
		UpdatedGameState->SetAliveEnemyCount(
			AliveEnemyCount
		);
	}

	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"Enemy removed - Alive: %d"
		),
		AliveEnemyCount
	);

	TryFinishWave();
}

void ACDGameMode::HandleSpawningCompleted()
{
	bSpawningCompleted = true;

	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"GameMode received spawning complete"
		)
	);

	TryFinishWave();
}

void ACDGameMode::TryFinishWave()
{
	if (bGameOver || bGameClear)
	{
		return;
	}

	if (bWaveFinishing)
	{
		return;
	}

	ACDGameState* CDGameState =
		GetGameState<ACDGameState>();

	if (!IsValid(CDGameState))
	{
		return;
	}

	if (CDGameState->CurrentPhase !=
		ECDGamePhase::Combat)
	{
		return;
	}

	if (!bSpawningCompleted ||
		AliveEnemyCount > 0)
	{
		return;
	}

	FinishWave();
}

void ACDGameMode::StartPreparation()
{
	if (bGameOver || bGameClear)
	{
		return;
	}

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

	CDGameState->SetCurrentWave(ActiveWaveIndex + 1);
	CDGameState->SetGamePhase(ECDGamePhase::Preparation);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Wave %d Preparation"),
		ActiveWaveIndex + 1
	);

	GetWorldTimerManager().ClearTimer(
		PhaseTimerHandle
	);

	CDGameState->SetRemainingTime(0.0f);

	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"Wave %d is waiting for "
			"manual start"
		),
		ActiveWaveIndex + 1
	);

}

void ACDGameMode::StartCombat()
{
	if (bGameOver || bGameClear)
	{
		return;
	}

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

	AliveEnemyCount = 0;

	CDGameState->SetAliveEnemyCount(
		AliveEnemyCount
	);

	bSpawningCompleted = false;
	bWaveFinishing = false;

	CDGameState->SetGamePhase(ECDGamePhase::Combat);

	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"Wave %d Start - CombatTime: %.1f, "
			"SpawnCount: %d, SpawnInterval: %.1f"
		),
		ActiveWaveIndex + 1,
		CurrentConfig.CombatTime,
		CurrentConfig.SpawnCount,
		CurrentConfig.SpawnInterval
	);

	if (IsValid(WaveSpawner))
	{
		WaveSpawner->StartSpawning(
			CurrentConfig.SpawnCount,
			CurrentConfig.SpawnInterval
		);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Cannot start spawning: WaveSpawner is invalid")
		);
	}

	StartPhaseTimer(CurrentConfig.CombatTime);
}

void ACDGameMode::FinishWave()
{
	if (bGameOver || bGameClear)
	{
		return;
	}

	if (bWaveFinishing)
	{
		return;
	}

	bWaveFinishing = true;

	GetWorldTimerManager().ClearTimer(
		PhaseTimerHandle
	);

	if (IsValid(WaveSpawner))
	{
		WaveSpawner->StopSpawning();
	}

	//commit
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
	if (bGameOver || bGameClear)
	{
		return;
	}

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
	if (bGameClear || bGameOver)
	{
		return;
	}

	bGameClear = true;
	bWaveFinishing = true;

	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
	ClearPlayerRespawnTimers();

	if (IsValid(WaveSpawner))
	{
		WaveSpawner->StopSpawning();
	}

	ACDGameState* CDGameState =
		GetGameState<ACDGameState>();

	if (IsValid(CDGameState))
	{
		CDGameState->SetRemainingTime(0.0f);
		CDGameState->SetGamePhase(ECDGamePhase::Victory);
	}

	UE_LOG(
		LogTemp,
		Display,
		TEXT("All Waves Clear - Victory")
	);
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
	if (bGameOver)
	{
		GetWorldTimerManager().ClearTimer(PhaseTimerHandle);

		return;
	}

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
		UE_LOG(
			LogTemp,
			Display,
			TEXT(
				"Combat timer expired - "
				"SpawningCompleted: %s, Alive: %d"
			),
			bSpawningCompleted
			? TEXT("true")
			: TEXT("false"),
			AliveEnemyCount
		);

		TryFinishWave();
		break;

	default:
		break;
	}
}

void ACDGameMode::HandleCoreDestroyed()
{
	if (bGameOver || bGameClear)
	{
		return;
	}

	bGameOver = true;
	bWaveFinishing = true;

	GetWorldTimerManager().ClearTimer(
		PhaseTimerHandle
	);

	ClearPlayerRespawnTimers();

	if (IsValid(WaveSpawner))
	{
		WaveSpawner->StopSpawning();
	}

	ACDGameState* CDGameState =
		GetGameState<ACDGameState>();

	if (IsValid(CDGameState))
	{
		CDGameState->SetRemainingTime(0.0f);

		CDGameState->SetGamePhase(
			ECDGamePhase::GameOver
		);
	}

	UE_LOG(
		LogTemp,
		Error,
		TEXT(
			"Game Over - Core has been destroyed"
		)
	);
}

void ACDGameMode::FindCore()
{
	AActor* FoundActor =
		UGameplayStatics::GetActorOfClass(
			this,
			ACDCore::StaticClass()
		);

	CoreActor = Cast<ACDCore>(FoundActor);

	if (!IsValid(CoreActor))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"GameMode could not find CDCore"
			)
		);

		return;
	}

	CoreActor->OnCoreDestroyed.RemoveDynamic(
		this,
		&ACDGameMode::HandleCoreDestroyed
	);

	CoreActor->OnCoreDestroyed.AddDynamic(
		this,
		&ACDGameMode::HandleCoreDestroyed
	);

	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"GameMode found Core and bound event: %s"
		),
		*CoreActor->GetName()
	);
}

void ACDGameMode::FindWaveSpawner()
{
	AActor* FoundActor =
		UGameplayStatics::GetActorOfClass(
			this,
			ACDWaveSpawner::StaticClass()
		);

	WaveSpawner =
		Cast<ACDWaveSpawner>(FoundActor);

	if (!IsValid(WaveSpawner))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"GameMode could not find CDWaveSpawner"
			)
		);

		return;
	}

	WaveSpawner->OnEnemySpawned.RemoveDynamic(
		this,
		&ACDGameMode::HandleEnemySpawned
	);

	WaveSpawner->OnSpawningCompleted.RemoveDynamic(
		this,
		&ACDGameMode::HandleSpawningCompleted
	);

	WaveSpawner->OnEnemySpawned.AddDynamic(
		this,
		&ACDGameMode::HandleEnemySpawned
	);

	WaveSpawner->OnSpawningCompleted.AddDynamic(
		this,
		&ACDGameMode::HandleSpawningCompleted
	);

	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"GameMode found WaveSpawner and bound events: %s"
		),
		*WaveSpawner->GetName()
	);
}

void ACDGameMode::RequestStartWave()
{
	if (bGameOver || bGameClear)
	{
		return;
	}

	if (!WaveConfig.IsValidIndex(ActiveWaveIndex))
	{
		return;
	}

	ACDGameState* CDGameState =
		GetGameState<ACDGameState>();

	if (!IsValid(CDGameState))
	{
		return;
	}

	if (
		CDGameState->CurrentPhase
		!= ECDGamePhase::Preparation
		)
	{
		UE_LOG(
			LogTemp,
			Display,
			TEXT(
				"Start wave request ignored - "
				"Current phase: %d"
			),
			static_cast<int32>(
				CDGameState->CurrentPhase
				)
		);
		return;
	}

	GetWorldTimerManager().ClearTimer(
		PhaseTimerHandle
	);

	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"Manual wave start requested - "
			"Wave: %d"
		),
		ActiveWaveIndex + 1
	);

	StartCombat();
}

void ACDGameMode::InitializePlayerRespawnSystem(
	ACDPlayerController* PlayerController
)
{
	if (!IsValid(PlayerController))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Player respawn initialization failed: PlayerController is invalid")
		);
		return;
	}

	if (IsValid(CachedPlayerState))
	{
		CachedPlayerState->OnPlayerDeath.RemoveDynamic(
			this,
			&ACDGameMode::HandlePlayerDeath
		);
	}

	CachedPlayerController = PlayerController;
	CachedPlayerState =
		PlayerController->GetPlayerState<ACDPlayerState>();

	if (!IsValid(CachedPlayerState))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Player respawn initialization failed: CDPlayerState is invalid")
		);
		CachedPlayerController = nullptr;
		return;
	}

	CachedPlayerState->OnPlayerDeath.RemoveDynamic(
		this,
		&ACDGameMode::HandlePlayerDeath
	);

	CachedPlayerState->OnPlayerDeath.AddDynamic(
		this,
		&ACDGameMode::HandlePlayerDeath
	);

	ClearPlayerRespawnTimers();
	CachedPlayerState->ResetForNewGame();
	CachedPlayerController->SetPlayerGameplayEnabled(true);
}

void ACDGameMode::HandlePlayerDeath()
{
	if (
		bGameOver
		|| bGameClear
		|| !IsValid(CachedPlayerController)
		|| !IsValid(CachedPlayerState)
		)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(
		PlayerRespawnTimerHandle
	);

	GetWorldTimerManager().ClearTimer(
		PlayerInvulnerabilityTimerHandle
	);

	CachedPlayerState->SetRespawnInvulnerable(false);
	CachedPlayerController->SetPlayerGameplayEnabled(false);

	if (PlayerRespawnDelay <= 0.0f)
	{
		RespawnPlayer();
		return;
	}

	GetWorldTimerManager().SetTimer(
		PlayerRespawnTimerHandle,
		this,
		&ACDGameMode::RespawnPlayer,
		PlayerRespawnDelay,
		false
	);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Player died - Respawn in %.1f seconds"),
		PlayerRespawnDelay
	);
}

void ACDGameMode::RespawnPlayer()
{
	GetWorldTimerManager().ClearTimer(
		PlayerRespawnTimerHandle
	);

	if (
		bGameOver
		|| bGameClear
		|| !IsValid(CachedPlayerController)
		|| !IsValid(CachedPlayerState)
		)
	{
		return;
	}

	CachedPlayerState->CompleteRespawn();

	CachedPlayerController->SetPlayerGameplayEnabled(true);

	if (PlayerInvulnerabilityDuration <= 0.0f)
	{
		EndPlayerInvulnerability();
		return;
	}

	GetWorldTimerManager().SetTimer(
		PlayerInvulnerabilityTimerHandle,
		this,
		&ACDGameMode::EndPlayerInvulnerability,
		PlayerInvulnerabilityDuration,
		false
	);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Player respawned - Invulnerable for %.1f seconds"),
		PlayerInvulnerabilityDuration
	);
}

void ACDGameMode::EndPlayerInvulnerability()
{
	GetWorldTimerManager().ClearTimer(
		PlayerInvulnerabilityTimerHandle
	);

	if (IsValid(CachedPlayerState))
	{
		CachedPlayerState->SetRespawnInvulnerable(false);
	}
}

void ACDGameMode::ClearPlayerRespawnTimers()
{
	GetWorldTimerManager().ClearTimer(
		PlayerRespawnTimerHandle
	);

	GetWorldTimerManager().ClearTimer(
		PlayerInvulnerabilityTimerHandle
	);

	if (IsValid(CachedPlayerState))
	{
		CachedPlayerState->SetRespawnInvulnerable(false);
	}
}
