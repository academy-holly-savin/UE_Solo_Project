#include "DefaultState.h"
#include "../SlimeCharacter.h"
#include "ClimbingState.h"
#include "FallingState.h"

void DefaultState::OnEnter()
{
	if (!Player->InputComponent) return;

	Player->ResetBindings();

	Player->SetUpBinding(Player->MoveAction, ETriggerEvent::Triggered, &ASlimeCharacter::Move);
	Player->SetUpBinding(Player->JumpAction, ETriggerEvent::Triggered, &ASlimeCharacter::Jump);
	Player->SetUpBinding(Player->ChargeJumpAction, ETriggerEvent::Ongoing, &ASlimeCharacter::ChargeJump);
	Player->SetUpBinding(Player->ChargeJumpAction, ETriggerEvent::Triggered, &ASlimeCharacter::ChargeJump);

	Player->SetMaterialOverTime(Player->DefaultMaterial);
}

void DefaultState::OnUpdate()
{
	if (Player->IsTransitioning) return;

	if (!Player->IsPlayerGrounded())
	{
		FVector NewGravity;
		FVector NewLocation;
		if (Player->HasPlayerFoundWrapAroundSurface(NewGravity, NewLocation))
		{
			Player->ApplyLocationTransition(NewLocation);
			Player->AttachToWall(NewGravity, false);
			Player->SetState<ClimbingState>();
		}
		else
		{
			Player->SetState<FallingState>();
		}
	}
	else
	{
		FVector NewGravity;
		if (Player->HasPlayerFoundNewSurface(NewGravity))
		{
			Player->AttachToWall(NewGravity, true);
			Player->SetState<ClimbingState>();
		}
	}
}
