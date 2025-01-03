// Fill out your copyright notice in the Description page of Project Settings.

#include "SlimeCharacter.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"

#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "PlayerState/DefaultState.h"
#include "PlayerState/JumpingState.h"
#include "PlayerState/FallingState.h"

#include "Logging/LogMacros.h"

#include "UObject/ConstructorHelpers.h"

// Sets default values
ASlimeCharacter::ASlimeCharacter()
{
	SlimeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SlimeMesh"));
	SlimeMesh->SetupAttachment(GetCapsuleComponent());
	SlimeMesh->SetCollisionProfileName(TEXT("Pawn"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh>
		SlimeFinder(TEXT("/Script/Engine.StaticMesh'/Game/Meshes/Slime/Slime_Slime_Body.Slime_Slime_Body'"));
	if (SlimeMesh && SlimeFinder.Succeeded())
	{
		SlimeMesh->SetStaticMesh(SlimeFinder.Object);
		SlimeMesh->SetRelativeLocation(FVector(0, 0, -65.0f));
	}

	FaceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FaceMesh"));
	FaceMesh->SetupAttachment(SlimeMesh);
	FaceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh>
		FaceFinder(TEXT("/Script/Engine.StaticMesh'/Game/Meshes/Slime/Slime_Slime_Face.Slime_Slime_Face'"));
	if (FaceMesh && FaceFinder.Succeeded())
	{
		FaceMesh->SetStaticMesh(FaceFinder.Object);
		FVector RelativeLocation = FVector(-53.5f, 0.0f, 63.0f);
		FRotator RelativeRotation = FRotator(0.0f, -180.0f, 0.0f);
		FaceMesh->SetRelativeLocationAndRotation(RelativeLocation, RelativeRotation);
	}

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);

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
	ItemLocation->SetRelativeLocation(FVector(0.0f, 0.0f, -35.0f));
}

// Called when the game starts or when spawned
void ASlimeCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Create and apply the dynamic material instance
	if (DefaultMaterial)
	{
		DynamicMaterialInstance = SlimeMesh->CreateDynamicMaterialInstance(0, DefaultMaterial);
	}
	//Add state
	SetState<DefaultState>();
}

FVector ASlimeCharacter::GetJumpVelocity()
{
	return JumpVelocity;
}

void ASlimeCharacter::SetJumpVelocity(const FVector& NewVelocity)
{
	JumpVelocity = NewVelocity;
}

void ASlimeCharacter::PlaySoundAtLocation(USoundCue* SoundCue)
{
	if (SoundCue)
	{
		FVector Location = GetActorLocation();
		FRotator Rotation = GetActorRotation();
		float VolumeMultiplier = 1.0f;
		float PitchMultiplier = 1.0f;
		float StartTime = 0.0f;

		UGameplayStatics::PlaySoundAtLocation(this, SoundCue, Location, Rotation, VolumeMultiplier, PitchMultiplier, StartTime);
	}
}

bool ASlimeCharacter::IsPlayerGrounded()
{
	FVector Down = GetActorUpVector() * -1.0f;
	return LineTraceInDirection(Down, MaxDistanceFromSurface) && (Down == FVector(0.0f, 0.0f, -1.0f));
}

bool ASlimeCharacter::IsPlayerOnClimbableSurface()
{
	FVector Down = GetActorUpVector() * -1.0f;
	return LineTraceInDirection(Down, MaxDistanceFromSurface);
}

bool ASlimeCharacter::HasPlayerFoundNewSurface(FVector& NewGravity)
{
	bool FoundNewSurface = false;

	if (TraceForNewGravity(GetActorForwardVector(), MaxDistanceFromSurface / 2.0f, NewGravity))
	{
		FoundNewSurface = true;
	}
	else if (TraceForNewGravity(GetActorUpVector(), MaxDistanceFromSurface, NewGravity))
	{
		FoundNewSurface = true;
	}
	else if (TraceForNewGravity(GetActorRightVector(), MaxDistanceFromSurface / 2.0f, NewGravity))
	{
		FoundNewSurface = true;
	}
	else if (TraceForNewGravity(GetActorRightVector() * -1.0f, MaxDistanceFromSurface / 2.0f, NewGravity))
	{
		FoundNewSurface = true;
	}

	return FoundNewSurface;
}

bool ASlimeCharacter::HasPlayerFoundWrapAroundSurface(FVector& NewGravity, FVector& NewLocation)
{
	const FVector Start = GetActorLocation() + GetActorUpVector() * -200.0f;
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

	//DrawDebugLine(GetWorld(), Start, End, HitResult.bBlockingHit ? FColor::Blue : FColor::Red, false, 5.0f, 0, 10.0f);

	return bIsHit;
}

void ASlimeCharacter::PickUp(AItem* Item)
{
	if (!Item || IsHolding) return;

	HeldItem = Item;
	IsHolding = true;

	HeldItem->Grab();

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, true);
	HeldItem->AttachToComponent(ItemLocation, AttachmentRules);
}

// Called every frame
void ASlimeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState)
	{
		CurrentState->OnUpdate();
	}
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

	InputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
}

