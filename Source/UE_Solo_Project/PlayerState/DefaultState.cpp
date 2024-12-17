#include "DefaultState.h"
#include "../SlimeCharacter.h"
#include "ClimbingState.h"
#include "FallingState.h"

void DefaultState::OnEnter()
{
	Player->BindDefaultInputs();
}

void DefaultState::OnExit()
{
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

void DefaultState::OnHit()
{
}
