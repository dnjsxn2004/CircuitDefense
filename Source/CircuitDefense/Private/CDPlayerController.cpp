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
#include "CDEnemy.h"
#include "CDGameState.h"
#include "CDHUDWidget.h"
#include "CDPlaceableDevice.h"
#include "Engine/OverlapResult.h"

ACDPlayerController::ACDPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;

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

    if (!IsValid(HUDWidgetClass))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("HUDWidgetClass is not assigned")
        );
        return;
    }

    HUDWidget = CreateWidget<UCDHUDWidget>(
        this,
        HUDWidgetClass
    );

    if (!IsValid(HUDWidget))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("Failed to create HUD widget")
        );
        return;
    }
    
    HUDWidget->SetVisibility(
        ESlateVisibility::Visible
    );

    HUDWidget->SetIsEnabled(true);

    HUDWidget->AddToViewport(100);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "HUD widget added - Class: %s"
        ),
        *GetNameSafe(HUDWidget->GetClass())
    );
}

void ACDPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (IsValid(PlacementPreview))
    {
        UpdatePlacementPreview();
    }
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

    if (!IsValid(StartPlacementAction))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("StartPlacementAction is not assigned")
        );
        return;
    }

    if (!IsValid(CancelPlacementAction))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("CancelPlacementAction is not assigned")
        );
        return;
    }

    EnhancedInputComponent->BindAction(
        AttackAction,
        ETriggerEvent::Started,
        this,
        &ACDPlayerController::HandleAttack
    );
    
    EnhancedInputComponent->BindAction(
        StartPlacementAction,
        ETriggerEvent::Started,
        this,
        &ACDPlayerController::HandleStartPlacement
    );
    
    EnhancedInputComponent->BindAction(
        CancelPlacementAction,
        ETriggerEvent::Started,
        this,
        &ACDPlayerController::HandleCancelPlacement
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Player input binding completed")
    );
}

void ACDPlayerController::HandleAttack(const FInputActionValue& InputActionValue)
{
    UWorld* World = GetWorld();

    if (!IsValid(World))
    {
        return;
    }

    ACDGameState* CDGameState = World->GetGameState<ACDGameState>();

    if (!IsValid(CDGameState))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("Input blocked: CDGameState is invalid")
        );
        return;
    }

    switch (CDGameState->CurrentPhase)
    {
    case ECDGamePhase::Combat:
        PerformAttackTrace();
        break;

    case ECDGamePhase::Preparation:
        ConfirmPlacement();
        break;

    default:
        UE_LOG(
            LogTemp,
            Display,
            TEXT("Input blocked: Current phase is %d"),
            static_cast<int32>(CDGameState->CurrentPhase)
        );
        break;
    }
}

void ACDPlayerController::HandleStartPlacement(const FInputActionValue& InputActionValue)
{
    UWorld* World = GetWorld();

    if (!IsValid(World))
    {
        return;
    }

    ACDGameState* CDGameState = World->GetGameState<ACDGameState>();

    if (!IsValid(CDGameState))
    {
        return;
    }

    if (CDGameState->CurrentPhase != ECDGamePhase::Preparation)
    {
        UE_LOG(
            LogTemp,
            Display,
            TEXT("Placement can only start during Prepartion")
        );
        return;
    }

    if (IsValid(PlacementPreview))
    {
        UE_LOG(
            LogTemp,
            Display,
            TEXT("Placement preview already exists")
        );
        return;
    }

    if (!IsValid(DefaultPlaceableDeviceClass))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("DefaultPlaceableDeviceCalss is not assigned")
        );
        return;
    }

    FActorSpawnParameters SpawnParameters;

    SpawnParameters.Owner = this;
    SpawnParameters.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    PlacementPreview =
        World->SpawnActor<ACDPlaceableDevice>(
            DefaultPlaceableDeviceClass,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            SpawnParameters
        );

    if (!IsValid(PlacementPreview))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("Failed to create placement preview")
        );
        return;
    }

    PlacementPreview->SetPlacementPreview(true);
    bCanPlacePreview = false;
    UpdatePlacementPreview();

    UE_LOG(
        LogTemp,
        Display,
        TEXT("Placement preview created: %s"),
        *GetNameSafe(PlacementPreview)
    );
}

void ACDPlayerController::HandleCancelPlacement(const FInputActionValue& InputActionValue)
{
    CancelPlacement();
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

    /*
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
    */


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

    /*
    DrawDebugPoint(
        World,
        HitResult.ImpactPoint,
        20.0f,
        FColor::Yellow,
        false,
        3.0f
    );
    */

    ACDEnemy* HitEnemy =
        Cast<ACDEnemy>(
            HitResult.GetActor()
        );

    if (!IsValid(HitEnemy))
    {
        UE_LOG(
            LogTemp,
            Display,
            TEXT(
                "Attack hit non-enemy actor: %s"
            ),
            *GetNameSafe(HitResult.GetActor())
        );

        return;
    }

    if (HitEnemy->IsActorBeingDestroyed())
    {
        UE_LOG(
            LogTemp,
            Display,
            TEXT(
                "Attack ignored: enemy is being destroyed"
            )
        );

        return;
    }

    const FString HitEnemyName =
        HitEnemy->GetName();

    HitEnemy->ApplyEnemyDamage(
        AttackDamage
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "Player damaged enemy: %s, Damage: %.0f"
        ),
        *HitEnemyName,
        AttackDamage
        );
}

