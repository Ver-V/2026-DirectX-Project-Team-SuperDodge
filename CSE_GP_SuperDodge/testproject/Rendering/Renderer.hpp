#pragma once

#define NOMINMAX
#include <windows.h>
#include <d3d11.h>
#include <d2d1.h>
#include <dwrite.h>
#include <algorithm>
#include <string>

#include "../Core/MathTypes.hpp"

class Renderer
{
private:
    HWND _hwnd = nullptr;
    ID3D11Device* _device = nullptr;
    ID3D11DeviceContext* _context = nullptr;
    IDXGISwapChain* _swapChain = nullptr;
    ID3D11RenderTargetView* _renderTargetView = nullptr;
    ID3D11VertexShader* _vertexShader = nullptr;
    ID3D11PixelShader* _pixelShader = nullptr;
    ID3D11InputLayout* _inputLayout = nullptr;
    ID3D11Buffer* _vertexBuffer = nullptr;

    // Direct2D / DirectWrite
    ID2D1Factory* _d2dFactory = nullptr;
    ID2D1RenderTarget* _d2dRenderTarget = nullptr;
    IDWriteFactory* _dWriteFactory = nullptr;
    IDWriteTextFormat* _textFormat = nullptr;
    ID2D1SolidColorBrush* _whiteBrush = nullptr;

    float _flashAmount = 0.0f;

public:
    ~Renderer();

    bool Initialize(HWND hwnd);
    void Clear(float r, float g, float b);
    void Present();
    void DrawRect(const Vector2& center, const Vector2& size, const Color& color);
    
    void BeginText();
    void DrawString(const std::wstring& text, const Vector2& position, float fontSize, const Color& color);
    void EndText();

    void SetFlash(float amount) { _flashAmount = amount; }
    void UpdateFlash(float deltaTime) { _flashAmount = (std::max)(0.0f, _flashAmount - deltaTime * 2.0f); }
    float GetFlash() const { return _flashAmount; }

private:
    bool CreateShaders();
    bool CreateVertexBuffer();
    bool InitDirectWrite();
    float PixelToNdcX(float x) const;
    float PixelToNdcY(float y) const;
    void Release();
};