#pragma once

#include <cmath>

#include "../Core/GameConstants.hpp"
#include "../Core/GameObject.hpp"
#include "../Core/GameWorld.hpp"
#include "../Components/AIComponent.hpp"
#include "../Components/ObstacleStatusComponent.hpp"
#include "../Components/RectRendererComponent.hpp"
#include "GameConfig.hpp"
#include "GameEnums.hpp"
#include "ObjectPool.hpp"

enum class BossEvent
{
    None,
    Started,
    Ended,
    FinalCleared
};

enum class BossState
{
    Waiting,
    Active
};

enum class BossPhase
{
    None,
    Phase1,
    Phase2,
    Phase3,
    Phase4,
    Final
};

struct BossPhaseConfig
{
    BossPhase phase;
    float spawnTime;
    float duration;
    float shotInterval;
    float projectileSpeed;
    int projectileCount;
    int requiredGraze;
    bool allowGrazeClear;
};

class BossManager
{
private:
    static constexpr float Pi = 3.14159265f;
    static constexpr int PhaseCount = 5;

    GameWorld* _world = nullptr;
    GameObject* _target = nullptr;
    GameObject* _bossObject = nullptr;
    ObjectPool _projectilePool;

    BossState _state = BossState::Waiting;
    BossPhase _phase = BossPhase::None;
    int _nextPhaseIndex = 0;
    float _bossElapsed = 0.0f;
    float _shotTimer = 0.0f;
    int _burstIndex = 0;
    int _grazeCount = 0;

public:
    void Initialize(GameWorld* world, const GameConfig& config, GameObject* target)
    {
        _world = world;
        _target = target;
        _projectilePool.Initialize(world, config, 180);

        _bossObject = new GameObject(
            Vector2(PlayAreaWidth * 0.5f, 90.0f),
            Vector2(140.0f, 70.0f));
        _bossObject->AddComponent(new RectRendererComponent(Color(0.9f, 0.15f, 0.75f)));
        _bossObject->SetActive(false);
        _world->AddObject(_bossObject);

        Reset();
    }

    void Reset()
    {
        _state = BossState::Waiting;
        _phase = BossPhase::None;
        _nextPhaseIndex = 0;
        _bossElapsed = 0.0f;
        _shotTimer = 0.0f;
        _burstIndex = 0;
        _grazeCount = 0;

        _projectilePool.Reset();
        if (_bossObject != nullptr)
            _bossObject->SetActive(false);
    }

    void Stop()
    {
        _state = BossState::Waiting;
        _phase = BossPhase::None;
        _projectilePool.Reset();

        if (_bossObject != nullptr)
            _bossObject->SetActive(false);
    }

    BossEvent Update(float deltaTime, float survivalTime)
    {
        if (_state == BossState::Waiting)
        {
            const BossPhaseConfig* phaseConfigs = GetPhaseConfigs();
            if (_nextPhaseIndex < PhaseCount &&
                survivalTime >= phaseConfigs[_nextPhaseIndex].spawnTime)
            {
                StartPhase(phaseConfigs[_nextPhaseIndex]);
                ++_nextPhaseIndex;
                return BossEvent::Started;
            }

            return BossEvent::None;
        }

        const BossPhaseConfig& config = GetCurrentConfig();
        _bossElapsed += deltaTime;
        _shotTimer += deltaTime;

        UpdateBossPosition();

        while (_shotTimer >= config.shotInterval)
        {
            _shotTimer -= config.shotInterval;
            ExecutePattern(config);
        }

        if (_bossElapsed >= config.duration)
        {
            const bool wasFinal = _phase == BossPhase::Final;
            EndPhase();
            return wasFinal ? BossEvent::FinalCleared : BossEvent::Ended;
        }

        return BossEvent::None;
    }

    bool RegisterGraze()
    {
        if (_state != BossState::Active) return false;

        const BossPhaseConfig& config = GetCurrentConfig();
        if (!config.allowGrazeClear)
            return false;

        ++_grazeCount;
        if (_grazeCount < config.requiredGraze)
            return false;

        EndPhase();
        return true;
    }

