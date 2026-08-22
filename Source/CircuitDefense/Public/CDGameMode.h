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
/**
 * 
 */
UCLASS()
class CIRCUITDEFENSE_API ACDGameMode : public AGameModeBase
{
	GENERATED_BODY()

public :
	ACDGameMode();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wave")
	TArray<FCDWaveConfig> WaveConfig;

private:
	FTimerHandle PhaseTimerHandle;

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

	UFUNCTION()
	void HandleCoreDestroyed();

	void FindCore();

	void FindWaveSpawner();

};
