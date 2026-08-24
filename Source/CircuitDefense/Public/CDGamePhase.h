#pragma once

#include "CoreMinimal.h"
#include "CDGamePhase.generated.h"

UENUM(BlueprintType)
enum class ECDGamePhase:uint8 // 게임 단계 Enum
{
	Preparation UMETA(DisplayName = "Preparation"), // 탑뷰 설치 단계
	Combat UMETA(DisplayName = "Combat"), // 1인칭 웨이브
	WaveClear UMETA(DisplayName = "Wave Clear"), // 웨이브 종료 처리
	GameOver UMETA(DisplayName = "Game Over"), // 코어 파괴
	Victory UMETA(DisplayName = "Victory") // 승리
};