    bool IsActive() const
    {
        return _state == BossState::Active;
    }

    bool IsFinalPhase() const
    {
        return _phase == BossPhase::Final;
    }

    BossPhase GetPhase() const
    {
        return _phase;
    }

    int GetPhaseNumber() const
    {
        switch (_phase)
        {
        case BossPhase::Phase1: return 1;
        case BossPhase::Phase2: return 2;
        case BossPhase::Phase3: return 3;
        case BossPhase::Phase4: return 4;
        case BossPhase::Final: return 5;
        default: return 0;
        }
    }

    int GetGrazeCount() const
    {
        return _grazeCount;
    }

    int GetGrazeTarget() const
    {
        return IsActive() ? GetCurrentConfig().requiredGraze : 0;
    }

    float GetRemainingTime() const
    {
        if (!IsActive()) return 0.0f;
        return (std::max)(0.0f, GetCurrentConfig().duration - _bossElapsed);
    }

    float GetNextPhaseRemainingTime(float survivalTime) const
    {
        if (_state != BossState::Waiting || _nextPhaseIndex >= PhaseCount)
            return -1.0f;

        const BossPhaseConfig* phaseConfigs = GetPhaseConfigs();
        return phaseConfigs[_nextPhaseIndex].spawnTime - survivalTime;
    }

    bool IsWarningTime(float survivalTime) const
    {
        const float remainingTime = GetNextPhaseRemainingTime(survivalTime);
        return remainingTime > 0.0f && remainingTime <= 5.0f;
    }

    void ForceStartPhase(int phaseIndex)
    {
        if (phaseIndex < 0 || phaseIndex >= PhaseCount)
            return;

        const BossPhaseConfig* phaseConfigs = GetPhaseConfigs();
        StartPhase(phaseConfigs[phaseIndex]);
        _nextPhaseIndex = phaseIndex + 1;
    }

private:
    static const BossPhaseConfig* GetPhaseConfigs()
    {
        static const BossPhaseConfig phaseConfigs[PhaseCount] = {
            { BossPhase::Phase1,  60.0f, 30.0f, 0.90f, 220.0f,  8, 15, true },
            { BossPhase::Phase2, 120.0f, 30.0f, 0.70f, 240.0f, 10, 15, true },
            { BossPhase::Phase3, 180.0f, 30.0f, 0.65f, 270.0f,  5, 15, true },
            { BossPhase::Phase4, 240.0f, 30.0f, 0.55f, 290.0f, 10, 15, true },
            { BossPhase::Final,  300.0f, 30.0f, 0.42f, 320.0f, 12,  0, false }
        };

        return phaseConfigs;
    }

    const BossPhaseConfig& GetCurrentConfig() const
    {
        const BossPhaseConfig* phaseConfigs = GetPhaseConfigs();
        for (int i = 0; i < PhaseCount; ++i)
        {
            if (phaseConfigs[i].phase == _phase)
                return phaseConfigs[i];
        }

        return phaseConfigs[0];
    }

    void StartPhase(const BossPhaseConfig& config)
    {
        _state = BossState::Active;
        _phase = config.phase;
        _bossElapsed = 0.0f;
        _shotTimer = config.shotInterval;
        _burstIndex = 0;
        _grazeCount = 0;
        _projectilePool.Reset();

        if (_bossObject != nullptr)
        {
            _bossObject->SetPosition(Vector2(PlayAreaWidth * 0.5f, 90.0f));
            _bossObject->SetSize(
                _phase == BossPhase::Final
                    ? Vector2(180.0f, 90.0f)
                    : Vector2(140.0f, 70.0f));
            _bossObject->SetActive(true);
        }
    }

    void EndPhase()
    {
        _state = BossState::Waiting;
        _phase = BossPhase::None;
        _projectilePool.Reset();

        if (_bossObject != nullptr)
            _bossObject->SetActive(false);
    }

