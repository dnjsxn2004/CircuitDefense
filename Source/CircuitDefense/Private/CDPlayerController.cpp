// Fill out your copyright notice in the Description page of Project Settings.


#include "CDPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Camera/PlayerCameraManager.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"

ACDPlayerController::ACDPlayerController()
{
	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
}

void ACDPlayerController::BeginPlay()
{
    Super::BeginPlay();

    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
    bShowMouseCursor = false;

    if (!IsLocalController())
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("This PlayerController is not local")
        );
        return;
    }

    ULocalPlayer* LocalPlayer = GetLocalPlayer();

    if (!IsValid(LocalPlayer))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("LocalPlayer is invalid")
        );
        return;
    }

    UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
        LocalPlayer->GetSubsystem<
        UEnhancedInputLocalPlayerSubsystem
        >();

    if (!IsValid(InputSubsystem))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("Enhanced Input Subsystem is invalid")
        );
        return;
    }

    if (!IsValid(CombatMappingContext))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("CombatMappingContext is not assigned")
        );
        return;
    }

    InputSubsystem->AddMappingContext(
        CombatMappingContext,
        0
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Combat input mapping context added")
    );
}

void ACDPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("CDPlayerController SetupInputComponent called")
    );

    UEnhancedInputComponent* EnhancedInputComponent =
        Cast<UEnhancedInputComponent>(InputComponent);

    if (!IsValid(EnhancedInputComponent))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("InputComponent is not EnhancedInputComponent")
        );
        return;
    }

    if (!IsValid(AttackAction))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("AttackAction is not assigned")
        );
        return;
    }

    EnhancedInputComponent->BindAction(
        AttackAction,
        ETriggerEvent::Started,
        this,
        &ACDPlayerController::HandleAttack
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("AttackAction binding completed")
    );
}

inline void ACDPlayerController::HandleAttack(const FInputActionValue& InputActionValue)
{
    UE_LOG(
        LogTemp,
        Display,
        TEXT("Player attack input")
    );

    PerformAttackTrace();
}

void ACDPlayerController::PerformAttackTrace()
{
    if (!IsValid(PlayerCameraManager))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("PlayerCameraManager is invalid")
        );
        return;
    }

    UWorld* World = GetWorld();

    if (!IsValid(World))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("World is invalid")
        );
        return;
    }

    const FVector TraceStart =
        PlayerCameraManager->GetCameraLocation();

    const FVector TraceDirection =
        PlayerCameraManager
        ->GetCameraRotation()
        .Vector();

    const FVector TraceEnd =
        TraceStart + TraceDirection * AttackRange;

    FCollisionQueryParams QueryParams(
        SCENE_QUERY_STAT(PlayerAttackTrace),
        false
    );

    if (APawn* ControlledPawn = GetPawn())
    {
        QueryParams.AddIgnoredActor(ControlledPawn);
    }

    FHitResult HitResult;

    const bool bHit =
        World->LineTraceSingleByChannel(
            HitResult,
            TraceStart,
            TraceEnd,
            ECC_Visibility,
            QueryParams
        );

    const FVector DebugEnd =
        bHit ? HitResult.ImpactPoint : TraceEnd;

    DrawDebugLine(
        World,
        TraceStart,
        DebugEnd,
        bHit ? FColor::Green : FColor::Red,
        false,
        3.0f,
        0,
        2.0f
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "Attack Trace Result - Hit: %s, "
            "Actor: %s, Component: %s, Distance: %.1f"
        ),
        bHit ? TEXT("true") : TEXT("false"),
        *GetNameSafe(HitResult.GetActor()),
        *GetNameSafe(HitResult.GetComponent()),
        HitResult.Distance
    );

    if (!bHit)
    {
        return;
    }

    DrawDebugPoint(
        World,
        HitResult.ImpactPoint,
        20.0f,
        FColor::Yellow,
        false,
        3.0f
    );
}

