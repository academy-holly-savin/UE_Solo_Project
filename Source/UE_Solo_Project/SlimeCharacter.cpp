// Fill out your copyright notice in the Description page of Project Settings.

#include "SlimeCharacter.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"

#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "PlayerState/DefaultState.h"

#include "UObject/ConstructorHelpers.h"

#include "Logging/LogMacros.h"

// Sets default values
ASlimeCharacter::ASlimeCharacter()
{
	SlimeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SlimeMesh"));
	SlimeMesh->SetupAttachment(GetCapsuleComponent());

	static ConstructorHelpers::FObjectFinder<UStaticMesh>
		meshFinder(TEXT("/Script/Engine.StaticMesh'/Game/Meshes/Slime/Slime_Slime_Body.Slime_Slime_Body'"));
	if (SlimeMesh && meshFinder.Succeeded())
	{
		SlimeMesh->SetStaticMesh(meshFinder.Object);
		SlimeMesh->SetRelativeLocation(FVector(0, 0, 0));
	}
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate


	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm


	// Initialize the camera boom
	ItemLocation = CreateDefaultSubobject<USceneComponent>(TEXT("ItemLocation"));
	ItemLocation->SetupAttachment(GetCapsuleComponent()); // Attach the boom to the capsule

	// Use ConstructorHelpers::FObjectFinder in the constructor
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> MaterialFinder(TEXT("/Script/Engine.MaterialInstanceConstant'/Game/Materials/Slime/MI_Slime_Default.MI_Slime_Default'"));
	if (MaterialFinder.Succeeded())
	{
		BaseMaterial = MaterialFinder.Object;
	}
}

// Called when the game starts or when spawned
void ASlimeCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Create and apply the dynamic material instance
	if (BaseMaterial)
	{
		DynamicMaterialInstance = SlimeMesh->CreateDynamicMaterialInstance(0, BaseMaterial);
	}

	//Add state
	SetState<DefaultState>();
}

bool ASlimeCharacter::IsPlayerGrounded()
{
	FVector Down = GetActorUpVector() * -1.0f;
	return LineTraceInDirection(Down, MaxDistFromSurface) && (Down == FVector(0.0f, 0.0f, -1.0f));
}

bool ASlimeCharacter::IsPlayerOnClimbableSurface()
{
	FVector Down = GetActorUpVector() * -1.0f;
	return LineTraceInDirection(Down, 100.0f);
}

bool ASlimeCharacter::HasPlayerFoundNewSurface(FVector& NewGravity)
{
	bool FoundNewSurface = false;

	if (TraceForNewGravity(GetActorForwardVector(), 50.0f, NewGravity))
	{
		FoundNewSurface = true;
	}
	else if (TraceForNewGravity(GetActorUpVector(), 100.0f, NewGravity))
	{
		FoundNewSurface = true;
	}
	else if (TraceForNewGravity(GetActorRightVector(), 50.0f, NewGravity))
	{
		FoundNewSurface = true;
	}
	else if (TraceForNewGravity(GetActorRightVector() * 1.0f, 50.0f, NewGravity))
	{
		FoundNewSurface = true;
	}

	return FoundNewSurface;
}

bool ASlimeCharacter::HasPlayerFoundWrapAroundSurface(FVector& NewGravity, FVector& NewLocation)
{
	const FVector Start = GetActorLocation() + GetActorUpVector() * 200.0f;
	const FVector End = Start + GetActorForwardVector() * -30.0f;

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);

	FHitResult HitResult;

	// Perform the line trace
	const bool bIsHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_GameTraceChannel1,
		CollisionParams
	);

	if (bIsHit)
	{
		//Set new gravity
		NewGravity = HitResult.Normal * -1.0f;

		SetUpMovementAxisUsingHitResult(HitResult);

		//Get location to move to
		NewLocation = HitResult.ImpactPoint + HitResult.Normal * 90.0f;
	}

	return bIsHit;
}

void ASlimeCharacter::PickUp()
{
}

void ASlimeCharacter::OnWallMovement(float InputX, float InputY)
{
	GetCharacterMovement()->SetPlaneConstraintEnabled(true);
	
	AddMovementInput(MovementVectorX, InputX);	
	AddMovementInput(MovementVectorY, InputY);

	GetCharacterMovement()->SetPlaneConstraintEnabled(false);
}

void ASlimeCharacter::DefaultMovement(float InputX, float InputY)
{
	GetCharacterMovement()->SetPlaneConstraintEnabled(true);
	// find out which way is forward
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// get forward vector
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	// get right vector 
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// add movement 
	AddMovementInput(ForwardDirection, InputY);
	AddMovementInput(RightDirection, InputX);

	GetCharacterMovement()->SetPlaneConstraintEnabled(false);
}

// Called every frame
void ASlimeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ASlimeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMapping, 1);
		}
	}

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASlimeCharacter::Move);
	Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASlimeCharacter::Look);
	Input->BindAction(ThrowAction, ETriggerEvent::Triggered, this, &ASlimeCharacter::Throw);
	Input->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ASlimeCharacter::Jump);
	Input->BindAction(DetachAction, ETriggerEvent::Triggered, this, &ASlimeCharacter::Detach);
	Input->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ASlimeCharacter::Interact);

	Input->BindAction(ChargeJumpAction, ETriggerEvent::Ongoing, this, &ASlimeCharacter::ChargeJump);
	Input->BindAction(ChargeJumpAction, ETriggerEvent::Triggered, this, &ASlimeCharacter::ChargeJump);

	Input->BindAction(ChargeThrowAction, ETriggerEvent::Ongoing, this, &ASlimeCharacter::ChargeThrow);
	Input->BindAction(ChargeThrowAction, ETriggerEvent::Triggered, this, &ASlimeCharacter::ChargeThrow);
}

void ASlimeCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	DefaultMovement(MovementVector.X, MovementVector.Y);
	// Use the X and Y components of the vector for movement
	//Call state.move
}

void ASlimeCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ASlimeCharacter::Throw(const FInputActionValue& Value)
{
	if (IsHolding)
	{
		//Actual throw stuff here
	}
	ResetMaterial();
	ThrowVelocity = FVector::Zero();
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ASlimeCharacter::OnThrowCooldownFinished, 3.0f, false);
}

void ASlimeCharacter::Jump(const FInputActionValue& Value)
{
	//Change state to jumping
}

void ASlimeCharacter::ChargeJump(const FInputActionValue& Value)
{
	const float MinVel = 1000.0f;
	const float MaxVel = 1300.0f;
	const float ChargeRate = 500.0f;

	JumpVelocity = GetChargedVelocity(JumpVelocity, MinVel, MaxVel, ChargeRate);

	//Set Material Overtime
	const float MaterialAlpha = JumpVelocity.X / MaxVel;
	//SetMaterialOvertime(M_Slime_Charged, Alpha);
}

void ASlimeCharacter::ChargeThrow(const FInputActionValue& Value)
{
	const float MinVel = 400.0f;
	const float MaxVel = 1000.0f;
	const float ChargeRate = 200.0f;

	ThrowVelocity = GetChargedVelocity(ThrowVelocity, MinVel, MaxVel, ChargeRate);

	//Set Material Overtime
	const float MaterialAlpha = ThrowVelocity.X / MaxVel;
	//SetMaterialOvertime(M_Slime_Charged, Alpha);
}

void ASlimeCharacter::Detach(const FInputActionValue& Value)
{
}

void ASlimeCharacter::Interact(const FInputActionValue& Value)
{
}

void ASlimeCharacter::ResetMaterial()
{
	if (BaseMaterial)
	{
		SetMaterialOverTime(BaseMaterial, 1.0f);
	}
}

void ASlimeCharacter::LerpGravity(const FVector& NewGravityDirection, const float Alpha)
{
	const FVector GravityDirection = GetCharacterMovement()->GetGravityDirection();

	GetCharacterMovement()->SetGravityDirection(FMath::Lerp(GravityDirection, NewGravityDirection, Alpha));
}

void ASlimeCharacter::LerpLocation(const FVector& NewLocation, const float Alpha)
{
	const FVector Location = GetActorLocation();

	SetActorLocation(FMath::Lerp(Location, NewLocation, Alpha));
}

void ASlimeCharacter::InterpolateMaterialInstances(UMaterialInstance* NewMaterial, const float Alpha)
{
	UMaterialInstance* CurrentMaterial = Cast<UMaterialInstance>(DynamicMaterialInstance->Parent);

	if (CurrentMaterial)
	{
		DynamicMaterialInstance->K2_InterpolateMaterialInstanceParams(CurrentMaterial, NewMaterial, Alpha);
	}
}

bool ASlimeCharacter::LineTraceInDirection(const FVector& Direction, const float LineLength, FHitResult& OutHit)
{
	const FVector Start = GetActorLocation();
	const FVector End = (Start + (Direction * LineLength));

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);

	// Perform the line trace
	const bool bIsHit = GetWorld()->LineTraceSingleByChannel(
		OutHit,
		Start,
		End,
		ECC_GameTraceChannel1,
		CollisionParams
	);

	return bIsHit;
}

bool ASlimeCharacter::LineTraceInDirection(const FVector& Direction, const float LineLength)
{
	FHitResult HitResult;
	return LineTraceInDirection(Direction, LineLength, HitResult);
}

bool ASlimeCharacter::TraceForNewGravity(const FVector& Direction, const float LineLength, FVector& NewGravity)
{
	FHitResult HitResult;
	const bool HasHit = LineTraceInDirection(Direction, LineLength, HitResult);
	if (HasHit)
	{
		//Set new gravity
		NewGravity = HitResult.Normal * -1.0f;

		SetUpMovementAxisUsingHitResult(HitResult);
	}
	return HasHit;
}

void ASlimeCharacter::OnThrowCooldownFinished()
{
	IsHolding = false;
}

void ASlimeCharacter::SetUpMovementAxisUsingHitResult(const FHitResult& HitResult)
{
	//Recalculate right vector
	FVector RightVector = FVector::CrossProduct(FVector::UpVector, HitResult.Normal);
	//Recalculate forward vector
	FVector ForwardVector = FVector::CrossProduct(HitResult.Normal, RightVector);

	//Set movement direction axis for on wall movement
	MovementVectorX = RightVector * -1.0f;
	MovementVectorY = ForwardVector;
}

FVector ASlimeCharacter::GetChargedVelocity(const FVector& CurrentVelocity, const float MinVel, const float MaxVel, const float ChargeRate)
{
	FVector ChargedVelocity = CurrentVelocity + ChargeRate * GetWorld()->GetDeltaSeconds();

	FMath::Clamp(ChargedVelocity.X, MinVel, MaxVel);
	FMath::Clamp(ChargedVelocity.Y, MinVel, MaxVel);
	FMath::Clamp(ChargedVelocity.Z, MinVel, MaxVel);

	return ChargedVelocity;
}