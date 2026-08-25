#pragma once

#include "CoreMinimal.h"
#include "CDDeviceType.generated.h"

UENUM(BlueprintType)
enum class ECDDeviceType : uint8
{
    PowerSource UMETA(DisplayName = "Power Source"),
    Relay UMETA(DisplayName = "Relay"),
    Attack UMETA(DisplayName = "Attack Device")
};