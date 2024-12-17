#include "JumpingState.h"
#include "../SlimeCharacter.h"

#include "DefaultState.h"
#include "ClimbingState.h"
#include "FallingState.h"

void JumpingState::OnEnter()
{
	if (!Player->InputComponent) return;

	Player->ResetBindings();

	Player->SetUpBinding(Player->MoveAction, ETriggerEvent::Triggered, &ASlimeCharacter::Move);

	const FVector LaunchVelocity = Player->GetJumpVelocity() * (Player->GetActorUpVector() + Player->GetActorForwardVector());

	Player->LaunchCharacter(LaunchVelocity, false, false);

	Player->PlaySoundAtLocation(Player->JumpSound);

	Player->SetMaterialOverTime(Player->FallingMaterial);
}

void JumpingState::OnExit()
{
	Player->PlaySoundAtLocation(Player->SplatSound);

	Player->SetJumpVelocity(FVector::Zero());
}

void JumpingState::OnHit()
{
	FVector NewGravity;

	if (Player->HasPlayerFoundNewSurface(NewGravity))
	{
		Player->AttachToWall(NewGravity, true);

		if (Player->IsPlayerGrounded())
		{
			Player->SetState<DefaultState>();
		}
		else
		{
			Player->SetState<ClimbingState>();
		}
	}
	else
	{
		if (Player->IsPlayerOnClimbableSurface())
		{
			if (Player->IsPlayerGrounded())
			{
				Player->SetState<DefaultState>();
			}
			else
			{
				Player->SetState<ClimbingState>();
			}
		}
		else
		{
			Player->DetachFromWall();
			Player->SetState<FallingState>();
		}
	}
}
