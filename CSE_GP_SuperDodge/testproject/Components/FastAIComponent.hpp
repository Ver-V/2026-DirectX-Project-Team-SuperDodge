#pragma once

#include "AIComponent.hpp"

class FastAIComponent : public AIComponent
{
private:
    float _speedMultiplier = 1.8f;

public:
    void Update(float deltaTime) override
    {
        Move(deltaTime, _speedMultiplier);
        DespawnIfOutOfScreen();
    }
};