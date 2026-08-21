// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CDWaveSpawner.generated.h"

class ACDEnemy;
class USceneComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnEnemySpawned,
	ACDEnemy*,
	SpawnedEnemy
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(
	FOnSpawningCompleted
);

UCLASS()
class CIRCUITDEFENSE_API ACDWaveSpawner : public AActor
{
	GENERATED_BODY()

public:
	ACDWaveSpawner();

	UFUNCTION(BlueprintCallable, Category = "Wave Spawner")
	void StartSpawning(
		int32 InSpawnCount,
		float InSpawnInterval
	);

	UFUNCTION(BlueprintCallable, Category = "Wave Spawner")
	void StopSpawning();

	UPROPERTY(
		BlueprintAssignable,
		Category = "Wave Spawner"
	)
	FOnEnemySpawned OnEnemySpawned;

	UPROPERTY(
		BlueprintAssignable,
		Category = "Wave Spawner"
	)
	FOnSpawningCompleted OnSpawningCompleted;

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(
		const EEndPlayReason::Type EndPlayReason
	) override;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Wave Spawner"
	)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Wave Spawner"
	)
	TSubclassOf<ACDEnemy> EnemyClass;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Wave Spawner|Test"
	)
	bool bAutoStartForTest = false;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Wave Spawner|Test",
		meta = (ClampMin = "1")
	)
	int32 TestSpawnCount = 5;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Wave Spawner|Test",
		meta = (ClampMin = "0.1")
	)
	float TestSpawnInterval = 1.0f;

private:
	void SpawnEnemy();

	void CompleteSpawning();

	FTimerHandle SpawnTimerHandle;

	int32 TargetSpawnCount = 0;

	int32 SpawnedCount = 0;

	bool bSpawningCompleted = false;
};