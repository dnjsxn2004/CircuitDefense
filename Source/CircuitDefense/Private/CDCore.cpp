// Fill out your copyright notice in the Description page of Project Settings.


#include "CDCore.h"

#include "Components/StaticMeshComponent.h"

// Sets default values
ACDCore::ACDCore()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CoreMesh = CreateDefaultSubobject<UStaticMeshComponent>(
		TEXT("CoreMesh")
	);

	SetRootComponent(CoreMesh);
}

// Called when the game starts or when spawned
void ACDCore::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
    bCoreDestroyed = false;

    OnCoreHealthChanged.Broadcast(
        CurrentHealth,
        MaxHealth
    );

    UE_LOG(
        LogTemp,
        Display,
        TEXT("Core initialized - HP: %.0f / %.0f"),
        CurrentHealth,
        MaxHealth
    );
}

// Called every frame
void ACDCore::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACDCore::ApplyCoreDamage(float DamageAmount)
{
    if (DamageAmount <= 0.0f || bCoreDestroyed)
    {
        return;
    }

    CurrentHealth = FMath::Clamp(
        CurrentHealth - DamageAmount,
        0.0f,
        MaxHealth
    );

    OnCoreHealthChanged.Broadcast(
        CurrentHealth,
        MaxHealth
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Core damaged - HP: %.0f / %.0f"),
        CurrentHealth,
        MaxHealth
    );

    if (CurrentHealth <= 0.0f)
    {
        HandleCoreDestroyed();
    }
}

float ACDCore::GetHealthPercent() const
{
    if (MaxHealth <= 0.0f)
    {
        return 0.0f;
    }

    return CurrentHealth / MaxHealth;
}

bool ACDCore::IsCoreDestroyed() const
{
    return bCoreDestroyed;
}

void ACDCore::HandleCoreDestroyed()
{
    if (bCoreDestroyed)
    {
        return;
    }

    bCoreDestroyed = true;

    UE_LOG(
        LogTemp,
        Error,
        TEXT("Core Destroyed - Game Over")
    );

    OnCoreDestroyed.Broadcast();
}