// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"

// Sets default values
AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = true;
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(RootComponent);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ItemMesh->SetSimulatePhysics(true);

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	BoxCollision->SetupAttachment(ItemMesh);

	RunningTime = 0.0f;
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();
	
}

void AItem::Bobbing(float DeltaTime)
{
	const float BobbingAmplitude = 0.5f; 
	const float BobbingFrequency = 5.0f;  

	// Calculate new Z position
	FVector NewLocation = GetActorLocation();
	NewLocation.Z += FMath::Sin(RunningTime * BobbingFrequency) * BobbingAmplitude;

	// Update the actor's location
	SetActorLocation(NewLocation);

	// Increment running time
	RunningTime += DeltaTime;
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsHeld)
	{
		Bobbing(DeltaTime);
	}
}

void AItem::Grab()
{
	if (IsHeld)  return;

	IsHeld = true;
	ItemMesh->SetSimulatePhysics(false);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AItem::Release()
{
	if (!IsHeld)  return;

	IsHeld = false;
	ItemMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ItemMesh->SetSimulatePhysics(true);
}

void AItem::Launch(const FVector& Impulse)
{
	ItemMesh->AddImpulse(Impulse, NAME_None, true);
}

