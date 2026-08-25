#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CDGamePhase.h"
#include "CDGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnGamePhaseChanged,
    ECDGamePhase,
    NewPhase
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnResourcesChanged,
    int32,
    NewResources
);

UCLASS()
class CIRCUITDEFENSE_API ACDGameState
    : public AGameStateBase
{
    GENERATED_BODY()

public:
    ACDGameState();

    UPROPERTY(
        BlueprintReadOnly,
        Category = "Game Flow"
    )
    ECDGamePhase CurrentPhase =
        ECDGamePhase::Preparation;

    UPROPERTY(
        BlueprintReadOnly,
        Category = "Wave"
    )
    int32 CurrentWave = 0;

    UPROPERTY(
        BlueprintReadOnly,
        Category = "Game State"
    )
    float RemainingTime = 0.0f;

    UPROPERTY(
        BlueprintReadOnly,
        Category = "Game Flow"
    )
    FOnGamePhaseChanged OnGamePhaseChanged;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Resources",
        meta = (ClampMin = "0")
    )
    int32 StartingResources = 100;

    UPROPERTY(
        VisibleInstanceOnly,
        BlueprintReadOnly,
        Category = "Resources"
    )
    int32 CurrentResources = 0;

    UPROPERTY(
        BlueprintAssignable,
        Category = "Resources"
    )
    FOnResourcesChanged OnResourcesChanged;

    UFUNCTION(
        BlueprintCallable,
        Category = "Game State"
    )
    void SetGamePhase(ECDGamePhase NewPhase);

    UFUNCTION(
        BlueprintPure,
        Category = "Resources"
    )
    bool CanAfford(int32 Cost) const;

    UFUNCTION(
        BlueprintCallable,
        Category = "Resources"
    )
    bool SpendResources(int32 Amount);

    UFUNCTION(
        BlueprintCallable,
        Category = "Resources"
    )
    void AddResources(int32 Amount);

    UFUNCTION(
        BlueprintPure,
        Category = "Resources"
    )
    int32 GetCurrentResources() const;

    void SetCurrentWave(int32 NewWave);
    void SetRemainingTime(float NewTime);

protected:
    virtual void BeginPlay() override;
};