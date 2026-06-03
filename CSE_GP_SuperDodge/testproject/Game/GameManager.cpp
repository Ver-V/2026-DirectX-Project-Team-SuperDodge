#include "GameManager.hpp"

#include "../Rendering/Renderer.hpp"
#include "../Core/GameObject.hpp"
#include "../Core/GameConstants.hpp"
#include "../Core/MathUtils.hpp"

#include "PrefabFactory.hpp"
#include "../Components/ObstacleSpawnerComponent.hpp"
#include "../Components/PlayerStatusComponent.hpp"
#include "../Components/ObstacleStatusComponent.hpp"

#include <cmath>
#include <algorithm>
#include <string>

namespace
{
    bool IsCircleOverlapRect(const Vector2& circleCenter, float circleRadius, const Rect& rect)
    {
        const float halfW = rect.size.x * 0.5f;
        const float halfH = rect.size.y * 0.5f;

        const float closestX = ClampFloat(circleCenter.x, rect.center.x - halfW, rect.center.x + halfW);
        const float closestY = ClampFloat(circleCenter.y, rect.center.y - halfH, rect.center.y + halfH);

        const float dx = circleCenter.x - closestX;
        const float dy = circleCenter.y - closestY;

        return dx * dx + dy * dy <= circleRadius * circleRadius;
    }
}

void GameManager::Initialize(HWND hwnd)
{
    _uiManager.Initialize(hwnd);
    _world.Clear();

    _player = _world.AddObject(PrefabFactory::CreatePlayer(_config));

    GameObject* spawnerObject = new GameObject(Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f));
    _spawner = spawnerObject->AddComponent(new ObstacleSpawnerComponent(&_world, _config, _player));
    _world.AddObject(spawnerObject);

    _world.Start();
    _scoreManager.ResetScore();
    _currentState = GameState::Ready;

    _uiManager.ShowStartUI();
}

void GameManager::Update(float deltaTime)
{
    bool spaceDown = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
    bool rDown = (GetAsyncKeyState('R') & 0x8000) != 0;
    bool xDown = (GetAsyncKeyState('X') & 0x8000) != 0;

    bool spacePressed = spaceDown && !_spaceWasDown;
    bool rPressed = rDown && !_rWasDown;
    bool xPressed = xDown && !_xWasDown;

    _spaceWasDown = spaceDown;
    _rWasDown = rDown;
    _xWasDown = xDown;

    if (_currentState == GameState::Ready && spacePressed)
    {
        StartGame();
        return;
    }

    if (_currentState == GameState::GameOver && (spacePressed || rPressed))
    {
        RestartGame();
        return;
    }

    if (_currentState != GameState::Playing)
        return;

    // 폭탄 사용
    if (xPressed)
    {
        PlayerStatusComponent* status = _player->GetComponent<PlayerStatusComponent>();
        if (status != nullptr && status->UseBomb())
        {
            _world.ClearActiveObstacles();
            _bombFlashRequest = true;
        }
    }

    _world.Update(deltaTime);

    // 죽음 or 그레이즈
    PlayerStatusComponent* playerStatus = _player->GetComponent<PlayerStatusComponent>();
    if (playerStatus != nullptr && !playerStatus->IsDead())
    {
        Vector2 playerPos = _player->GetPosition();
        Vector2 playerSize = _player->GetSize();

        const auto& objects = _world.GetObjects();
        for (const auto& obj : objects)
        {
            if (!obj->IsActive() || obj.get() == _player)
                continue;

            ObstacleStatusComponent* obstacleStatus = obj->GetComponent<ObstacleStatusComponent>();
            if (obstacleStatus == nullptr)
                continue;

            Vector2 obsPos = obj->GetPosition();
            Vector2 obsSize = obj->GetSize();

            const float playerHitboxR = playerStatus->GetHitboxRadius();
            Rect obstacleBounds{ obsPos, obsSize };

            if (IsCircleOverlapRect(playerPos, playerHitboxR, obstacleBounds))
            {
                playerStatus->TakeDamage(obstacleStatus->GetDamage());
                obj->SetActive(false);
                GameOver();
                return;
            }

            Rect grazeBounds{ playerPos, playerSize };

            if (!obstacleStatus->HasGrazed() && IsOverlap(grazeBounds, obstacleBounds))
            {
                _scoreManager.AddGrazeScore();
                obstacleStatus->MarkGrazed();
            }
        }
    }

    _scoreManager.UpdateScore(deltaTime);
}

