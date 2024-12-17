#include "ClimbingState.h"
#include "../SlimeCharacter.h"
#include "FallingState.h"
#include "DefaultState.h"

void ClimbingState::OnEnter()
{
	if (!Player->InputComponent) return;

	Player->ResetBindings();

	Player->SetUpBinding(Player->MoveAction, ETriggerEvent::Triggered, &ASlimeCharacter::OnWallMove);
	Player->SetUpBinding(Player->JumpAction, ETriggerEvent::Triggered, &ASlimeCharacter::Jump);
	Player->SetUpBinding(Player->DetachAction, ETriggerEvent::Triggered, &ASlimeCharacter::Detach);
	Player->SetUpBinding(Player->ChargeJumpAction, ETriggerEvent::Ongoing, &ASlimeCharacter::ChargeJump);
	Player->SetUpBinding(Player->ChargeJumpAction, ETriggerEvent::Triggered, &ASlimeCharacter::ChargeJump);

	Player->SetMaterialOverTime(Player->DefaultMaterial);
}

void ClimbingState::OnUpdate()
{
	if (Player->IsTransitioning) return;

	if (!Player->IsPlayerOnClimbableSurface())
	{
		FVector NewGravity;
		FVector NewLocation;
		if (Player->HasPlayerFoundWrapAroundSurface(NewGravity, NewLocation))
		{
			Player->ApplyLocationTransition(NewLocation);
			Player->AttachToWall(NewGravity, false);
		}
		else
		{
			Player->DetachFromWall();
			Player->SetState<FallingState>();
		}
	}
	else
	{
		FVector NewGravity;
		if (Player->HasPlayerFoundNewSurface(NewGravity))
		{
			Player->AttachToWall(NewGravity, true);
		}
	}

	if (Player->IsPlayerGrounded())
	{
		Player->SetState<DefaultState>();
	}
}
