#pragma once

#include "../Core/Component.hpp"

class PlayerStatusComponent : public Component
{
private:
    int _hp = 1;
    int _bombCount = 3;
    bool _isDead = false;
    float _hitboxRadius = 5.0f;

public:
    void Reset(int hp)
    {
        _hp = hp;
        _bombCount = 3;
        _isDead = false;
    }

    void TakeDamage(int damage)
    {
        if (_isDead) return;
        _hp -= damage;
        if (_hp <= 0) _isDead = true;
    }

    bool UseBomb()
    {
        if (_bombCount > 0 && !_isDead)
        {
            _bombCount--;
            return true;
        }
        return false;
    }

    int GetBombCount() const { return _bombCount; }
    float GetHitboxRadius() const { return _hitboxRadius; }
    bool IsDead() const { return _isDead; }
};
