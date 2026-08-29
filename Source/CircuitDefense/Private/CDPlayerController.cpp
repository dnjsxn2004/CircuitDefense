// Fill out your copyright notice in the Description page of Project Settings.


#include "CDPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "CDEnemy.h"
#include "CDGameState.h"
#include "CDPlayerState.h"
#include "CDHUDWidget.h"
#include "CDPlaceableDevice.h"
#include "Engine/OverlapResult.h"
#include "CDCircuitManager.h"
#include "Kismet/GameplayStatics.h"
#include "CDCore.h"
#include "CDWaveSpawner.h"

ACDPlayerController::ACDPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;

    bShowMouseCursor = false;
    bEnableClickEvents = false;
    bEnableMouseOverEvents = false;
}

bool ACDPlayerController::ApplyDamageToPlayer(
    float DamageAmount
)
{
    ACDPlayerState* CDPlayerState =
        GetPlayerState<ACDPlayerState>();

    if (!IsValid(CDPlayerState))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("Player damage failed: CDPlayerState is invalid")
        );
        return false;
    }

    return CDPlayerState->ApplyPlayerDamage(
        DamageAmount
    );
}

void ACDPlayerController::SetPlayerGameplayEnabled(
    bool bEnabled
)
{
    bPlayerGameplayEnabled = bEnabled;

    if (!bPlayerGameplayEnabled)
    {
        if (IsValid(PlacementPreview))
        {
            CancelPlacement();
        }

        FInputModeGameOnly InputMode;
        SetInputMode(InputMode);
        bShowMouseCursor = false;
        bEnableClickEvents = false;
        bEnableMouseOverEvents = false;
        return;
    }

    UWorld* World = GetWorld();

    if (!IsValid(World))
    {
        return;
    }

    ACDGameState* CDGameState =
        World->GetGameState<ACDGameState>();

    if (IsValid(CDGameState))
    {
        HandleGamePhaseChanged(
            CDGameState->CurrentPhase
        );
    }
}

bool ACDPlayerController::CanProcessGameplayInput() const
{
    if (!bPlayerGameplayEnabled)
    {
        return false;
    }

    const ACDPlayerState* CDPlayerState =
        GetPlayerState<ACDPlayerState>();

    return IsValid(CDPlayerState)
        && !CDPlayerState->IsDead();
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

    CircuitManager =
        Cast<ACDCircuitManager>(
            UGameplayStatics::GetActorOfClass(
                this,
                ACDCircuitManager::StaticClass()
            )
        );

    if (!IsValid(CircuitManager))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "CDCircuitManager was not found "
                "in the level"
            )
        );
    }
    else
    {
        UE_LOG(
            LogTemp,
            Display,
            TEXT("CDCircuitManager found")
        );
    }

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

    ACDGameState* CDGameState =
        GetWorld()->GetGameState<ACDGameState>();

    if (IsValid(CDGameState))
    {
        CDGameState->OnGamePhaseChanged.RemoveDynamic(
            this,
            &ACDPlayerController::HandleGamePhaseChanged
        );

        CDGameState->OnGamePhaseChanged.AddDynamic(
            this,
            &ACDPlayerController::HandleGamePhaseChanged
        );

        HandleGamePhaseChanged(
            CDGameState->CurrentPhase
        );
    }

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

    if (
        IsValid(PlacementPreview)
        && CanProcessGameplayInput()
        )
    {
        UpdatePlacementPreview();
    }
}

