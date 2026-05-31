#pragma once

class ScoreManager
{
private:
    float _survivalTime = 0.0f;
    int _score = 0;

public:
    void ResetScore()
    {
        _survivalTime = 0.0f;
        _score = 0;
    }

    void UpdateScore(float deltaTime)
    {
        _survivalTime += deltaTime;
        _score = static_cast<int>(_survivalTime * 10.0f);
    }

    int GetScore() const
    {
        return _score;
    }

    float GetSurvivalTime() const
    {
        return _survivalTime;
    }
};