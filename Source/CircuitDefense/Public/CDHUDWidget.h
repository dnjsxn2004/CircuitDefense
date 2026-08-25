// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CDHUDWidget.generated.h"

class ACDCore;
class UProgressBar;
class UTextBlock;
class UButton;

UCLASS()
class CIRCUITDEFENSE_API UCDHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	

protected:
	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PhaseText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> WaveText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> CoreHPBar;

	UPROPERTY(
		meta = (BindWidget)
	)
	TObjectPtr<UTextBlock> RemainingTimeText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CoreHPText;
	
	UPROPERTY()
	TObjectPtr<ACDCore> CoreActor;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> StartWaveButton;

private:
	UFUNCTION()
	void HandleStartWaveClicked();

};
