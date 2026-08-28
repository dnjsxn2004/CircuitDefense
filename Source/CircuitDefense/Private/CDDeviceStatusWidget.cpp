#include "CDDeviceStatusWidget.h"

#include "Components/TextBlock.h"

void UCDDeviceStatusWidget::SetPowerState(
	bool bIsPowered
)
{
	if (!IsValid(PowerStatusText))
	{
		return;
	}

	PowerStatusText->SetText(
		FText::FromString(
			bIsPowered
			? TEXT("POWERED")
			: TEXT("OFFLINE")
		)
	);

	PowerStatusText->SetColorAndOpacity(
		FSlateColor(
			bIsPowered
			? FLinearColor(
				0.1f,
				1.0f,
				0.2f,
				1.0f
			)
			: FLinearColor(
				1.0f,
				0.1f,
				0.1f,
				1.0f
			)
		)
	);
}