void ACDPlayerController::HandleAttack(const FInputActionValue& InputActionValue)
{
    if (!CanProcessGameplayInput())
    {
        return;
    }

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

void ACDPlayerController::HandleSelectPowerSource(
    const FInputActionValue& InputActionValue
)
{
    if (!CanProcessGameplayInput())
    {
        return;
    }

    StartPlacement(PowerSourceDeviceClass);
}

void ACDPlayerController::HandleSelectRelay(
    const FInputActionValue& InputActionValue
)
{
    if (!CanProcessGameplayInput())
    {
        return;
    }

    StartPlacement(RelayDeviceClass);
}

void ACDPlayerController::HandleSelectAttackDevice(
    const FInputActionValue& InputActionValue
)
{
    if (!CanProcessGameplayInput())
    {
        return;
    }

    StartPlacement(AttackDeviceClass);
}

void ACDPlayerController::HandleCancelPlacement(const FInputActionValue& InputActionValue)
{
    if (!CanProcessGameplayInput())
    {
        return;
    }

    CancelPlacement();
}

void ACDPlayerController::PerformAttackTrace()
{
    if (!CanProcessGameplayInput())
    {
        return;
    }

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

    UWorld* World = GetWorld();

    if (!IsValid(World))
    {
        bCanPlacePreview = false;
        return;
    }

    FVector TraceStart;
    FVector TraceDirection;

    const bool bDeprojected =
        DeprojectMousePositionToWorld(
            TraceStart,
            TraceDirection
        );

    if (!bDeprojected)
    {
        if (IsValid(HUDWidget))
        {
            HUDWidget->SetPlacementStatusMessage(
                TEXT("INVALID CURSOR POSITION")
            );
        }

        bCanPlacePreview = false;

        PlacementPreview->SetActorHiddenInGame(true);
        return;
    }
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
        if (IsValid(HUDWidget))
        {
            HUDWidget->SetPlacementStatusMessage(
                TEXT("NO PLACEMENT SURFACE")
            );
        }

        bCanPlacePreview = false;

        PlacementPreview->SetActorHiddenInGame(true);
        return;
    }

    PlacementPreview->SetActorHiddenInGame(false);

    PlacementPreview->SetActorLocation(
        HitResult.ImpactPoint
    );

    FString FailureMessage;

    bCanPlacePreview =
        IsPlacementLocationValid(
            HitResult,
            FailureMessage
        );

    PlacementPreview->SetPlacementValid(
        bCanPlacePreview
    );

    if (IsValid(HUDWidget))
    {
        if (bCanPlacePreview)
        {
            HUDWidget
                ->ClearPlacementStatusMessage();
        }
        else
        {
            HUDWidget
                ->SetPlacementStatusMessage(
                    FailureMessage
                );
        }
    }
}

void ACDPlayerController::ConfirmPlacement()
{
    if (!CanProcessGameplayInput())
    {
        return;
    }

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

    UWorld* World = GetWorld();

    if (!IsValid(World))
    {
        return;
    }

    ACDGameState* CDGameState =
        World->GetGameState<ACDGameState>();

    if (!IsValid(CDGameState))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "Placement failed: "
                "CDGameState is invalid"
            )
        );
        return;
    }

    const int32 InstallationCost =
        PlacementPreview->GetInstallationCost();

    if (
        !CDGameState->SpendResources(
            InstallationCost
        )
        )
    {
        return;
    }

    PlacementPreview->SetActorHiddenInGame(false);
    PlacementPreview->CompletePlacement();

    if (IsValid(CircuitManager))
    {
        CircuitManager->RefreshCircuit();
    }

    UE_LOG(
        LogTemp,
        Display,
        TEXT(
            "Device placement confirmed - "
            "Device: %s, Cost: %d, Remaining: %d"
        ),
        *GetNameSafe(PlacementPreview),
        InstallationCost,
        CDGameState->GetCurrentResources()
    );

    PlacementPreview = nullptr;
    bCanPlacePreview = false;

    if (IsValid(HUDWidget))
    {
        HUDWidget->ClearSelectedDeviceInfo();
    }
}

void ACDPlayerController::CancelPlacement()
{
    if (IsValid(HUDWidget))
    {
        HUDWidget->ClearPlacementStatusMessage();
    }

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

    if (IsValid(HUDWidget))
    {
        HUDWidget->ClearSelectedDeviceInfo();
    }
}

