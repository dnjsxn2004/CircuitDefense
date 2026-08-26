#pragma once

#include "CoreMinimal.h"
#include "CDPlaceableDevice.h"
#include "CDAttackDevice.generated.h"

class ACDEnemy;

UCLASS()
class CIRCUITDEFENSE_API ACDAttackDevice
    : public ACDPlaceableDevice
{
    GENERATED_BODY()

public:
    ACDAttackDevice();

protected:
    virtual void BeginPlay() override;

    virtual void EndPlay(
        const EEndPlayReason::Type EndPlayReason
    ) override;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Attack",
        meta = (ClampMin = "0.0")
    )
    float AttackRange = 800.0f;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Attack",
        meta = (ClampMin = "0.0")
    )
    float AttackDamage = 10.0f;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Attack",
        meta = (ClampMin = "0.1")
    )
    float AttackInterval = 1.0f;

private:
    void TryAttack();

    ACDEnemy* FindClosestEnemy() const;

    FTimerHandle AttackTimerHandle;
};