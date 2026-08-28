#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CDEnemyHealthWidget.generated.h"

class UProgressBar;

UCLASS()
class CIRCUITDEFENSE_API UCDEnemyHealthWidget
	: public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(
		BlueprintCallable,
		Category = "Enemy Health"
	)
	void SetHealthPercent(
		float InHealthPercent
	);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;
};