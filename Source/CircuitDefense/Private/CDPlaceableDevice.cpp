// Fill out your copyright notice in the Description page of Project Settings.


#include "CDPlaceableDevice.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ACDPlaceableDevice::ACDPlaceableDevice()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
	SceneRoot->SetMobility(EComponentMobility::Movable);

	DeviceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DeviceMesh"));
	DeviceMesh->SetupAttachment(RootComponent);
	DeviceMesh->SetMobility(EComponentMobility::Movable);

	DeviceMesh->SetUsingAbsoluteLocation(false);
	DeviceMesh->SetUsingAbsoluteRotation(false);
	DeviceMesh->SetUsingAbsoluteScale(false);
	DeviceMesh->SetRelativeLocation(FVector::ZeroVector);
	DeviceMesh->SetRelativeRotation(FRotator::ZeroRotator);
	DeviceMesh->SetRelativeScale3D(FVector::OneVector);


}

void ACDPlaceableDevice::CompletePlacement()
{
	if (bInstalled)
	{
		return;
	}

	bPlacementPreview = false;
	bInstalled = true;

	SetActorEnableCollision(true);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Device Placement completed - Device: %s, Cost: %d"),
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

void ACDPlaceableDevice::SetPlacementPreview(bool bIsPreview)
{
	bPlacementPreview = bIsPreview;

	if (bPlacementPreview)
	{
		bInstalled = false;
	}

	SetActorEnableCollision(!bPlacementPreview);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Device preview changed -""Device: %s, Preview: %s"),
		*GetName(),
		bPlacementPreview ? TEXT("true") : TEXT("false")
	);
}

bool ACDPlaceableDevice::IsPlacementPreview() const
{
	return bPlacementPreview;
}

// Called when the game starts or when spawned
void ACDPlaceableDevice::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ACDPlaceableDevice::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

