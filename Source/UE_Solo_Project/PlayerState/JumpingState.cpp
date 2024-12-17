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

	const FVector LaunchVelocity = Player->JumpVelocity * (Player->GetActorUpVector() + Player->GetActorForwardVector());


	UE_LOG(LogTemp, Warning, TEXT("Jump vel : % f"), Player->JumpVelocity.X);

	Player->LaunchCharacter(LaunchVelocity, false, false);


	Player->PlaySoundAtLocation(Player->JumpSound);

	//Player->SetMaterialOverTime(, 1.0f);
}

void JumpingState::OnExit()
{
	Player->PlaySoundAtLocation(Player->SplatSound);
	Player->JumpVelocity = FVector::Zero();
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
