#pragma once
#include "PlayerStateInterface.h"

class JumpingState : public IPlayerState
{
public:
    using IPlayerState::IPlayerState; // Inherit constructors

    void OnEnter() override;
    void OnExit() override;
    void OnHit() override;

    FName GetName() override {
        return TEXT("JUMPING");
    };
};