void ACDPlayerController::UpdatePlacementPreview()
{
    if (!IsValid(PlacementPreview))
    {
        bCanPlacePreview = false;
        return;
    }

    if (!IsValid(PlayerCameraManager))
    {
        bCanPlacePreview = false;
        return;
    }

    UWorld* World = GetWorld();

    if (!IsValid(World))
    {
        bCanPlacePreview = false;
        return;
    }

    const FVector TraceStart = PlayerCameraManager->GetCameraLocation();

    const FVector TraceDirection = PlayerCameraManager->GetCameraRotation().Vector();

    const FVector TraceEnd = TraceStart + TraceDirection * PlacementTraceDistance;

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(DevicePlacementTrace), false);

    if (APawn* ControlledPawn = GetPawn())
    {
        QueryParams.AddIgnoredActor(ControlledPawn);
    }
    
    QueryParams.AddIgnoredActor(PlacementPreview);

    FHitResult HitResult;

    const bool bHit =
        World->LineTraceSingleByChannel(
            HitResult,
            TraceStart,
            TraceEnd,
            ECC_Visibility,
            QueryParams
        );

    if (!bHit)
    {
        bCanPlacePreview = false;

        PlacementPreview->SetActorHiddenInGame(true);
        return;
    }

    PlacementPreview->SetActorHiddenInGame(false);

    PlacementPreview->SetActorLocation(
        HitResult.ImpactPoint
    );

    bCanPlacePreview =
        IsPlacementLocationValid(HitResult);

    DrawDebugBox(
        World,
        HitResult.ImpactPoint,
        PlacementCheckExtent,
        bCanPlacePreview
        ? FColor::Green
        : FColor::Red,
        false,
        0.0f,
        0,
        2.0f
    );
}

void ACDPlayerController::ConfirmPlacement()
{
    if (!IsValid(PlacementPreview))
    {
        UE_LOG(
            LogTemp,
            Display,
            TEXT("No placement preview to confirm")
        );
        return;
    }

    if (!bCanPlacePreview)
    {
        UE_LOG(
            LogTemp,
            Display,
            TEXT("Current location cannot be used")
        );
        return;
    }

    PlacementPreview->SetActorHiddenInGame(false);
    PlacementPreview->CompletePlacement();

    UE_LOG(
        LogTemp,
        Display,
        TEXT(
            "Device placement confirmed: %s"
        ),
        *GetNameSafe(PlacementPreview)
    );

    PlacementPreview = nullptr;
    bCanPlacePreview = false;
}

void ACDPlayerController::CancelPlacement()
{
    if (!IsValid(PlacementPreview))
    {
        return;
    }

    UE_LOG(
        LogTemp,
        Display,
        TEXT(
            "Device placement canceled: %s"
        ),
        *GetNameSafe(PlacementPreview)
    );

    PlacementPreview->Destroy();
    PlacementPreview = nullptr;
    bCanPlacePreview = false;
}

bool ACDPlayerController::IsPlacementLocationValid(const FHitResult& SurfaceHit) const
{
    UWorld* World = GetWorld();

    if (!IsValid(World))
    {
        return false;
    }

    if (!IsValid(PlacementPreview))
    {
        return false;
    }

    if (
        SurfaceHit.ImpactNormal.Z
        < MinimumPlacementNormalZ
        )
    {
        return false;
    }

    FCollisionObjectQueryParams ObjectQueryParams;

    ObjectQueryParams.AddObjectTypesToQuery(
        ECC_WorldStatic
    );

    ObjectQueryParams.AddObjectTypesToQuery(
        ECC_WorldDynamic
    );

    FCollisionQueryParams QueryParams(
        SCENE_QUERY_STAT(DevicePlacementOverlap),
        false
    );

    QueryParams.AddIgnoredActor(PlacementPreview);

    if (APawn* ControlledPawn = GetPawn())
    {
        QueryParams.AddIgnoredActor(ControlledPawn);
    }

    TArray<FOverlapResult> OverlapResults;

    World->OverlapMultiByObjectType(
        OverlapResults,
        SurfaceHit.ImpactPoint,
        FQuat::Identity,
        ObjectQueryParams,
        FCollisionShape::MakeBox(
            PlacementCheckExtent
        ),
        QueryParams
    );

    for (const FOverlapResult& OverlapResult
        : OverlapResults)
    {
        ACDPlaceableDevice* OtherDevice =
            Cast<ACDPlaceableDevice>(
                OverlapResult.GetActor()
            );

        if (!IsValid(OtherDevice))
        {
            continue;
        }

        if (OtherDevice == PlacementPreview)
        {
            continue;
        }

        return false;
    }

    return true;
}