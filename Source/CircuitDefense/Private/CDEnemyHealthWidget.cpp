#include "CDEnemyHealthWidget.h"

#include "Components/ProgressBar.h"

void UCDEnemyHealthWidget::SetHealthPercent(
	float InHealthPercent
)
{
	if (!IsValid(HealthBar))
	{
		return;
	}

	HealthBar->SetPercent(
		FMath::Clamp(
			InHealthPercent,
			0.0f,
			1.0f
		)
	);
}