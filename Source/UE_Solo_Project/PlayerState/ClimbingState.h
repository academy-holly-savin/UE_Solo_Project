
#pragma once

#include "PlayerStateInterface.h"

class ClimbingState : public IPlayerState
{
public:
	using IPlayerState::IPlayerState; // Inherit constructors

	void OnEnter() override;
	void OnUpdate() override;

	FName GetName() override {
		return TEXT("CLIMBING");
	};
};