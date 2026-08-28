// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CDEnemy.generated.h"

class ACDCore;
class UStaticMeshComponent;
class UWidgetComponent;
class UCDEnemyHealthWidget;

UCLASS()
class CIRCUITDEFENSE_API ACDEnemy : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACDEnemy();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void ApplyEnemyDamage(float DamageAmount);

	UFUNCTION(BlueprintPure, Category = "Enemy")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Enemy|Reward")
	int32 GetResourceReward() const;

	UFUNCTION(BlueprintPure, Category = "Enemy|State")
	bool WasKilled() const;

	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void InitializeForWave(
		float InMaxHealth,
		float InMoveSpeed,
		float InCoreDamage,
		int32 InResourceReward
	);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	TObjectPtr<UStaticMeshComponent> EnemyMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Movement", meta = (ClampMin = "0.0"))
	float MoveSpeed = 200.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy|Attack", meta = (ClampMin = "0.0"))
	float CoreDamage = 10.0f;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy|Attack", meta = (ClampMin = "0.0"))
	float ReachDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 30.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy|Health")
	float CurrentHealth = 30.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Enemy|Reward",
		meta = (ClampMin = "0")
	)
	int32 ResourceReward = 5;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Enemy|UI"
	)
	TObjectPtr<UWidgetComponent> HealthWidgetComponent;

private:
	UPROPERTY()
	TObjectPtr<ACDCore> TargetCore;

	UPROPERTY()
	TObjectPtr<UCDEnemyHealthWidget> HealthWidget;

	void UpdateHealthWidget();

	bool bReachedCore = false;
	bool bDead = false;

	void FindTargetCore();
	void ReachCore();

};
