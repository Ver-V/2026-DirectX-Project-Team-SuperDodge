#pragma once

#define NOMINMAX
#include <windows.h>
#include <d3d11.h>

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

public:
    ~Renderer();

    bool Initialize(HWND hwnd);
    void Clear(float r, float g, float b);
    void Present();
    void DrawRect(const Vector2& center, const Vector2& size, const Color& color);

private:
    bool CreateShaders();
    bool CreateVertexBuffer();
    float PixelToNdcX(float x) const;
    float PixelToNdcY(float y) const;
    void Release();
};