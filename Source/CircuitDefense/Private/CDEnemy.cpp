// Fill out your copyright notice in the Description page of Project Settings.


#include "CDEnemy.h"

#include "CDCore.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ACDEnemy::ACDEnemy()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	EnemyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EnemyMesh"));

	SetRootComponent(EnemyMesh);

	EnemyMesh->SetMobility(EComponentMobility::Movable);
}

// Called when the game starts or when spawned
void ACDEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHealth = MaxHealth;
	bReachedCore = false;
	bDead = false;

	FindTargetCore();
}

void ACDEnemy::FindTargetCore()
{
	AActor* FoundActor = UGameplayStatics::GetActorOfClass(this, ACDCore::StaticClass());

	TargetCore = Cast<ACDCore>(FoundActor);

	if (IsValid(TargetCore))
	{
		UE_LOG(
			LogTemp,
			Display,
			TEXT("Enemy found core: %s"),
			*TargetCore->GetName()
		);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Enemy could not find Core")
		);
	}
}

void ACDEnemy::ReachCore()
{
	if (bReachedCore||!IsValid(TargetCore))
	{
		return;
	}

	bReachedCore = true;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Enemy reached core - Damage: %.0f"),
		CoreDamage
	);

	TargetCore->ApplyCoreDamage(CoreDamage);

	Destroy();
}

// Called every frame
void ACDEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bDead||bReachedCore||!IsValid(TargetCore)||TargetCore->IsCoreDestroyed())
	{
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	const FVector TargetLocation = TargetCore->GetActorLocation();

	FVector Direction = TargetLocation - CurrentLocation;
	Direction.Z = 0.0f;

	const float DistanceToCore = Direction.Size();

	if (DistanceToCore <= ReachDistance)
	{
		ReachCore();
		return;
	}

	Direction.Normalize();

	const float MoveDistance = MoveSpeed * DeltaTime;

	SetActorLocation(CurrentLocation + Direction * MoveDistance);

}

void ACDEnemy::ApplyEnemyDamge(float DamageAmount)
{
	if (DamageAmount <= 0.0f || bDead)
	{
		return;
	}

	CurrentHealth = FMath::Clamp(
		CurrentHealth = DamageAmount,
		0.0f,
		MaxHealth
	);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Enemy Damaged - HP : %.0f / %.0f"),
		CurrentHealth,
		MaxHealth
	);

	if (CurrentHealth <= 0.0f)
	{
		bDead = true;

		UE_LOG(
			LogTemp,
			Display,
			TEXT("Enemy destroyed")
		);

		Destroy();
	}
}

float ACDEnemy::GetHealthPercent() const
{
	if (MaxHealth <= 0.0f)
	{
		return 0.0f;
	}

	return CurrentHealth / MaxHealth;
}

