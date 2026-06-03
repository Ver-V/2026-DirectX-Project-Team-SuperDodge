#pragma once

#define NOMINMAX
#include <windows.h>

#include "../Core/GameWorld.hpp"
#include "GameEnums.hpp"
#include "GameConfig.hpp"
#include "ScoreManager.hpp"
#include "UIManager.hpp"

class Renderer;
class GameObject;
class ObstacleSpawnerComponent;

class GameManager
{
private:
    GameState _currentState = GameState::Ready;
    GameConfig _config;
    GameWorld _world;

    GameObject* _player = nullptr;
    ObstacleSpawnerComponent* _spawner = nullptr;

    ScoreManager _scoreManager;
    UIManager _uiManager;

    bool _spaceWasDown = false;
    bool _rWasDown = false;
    bool _xWasDown = false;
    bool _bombFlashRequest = false;

public:
    void Initialize(HWND hwnd);
    void Update(float deltaTime);
    void Draw(Renderer& renderer, float deltaTime);

    void StartGame();
    void GameOver();
    void RestartGame();

private:
    void ResetPlayer();
    void DrawUI(Renderer& renderer);
};
