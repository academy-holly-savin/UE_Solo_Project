#include "ClimbingState.h"
#include "../SlimeCharacter.h"
#include "FallingState.h"
#include "DefaultState.h"

void ClimbingState::OnEnter()
{
	Player->BindClimbingInputs();
}

void ClimbingState::OnExit()
{
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

void ClimbingState::OnHit()
{
}