void ASlimeCharacter::ResetBindings()
{
	if (!InputComponent) return;

	InputComponent->ClearActionBindings();

	SetUpBinding(LookAction, ETriggerEvent::Triggered, &ASlimeCharacter::Look);
	SetUpBinding(ThrowAction, ETriggerEvent::Triggered, &ASlimeCharacter::Throw);
	SetUpBinding(InteractAction, ETriggerEvent::Triggered, &ASlimeCharacter::Interact);
	SetUpBinding(ChargeThrowAction, ETriggerEvent::Triggered, &ASlimeCharacter::ChargeThrow);
	SetUpBinding(ChargeThrowAction, ETriggerEvent::Ongoing, &ASlimeCharacter::ChargeThrow);

}

void ASlimeCharacter::SetUpBinding(const UInputAction* Action, ETriggerEvent TriggerEvent, FPointer FunctionPointer)
{
	InputComponent->BindAction(Action, TriggerEvent, this, FunctionPointer);
}

void ASlimeCharacter::Move(const FInputActionValue& Value)
{
	if (IsTransitioning) return;

	FVector2D MovementVector = Value.Get<FVector2D>();

	GetCharacterMovement()->SetPlaneConstraintEnabled(true);
	// find out which way is forward
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// get forward vector
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	// get right vector 
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// add movement 
	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);

	GetCharacterMovement()->SetPlaneConstraintEnabled(false);
}


void ASlimeCharacter::OnWallMove(const FInputActionValue& Value)
{
	if (IsTransitioning) return;

	FVector2D MovementVector = Value.Get<FVector2D>();

	GetCharacterMovement()->SetPlaneConstraintEnabled(true);

	AddMovementInput(MovementVectorX, MovementVector.X);
	AddMovementInput(MovementVectorY, MovementVector.Y);

	GetCharacterMovement()->SetPlaneConstraintEnabled(false);
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
	if (!IsHolding || !HeldItem) return;

	HeldItem->Release();
	HeldItem->DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);
	HeldItem->SetActorLocationAndRotation(ItemLocation->GetComponentLocation(), ItemLocation->GetComponentRotation());

	const FVector Impulse = ThrowVelocity * (GetActorUpVector() + GetActorForwardVector());

	HeldItem->Launch(Impulse);

	SetMaterialOverTime(DefaultMaterial);

	ThrowVelocity = FVector::Zero();
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ASlimeCharacter::OnThrowCooldownFinished, 3.0f, false);
}

void ASlimeCharacter::Jump(const FInputActionValue& Value)
{
	if (IsTransitioning) return;
	//Change state to jumping
	SetState<JumpingState>();
}

void ASlimeCharacter::ChargeJump(const FInputActionValue& Value)
{
	const double MinVel = 1000.0f;
	const double MaxVel = 1300.0f;
	const double ChargeRate = 500.0f;

	JumpVelocity = GetChargedVelocity(JumpVelocity, MinVel, MaxVel, ChargeRate);

	//Set Material Overtime
	const float MaterialAlpha = JumpVelocity.X / MaxVel;
	SetMaterialOverTime(ChargingMaterial, MaterialAlpha);
}

void ASlimeCharacter::ChargeThrow(const FInputActionValue& Value)
{
	if (!IsHolding || !HeldItem) return;
	const double MinVel = 600.0f;
	const double MaxVel = 800.0f;
	const double ChargeRate = 50.0f;

	ThrowVelocity = GetChargedVelocity(ThrowVelocity, MinVel, MaxVel, ChargeRate);

	//Set Material Overtime
	const float MaterialAlpha = ThrowVelocity.X / MaxVel;
	SetMaterialOverTime(ChargingMaterial, MaterialAlpha);
}

void ASlimeCharacter::Detach(const FInputActionValue& Value)
{
	DetachFromWall();
	SetState<FallingState>();
}

void ASlimeCharacter::Interact(const FInputActionValue& Value)
{
}

void ASlimeCharacter::OnHit()
{
	CurrentState->OnHit();
}

bool ASlimeCharacter::GetIsHolding()
{
	return IsHolding;
}

void ASlimeCharacter::SetIsHolding(const bool Holding)
{
	IsHolding = Holding;
}

void ASlimeCharacter::AttachToWall(const FVector& NewGravity, const bool Boost)
{
	if (Boost)
	{
		ACharacter::Jump();
	}
	PlaySoundAtLocation(SlideSound);
	ApplyGravityTransition(NewGravity);
}

void ASlimeCharacter::DetachFromWall()
{
	ACharacter::Jump();
	PlaySoundAtLocation(SlideSound);
	ApplyGravityTransition(FVector(0, 0, -1));
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
	UMaterialInstance* CurrentMaterial = Cast<UMaterialInstance>(SlimeMesh->GetMaterial(0));

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

	//DrawDebugLine(GetWorld(), Start, End, OutHit.bBlockingHit ? FColor::Blue : FColor::Red, false, 5.0f, 0, 10.0f);

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
	HeldItem = nullptr;
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

	ChargedVelocity.X = FMath::Clamp(ChargedVelocity.X, MinVel, MaxVel);
	ChargedVelocity.Y = FMath::Clamp(ChargedVelocity.Y, MinVel, MaxVel);
	ChargedVelocity.Z = FMath::Clamp(ChargedVelocity.Z, MinVel, MaxVel);

	return ChargedVelocity;
}