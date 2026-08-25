#include "CDCircuitManager.h"

#include "CDPlaceableDevice.h"
#include "Kismet/GameplayStatics.h"
#include "Containers/Queue.h"

ACDCircuitManager::ACDCircuitManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ACDCircuitManager::BeginPlay()
{
    Super::BeginPlay();

    RefreshCircuit();
}

void ACDCircuitManager::RefreshCircuit()
{
    TArray<ACDPlaceableDevice*> InstalledDevices;

    GatherInstalledDevices(InstalledDevices);

    TSet<ACDPlaceableDevice*> PoweredDevices;
    TQueue<ACDPlaceableDevice*> DeviceQueue;

    // 모든 발전기를 전력 탐색 시작점으로 등록합니다.
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

        // 공격 장치는 전력을 받아도
        // 다른 장치로 전달하지 않습니다.
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

            // 릴레이만 다음 전력 탐색 지점으로
            // Queue에 추가합니다.
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
            "Installed: %d, Powered: %d"
        ),
        InstalledDevices.Num(),
        PoweredDevices.Num()
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