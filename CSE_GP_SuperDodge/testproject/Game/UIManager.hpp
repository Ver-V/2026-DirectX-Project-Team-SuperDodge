#pragma once

#define NOMINMAX
#include <windows.h>
#include <sstream>

class UIManager
{
private:
    HWND _hwnd = nullptr;

public:
    void Initialize(HWND hwnd)
    {
        _hwnd = hwnd;
    }

    void ShowStartUI()
    {
        SetWindowTextW(_hwnd, L"Dodge DX11 - Space: Start | WASD/Arrow: Move | ESC: Quit");
    }

    void UpdateScoreText(int score, float survivalTime)
    {
        std::wstringstream ss;
        ss << L"Dodge DX11 - Score: " << score << L" | Time: " << static_cast<int>(survivalTime) << L"s | ESC: Quit";
        SetWindowTextW(_hwnd, ss.str().c_str());
    }

    void ShowGameOverUI(int score)
    {
        std::wstringstream ss;
        ss << L"Dodge DX11 - Game Over | Score: " << score << L" | Space/R: Restart | ESC: Quit";
        SetWindowTextW(_hwnd, ss.str().c_str());
    }
};