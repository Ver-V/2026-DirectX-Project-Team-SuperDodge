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

void GameManager::Initialize(HWND hwnd)
{
    _uiManager.Initialize(hwnd);
    _world.Clear();

    _player = _world.AddObject(PrefabFactory::CreatePlayer(_config));

    GameObject* spawnerObject = new GameObject(Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f));
    _spawner = spawnerObject->AddComponent(new ObstacleSpawnerComponent(&_world, _config, _player));
    _world.AddObject(spawnerObject);

    _bossManager.Initialize(&_world, _config, _player);

    _world.Start();
    _scoreManager.Initialize();
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

    if ((_currentState == GameState::GameOver || _currentState == GameState::GameClear) &&
        (spacePressed || rPressed))
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

    _scoreManager.UpdateScore(deltaTime);
    BossEvent bossEvent = _bossManager.Update(deltaTime, _scoreManager.GetSurvivalTime());
    if (bossEvent == BossEvent::Started)
    {
        if (_spawner != nullptr)
            _spawner->SetSpawnCountScale(1.0f / 3.0f);
    }
    else if (bossEvent == BossEvent::Ended)
    {
        if (_spawner != nullptr)
            _spawner->SetSpawnCountScale(1.0f);
    }
    else if (bossEvent == BossEvent::FinalCleared)
    {
        GameClear();
        return;
    }

    if (_scoreManager.IsTimeUp() && !_bossManager.IsActive())
    {
        GameClear();
        return;
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

            const float playerHitboxRadius = playerStatus->GetHitboxRadius();
            const float playerGrazeRadius = GetCircumscribedRadius(playerSize);
            const float obstacleHitboxRadius = obstacleStatus->GetHitboxRadius(obsSize);

            if (IsCircleOverlap(
                playerPos,
                playerHitboxRadius,
                obsPos,
                obstacleHitboxRadius))
            {
                playerStatus->TakeDamage(obstacleStatus->GetDamage());
                obj->SetActive(false);
                GameOver();
                return;
            }

            if (!obstacleStatus->HasGrazed() &&
                IsCircleOverlap(
                    playerPos,
                    playerGrazeRadius,
                    obsPos,
                    obstacleHitboxRadius))
            {
                _scoreManager.AddGrazeScore();
                obstacleStatus->MarkGrazed();

                if (obstacleStatus->IsBossProjectile() && _bossManager.RegisterGraze())
                {
                    if (_spawner != nullptr)
                        _spawner->SetSpawnCountScale(1.0f);
                    break;
                }
            }
        }
    }

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
    else if (_currentState == GameState::GameClear)
    {
        renderer.DrawRect(Vector2(PlayAreaWidth * 0.5f, PlayAreaHeight * 0.5f),
                          Vector2((float)PlayAreaWidth, (float)PlayAreaHeight),
                          Color(0.1f, 0.4f, 1.0f, 0.35f));
    }

    DrawUI(renderer);
}

void GameManager::StartGame()
{
    _currentState = GameState::Playing;
    _world.ClearActiveObstacles();
    _bossManager.Reset();
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
    FinalizeScore(false);

    _spaceWasDown = true;
    _rWasDown = true;
    _xWasDown = true;

    if (_spawner != nullptr)
        _spawner->StopSpawn();

    _bossManager.Stop();

    if (_player != nullptr)
        _player->SetActive(false);
}

void GameManager::GameClear()
{
    _currentState = GameState::GameClear;
    FinalizeScore(true);

    _spaceWasDown = true;
    _rWasDown = true;
    _xWasDown = true;

    if (_spawner != nullptr)
        _spawner->StopSpawn();

    _bossManager.Stop();
    _world.ClearActiveObstacles();
}

void GameManager::RestartGame()
{
    StartGame();
}

