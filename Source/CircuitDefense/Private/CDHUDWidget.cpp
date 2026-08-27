// Fill out your copyright notice in the Description page of Project Settings.


#include "CDHUDWidget.h"

#include "CDCore.h"
#include "CDGameState.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "CDGameMode.h"
#include "Components/Button.h"

void UCDHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(StartWaveButton))
	{
		StartWaveButton->OnClicked.RemoveDynamic(
			this,
			&UCDHUDWidget::HandleStartWaveClicked
		);

		StartWaveButton->OnClicked.AddDynamic(
			this,
			&UCDHUDWidget::HandleStartWaveClicked
		);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"StartWaveButton is not bound"
			)
		);
	}

	AActor* FoundCore =
		UGameplayStatics::GetActorOfClass(
			this,
			ACDCore::StaticClass()
		);

	CoreActor = Cast<ACDCore>(FoundCore);

	if (IsValid(CoreActor))
	{
		UE_LOG(
			LogTemp,
			Display,
			TEXT("HUD found Core: %s"),
			*CoreActor->GetName()
		);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("HUD could not find Core")
		);
	}
}

void UCDHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

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

	if (IsValid(StartWaveButton))
	{
		const bool bCanStartWave =
			CDGameState->CurrentPhase
			== ECDGamePhase::Preparation;

		StartWaveButton->SetVisibility(
			bCanStartWave
			? ESlateVisibility::Visible
			: ESlateVisibility::Collapsed
		);

		StartWaveButton->SetIsEnabled(
			bCanStartWave
		);
	}

	FString PhaseName;

	switch (CDGameState->CurrentPhase)
	{
	case ECDGamePhase::Preparation:
		PhaseName = TEXT("PREPARATION");
		break;
			
	case ECDGamePhase::Combat:
		PhaseName = TEXT("COMBAT");
		break;
			
	case ECDGamePhase::GameOver:
		PhaseName = TEXT("GAMEOVER");
		break;
			
	case ECDGamePhase::WaveClear:
		PhaseName = TEXT("WAVECLEAR");
		break;
				
	case ECDGamePhase::Victory:
		PhaseName = TEXT("VICTORY");
		break;

	default:
		PhaseName = TEXT("UNKNOWN");
		break;
	}

	if (IsValid(PhaseText))
	{
		PhaseText->SetText(
			FText::FromString(
				PhaseName
			)
		);
	}

	if (IsValid(WaveText))
	{
		WaveText->SetText(
			FText::FromString(
				FString::Printf(
					TEXT("WAVE %d"),
					CDGameState->CurrentWave
				)
			)
		);
	}

	if (IsValid(RemainingTimeText))
	{
		const int32 DisplayTime =
			FMath::CeilToInt(
				CDGameState->RemainingTime
			);

		RemainingTimeText->SetText(
			FText::FromString(
				FString::Printf(
					TEXT("TIME %d"),
					DisplayTime
				)
			)
		);
	}

	if (IsValid(ResourceText))
	{
		ResourceText->SetText(
			FText::FromString(
				FString::Printf(
					TEXT("RESOURCE %d"),
					CDGameState->GetCurrentResources()
				)
			)
		);
	}

	if (!IsValid(CoreActor))
	{
		return;
	}

	const float CoreHealthPercent =
		CoreActor->GetHealthPercent();

	if (IsValid(CoreHPBar))
	{
		CoreHPBar->SetPercent(
			CoreHealthPercent
		);
	}

	if (IsValid(CoreHPText))
	{
		const int32 DisplayPercent =
			FMath::RoundToInt(
				CoreHealthPercent * 100.0f
			);

		CoreHPText->SetText(
			FText::FromString(
				FString::Printf(
					TEXT("CORE %d%%"),
					DisplayPercent
				)
			)
		);
	}
}

void UCDHUDWidget::HandleStartWaveClicked()
{
	UWorld* World = GetWorld();

	if (!IsValid(World))
	{
		return;
	}

	ACDGameMode* CDGameMode =
		Cast<ACDGameMode>(
			UGameplayStatics::GetGameMode(this)
		);

	if (!IsValid(CDGameMode))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"Start wave failed: "
				"CDGameMode is invalid"
			)
		);
		return;
	}

	CDGameMode->RequestStartWave();
}