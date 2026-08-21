// Fill out your copyright notice in the Description page of Project Settings.

#include "CDWaveSpawner.h"

#include "CDEnemy.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ACDWaveSpawner::ACDWaveSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(
		TEXT("SceneRoot")
	);

	SetRootComponent(SceneRoot);
}

void ACDWaveSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoStartForTest)
	{
		StartSpawning(
			TestSpawnCount,
			TestSpawnInterval
		);
	}
}

void ACDWaveSpawner::EndPlay(
	const EEndPlayReason::Type EndPlayReason
)
{
	StopSpawning();

	Super::EndPlay(EndPlayReason);
}

void ACDWaveSpawner::StartSpawning(
	int32 InSpawnCount,
	float InSpawnInterval
)
{
	StopSpawning();

	if (!EnemyClass)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"WaveSpawner: EnemyClass is not assigned"
			)
		);

		return;
	}

	if (InSpawnCount <= 0)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"WaveSpawner: SpawnCount must be greater than 0"
			)
		);

		return;
	}

	TargetSpawnCount = InSpawnCount;
	SpawnedCount = 0;
	bSpawningCompleted = false;

	const float ValidSpawnInterval =
		FMath::Max(InSpawnInterval, 0.1f);

	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"WaveSpawner Start - Count: %d, Interval: %.1f"
		),
		TargetSpawnCount,
		ValidSpawnInterval
	);

	// 첫 번째 적은 전투 시작과 동시에 생성합니다.
	SpawnEnemy();

	// 첫 생성으로 목표 수를 채우지 않았다면 반복 타이머를 등록합니다.
	if (SpawnedCount < TargetSpawnCount)
	{
		GetWorldTimerManager().SetTimer(
			SpawnTimerHandle,
			this,
			&ACDWaveSpawner::SpawnEnemy,
			ValidSpawnInterval,
			true,
			ValidSpawnInterval
		);

		UE_LOG(
			LogTemp,
			Display,
			TEXT(
				"Spawn timer registered - Active: %s, "
				"Rate: %.1f, Remaining: %.1f"
			),
			GetWorldTimerManager().IsTimerActive(
				SpawnTimerHandle
			)
			? TEXT("true")
			: TEXT("false"),
			GetWorldTimerManager().GetTimerRate(
				SpawnTimerHandle
			),
			GetWorldTimerManager().GetTimerRemaining(
				SpawnTimerHandle
			)
		);
	}
}

void ACDWaveSpawner::StopSpawning()
{
	const bool bWasActive =
		GetWorldTimerManager().IsTimerActive(
			SpawnTimerHandle
		);

	const float RemainingTime =
		GetWorldTimerManager().GetTimerRemaining(
			SpawnTimerHandle
		);

	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"StopSpawning called - Active: %s, "
			"Spawned: %d / %d, Remaining: %.1f"
		),
		bWasActive
		? TEXT("true")
		: TEXT("false"),
		SpawnedCount,
		TargetSpawnCount,
		RemainingTime
	);

	GetWorldTimerManager().ClearTimer(
		SpawnTimerHandle
	);
}

void ACDWaveSpawner::CompleteSpawning()
{
	if (bSpawningCompleted)
	{
		return;
	}

	bSpawningCompleted = true;

	GetWorldTimerManager().ClearTimer(
		SpawnTimerHandle
	);

	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"WaveSpawner Complete - Spawned: %d"
		),
		SpawnedCount
	);

	OnSpawningCompleted.Broadcast();
}

void ACDWaveSpawner::SpawnEnemy()
{
	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"SpawnEnemy ENTER - Spawned: %d / %d, "
			"TimerExists: %s, Active: %s"
		),
		SpawnedCount,
		TargetSpawnCount,
		GetWorldTimerManager().TimerExists(
			SpawnTimerHandle
		)
		? TEXT("true")
		: TEXT("false"),
		GetWorldTimerManager().IsTimerActive(
			SpawnTimerHandle
		)
		? TEXT("true")
		: TEXT("false")
	);

	if (SpawnedCount >= TargetSpawnCount)
	{
		CompleteSpawning();

		return;
	}

	if (!EnemyClass)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"WaveSpawner: EnemyClass became invalid"
			)
		);

		StopSpawning();
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"WaveSpawner: World is invalid"
			)
		);

		StopSpawning();
		return;
	}

	FActorSpawnParameters SpawnParameters;

	SpawnParameters.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACDEnemy* SpawnedEnemy =
		World->SpawnActor<ACDEnemy>(
			EnemyClass,
			GetActorTransform(),
			SpawnParameters
		);

	if (!IsValid(SpawnedEnemy))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"WaveSpawner: Enemy spawn failed"
			)
		);

		StopSpawning();
		return;
	}

	++SpawnedCount;

	OnEnemySpawned.Broadcast(SpawnedEnemy);

	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"Enemy Spawned: %d / %d"
		),
		SpawnedCount,
		TargetSpawnCount
	);

	if (SpawnedCount >= TargetSpawnCount)
	{
		CompleteSpawning();
	}
}


