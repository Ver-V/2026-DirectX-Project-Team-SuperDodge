#pragma once

class ScoreManager
{
private:
    float _survivalTime = 0.0f;
    int _bonusScore = 0;

public:
    void ResetScore()
    {
        _survivalTime = 0.0f;
        _bonusScore = 0;
    }

    void UpdateScore(float deltaTime)
    {
        _survivalTime += deltaTime;
    }

    void AddGrazeScore()
    {
        _bonusScore += 50;
    }

    void AddScore(int amount)
    {
        _bonusScore += amount;
    }

    int GetScore() const
    {
        return static_cast<int>(_survivalTime * 10.0f) + _bonusScore;
    }

    float GetSurvivalTime() const
    {
        return _survivalTime;
    }
};
