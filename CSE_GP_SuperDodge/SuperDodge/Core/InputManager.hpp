#pragma once

#define NOMINMAX
#include <windows.h>
#include <array>

class InputManager
{
private:
    static constexpr int KeyCount = 256;
    std::array<bool, KeyCount> _currentKeys = {};
    std::array<bool, KeyCount> _previousKeys = {};
    HWND _window = nullptr;
    bool _wasFocused = false;

public:
    void SetWindowHandle(HWND window)
    {
        _window = window;
        _currentKeys.fill(false);
        _previousKeys.fill(false);
        _wasFocused = false;
    }

    void Update()
    {
        const bool isFocused = _window != nullptr && GetForegroundWindow() == _window;
        if (!isFocused)
        {
            _currentKeys.fill(false);
            _previousKeys.fill(false);
            _wasFocused = false;
            return;
        }

        _previousKeys = _currentKeys;

        for (int key = 0; key < KeyCount; ++key)
            _currentKeys[key] = (GetAsyncKeyState(key) & 0x8000) != 0;

        if (!_wasFocused)
            _previousKeys = _currentKeys;

        _wasFocused = true;
    }

    bool IsKeyDown(int key) const
    {
        return IsValidKey(key) && _currentKeys[key];
    }

    bool IsKeyPressed(int key) const
    {
        return IsValidKey(key) && _currentKeys[key] && !_previousKeys[key];
    }

private:
    static bool IsValidKey(int key)
    {
        return key >= 0 && key < KeyCount;
    }
};
