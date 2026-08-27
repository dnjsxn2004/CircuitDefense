#pragma once

#include "CoreMinimal.h"
#include "CDWaveConfig.generated.h"

USTRUCT(BlueprintType)
struct FCDWaveConfig
{
	GENERATED_BODY()

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Wave",
		meta = (ClampMin = "0.0")
	)
	float PreparationTime = 0.0f;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Wave",
		meta = (ClampMin = "1.0")
	)
	float CombatTime = 30.0f;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Spawning",
		meta = (ClampMin = "1")
	)
	int32 SpawnCount = 5;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Spawning",
		meta = (ClampMin = "0.1")
	)
	float SpawnInterval = 2.0f;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Enemy",
		meta = (ClampMin = "1.0")
	)
	float EnemyMaxHealth = 30.0f;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Enemy",
		meta = (ClampMin = "0.0")
	)
	float EnemyMoveSpeed = 200.0f;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Enemy",
		meta = (ClampMin = "0.0")
	)
	float EnemyCoreDamage = 5.0f;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Enemy",
		meta = (ClampMin = "0")
	)
	int32 EnemyResourceReward = 5;
};