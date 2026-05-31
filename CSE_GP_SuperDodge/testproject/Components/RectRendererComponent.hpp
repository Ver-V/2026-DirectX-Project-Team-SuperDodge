#pragma once

#include "../Core/Component.hpp"
#include "../Core/GameObject.hpp"
#include "../Rendering/Renderer.hpp"

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
        renderer.DrawRect(owner->GetPosition(), owner->GetSize(), _color);
    }
};