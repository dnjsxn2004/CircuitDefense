#include "CDAttackDevice.h"

#include "CDEnemy.h"
#include "CDGameState.h"
#include "CDDeviceType.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ACDAttackDevice::ACDAttackDevice()
{
    PrimaryActorTick.bCanEverTick = false;

    DeviceType = ECDDeviceType::Attack;
}

void ACDAttackDevice::BeginPlay()
{
    Super::BeginPlay();

    UWorld* World = GetWorld();

    if (!IsValid(World))
    {
        return;
    }

    World->GetTimerManager().SetTimer(
        AttackTimerHandle,
        this,
        &ACDAttackDevice::TryAttack,
        AttackInterval,
        true
    );
}

void ACDAttackDevice::EndPlay(
    const EEndPlayReason::Type EndPlayReason
)
{
    UWorld* World = GetWorld();

    if (IsValid(World))
    {
        World->GetTimerManager().ClearTimer(
            AttackTimerHandle
        );
    }

    Super::EndPlay(EndPlayReason);
}

void ACDAttackDevice::TryAttack()
{
    if (!IsInstalled() || !IsPowered())
    {
        return;
    }

    UWorld* World = GetWorld();

    if (!IsValid(World))
    {
        return;
    }

    ACDGameState* CDGameState =
        World->GetGameState<ACDGameState>();

    if (!IsValid(CDGameState))
    {
        return;
    }

    if (CDGameState->CurrentPhase
        != ECDGamePhase::Combat)
    {
        return;
    }

    ACDEnemy* TargetEnemy = FindClosestEnemy();

    if (!IsValid(TargetEnemy))
    {
        return;
    }

    const FString TargetName =
        TargetEnemy->GetName();

    TargetEnemy->ApplyEnemyDamage(
        AttackDamage
    );

    UE_LOG(
        LogTemp,
        Display,
        TEXT(
            "Attack device fired - "
            "Device: %s, Target: %s, Damage: %.1f"
        ),
        *GetName(),
        *TargetName,
        AttackDamage
    );
}

ACDEnemy* ACDAttackDevice::FindClosestEnemy() const
{
    UWorld* World = GetWorld();

    if (!IsValid(World))
    {
        return nullptr;
    }

    TArray<AActor*> FoundEnemies;

    UGameplayStatics::GetAllActorsOfClass(
        World,
        ACDEnemy::StaticClass(),
        FoundEnemies
    );

    ACDEnemy* ClosestEnemy = nullptr;

    float ClosestDistanceSquared =
        AttackRange * AttackRange;

    const FVector DeviceLocation =
        GetActorLocation();

    for (AActor* FoundActor : FoundEnemies)
    {
        ACDEnemy* Enemy =
            Cast<ACDEnemy>(FoundActor);

        if (!IsValid(Enemy))
        {
            continue;
        }

        if (Enemy->IsActorBeingDestroyed())
        {
            continue;
        }

        const float DistanceSquared =
            FVector::DistSquared(
                DeviceLocation,
                Enemy->GetActorLocation()
            );

        if (DistanceSquared
            > ClosestDistanceSquared)
        {
            continue;
        }

        ClosestDistanceSquared =
            DistanceSquared;

        ClosestEnemy = Enemy;
    }

    return ClosestEnemy;
}