    void UpdateBossPosition()
    {
        if (_bossObject == nullptr) return;

        const float centerX = PlayAreaWidth * 0.5f;
        float frequency = 1.0f;
        float moveRange = PlayAreaWidth * 0.30f;

        if (_phase == BossPhase::Phase3)
            frequency = 1.7f;
        else if (_phase == BossPhase::Phase4)
            frequency = 2.0f;
        else if (_phase == BossPhase::Final)
        {
            frequency = 2.3f;
            moveRange = PlayAreaWidth * 0.36f;
        }

        const float x = centerX + std::sin(_bossElapsed * frequency) * moveRange;
        _bossObject->SetPosition(Vector2(x, 90.0f));
    }

    void ExecutePattern(const BossPhaseConfig& config)
    {
        switch (config.phase)
        {
        case BossPhase::Phase1:
            SpawnRadial(config.projectileCount, config.projectileSpeed, AlternatingOffset(config.projectileCount));
            break;

        case BossPhase::Phase2:
            SpawnRadial(config.projectileCount, config.projectileSpeed, _burstIndex * 0.16f);
            break;

        case BossPhase::Phase3:
            SpawnAimedFan(config.projectileCount, config.projectileSpeed, 0.16f);
            break;

        case BossPhase::Phase4:
            if (_burstIndex % 2 == 0)
                SpawnRadial(config.projectileCount, config.projectileSpeed, _burstIndex * 0.12f);
            else
                SpawnAimedFan(7, config.projectileSpeed + 20.0f, 0.13f);
            break;

        case BossPhase::Final:
            SpawnRadial(config.projectileCount, config.projectileSpeed, _burstIndex * 0.20f);
            if (_burstIndex % 2 == 0)
                SpawnAimedFan(5, config.projectileSpeed + 40.0f, 0.11f);
            break;

        default:
            break;
        }

        ++_burstIndex;
    }

    float AlternatingOffset(int projectileCount) const
    {
        return (_burstIndex % 2 == 0) ? 0.0f : Pi / projectileCount;
    }

    void SpawnRadial(int projectileCount, float speed, float angleOffset)
    {
        for (int i = 0; i < projectileCount; ++i)
        {
            const float angle = angleOffset + (2.0f * Pi * i / projectileCount);
            SpawnProjectile(Vector2(std::cos(angle), std::sin(angle)), speed);
        }
    }

    void SpawnAimedFan(int projectileCount, float speed, float angleStep)
    {
        if (_bossObject == nullptr || _target == nullptr) return;

        const Vector2 spawnPosition = _bossObject->GetPosition();
        const Vector2 targetPosition = _target->GetPosition();
        const float centerAngle = std::atan2(
            targetPosition.y - spawnPosition.y,
            targetPosition.x - spawnPosition.x);
        const float centerIndex = (projectileCount - 1) * 0.5f;

        for (int i = 0; i < projectileCount; ++i)
        {
            const float angle = centerAngle + (i - centerIndex) * angleStep;
            SpawnProjectile(Vector2(std::cos(angle), std::sin(angle)), speed);
        }
    }

    void SpawnProjectile(const Vector2& direction, float speed)
    {
        if (_bossObject == nullptr) return;

        const Vector2 spawnPosition = _bossObject->GetPosition();
        GameObject* projectile = _projectilePool.GetObject(ObstacleType::Normal);
        if (projectile == nullptr) return;

        projectile->SetPosition(spawnPosition);
        projectile->SetSize(
            _phase == BossPhase::Final
                ? Vector2(20.0f, 20.0f)
                : Vector2(24.0f, 24.0f));

        ObstacleStatusComponent* status = projectile->GetComponent<ObstacleStatusComponent>();
        if (status != nullptr)
        {
            status->ResetGraze();
            status->SetBossProjectile(true);
        }

        AIComponent* ai = projectile->GetComponent<AIComponent>();
        if (ai != nullptr)
        {
            const Vector2 targetPosition(
                spawnPosition.x + direction.x * 100.0f,
                spawnPosition.y + direction.y * 100.0f);
            ai->Initialize(targetPosition, speed, _target);
        }

        projectile->SetActive(true);
    }
};
