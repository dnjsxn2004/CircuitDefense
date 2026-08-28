#include "CDPlaceableDevice.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Components/WidgetComponent.h"
#include "CDDeviceStatusWidget.h"

ACDPlaceableDevice::ACDPlaceableDevice()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot =
        CreateDefaultSubobject<USceneComponent>(
            TEXT("SceneRoot")
        );

    RootComponent = SceneRoot;

    SceneRoot->SetMobility(
        EComponentMobility::Movable
    );

    DeviceMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(
            TEXT("DeviceMesh")
        );

    DeviceMesh->SetupAttachment(RootComponent);

    DeviceMesh->SetMobility(
        EComponentMobility::Movable
    );

    DeviceMesh->SetUsingAbsoluteLocation(false);
    DeviceMesh->SetUsingAbsoluteRotation(false);
    DeviceMesh->SetUsingAbsoluteScale(false);

    DeviceMesh->SetRelativeLocation(
        FVector::ZeroVector
    );

    DeviceMesh->SetRelativeRotation(
        FRotator::ZeroRotator
    );

    DeviceMesh->SetRelativeScale3D(
        FVector::OneVector
    );

    PowerStatusWidgetComponent =
        CreateDefaultSubobject<UWidgetComponent>(
            TEXT("PowerStatusWidgetComponent")
        );

    PowerStatusWidgetComponent->SetupAttachment(
        SceneRoot
    );

    PowerStatusWidgetComponent->SetRelativeLocation(
        FVector(0.0f, 0.0f, 100.0f)
    );

    PowerStatusWidgetComponent->SetWidgetSpace(
        EWidgetSpace::Screen
    );

    PowerStatusWidgetComponent->SetDrawSize(
        FVector2D(140.0f, 30.0f)
    );

    PowerStatusWidgetComponent->SetCollisionEnabled(
        ECollisionEnabled::NoCollision
    );

    PowerStatusWidgetComponent->SetVisibility(
        false
    );
}

void ACDPlaceableDevice::BeginPlay()
{
    Super::BeginPlay();

    if (IsValid(PowerStatusWidgetComponent))
    {
        PowerStatusWidgetComponent->InitWidget();

        PowerStatusWidget =
            Cast<UCDDeviceStatusWidget>(
                PowerStatusWidgetComponent
                ->GetUserWidgetObject()
            );
    }

    if (!IsValid(PowerStatusWidget))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "Device status widget is invalid - "
                "Device: %s"
            ),
            *GetName()
        );
    }

    UpdatePowerStatusWidget();
}

void ACDPlaceableDevice::CompletePlacement()
{
    if (bInstalled)
    {
        return;
    }

    bPlacementPreview = false;
    bInstalled = true;

    if (DeviceType == ECDDeviceType::PowerSource)
    {
        SetPowered(true);
    }
    else
    {
        SetPowered(false);
    }

    RestoreOriginalMaterials();
    SetActorEnableCollision(true);
    UpdatePowerStatusWidget();

    UE_LOG(
        LogTemp,
        Display,
        TEXT(
            "Device placement completed - "
            "Device: %s, Cost: %d"
        ),
        *GetName(),
        InstallationCost
    );
}

bool ACDPlaceableDevice::IsInstalled() const
{
    return bInstalled;
}

int32 ACDPlaceableDevice::GetInstallationCost() const
{
    return InstallationCost;
}

void ACDPlaceableDevice::SetPlacementPreview(
    bool bIsPreview
)
{
    bPlacementPreview = bIsPreview;

    if (bPlacementPreview)
    {
        bInstalled = false;
        SetPowered(false);

        SetActorEnableCollision(false);
        ApplyPreviewMaterial();
    }
    else
    {
        RestoreOriginalMaterials();
        SetActorEnableCollision(true);
    }

    UE_LOG(
        LogTemp,
        Display,
        TEXT(
            "Device preview changed - "
            "Device: %s, Preview: %s"
        ),
        *GetName(),
        bPlacementPreview
        ? TEXT("true")
        : TEXT("false")
    );
}

