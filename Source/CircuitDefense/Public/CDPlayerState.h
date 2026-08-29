#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CDPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnCDPlayerHealthChanged,
	float,
	CurrentHealth,
	float,
	MaxHealth
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(
	FOnCDPlayerDeath
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnCDPlayerRespawned,
	int32,
	RespawnCount
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnCDPlayerInvulnerabilityChanged,
	bool,
	bInvulnerable
);

UCLASS()
class CIRCUITDEFENSE_API ACDPlayerState
	: public APlayerState
{
	GENERATED_BODY()

public:
	ACDPlayerState();

	UFUNCTION(BlueprintCallable, Category = "Player|Health")
	bool ApplyPlayerDamage(
		float DamageAmount
	);

	UFUNCTION(BlueprintCallable, Category = "Player|Health")
	bool HealPlayer(
		float HealAmount
	);

	UFUNCTION(BlueprintCallable, Category = "Player|Respawn")
	bool CompleteRespawn();

	UFUNCTION(BlueprintCallable, Category = "Player|Respawn")
	void SetRespawnInvulnerable(
		bool bNewInvulnerable
	);

	UFUNCTION(BlueprintCallable, Category = "Player|State")
	void ResetForNewGame();

	UFUNCTION(BlueprintPure, Category = "Player|Health")
	float GetMaxHealth() const
	{
		return MaxHealth;
	}

	UFUNCTION(BlueprintPure, Category = "Player|Health")
	float GetCurrentHealth() const
	{
		return CurrentHealth;
	}

	UFUNCTION(BlueprintPure, Category = "Player|Health")
	float GetHealthRatio() const;

	UFUNCTION(BlueprintPure, Category = "Player|State")
	bool IsDead() const
	{
		return bDead;
	}

	UFUNCTION(BlueprintPure, Category = "Player|Respawn")
	bool IsRespawnInvulnerable() const
	{
		return bRespawnInvulnerable;
	}

	UFUNCTION(BlueprintPure, Category = "Player|Respawn")
	int32 GetRespawnCount() const
	{
		return RespawnCount;
	}

	UPROPERTY(
		BlueprintAssignable,
		Category = "Player|Health"
	)
	FOnCDPlayerHealthChanged
		OnPlayerHealthChanged;

	UPROPERTY(
		BlueprintAssignable,
		Category = "Player|State"
	)
	FOnCDPlayerDeath
		OnPlayerDeath;

	UPROPERTY(
		BlueprintAssignable,
		Category = "Player|Respawn"
	)
	FOnCDPlayerRespawned
		OnPlayerRespawned;

	UPROPERTY(
		BlueprintAssignable,
		Category = "Player|Respawn"
	)
	FOnCDPlayerInvulnerabilityChanged
		OnPlayerInvulnerabilityChanged;

protected:
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Player|Health",
		meta = (ClampMin = "1.0")
	)
	float MaxHealth;

	UPROPERTY(
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Category = "Player|Health"
	)
	float CurrentHealth;

	UPROPERTY(
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Category = "Player|State"
	)
	bool bDead;

	UPROPERTY(
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Category = "Player|Respawn"
	)
	bool bRespawnInvulnerable;

	UPROPERTY(
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Category = "Player|Respawn"
	)
	int32 RespawnCount;

private:
	void BroadcastHealthChanged();
};