void GameManager::Draw(Renderer& renderer, float deltaTime)
{
    if (_bombFlashRequest)
    {
        renderer.SetFlash(1.0f);
        _bombFlashRequest = false;
    }

    renderer.UpdateFlash(deltaTime);

    renderer.Clear(0.05f, 0.05f, 0.1f);

    _world.Render(renderer);

    if (_currentState == GameState::GameOver)
    {
        renderer.DrawRect(Vector2(PlayAreaWidth * 0.5f, PlayAreaHeight * 0.5f), 
                          Vector2((float)PlayAreaWidth, (float)PlayAreaHeight), 
                          Color(1.0f, 0.0f, 0.0f, 0.4f));
    }

    DrawUI(renderer);
}

void GameManager::StartGame()
{
    _currentState = GameState::Playing;
    _world.ClearActiveObstacles();
    ResetPlayer();
    if (_spawner != nullptr)
    {
        _spawner->Reset();
        _spawner->StartSpawn();
    }
    _scoreManager.ResetScore();
}

void GameManager::GameOver()
{
    _currentState = GameState::GameOver;

    _spaceWasDown = true;
    _rWasDown = true;
    _xWasDown = true;

    if (_spawner != nullptr)
        _spawner->StopSpawn();

    if (_player != nullptr)
        _player->SetActive(false);
}

void GameManager::RestartGame()
{
    StartGame();
}

void GameManager::ResetPlayer()
{
    if (_player == nullptr) return;
    _player->SetPosition(Vector2(PlayAreaWidth * 0.5f, PlayAreaHeight * 0.5f));
    _player->SetSize(_config.playerSize);
    _player->SetActive(true);
    PlayerStatusComponent* status = _player->GetComponent<PlayerStatusComponent>();
    if (status != nullptr) status->Reset(_config.playerHp);
}

void GameManager::DrawUI(Renderer& renderer)
{
    // UI
    renderer.DrawRect(Vector2(PlayAreaWidth + UIAreaWidth * 0.5f, ScreenHeight * 0.5f), 
                      Vector2((float)UIAreaWidth, (float)ScreenHeight), 
                      Color(0.1f, 0.1f, 0.12f));

    // 구분선
    renderer.DrawRect(Vector2(PlayAreaWidth, ScreenHeight * 0.5f), 
                      Vector2(4.0f, (float)ScreenHeight), 
                      Color(0.25f, 0.25f, 0.3f));

    renderer.BeginText();

    float uiLeft = PlayAreaWidth + 25.0f;

    // Score
    renderer.DrawString(L"SCORE", Vector2(uiLeft, 40.0f), 18.0f, Color(0.7f, 0.7f, 0.7f));
    renderer.DrawString(std::to_wstring(_scoreManager.GetScore()), Vector2(uiLeft, 65.0f), 32.0f, Color(1.0f, 1.0f, 1.0f));

    // Timer
    renderer.DrawString(L"TIME", Vector2(uiLeft, 130.0f), 18.0f, Color(0.7f, 0.7f, 0.7f));
    int totalSeconds = static_cast<int>(_scoreManager.GetSurvivalTime());
    std::wstring timeStr = std::to_wstring(totalSeconds / 60) + L":" + (totalSeconds % 60 < 10 ? L"0" : L"") + std::to_wstring(totalSeconds % 60);
    renderer.DrawString(timeStr, Vector2(uiLeft, 155.0f), 32.0f, Color(1.0f, 1.0f, 1.0f));

    // Bombs
    renderer.DrawString(L"BOMBS", Vector2(uiLeft, 220.0f), 18.0f, Color(0.7f, 0.7f, 0.7f));
    PlayerStatusComponent* status = _player->GetComponent<PlayerStatusComponent>();
    if (status != nullptr)
    {
        int bombs = status->GetBombCount();
        for (int i = 0; i < bombs; ++i)
        {
            renderer.DrawRect(Vector2(uiLeft + 12.0f + i * 30.0f, 260.0f), 
                              Vector2(18.0f, 18.0f), Color(0.2f, 0.9f, 0.2f));
        }
    }

    // GameOver
    if (_currentState == GameState::GameOver)
    {
        renderer.DrawString(L"GAME OVER", Vector2(120.0f, 300.0f), 80.0f, Color(1.0f, 0.1f, 0.1f));
        renderer.DrawString(L"PRESS SPACE TO RESTART", Vector2(160.0f, 420.0f), 28.0f, Color(1.0f, 1.0f, 1.0f));
    }
    else if (_currentState == GameState::Ready)
    {
        renderer.DrawString(L"PRESS SPACE TO START", Vector2(140.0f, 320.0f), 32.0f, Color(1.0f, 1.0f, 1.0f));
    }

    renderer.EndText();
}
