#pragma once

#include "../Core/GameObject.hpp"
#include "GameConfig.hpp"

#include "../Components/RectRendererComponent.hpp"
#include "../Components/PlayerStatusComponent.hpp"
#include "../Components/PlayerControllerComponent.hpp"
#include "../Components/ObstacleStatusComponent.hpp"
#include "../Components/NormalAIComponent.hpp"
#include "../Components/FastAIComponent.hpp"
#include "../Components/GuidedAIComponent.hpp"

class PrefabFactory
{
public:
    static GameObject* CreatePlayer(const GameConfig& config)
    {
        GameObject* player = new GameObject(Vector2(ScreenWidth * 0.5f, ScreenHeight * 0.5f), config.playerSize);

        player->AddComponent(new RectRendererComponent(Color(0.25f, 0.75f, 1.0f)));
        player->AddComponent(new PlayerStatusComponent());
        player->AddComponent(new PlayerControllerComponent(config.playerMoveSpeed));

        PlayerStatusComponent* status = player->GetComponent<PlayerStatusComponent>();
        if (status != nullptr)
            status->Reset(config.playerHp);

        return player;
    }

    static GameObject* CreateNormalObstacle(const GameConfig& config, GameObject* target)
    {
        const ObstacleData& data = config.normalObstacle;

        GameObject* obstacle = new GameObject(Vector2(-1000.0f, -1000.0f), Vector2(data.size, data.size));

        obstacle->AddComponent(new RectRendererComponent(data.color));
        obstacle->AddComponent(new ObstacleStatusComponent(target));
        obstacle->AddComponent(new NormalAIComponent());
        obstacle->SetActive(false);

        return obstacle;
    }

    static GameObject* CreateFastObstacle(const GameConfig& config, GameObject* target)
    {
        const ObstacleData& data = config.fastObstacle;

        GameObject* obstacle = new GameObject(Vector2(-1000.0f, -1000.0f), Vector2(data.size, data.size));

        obstacle->AddComponent(new RectRendererComponent(data.color));
        obstacle->AddComponent(new ObstacleStatusComponent(target));
        obstacle->AddComponent(new FastAIComponent());
        obstacle->SetActive(false);

        return obstacle;
    }

    static GameObject* CreateGuidedObstacle(const GameConfig& config, GameObject* target)
    {
        const ObstacleData& data = config.guidedObstacle;

        GameObject* obstacle = new GameObject(Vector2(-1000.0f, -1000.0f), Vector2(data.size, data.size));

        obstacle->AddComponent(new RectRendererComponent(data.color));
        obstacle->AddComponent(new ObstacleStatusComponent(target));
        obstacle->AddComponent(new GuidedAIComponent());
        obstacle->SetActive(false);

        return obstacle;
    }
};