bool ACDPlayerController::IsPlacementLocationValid(
    const FHitResult& SurfaceHit,
    FString& OutFailureMessage
) const
{
    OutFailureMessage.Reset();

    UWorld* World = GetWorld();

    if (!IsValid(World))
    {
        OutFailureMessage =
            TEXT("PLACEMENT UNAVAILABLE");

        return false;
    }

    if (!IsValid(PlacementPreview))
    {
        OutFailureMessage =
            TEXT("PLACEMENT PREVIEW INVALID");

        return false;
    }

    if (
        SurfaceHit.ImpactNormal.Z
        < MinimumPlacementNormalZ
        )
    {
        OutFailureMessage =
            TEXT("INVALID SURFACE");

        return false;
    }

    const ACDGameState* CDGameState =
        World->GetGameState<ACDGameState>();

    if (!IsValid(CDGameState))
    {
        OutFailureMessage =
            TEXT("PLACEMENT UNAVAILABLE");

        return false;
    }

    const int32 InstallationCost =
        PlacementPreview->GetInstallationCost();

    if (!CDGameState->CanAfford(InstallationCost))
    {
        OutFailureMessage =
            TEXT("NOT ENOUGH RESOURCES");

        return false;
    }

    const float ProtectedRadiusSquared =
        FMath::Square(ProtectedActorRadius);

    AActor* CoreActor =
        UGameplayStatics::GetActorOfClass(
            World,
            ACDCore::StaticClass()
        );

    if (
        IsValid(CoreActor)
        && FVector::DistSquared2D(
            SurfaceHit.ImpactPoint,
            CoreActor->GetActorLocation()
        ) <= ProtectedRadiusSquared
        )
    {
        OutFailureMessage =
            TEXT("PROTECTED AREA");

        return false;
    }

    AActor* WaveSpawnerActor =
        UGameplayStatics::GetActorOfClass(
            World,
            ACDWaveSpawner::StaticClass()
        );

    if (
        IsValid(WaveSpawnerActor)
        && FVector::DistSquared2D(
            SurfaceHit.ImpactPoint,
            WaveSpawnerActor->GetActorLocation()
        ) <= ProtectedRadiusSquared
        )
    {
        OutFailureMessage =
            TEXT("PROTECTED AREA");

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

    QueryParams.AddIgnoredActor(
        PlacementPreview
    );

    if (APawn* ControlledPawn = GetPawn())
    {
        QueryParams.AddIgnoredActor(
            ControlledPawn
        );
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

    for (
        const FOverlapResult& OverlapResult
        : OverlapResults
        )
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

        OutFailureMessage =
            TEXT("SPACE OCCUPIED");

        return false;
    }

    return true;
}

void ACDPlayerController::HandleRemoveDevice(
    const FInputActionValue& InputActionValue
)
{
    if (!CanProcessGameplayInput())
    {
        return;
    }

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

    if (
        CDGameState->CurrentPhase
        != ECDGamePhase::Preparation
        )
    {
        UE_LOG(
            LogTemp,
            Display,
            TEXT(
                "Devices can only be removed "
                "during Preparation"
            )
        );
        return;
    }

    if (IsValid(PlacementPreview))
    {
        UE_LOG(
            LogTemp,
            Display,
            TEXT(
                "Cancel the placement preview "
                "before removing a device"
            )
        );
        return;
    }

    TryRemoveDevice();
}

void ACDPlayerController::TryRemoveDevice()
{
    if (!CanProcessGameplayInput())
    {
        return;
    }

    if (!IsValid(PlayerCameraManager))
    {
        return;
    }

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

    const FVector TraceStart =
        PlayerCameraManager->GetCameraLocation();

    const FVector TraceDirection =
        PlayerCameraManager
        ->GetCameraRotation()
        .Vector();

    const FVector TraceEnd =
        TraceStart
        + TraceDirection * PlacementTraceDistance;

    FCollisionQueryParams QueryParams(
        SCENE_QUERY_STAT(DeviceRemovalTrace),
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

    if (!bHit)
    {
        UE_LOG(
            LogTemp,
            Display,
            TEXT("Device removal trace missed")
        );
        return;
    }

    ACDPlaceableDevice* HitDevice =
        Cast<ACDPlaceableDevice>(
            HitResult.GetActor()
        );

    if (!IsValid(HitDevice))
    {
        UE_LOG(
            LogTemp,
            Display,
            TEXT(
                "Removal target is not a device: %s"
            ),
            *GetNameSafe(HitResult.GetActor())
        );
        return;
    }

    if (!HitDevice->IsInstalled())
    {
        UE_LOG(
            LogTemp,
            Display,
            TEXT(
                "Removal target is not installed"
            )
        );
        return;
    }

    const int32 RefundAmount =
        HitDevice->GetRefundAmount();

    const FString DeviceName =
        HitDevice->GetName();

    const bool bDestroyed =
        HitDevice->Destroy();

    if (!bDestroyed)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "Failed to remove device: %s"
            ),
            *DeviceName
        );
        return;
    }

    CDGameState->AddResources(RefundAmount);

    if (IsValid(CircuitManager))
    {
        CircuitManager->RefreshCircuit();
    }

    UE_LOG(
        LogTemp,
        Display,
        TEXT(
            "Device removed - "
            "Device: %s, Refund: %d, "
            "Resources: %d"
        ),
        *DeviceName,
        RefundAmount,
        CDGameState->GetCurrentResources()
    );
}

void ACDPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    UEnhancedInputComponent* EnhancedInputComponent =
        Cast<UEnhancedInputComponent>(InputComponent);

    if (!IsValid(EnhancedInputComponent))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "InputComponent is not "
                "EnhancedInputComponent"
            )
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

    if (!IsValid(SelectPowerSourceAction))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "SelectPowerSourceAction is not assigned"
            )
        );
        return;
    }

    if (!IsValid(SelectRelayAction))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "SelectRelayAction is not assigned"
            )
        );
        return;
    }

    if (!IsValid(SelectAttackDeviceAction))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "SelectAttackDeviceAction is not assigned"
            )
        );
        return;
    }

    if (!IsValid(CancelPlacementAction))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "CancelPlacementAction is not assigned"
            )
        );
        return;
    }

    if (!IsValid(RemoveDeviceAction))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "RemoveDeviceAction is not assigned"
            )
        );
        return;
    }

    if (IsValid(RestartGameAction))
    {
        EnhancedInputComponent->BindAction(
            RestartGameAction,
            ETriggerEvent::Started,
            this,
            &ACDPlayerController::HandleRestartGame
        );
    }
    else
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("RestartGameAction is not assigned")
        );
    }

    EnhancedInputComponent->BindAction(
        AttackAction,
        ETriggerEvent::Started,
        this,
        &ACDPlayerController::HandleAttack
    );

    EnhancedInputComponent->BindAction(
        SelectPowerSourceAction,
        ETriggerEvent::Started,
        this,
        &ACDPlayerController::HandleSelectPowerSource
    );

    EnhancedInputComponent->BindAction(
        SelectRelayAction,
        ETriggerEvent::Started,
        this,
        &ACDPlayerController::HandleSelectRelay
    );

    EnhancedInputComponent->BindAction(
        SelectAttackDeviceAction,
        ETriggerEvent::Started,
        this,
        &ACDPlayerController::HandleSelectAttackDevice
    );

    EnhancedInputComponent->BindAction(
        CancelPlacementAction,
        ETriggerEvent::Started,
        this,
        &ACDPlayerController::HandleCancelPlacement
    );

    EnhancedInputComponent->BindAction(
        RemoveDeviceAction,
        ETriggerEvent::Started,
        this,
        &ACDPlayerController::HandleRemoveDevice
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Player input binding completed")
    );
}

void ACDPlayerController::StartPlacement(
    TSubclassOf<ACDPlaceableDevice> DeviceClass
)
{
    if (!CanProcessGameplayInput())
    {
        return;
    }

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

    if (
        CDGameState->CurrentPhase
        != ECDGamePhase::Preparation
        )
    {
        UE_LOG(
            LogTemp,
            Display,
            TEXT(
                "Placement can only start "
                "during Preparation"
            )
        );
        return;
    }

    if (!DeviceClass)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "Selected device class "
                "is not assigned"
            )
        );
        return;
    }

    if (IsValid(PlacementPreview))
    {
        CancelPlacement();
    }

    FActorSpawnParameters SpawnParameters;

    SpawnParameters.Owner = this;

    SpawnParameters.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    PlacementPreview =
        World->SpawnActor<ACDPlaceableDevice>(
            DeviceClass,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            SpawnParameters
        );

    if (!IsValid(PlacementPreview))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "Failed to create placement preview"
            )
        );
        return;
    }

    PlacementPreview->SetPlacementPreview(true);

    if (IsValid(HUDWidget))
    {
        HUDWidget->SetSelectedDeviceInfo(
            PlacementPreview->GetDeviceType(),
            PlacementPreview->GetInstallationCost()
        );
    }

    bCanPlacePreview = false;

    UpdatePlacementPreview();

    UE_LOG(
        LogTemp,
        Display,
        TEXT(
            "Placement preview created - "
            "Class: %s, Cost: %d"
        ),
        *GetNameSafe(DeviceClass.Get()),
        PlacementPreview->GetInstallationCost()
    );
}

