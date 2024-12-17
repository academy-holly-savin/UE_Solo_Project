#include "JumpingState.h"
#include "../SlimeCharacter.h"
#include "DefaultState.h"
#include "ClimbingState.h"
#include "FallingState.h"

void JumpingState::OnEnter()
{
	Player->BindJumpingInputs();

	const FVector LaunchVelocity = Player->JumpVelocity * (Player->GetActorUpVector() + Player->GetActorForwardVector());


	UE_LOG(LogTemp, Warning, TEXT("Jump vel : % f"), Player->JumpVelocity.X);

	Player->LaunchCharacter(LaunchVelocity, false, false);


	//Play sound at location

	//Player->SetMaterialOverTime(, 1.0f);
}

void JumpingState::OnExit()
{
	//Play sound at location
	Player->JumpVelocity = FVector::Zero();
}

void JumpingState::OnUpdate()
{
}

void JumpingState::OnHit()
{
	Player->ResetMaterial();

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
