// Fill out your copyright notice in the Description page of Project Settings.


#include "CDEnemy.h"

#include "CDCore.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/WidgetComponent.h"
#include "CDEnemyHealthWidget.h"

// Sets default values
ACDEnemy::ACDEnemy()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	EnemyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EnemyMesh"));

	SetRootComponent(EnemyMesh);

	EnemyMesh->SetMobility(EComponentMobility::Movable);

	HealthWidgetComponent =
		CreateDefaultSubobject<UWidgetComponent>(
			TEXT("HealthWidgetComponent")
		);

	HealthWidgetComponent->SetupAttachment(EnemyMesh);

	HealthWidgetComponent->SetRelativeLocation(
		FVector(0.0f, 0.0f, 100.0f)
	);

	HealthWidgetComponent->SetWidgetSpace(
		EWidgetSpace::Screen
	);

	HealthWidgetComponent->SetDrawSize(
		FVector2D(120.0f, 16.0f)
	);

	HealthWidgetComponent->SetCollisionEnabled(
		ECollisionEnabled::NoCollision
	);
}

// Called when the game starts or when spawned
void ACDEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHealth = MaxHealth;
	bReachedCore = false;
	bDead = false;

	FindTargetCore();

	if (IsValid(HealthWidgetComponent))
	{
		HealthWidgetComponent->InitWidget();

		HealthWidget =
			Cast<UCDEnemyHealthWidget>(
				HealthWidgetComponent
				->GetUserWidgetObject()
			);
	}

	if (!IsValid(HealthWidget))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"Enemy health widget is invalid - "
				"Enemy: %s"
			),
			*GetName()
		);
	}

	UpdateHealthWidget();
}

void ACDEnemy::InitializeForWave(
	float InMaxHealth,
	float InMoveSpeed,
	float InCoreDamage,
	int32 InResourceReward
)
{
	MaxHealth = FMath::Max(
		InMaxHealth,
		1.0f
	);

	CurrentHealth = MaxHealth;

	MoveSpeed = FMath::Max(
		InMoveSpeed,
		0.0f
	);

	CoreDamage = FMath::Max(
		InCoreDamage,
		0.0f
	);

	ResourceReward = FMath::Max(
		InResourceReward,
		0
	);

	UpdateHealthWidget();

	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"Enemy wave stats initialized - "
			"Enemy: %s, Health: %.0f, "
			"Speed: %.0f, CoreDamage: %.0f, "
			"Reward: %d"
		),
		*GetName(),
		MaxHealth,
		MoveSpeed,
		CoreDamage,
		ResourceReward
	);
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

void ACDEnemy::ApplyEnemyDamage(float DamageAmount)
{
	if (DamageAmount <= 0.0f || bDead||IsActorBeingDestroyed())
	{
		return;
	}

	CurrentHealth = FMath::Clamp(
		CurrentHealth - DamageAmount,
		0.0f,
		MaxHealth
	);

	UpdateHealthWidget();

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Enemy Damaged - HP : %.0f / %.0f"),
		CurrentHealth,
		MaxHealth
	);

	if (CurrentHealth > 0.0f)
	{
		return;
	}

	bDead = true;

	SetActorTickEnabled(false);
	SetActorEnableCollision(false);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Enemy destroyed")
	);

	Destroy();
}

void ACDEnemy::UpdateHealthWidget()
{
	if (
		!IsValid(HealthWidget)
		|| !IsValid(HealthWidgetComponent)
		)
	{
		return;
	}

	const float HealthPercent =
		GetHealthPercent();

	HealthWidget->SetHealthPercent(
		HealthPercent
	);

	const bool bShouldShowHealthBar =
		HealthPercent > 0.0f
		&& HealthPercent < 1.0f;

	HealthWidgetComponent->SetVisibility(
		bShouldShowHealthBar,
		true
	);
}

float ACDEnemy::GetHealthPercent() const
{
	if (MaxHealth <= 0.0f)
	{
		return 0.0f;
	}

	return CurrentHealth / MaxHealth;
}

int32 ACDEnemy::GetResourceReward() const
{
	return ResourceReward;
}

bool ACDEnemy::WasKilled() const
{
	return bDead;
}