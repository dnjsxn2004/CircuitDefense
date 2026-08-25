#include "CDGameState.h"

ACDGameState::ACDGameState()
{
    CurrentPhase = ECDGamePhase::Preparation;
    CurrentWave = 0;
    RemainingTime = 0.0f;
    CurrentResources = 0;
}

void ACDGameState::BeginPlay()
{
    Super::BeginPlay();

    CurrentResources =
        FMath::Max(StartingResources, 0);

    OnResourcesChanged.Broadcast(
        CurrentResources
    );

    UE_LOG(
        LogTemp,
        Display,
        TEXT(
            "Resources initialized: %d"
        ),
        CurrentResources
    );
}

void ACDGameState::SetGamePhase(
    ECDGamePhase NewPhase
)
{
    if (CurrentPhase == NewPhase)
    {
        return;
    }

    CurrentPhase = NewPhase;

    OnGamePhaseChanged.Broadcast(CurrentPhase);

    UE_LOG(
        LogTemp,
        Log,
        TEXT("Game phase changed: %d"),
        static_cast<int32>(CurrentPhase)
    );
}

void ACDGameState::SetCurrentWave(
    int32 NewWave
)
{
    CurrentWave = FMath::Max(NewWave, 0);

    UE_LOG(
        LogTemp,
        Display,
        TEXT("Current wave changed: %d"),
        CurrentWave
    );
}

void ACDGameState::SetRemainingTime(
    float NewTime
)
{
    RemainingTime =
        FMath::Max(NewTime, 0.0f);
}

bool ACDGameState::CanAfford(
    int32 Cost
) const
{
    if (Cost < 0)
    {
        return false;
    }

    return CurrentResources >= Cost;
}

bool ACDGameState::SpendResources(
    int32 Amount
)
{
    if (Amount < 0)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "Resource spending failed: "
                "Amount cannot be negative"
            )
        );
        return false;
    }

    if (Amount == 0)
    {
        return true;
    }

    if (!CanAfford(Amount))
    {
        UE_LOG(
            LogTemp,
            Display,
            TEXT(
                "Not enough resources - "
                "Required: %d, Current: %d"
            ),
            Amount,
            CurrentResources
        );
        return false;
    }

    CurrentResources -= Amount;

    OnResourcesChanged.Broadcast(
        CurrentResources
    );

    UE_LOG(
        LogTemp,
        Display,
        TEXT(
            "Resources spent - "
            "Amount: %d, Remaining: %d"
        ),
        Amount,
        CurrentResources
    );

    return true;
}

void ACDGameState::AddResources(
    int32 Amount
)
{
    if (Amount <= 0)
    {
        return;
    }

    CurrentResources += Amount;

    OnResourcesChanged.Broadcast(
        CurrentResources
    );

    UE_LOG(
        LogTemp,
        Display,
        TEXT(
            "Resources added - "
            "Amount: %d, Current: %d"
        ),
        Amount,
        CurrentResources
    );
}

int32 ACDGameState::GetCurrentResources() const
{
    return CurrentResources;
}