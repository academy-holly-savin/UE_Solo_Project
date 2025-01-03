// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <memory>
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

#include "Sound/SoundCue.h"

#include "PlayerState/PlayerStateInterface.h"
#include "Item.h"

#include "SlimeCharacter.generated.h"

template <typename T>
concept InheritsPlayerState = std::is_base_of<IPlayerState, T>::value;

UCLASS()
class UE_SOLO_PROJECT_API ASlimeCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// ----------- UPROPERTIES -----------
	//Slime
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* SlimeMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* FaceMesh;

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

	//Sounds
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	USoundCue* JumpSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	USoundCue* PopSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	USoundCue* DetachSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	USoundCue* SlideSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	USoundCue* SplatSound;

	//Materials

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Material")
	UMaterialInstanceDynamic* DynamicMaterialInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Material")
	UMaterialInstance* DefaultMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Material")
	UMaterialInstance* ChargingMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Material")
	UMaterialInstance* FallingMaterial;


	//Item
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USceneComponent* ItemLocation;

	AItem* HeldItem;

	//Variables
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool IsHolding = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool IsTransitioning = false;

	FTimerHandle TimerHandle;

	UEnhancedInputComponent* InputComponent;

private:

	std::unique_ptr<IPlayerState> CurrentState;

	float PickUpCooldown;
	float JumpCooldown;
	float IncrementRate;
	float MaxDistanceFromSurface = 100.f;

	FVector JumpVelocity;
	FVector ThrowVelocity;
	FVector MovementVectorX;
	FVector MovementVectorY;

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

public:

	// Sets default values for this character's properties
	ASlimeCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// ----------- UFUNCTIONS -----------

	UFUNCTION(BlueprintCallable)
	void LerpGravity(const FVector& NewGravityDirection, const float Alpha);

	UFUNCTION(BlueprintCallable)
	void LerpLocation(const FVector& NewLocation, const float Alpha);

	UFUNCTION(BlueprintCallable)
	void InterpolateMaterialInstances(UMaterialInstance* NewMaterial, const float Alpha);

	UFUNCTION(BlueprintCallable)
	void OnHit();

	// Timeline Events
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ApplyGravityTransition(const FVector& NewGravityDirection, const float PlaybackRate = 1.0f);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ApplyLocationTransition(const FVector& NewLocation, const float PlaybackRate = 1.0f);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetMaterialOverTime(UMaterialInstance* NewMaterial, const float Alpha = 1.0f);

	// Methods

	FVector GetJumpVelocity();

	void SetJumpVelocity(const FVector& NewVelocity);

	bool GetIsHolding();

	void SetIsHolding(const bool Holding);

	template<typename InheritsPlayerState>
	void SetState();

	void PlaySoundAtLocation(USoundCue* SoundCue);

	bool IsPlayerGrounded();

	bool IsPlayerOnClimbableSurface();

	bool HasPlayerFoundNewSurface(FVector& NewGravity);

	bool HasPlayerFoundWrapAroundSurface(FVector& NewGravity, FVector& NewLocation);

	bool TraceForNewGravity(const FVector& Direction, const float LineLength, FVector& NewGravity);

	UFUNCTION(BlueprintCallable)
	void PickUp(AItem* Item);

	void AttachToWall(const FVector& NewGravity, const bool Boost);

	void DetachFromWall();

	// Input Binding

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	typedef void (ASlimeCharacter::* FPointer)(const FInputActionValue&);

	void ResetBindings();

	void SetUpBinding(const UInputAction* Action, ETriggerEvent TriggerEvent, FPointer FunctionName);

	// Input Callbacks

	void Move(const FInputActionValue& Value);

	void OnWallMove(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void Throw(const FInputActionValue& Value);

	void Jump(const FInputActionValue& Value);

	void ChargeJump(const FInputActionValue& Value);

	void ChargeThrow(const FInputActionValue& Value);

	void Detach(const FInputActionValue& Value);

	void Interact(const FInputActionValue& Value);

};

template<typename InheritsPlayerState>
inline void ASlimeCharacter::SetState()
{
	if (CurrentState)
	{
		CurrentState->OnExit();
	}
	CurrentState = std::make_unique<InheritsPlayerState>(this);

	FString NameString = CurrentState->GetName().ToString();
	UE_LOG(LogTemp, Warning, TEXT("New State : % s"), *NameString);

	CurrentState->OnEnter();
};
