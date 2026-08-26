#include "CDCircuitManager.h"

#include "CDPlaceableDevice.h"

#include "Containers/Queue.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ACDCircuitManager::ACDCircuitManager()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ACDCircuitManager::BeginPlay()
{
    Super::BeginPlay();

    RefreshCircuit();
}

void ACDCircuitManager::Tick(
    float DeltaSeconds
)
{
    Super::Tick(DeltaSeconds);

    DrawPowerConnections();
}

void ACDCircuitManager::RefreshCircuit()
{
    ActiveConnections.Reset();

    TArray<ACDPlaceableDevice*> InstalledDevices;

    GatherInstalledDevices(InstalledDevices);

    TSet<ACDPlaceableDevice*> PoweredDevices;
    TQueue<ACDPlaceableDevice*> DeviceQueue;

    for (
        ACDPlaceableDevice* Device
        : InstalledDevices
        )
    {
        if (!IsValid(Device))
        {
            continue;
        }

        if (
            Device->GetDeviceType()
            != ECDDeviceType::PowerSource
            )
        {
            continue;
        }

        PoweredDevices.Add(Device);
        DeviceQueue.Enqueue(Device);
    }

    ACDPlaceableDevice* CurrentDevice = nullptr;

    while (DeviceQueue.Dequeue(CurrentDevice))
    {
        if (!IsValid(CurrentDevice))
        {
            continue;
        }

        if (
            CurrentDevice->GetDeviceType()
            == ECDDeviceType::Attack
            )
        {
            continue;
        }

        const float ConnectionRange =
            CurrentDevice->GetConnectionRange();

        const float ConnectionRangeSquared =
            FMath::Square(ConnectionRange);

        for (
            ACDPlaceableDevice* CandidateDevice
            : InstalledDevices
            )
        {
            if (!IsValid(CandidateDevice))
            {
                continue;
            }

            if (CandidateDevice == CurrentDevice)
            {
                continue;
            }

            if (PoweredDevices.Contains(CandidateDevice))
            {
                continue;
            }

            const float DistanceSquared =
                FVector::DistSquared(
                    CurrentDevice->GetActorLocation(),
                    CandidateDevice->GetActorLocation()
                );

            if (
                DistanceSquared
                > ConnectionRangeSquared
                )
            {
                continue;
            }

            PoweredDevices.Add(CandidateDevice);

            FCDPowerConnection NewConnection;
            NewConnection.SourceDevice = CurrentDevice;
            NewConnection.TargetDevice = CandidateDevice;

            ActiveConnections.Add(
                MoveTemp(NewConnection)
            );

            if (
                CandidateDevice->GetDeviceType()
                == ECDDeviceType::Relay
                )
            {
                DeviceQueue.Enqueue(CandidateDevice);
            }
        }
    }

    for (
        ACDPlaceableDevice* Device
        : InstalledDevices
        )
    {
        if (!IsValid(Device))
        {
            continue;
        }

        Device->SetPowered(
            PoweredDevices.Contains(Device)
        );
    }

    UE_LOG(
        LogTemp,
        Display,
        TEXT(
            "Circuit refreshed - "
            "Installed: %d, Powered: %d, "
            "Connections: %d"
        ),
        InstalledDevices.Num(),
        PoweredDevices.Num(),
        ActiveConnections.Num()
    );
}

void ACDCircuitManager::GatherInstalledDevices(
    TArray<ACDPlaceableDevice*>& OutDevices
) const
{
    OutDevices.Reset();

    TArray<AActor*> FoundActors;

    UGameplayStatics::GetAllActorsOfClass(
        this,
        ACDPlaceableDevice::StaticClass(),
        FoundActors
    );

    for (AActor* FoundActor : FoundActors)
    {
        ACDPlaceableDevice* Device =
            Cast<ACDPlaceableDevice>(FoundActor);

        if (!IsValid(Device))
        {
            continue;
        }

        if (Device->IsActorBeingDestroyed())
        {
            continue;
        }

        if (!Device->IsInstalled())
        {
            continue;
        }

        OutDevices.Add(Device);
    }
}

void ACDCircuitManager::DrawPowerConnections() const
{
    if (!bDrawDebugConnections)
    {
        return;
    }

    UWorld* World = GetWorld();

    if (!IsValid(World))
    {
        return;
    }

    const FVector HeightOffset =
        FVector::UpVector * ConnectionLineHeight;

    for (
        const FCDPowerConnection& Connection
        : ActiveConnections
        )
    {
        ACDPlaceableDevice* SourceDevice =
            Connection.SourceDevice.Get();

        ACDPlaceableDevice* TargetDevice =
            Connection.TargetDevice.Get();

        if (
            !IsValid(SourceDevice)
            || !IsValid(TargetDevice)
            )
        {
            continue;
        }

        DrawDebugLine(
            World,
            SourceDevice->GetActorLocation()
            + HeightOffset,
            TargetDevice->GetActorLocation()
            + HeightOffset,
            ConnectionLineColor,
            false,
            0.0f,
            0,
            ConnectionLineThickness
        );
    }
}