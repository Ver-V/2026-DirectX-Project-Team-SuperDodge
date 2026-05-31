#pragma once

#include "../Core/GameConstants.hpp"
#include "../Core/MathTypes.hpp"
#include "../Game/ObstacleData.hpp"

struct GameConfig
{
    float playerMoveSpeed = 430.0f;
    Vector2 playerSize = Vector2(42.0f, 42.0f);
    int playerHp = 1;

    ObstacleData normalObstacle = ObstacleData{ 48.0f, Color(1.0f, 0.25f, 0.25f) };
    ObstacleData fastObstacle = ObstacleData{ 32.0f, Color(1.0f, 0.85f, 0.15f) };
    ObstacleData guidedObstacle = ObstacleData{ 60.0f, Color(0.75f, 0.25f, 1.0f) };

    float obstacleStartSpeed = 180.0f;
    float obstacleMaxSpeed = 540.0f;
    float spawnStartInterval = 0.75f;
    float spawnMinInterval = 0.16f;
};