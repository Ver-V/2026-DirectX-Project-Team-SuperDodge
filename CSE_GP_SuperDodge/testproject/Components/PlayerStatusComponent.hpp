#pragma once

#include "../Core/Component.hpp"

class PlayerStatusComponent : public Component
{
private:
    int _hp = 1;
    bool _isDead = false;

public:
    void Reset(int hp)
    {
        _hp = hp;
        _isDead = false;
    }

    void TakeDamage(int damage)
    {
        if (_isDead) return;

        _hp -= damage;

        if (_hp <= 0)
            _isDead = true;
    }

    bool IsDead() const
    {
        return _isDead;
    }
};