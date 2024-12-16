#pragma once
#include "PlayerStateInterface.h"

class ClimbingState : public IPlayerState
{
public:
    using IPlayerState::IPlayerState; // Inherit constructors

    void OnEnter() override;
    void OnExit() override;
    void OnUpdate() override;
    void OnHit() override;

    FName GetName() override {
        return TEXT("CLIMBING");
    };
};