void GameManager::FinalizeScore(bool awardBombBonus)
{
    int remainingBombs = 0;

    if (_player != nullptr)
    {
        PlayerStatusComponent* status = _player->GetComponent<PlayerStatusComponent>();
        if (status != nullptr)
            remainingBombs = status->GetBombCount();
    }

    _scoreManager.FinalizeScore(remainingBombs, awardBombBonus);
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

    // High score
    renderer.DrawString(L"HIGH SCORE", Vector2(uiLeft, 115.0f), 18.0f, Color(0.7f, 0.7f, 0.7f));
    renderer.DrawString(std::to_wstring(_scoreManager.GetHighScore()), Vector2(uiLeft, 140.0f), 28.0f, Color(1.0f, 0.85f, 0.2f));

    // Timer
    renderer.DrawString(L"TIME", Vector2(uiLeft, 195.0f), 18.0f, Color(0.7f, 0.7f, 0.7f));
    int totalSeconds = static_cast<int>(_scoreManager.GetSurvivalTime());
    std::wstring timeStr = std::to_wstring(totalSeconds / 60) + L":" + (totalSeconds % 60 < 10 ? L"0" : L"") + std::to_wstring(totalSeconds % 60);
    renderer.DrawString(timeStr, Vector2(uiLeft, 220.0f), 32.0f, Color(1.0f, 1.0f, 1.0f));

    // Bombs
    renderer.DrawString(L"BOMBS", Vector2(uiLeft, 280.0f), 18.0f, Color(0.7f, 0.7f, 0.7f));
    PlayerStatusComponent* status = _player->GetComponent<PlayerStatusComponent>();
    if (status != nullptr)
    {
        int bombs = status->GetBombCount();
        for (int i = 0; i < bombs; ++i)
        {
            renderer.DrawStar(
                Vector2(uiLeft + 12.0f + i * 30.0f, 320.0f),
                12.0f,
                5.5f,
                Color(0.2f, 0.9f, 0.2f));
        }
    }

    if (_bossManager.IsActive())
    {
        renderer.DrawString(
            L"BOSS PHASE " + std::to_wstring(_bossManager.GetPhaseNumber()),
            Vector2(uiLeft, 370.0f),
            18.0f,
            Color(1.0f, 0.3f, 0.8f));
        renderer.DrawString(
            std::to_wstring(static_cast<int>(std::ceil(_bossManager.GetRemainingTime()))),
            Vector2(uiLeft, 395.0f),
            32.0f,
            Color(1.0f, 0.4f, 0.8f));

        if (_bossManager.IsFinalPhase())
        {
            renderer.DrawString(
                L"SURVIVE",
                Vector2(uiLeft, 440.0f),
                18.0f,
                Color(1.0f, 0.7f, 0.9f));
        }
        else
        {
            renderer.DrawString(
                L"GRAZE " + std::to_wstring(_bossManager.GetGrazeCount()) +
                    L"/" + std::to_wstring(_bossManager.GetGrazeTarget()),
                Vector2(uiLeft, 440.0f),
                18.0f,
                Color(1.0f, 0.7f, 0.9f));
        }
    }

    // GameOver
    const float playCenterY = PlayAreaHeight * 0.5f;

    if (_currentState == GameState::GameOver)
    {
        renderer.DrawCenteredString(L"GAME OVER", playCenterY - 60.0f, 80.0f, Color(1.0f, 0.1f, 0.1f));
        renderer.DrawCenteredString(L"PRESS SPACE TO RESTART", playCenterY + 60.0f, 28.0f, Color(1.0f, 1.0f, 1.0f));
    }
    else if (_currentState == GameState::GameClear)
    {
        renderer.DrawCenteredString(L"GAME CLEAR", playCenterY - 90.0f, 80.0f, Color(0.2f, 0.8f, 1.0f));
        renderer.DrawCenteredString(
            L"BOMB BONUS +" + std::to_wstring(_scoreManager.GetBombBonusScore()),
            playCenterY + 20.0f,
            28.0f,
            Color(1.0f, 0.85f, 0.2f));
        renderer.DrawCenteredString(L"PRESS SPACE TO RESTART", playCenterY + 80.0f, 28.0f, Color(1.0f, 1.0f, 1.0f));
    }
    else if (_currentState == GameState::Ready)
    {
        renderer.DrawCenteredString(L"PRESS SPACE TO START", playCenterY, 32.0f, Color(1.0f, 1.0f, 1.0f));
    }

    renderer.EndText();
}
