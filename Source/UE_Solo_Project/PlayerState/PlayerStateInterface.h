
#pragma once

class ASlimeCharacter;

class IPlayerState
{
public:
    IPlayerState(ASlimeCharacter* Player) : Player(Player) {};

    virtual void OnEnter() {};
    virtual void OnExit() {};
    virtual void OnUpdate() {};
    virtual void OnHit() {};

    virtual FName GetName() = 0;

protected:
    ASlimeCharacter* Player; // Reference to Player character

};