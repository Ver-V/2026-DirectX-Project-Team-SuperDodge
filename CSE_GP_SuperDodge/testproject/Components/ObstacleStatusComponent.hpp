#pragma once

#include "../Core/Component.hpp"

class GameObject;

class ObstacleStatusComponent : public Component
{
private:
    GameObject* _target = nullptr;
    int _damage = 1;
    bool _hasGrazed = false;

public:
    ObstacleStatusComponent(GameObject* target, int damage = 1)
        : _target(target), _damage(damage)
    {
    }

    void SetTarget(GameObject* target)
    {
        _target = target;
    }

    int GetDamage() const { return _damage; }
    bool HasGrazed() const { return _hasGrazed; }
    void MarkGrazed() { _hasGrazed = true; }
    void ResetGraze() { _hasGrazed = false; }
};
