// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CDGameMode.generated.h"

/**
 * 
 */
UCLASS()
class CIRCUITDEFENSE_API ACDGameMode : public AGameModeBase
{
	GENERATED_BODY()

public :
	ACDGameMode();

protected:
	virtual void StartPlay() override;


};
