#pragma once

#include "CoreMinimal.h"
#include "CDWaveConfig.generated.h"

USTRUCT(BlueprintType)
struct FCDWaveConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
    float PreparationTime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
    float CombatTime = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
    int32 SpawnCount = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
    float SpawnInterval = 2.0f;
};