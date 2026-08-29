#include "CDPlayerState.h"

ACDPlayerState::ACDPlayerState()
{
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;

	bDead = false;
	bRespawnInvulnerable = false;

	RespawnCount = 0;
}

bool ACDPlayerState::ApplyPlayerDamage(
	float DamageAmount
)
{
	if (
		!HasAuthority()
		|| bDead
		|| bRespawnInvulnerable
		|| DamageAmount <= 0.0f
		)
	{
		return false;
	}

	const float PreviousHealth =
		CurrentHealth;

	CurrentHealth = FMath::Clamp(
		CurrentHealth - DamageAmount,
		0.0f,
		MaxHealth
	);

	if (
		FMath::IsNearlyEqual(
			PreviousHealth,
			CurrentHealth
		)
		)
	{
		return false;
	}

	const bool bDiedFromDamage =
		CurrentHealth <= 0.0f;

	if (bDiedFromDamage)
	{
		bDead = true;
	}

	BroadcastHealthChanged();

	if (bDiedFromDamage)
	{
		OnPlayerDeath.Broadcast();
	}

	return true;
}

bool ACDPlayerState::HealPlayer(
	float HealAmount
)
{
	if (
		!HasAuthority()
		|| bDead
		|| HealAmount <= 0.0f
		)
	{
		return false;
	}

	const float PreviousHealth =
		CurrentHealth;

	CurrentHealth = FMath::Clamp(
		CurrentHealth + HealAmount,
		0.0f,
		MaxHealth
	);

	if (
		FMath::IsNearlyEqual(
			PreviousHealth,
			CurrentHealth
		)
		)
	{
		return false;
	}

	BroadcastHealthChanged();

	return true;
}

bool ACDPlayerState::CompleteRespawn()
{
	if (
		!HasAuthority()
		|| !bDead
		)
	{
		return false;
	}

	MaxHealth = FMath::Max(
		MaxHealth,
		1.0f
	);

	bDead = false;
	CurrentHealth = MaxHealth;

	++RespawnCount;

	SetRespawnInvulnerable(
		true
	);

	BroadcastHealthChanged();

	OnPlayerRespawned.Broadcast(
		RespawnCount
	);

	return true;
}

void ACDPlayerState::SetRespawnInvulnerable(
	bool bNewInvulnerable
)
{
	if (
		!HasAuthority()
		|| bRespawnInvulnerable
		== bNewInvulnerable
		)
	{
		return;
	}

	bRespawnInvulnerable =
		bNewInvulnerable;

	OnPlayerInvulnerabilityChanged.Broadcast(
		bRespawnInvulnerable
	);
}

void ACDPlayerState::ResetForNewGame()
{
	if (!HasAuthority())
	{
		return;
	}

	MaxHealth = FMath::Max(
		MaxHealth,
		1.0f
	);

	CurrentHealth = MaxHealth;
	bDead = false;
	RespawnCount = 0;

	SetRespawnInvulnerable(
		false
	);

	BroadcastHealthChanged();
}

float ACDPlayerState::GetHealthRatio() const
{
	if (MaxHealth <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(
		CurrentHealth / MaxHealth,
		0.0f,
		1.0f
	);
}

void ACDPlayerState::BroadcastHealthChanged()
{
	OnPlayerHealthChanged.Broadcast(
		CurrentHealth,
		MaxHealth
	);
}