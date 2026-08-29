// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CDWaveConfig.h"
#include "CDGameMode.generated.h"

class ACDEnemy;
class ACDWaveSpawner;
class ACDEnemySpawner;
class ACDCore;
class ACDPlayerController;
class ACDPlayerState;
/**
 *
 */
UCLASS()
class CIRCUITDEFENSE_API ACDGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACDGameMode();

	UFUNCTION(
		BlueprintCallable,
		Category = "Wave"
	)
	void RequestStartWave();

protected:
	virtual void BeginPlay() override;
	virtual void PostLogin(
		APlayerController* NewPlayer
	) override;
	virtual void EndPlay(
		const EEndPlayReason::Type EndPlayReason
	) override;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Player|Respawn",
		meta = (ClampMin = "0.0")
	)
	float PlayerRespawnDelay = 3.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Player|Respawn",
		meta = (ClampMin = "0.0")
	)
	float PlayerInvulnerabilityDuration = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wave")
	TArray<FCDWaveConfig> WaveConfig;

private:
	FTimerHandle PhaseTimerHandle;
	FTimerHandle PlayerRespawnTimerHandle;
	FTimerHandle PlayerInvulnerabilityTimerHandle;

	int32 ActiveWaveIndex = 0;
	int32 AliveEnemyCount = 0;

	bool bSpawningCompleted = false;
	bool bWaveFinishing = false;

	UFUNCTION()
	void HandleEnemySpawned(ACDEnemy* SpawnedEnemy);

	UFUNCTION()
	void HandleEnemyDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void HandleSpawningCompleted();

	void TryFinishWave();
	void StartPreparation();
	void StartCombat();
	void FinishWave();
	void StartNextWave();
	void CompleteAllWaves();

	void StartPhaseTimer(float Duration);
	void UpdatePhaseTimer();

	UPROPERTY()
	TObjectPtr<ACDWaveSpawner> WaveSpawner;

	UPROPERTY()
	TObjectPtr<ACDCore> CoreActor;

	bool bGameOver = false;
	bool bGameClear = false;

	UFUNCTION()
	void HandleCoreDestroyed();

	void FindCore();

	void FindWaveSpawner();

	void InitializePlayerRespawnSystem(
		ACDPlayerController* PlayerController
	);

	void RespawnPlayer();
	void EndPlayerInvulnerability();
	void ClearPlayerRespawnTimers();

	UFUNCTION()
	void HandlePlayerDeath();

	UPROPERTY()
	TObjectPtr<ACDPlayerController>
		CachedPlayerController;

	UPROPERTY()
	TObjectPtr<ACDPlayerState>
		CachedPlayerState;

};
