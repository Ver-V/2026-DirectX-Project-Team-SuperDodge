#pragma once

#include <random>
#include <chrono>
#include <algorithm>

#include "../Core/Component.hpp"
#include "../Core/GameObject.hpp"
#include "../Core/GameWorld.hpp"
#include "../Core/GameConstants.hpp"
#include "../Core/MathUtils.hpp"

#include "../Game/GameConfig.hpp"
#include "../Game/GameEnums.hpp"
#include "../Game/ObstacleData.hpp"
#include "../Game/ObjectPool.hpp"

#include "AIComponent.hpp"

class ObstacleSpawnerComponent : public Component
{
private:
    GameWorld* _world = nullptr;
    GameObject* _target = nullptr;
    GameConfig _config;
    ObjectPool _objectPool;

    float _spawnTimer = 0.0f;
    float _difficultyTimer = 0.0f;
    float _currentSpawnInterval = 0.75f;
    float _currentObstacleSpeed = 180.0f;
    bool _isSpawning = false;

    std::mt19937 _randomEngine;

public:
    ObstacleSpawnerComponent(GameWorld* world, const GameConfig& config, GameObject* target)
        : _world(world), _target(target), _config(config)
    {
        _randomEngine.seed(static_cast<unsigned int>(std::chrono::steady_clock::now().time_since_epoch().count()));

        _objectPool.Initialize(_world, _config, _target, 150);

        Reset();
    }

    void Reset()
    {
        _spawnTimer = 0.0f;
        _difficultyTimer = 0.0f;
        _currentSpawnInterval = _config.spawnStartInterval;
        _currentObstacleSpeed = _config.obstacleStartSpeed;
        _isSpawning = false;

        _objectPool.Reset();
    }

    void StartSpawn()
    {
        _isSpawning = true;
    }

    void StopSpawn()
    {
        _isSpawning = false;
    }

    void Update(float deltaTime) override
    {
        if (!_isSpawning) return;
        if (_target == nullptr) return;

        _spawnTimer += deltaTime;
        _difficultyTimer += deltaTime;

        if (_spawnTimer >= _currentSpawnInterval)
        {
            _spawnTimer = 0.0f;
            SpawnObstacle();
        }

        IncreaseDifficulty(deltaTime);
    }

private:
    void SpawnObstacle()
    {
        std::uniform_real_distribution<float> xDist(0.0f, static_cast<float>(ScreenWidth));
        std::uniform_real_distribution<float> yDist(0.0f, static_cast<float>(ScreenHeight));
        std::uniform_real_distribution<float> aimOffsetDist(-120.0f, 120.0f);
        std::uniform_int_distribution<int> sideDist(0, 3);

        ObstacleType obstacleType = PickObstacleType();
        const ObstacleData& obstacleData = GetObstacleData(obstacleType);

        float size = obstacleData.size;
        float halfSize = size * 0.5f;
        int side = sideDist(_randomEngine);

        Vector2 spawnPosition;

        if (side == 0)
            spawnPosition = Vector2(xDist(_randomEngine), -halfSize);
        else if (side == 1)
            spawnPosition = Vector2(xDist(_randomEngine), ScreenHeight + halfSize);
        else if (side == 2)
            spawnPosition = Vector2(-halfSize, yDist(_randomEngine));
        else
            spawnPosition = Vector2(ScreenWidth + halfSize, yDist(_randomEngine));

        Vector2 targetPosition = _target->GetPosition();
        targetPosition.x = ClampFloat(targetPosition.x + aimOffsetDist(_randomEngine), 0.0f, static_cast<float>(ScreenWidth));
        targetPosition.y = ClampFloat(targetPosition.y + aimOffsetDist(_randomEngine), 0.0f, static_cast<float>(ScreenHeight));

        GameObject* obstacle = _objectPool.GetObject(obstacleType);

        if (obstacle == nullptr)
            return;

        obstacle->SetPosition(spawnPosition);
        obstacle->SetSize(Vector2(size, size));
        obstacle->SetActive(true);

        AIComponent* ai = obstacle->GetComponent<AIComponent>();

        if (ai != nullptr)
            ai->Initialize(targetPosition, _currentObstacleSpeed, _target);
    }

    ObstacleType PickObstacleType()
    {
        std::uniform_int_distribution<int> typeDist(0, 99);
        int typeValue = typeDist(_randomEngine);

        if (typeValue < 60)
            return ObstacleType::Normal;

        if (typeValue < 85)
            return ObstacleType::Fast;

        return ObstacleType::Guided;
    }

    const ObstacleData& GetObstacleData(ObstacleType type) const
    {
        if (type == ObstacleType::Normal)
            return _config.normalObstacle;

        if (type == ObstacleType::Fast)
            return _config.fastObstacle;

        return _config.guidedObstacle;
    }

    void IncreaseDifficulty(float deltaTime)
    {
        _currentSpawnInterval = std::max(_config.spawnMinInterval, _currentSpawnInterval - 0.018f * deltaTime);
        _currentObstacleSpeed = std::min(_config.obstacleMaxSpeed, _currentObstacleSpeed + 14.0f * deltaTime);
    }
};