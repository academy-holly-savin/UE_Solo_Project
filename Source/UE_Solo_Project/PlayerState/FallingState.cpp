#include "FallingState.h"
#include "EnhancedInputComponent.h"
#include "../SlimeCharacter.h"
#include "DefaultState.h"

void FallingState::OnEnter()
{
	if (!Player->InputComponent) return;

	Player->ResetBindings();

	Player->SetUpBinding(Player->MoveAction, ETriggerEvent::Triggered, &ASlimeCharacter::Move);

	Player->SetMaterialOverTime(Player->FallingMaterial);

	if (Player->JumpVelocity != FVector::Zero())
	{
		Player->JumpVelocity = FVector::Zero();
	}

	Player->ApplyGravityTransition(FVector(0, 0, -1));
}

void FallingState::OnExit()
{
	Player->PlaySoundAtLocation(Player->SplatSound);
}

void FallingState::OnUpdate()
{
	if (Player->IsPlayerGrounded())
	{
		Player->SetState<DefaultState>();
	}
}

void FallingState::OnHit()
{
	if (Player->IsPlayerGrounded())
	{
		Player->SetState<DefaultState>();
	}
}
