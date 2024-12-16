// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMaterialLibrary.h"

#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"

#include "TimerManager.h"
#include "Engine/World.h"

#include "PlayerState/PlayerStateInterface.h"
#include <memory>

#include "SlimeCharacter.generated.h"

template <typename T>
concept InheritsPlayerState = std::is_base_of<IPlayerState, T>::value;

UCLASS()
class UE_SOLO_PROJECT_API ASlimeCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// ----------- Properites -----------
	//Slime
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* SlimeMesh;

	UPROPERTY(VisibleAnywhere)
	UMaterialInstanceDynamic* DynamicMaterialInstance;

	UMaterialInstance* BaseMaterial;
	//Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;

	//Inputs
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputMappingContext* InputMapping;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* ThrowAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* InteractAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* DetachAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* ChargeJumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* ChargeThrowAction;

	//Item
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USceneComponent* ItemLocation;

	//Variables
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool IsHolding = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool IsTransitioning = false;

	FTimerHandle TimerHandle;

	// ----------- Methods -----------
private:
	void OnThrowCooldownFinished();

	void SetUpMovementAxisUsingHitResult(const FHitResult& HitResult);

	FVector GetChargedVelocity(const FVector& CurrentVelocity, const float MinVel, const float MaxVel, const float ChargeRate);

	bool LineTraceInDirection(const FVector& Direction, const float LineLength, FHitResult& OutHit);
	bool LineTraceInDirection(const FVector& Direction, const float LineLength);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	template<typename InheritsPlayerState>
	void SetState();

public:
	// Sets default values for this character's properties
	ASlimeCharacter();

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	bool IsPlayerGrounded();
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	bool IsPlayerOnClimbableSurface();
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	bool HasPlayerFoundNewSurface(FVector& NewGravity);
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	bool HasPlayerFoundWrapAroundSurface(FVector& NewGravity, FVector& NewLocation);
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void PickUp();
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void OnWallMovement(float inputX, float inputY);
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void DefaultMovement(float inputX, float inputY);

	UFUNCTION(BlueprintCallable)
	void LerpGravity(const FVector& NewGravityDirection, const float Alpha);
	UFUNCTION(BlueprintCallable)
	void LerpLocation(const FVector& NewLocation, const float Alpha);

	UFUNCTION(BlueprintCallable)
	void ResetMaterial();
	UFUNCTION(BlueprintCallable)
	void InterpolateMaterialInstances(UMaterialInstance* NewMaterial, const float Alpha);

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	bool TraceForNewGravity(const FVector& Direction, const float LineLength, FVector& NewGravity);

	//Blueprint implementable events
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Gameplay")
	void AttachToWall(const FVector& NewGravity, const bool Boost);
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Gameplay")
	void DetachFromWall();
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ApplyGravityTransition(const FVector& NewGravityDirection, const float PlaybackRate = 1.0f);
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ApplyLocationTransition(const FVector& NewLocation, const float PlaybackRate = 1.0f);
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetMaterialOverTime(UMaterialInstance* NewMaterial, const float Alpha);


	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//Input callback functions
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Throw(const FInputActionValue& Value);
	void Jump(const FInputActionValue& Value);
	void ChargeJump(const FInputActionValue& Value);
	void ChargeThrow(const FInputActionValue& Value);
	void Detach(const FInputActionValue& Value);
	void Interact(const FInputActionValue& Value);

private:

	std::unique_ptr<IPlayerState> PlayerState;

	float PickUpCooldown;
	float JumpCooldown;
	float IncrementRate;
	float MaxDistFromSurface;

	FVector JumpVelocity;
	FVector ThrowVelocity;
	FVector MovementVectorX;
	FVector MovementVectorY;


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

template<typename InheritsPlayerState>
inline void ASlimeCharacter::SetState()
{
	if (PlayerState)
	{
		PlayerState->OnExit();
	}
	PlayerState = std::make_unique<InheritsPlayerState>(this);

	FString NameString = PlayerState->GetName().ToString();
	UE_LOG(LogTemp, Warning, TEXT("New State : % s"), *NameString);

	PlayerState->OnEnter();
};
