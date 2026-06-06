#pragma once

#define NOMINMAX
#include <windows.h>
#include <algorithm>
#include <string>

class ScoreManager
{
private:
    float _survivalTime = 0.0f;
    int _bonusScore = 0;
    int _bombBonusScore = 0;
    int _highScore = 0;
    bool _isFinalized = false;

public:
    static constexpr int BombBonusPerCount = 10000;

    void Initialize()
    {
        LoadHighScore();
    }

    void ResetScore()
    {
        _survivalTime = 0.0f;
        _bonusScore = 0;
        _bombBonusScore = 0;
        _isFinalized = false;
    }

    void UpdateScore(float deltaTime)
    {
        _survivalTime = (std::min)(_survivalTime + deltaTime, GetGameDuration());
    }

    void AddGrazeScore()
    {
        _bonusScore += 50;
    }

    void AddScore(int amount)
    {
        _bonusScore += amount;
    }

    void FinalizeScore(int remainingBombs, bool awardBombBonus)
    {
        if (_isFinalized) return;

        _bombBonusScore = awardBombBonus
            ? (std::max)(0, remainingBombs) * BombBonusPerCount
            : 0;
        _isFinalized = true;

        if (GetScore() > _highScore)
        {
            _highScore = GetScore();
            SaveHighScore();
        }
    }

    int GetScore() const
    {
        return static_cast<int>(_survivalTime * 10.0f) + _bonusScore + _bombBonusScore;
    }

    float GetSurvivalTime() const
    {
        return _survivalTime;
    }

    int GetBombBonusScore() const
    {
        return _bombBonusScore;
    }

    int GetHighScore() const
    {
        return _highScore;
    }

    bool IsTimeUp() const
    {
        return _survivalTime >= GetGameDuration();
    }

    static constexpr float GetGameDuration()
    {
        return 330.0f;
    }

private:
    static std::wstring GetHighScorePath()
    {
        wchar_t modulePath[MAX_PATH] = {};
        DWORD length = GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
        std::wstring path(modulePath, length);

        size_t separator = path.find_last_of(L"\\/");
        if (separator != std::wstring::npos)
            path.resize(separator + 1);
        else
            path.clear();

        return path + L"highscore.dat";
    }

    void LoadHighScore()
    {
        HANDLE file = CreateFileW(
            GetHighScorePath().c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        if (file == INVALID_HANDLE_VALUE)
            return;

        int storedScore = 0;
        DWORD bytesRead = 0;
        BOOL success = ReadFile(file, &storedScore, sizeof(storedScore), &bytesRead, nullptr);
        CloseHandle(file);

        if (success && bytesRead == sizeof(storedScore) && storedScore >= 0)
            _highScore = storedScore;
    }

    void SaveHighScore() const
    {
        HANDLE file = CreateFileW(
            GetHighScorePath().c_str(),
            GENERIC_WRITE,
            0,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        if (file == INVALID_HANDLE_VALUE)
            return;

        DWORD bytesWritten = 0;
        WriteFile(file, &_highScore, sizeof(_highScore), &bytesWritten, nullptr);
        CloseHandle(file);
    }
};
