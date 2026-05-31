#pragma once

#define NOMINMAX
#include <windows.h>
#include <cmath>

#include "../Core/Component.hpp"
#include "../Core/GameObject.hpp"
#include "../Core/GameConstants.hpp"
#include "../Core/MathUtils.hpp"
#include "PlayerStatusComponent.hpp"

class PlayerControllerComponent : public Component
{
private:
    float _moveSpeed = 0.0f;

public:
    PlayerControllerComponent(float moveSpeed) : _moveSpeed(moveSpeed)
    {
    }

    void Update(float deltaTime) override
    {
        if (owner == nullptr) return;

        PlayerStatusComponent* status = owner->GetComponent<PlayerStatusComponent>();
        if (status != nullptr && status->IsDead()) return;

        Vector2 input;

        if ((GetAsyncKeyState('A') & 0x8000) || (GetAsyncKeyState(VK_LEFT) & 0x8000)) input.x -= 1.0f;
        if ((GetAsyncKeyState('D') & 0x8000) || (GetAsyncKeyState(VK_RIGHT) & 0x8000)) input.x += 1.0f;
        if ((GetAsyncKeyState('W') & 0x8000) || (GetAsyncKeyState(VK_UP) & 0x8000)) input.y -= 1.0f;
        if ((GetAsyncKeyState('S') & 0x8000) || (GetAsyncKeyState(VK_DOWN) & 0x8000)) input.y += 1.0f;

        float length = std::sqrt(input.x * input.x + input.y * input.y);

        if (length > 0.0f)
        {
            input.x /= length;
            input.y /= length;
        }

        Vector2 position = owner->GetPosition();
        Vector2 size = owner->GetSize();

        position.x += input.x * _moveSpeed * deltaTime;
        position.y += input.y * _moveSpeed * deltaTime;

        float halfW = size.x * 0.5f;
        float halfH = size.y * 0.5f;

        position.x = ClampFloat(position.x, halfW, static_cast<float>(ScreenWidth) - halfW);
        position.y = ClampFloat(position.y, halfH, static_cast<float>(ScreenHeight) - halfH);

        owner->SetPosition(position);
    }
};