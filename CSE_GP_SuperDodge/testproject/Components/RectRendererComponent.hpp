#pragma once

#include "../Core/Component.hpp"
#include "../Core/GameObject.hpp"
#include "../Rendering/Renderer.hpp"
#include "PlayerControllerComponent.hpp"
#include "PlayerStatusComponent.hpp"

class RectRendererComponent : public Component
{
private:
    Color _color;

public:
    RectRendererComponent(const Color& color) : _color(color)
    {
    }

    void SetColor(const Color& color)
    {
        _color = color;
    }

    void Render(Renderer& renderer) override
    {
        if (owner == nullptr) return;
        
        // 기본 기체 그리기
        renderer.DrawRect(owner->GetPosition(), owner->GetSize(), _color);

        // 피탄점 표시
        auto* controller = owner->GetComponent<PlayerControllerComponent>();
        auto* status = owner->GetComponent<PlayerStatusComponent>();
        
        if (controller != nullptr && status != nullptr && controller->IsFocusMode())
        {
            float r = status->GetHitboxRadius();
            renderer.DrawRect(owner->GetPosition(), Vector2(r * 2.0f, r * 2.0f), Color(1.0f, 1.0f, 1.0f));
        }
    }
};