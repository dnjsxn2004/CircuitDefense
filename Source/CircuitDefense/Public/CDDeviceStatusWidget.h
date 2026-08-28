#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CDDeviceStatusWidget.generated.h"

class UTextBlock;

UCLASS()
class CIRCUITDEFENSE_API UCDDeviceStatusWidget
	: public UUserWidget
{
	GENERATED_BODY()

public:
	void SetPowerState(
		bool bIsPowered
	);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PowerStatusText;
};