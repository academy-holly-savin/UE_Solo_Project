// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Item.generated.h"

UCLASS()
class UE_SOLO_PROJECT_API AItem : public AActor
{
	GENERATED_BODY()
public:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* ItemMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Collision")
	UBoxComponent* BoxCollision;
private:
	bool IsHeld = false;
public:	
	// Sets default values for this actor's properties
	AItem();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Grab();
	void Release();
	void Launch(const FVector& Impulse);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	void Bobbing(float DeltaTime);
};