bool ACDPlaceableDevice::IsPlacementPreview() const
{
    return bPlacementPreview;
}

void ACDPlaceableDevice::SetPlacementValid(
    bool bIsValid
)
{
    if (!IsValid(PreviewDynamicMaterial))
    {
        return;
    }

    const FLinearColor PreviewColor =
        bIsValid
        ? FLinearColor(
            0.05f,
            1.0f,
            0.1f,
            1.0f
        )
        : FLinearColor(
            1.0f,
            0.05f,
            0.05f,
            1.0f
        );

    PreviewDynamicMaterial->SetVectorParameterValue(
        TEXT("PreviewColor"),
        PreviewColor
    );
}

void ACDPlaceableDevice::ApplyPreviewMaterial()
{
    if (!IsValid(DeviceMesh))
    {
        return;
    }

    OriginalMaterials.Reset();

    const int32 MaterialCount =
        DeviceMesh->GetNumMaterials();

    for (
        int32 MaterialIndex = 0;
        MaterialIndex < MaterialCount;
        ++MaterialIndex
        )
    {
        OriginalMaterials.Add(
            DeviceMesh->GetMaterial(MaterialIndex)
        );
    }

    if (!IsValid(PreviewMaterialBase))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "PreviewMaterialBase is not assigned - "
                "Device: %s"
            ),
            *GetName()
        );
        return;
    }

    PreviewDynamicMaterial =
        UMaterialInstanceDynamic::Create(
            PreviewMaterialBase,
            this
        );

    if (!IsValid(PreviewDynamicMaterial))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "Failed to create preview material - "
                "Device: %s"
            ),
            *GetName()
        );
        return;
    }

    for (
        int32 MaterialIndex = 0;
        MaterialIndex < MaterialCount;
        ++MaterialIndex
        )
    {
        DeviceMesh->SetMaterial(
            MaterialIndex,
            PreviewDynamicMaterial
        );
    }

    SetPlacementValid(true);
}

void ACDPlaceableDevice::RestoreOriginalMaterials()
{
    if (!IsValid(DeviceMesh))
    {
        return;
    }

    for (
        int32 MaterialIndex = 0;
        MaterialIndex < OriginalMaterials.Num();
        ++MaterialIndex
        )
    {
        DeviceMesh->SetMaterial(
            MaterialIndex,
            OriginalMaterials[MaterialIndex]
        );
    }

    OriginalMaterials.Reset();
    PreviewDynamicMaterial = nullptr;
}

int32 ACDPlaceableDevice::GetRefundAmount() const
{
    return FMath::RoundToInt(
        static_cast<float>(InstallationCost)
        * RefundRate
    );
}

ECDDeviceType ACDPlaceableDevice::GetDeviceType() const
{
    return DeviceType;
}

bool ACDPlaceableDevice::IsPowered() const
{
    return bPowered;
}

void ACDPlaceableDevice::SetPowered(
    bool bNewPowered
)
{
    if (bPowered == bNewPowered)
    {
        UpdatePowerStatusWidget();
        return;
    }

    bPowered = bNewPowered;

    UpdatePowerStatusWidget();

    OnDevicePowerChanged.Broadcast(
        bPowered
    );

    UE_LOG(
        LogTemp,
        Display,
        TEXT(
            "Device power changed - "
            "Device: %s, Powered: %s"
        ),
        *GetName(),
        bPowered
        ? TEXT("true")
        : TEXT("false")
    );
}

float ACDPlaceableDevice::GetConnectionRange() const
{
    return ConnectionRange;
}

void ACDPlaceableDevice::UpdatePowerStatusWidget()
{
    if (!IsValid(PowerStatusWidgetComponent))
    {
        return;
    }

    const bool bShouldShowStatus =
        bInstalled
        && !bPlacementPreview;

    PowerStatusWidgetComponent->SetVisibility(
        bShouldShowStatus,
        true
    );

    if (
        !bShouldShowStatus
        || !IsValid(PowerStatusWidget)
        )
    {
        return;
    }

    PowerStatusWidget->SetPowerState(
        bPowered
    );
}