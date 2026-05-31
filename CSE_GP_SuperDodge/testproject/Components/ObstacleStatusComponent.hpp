#pragma once

#include "../Core/Component.hpp"
#include "../Core/GameObject.hpp"
#include "../Core/MathUtils.hpp"
#include "PlayerStatusComponent.hpp"

class ObstacleStatusComponent : public Component
{
private:
    GameObject* _target = nullptr;
    int _damage = 1;

public:
    ObstacleStatusComponent(GameObject* target, int damage = 1)
        : _target(target), _damage(damage)
    {
    }

    void SetTarget(GameObject* target)
    {
        _target = target;
    }

    void Update(float deltaTime) override
    {
        UNREFERENCED_PARAMETER(deltaTime);

        if (owner == nullptr) return;
        if (_target == nullptr) return;
        if (!_target->IsActive()) return;

        PlayerStatusComponent* playerStatus = _target->GetComponent<PlayerStatusComponent>();

        if (playerStatus == nullptr) return;
        if (playerStatus->IsDead()) return;
        if (!IsOverlap(owner->GetBounds(), _target->GetBounds())) return;

        playerStatus->TakeDamage(_damage);
        owner->SetActive(false);
    }
};