#include "FallingState.h"
#include "EnhancedInputComponent.h"
#include "../SlimeCharacter.h"
#include "DefaultState.h"

void FallingState::OnEnter()
{
	Player->BindAirborneInput();
	if (Player->JumpVelocity != FVector::Zero())
	{
		Player->JumpVelocity = FVector::Zero();
		Player->ApplyGravityTransition(FVector(0,0,-1));
		//Falling material Player->SetMaterialOverTime();
	}
}

void FallingState::OnExit()
{
	//Play sound at location
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