void ACDPlayerController::HandleGamePhaseChanged(
    ECDGamePhase NewPhase
)
{
    const bool bIsResultPhase =
        NewPhase == ECDGamePhase::Victory
        || NewPhase == ECDGamePhase::GameOver;

    if (!bPlayerGameplayEnabled && !bIsResultPhase)
    {
        if (IsValid(PlacementPreview))
        {
            CancelPlacement();
        }

        FInputModeGameOnly InputMode;
        SetInputMode(InputMode);
        bShowMouseCursor = false;
        bEnableClickEvents = false;
        bEnableMouseOverEvents = false;
        return;
    }

    if (NewPhase == ECDGamePhase::Preparation)
    {
        FInputModeGameAndUI InputMode;

        if (IsValid(HUDWidget))
        {
            InputMode.SetWidgetToFocus(
                HUDWidget->TakeWidget()
            );
        }

        InputMode.SetLockMouseToViewportBehavior(
            EMouseLockMode::DoNotLock
        );

        InputMode.SetHideCursorDuringCapture(false);

        SetInputMode(InputMode);

        bShowMouseCursor = true;
        bEnableClickEvents = true;
        bEnableMouseOverEvents = true;

        return;
    }

    if (
        NewPhase == ECDGamePhase::Victory
        || NewPhase == ECDGamePhase::GameOver
        )
    {
        if (IsValid(PlacementPreview))
        {
            CancelPlacement();
        }

        FInputModeGameAndUI InputMode;

        if (IsValid(HUDWidget))
        {
            InputMode.SetWidgetToFocus(
                HUDWidget->TakeWidget()
            );
        }

        InputMode.SetLockMouseToViewportBehavior(
            EMouseLockMode::DoNotLock
        );

        InputMode.SetHideCursorDuringCapture(false);

        SetInputMode(InputMode);

        bShowMouseCursor = true;
        bEnableClickEvents = true;
        bEnableMouseOverEvents = true;

        return;
    }

    if (IsValid(PlacementPreview))
    {
        CancelPlacement();
    }

    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);

    bShowMouseCursor = false;
    bEnableClickEvents = false;
    bEnableMouseOverEvents = false;
}

void ACDPlayerController::HandleRestartGame(
    const FInputActionValue& InputActionValue
)
{
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

    if (
        CDGameState->CurrentPhase
        != ECDGamePhase::Victory
        && CDGameState->CurrentPhase
        != ECDGamePhase::GameOver
        )
    {
        UE_LOG(
            LogTemp,
            Display,
            TEXT(
                "Restart ignored - Current phase: %d"
            ),
            static_cast<int32>(
                CDGameState->CurrentPhase
                )
        );

        return;
    }

    RestartCurrentLevel();
}

void ACDPlayerController::RestartCurrentLevel()
{
    UWorld* World = GetWorld();

    if (!IsValid(World))
    {
        return;
    }

    const FString CurrentLevelName =
        UGameplayStatics::GetCurrentLevelName(
            this,
            true
        );

    if (CurrentLevelName.IsEmpty())
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("Restart failed: level name is empty")
        );

        return;
    }

    UE_LOG(
        LogTemp,
        Display,
        TEXT("Restarting level: %s"),
        *CurrentLevelName
    );

    UGameplayStatics::OpenLevel(
        this,
        FName(*CurrentLevelName)
    );
}
