#pragma once

enum class GameState
{
    Ready,
    Playing,
    Paused,
    GameOver
};

enum class ObstacleType
{
    Normal,
    Fast,
    Guided
};