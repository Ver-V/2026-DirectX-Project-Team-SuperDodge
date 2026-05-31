#include "GameManager.hpp"

#include "../Rendering/Renderer.hpp"
#include "../Core/GameObject.hpp"

#include "PrefabFactory.hpp"
#include "../Components/ObstacleSpawnerComponent.hpp"
#include "../Components/PlayerStatusComponent.hpp"

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

    bool spacePressed = spaceDown && !_spaceWasDown;
    bool rPressed = rDown && !_rWasDown;

    _spaceWasDown = spaceDown;
    _rWasDown = rDown;

    if (_currentState == GameState::Ready && spacePressed)
        StartGame();

    if (_currentState == GameState::GameOver && (spacePressed || rPressed))
        RestartGame();

    if (_currentState != GameState::Playing)
        return;

    _world.Update(deltaTime);

    _scoreManager.UpdateScore(deltaTime);
    _uiManager.UpdateScoreText(_scoreManager.GetScore(), _scoreManager.GetSurvivalTime());

    PlayerStatusComponent* playerStatus = _player->GetComponent<PlayerStatusComponent>();

    if (playerStatus != nullptr && playerStatus->IsDead())
        GameOver();
}

void GameManager::Draw(Renderer& renderer)
{
    if (_currentState == GameState::GameOver)
        renderer.Clear(0.16f, 0.04f, 0.04f);
    else
        renderer.Clear(0.04f, 0.05f, 0.08f);

    DrawPlayArea(renderer);
    _world.Render(renderer);
}

void GameManager::StartGame()
{
    _currentState = GameState::Playing;

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

    if (_spawner != nullptr)
        _spawner->StopSpawn();

    _uiManager.ShowGameOverUI(_scoreManager.GetScore());
}

void GameManager::RestartGame()
{
    StartGame();
}

void GameManager::ResetPlayer()
{
    if (_player == nullptr) return;

    _player->SetPosition(Vector2(ScreenWidth * 0.5f, ScreenHeight * 0.5f));
    _player->SetSize(_config.playerSize);
    _player->SetActive(true);

    PlayerStatusComponent* status = _player->GetComponent<PlayerStatusComponent>();

    if (status != nullptr)
        status->Reset(_config.playerHp);
}

void GameManager::DrawPlayArea(Renderer& renderer)
{
    renderer.DrawRect(Vector2(ScreenWidth * 0.5f, 10.0f), Vector2(static_cast<float>(ScreenWidth), 20.0f), Color(0.12f, 0.15f, 0.22f));
    renderer.DrawRect(Vector2(ScreenWidth * 0.5f, ScreenHeight - 10.0f), Vector2(static_cast<float>(ScreenWidth), 20.0f), Color(0.12f, 0.15f, 0.22f));
    renderer.DrawRect(Vector2(10.0f, ScreenHeight * 0.5f), Vector2(20.0f, static_cast<float>(ScreenHeight)), Color(0.12f, 0.15f, 0.22f));
    renderer.DrawRect(Vector2(ScreenWidth - 10.0f, ScreenHeight * 0.5f), Vector2(20.0f, static_cast<float>(ScreenHeight)), Color(0.12f, 0.15f, 0.22f));
}