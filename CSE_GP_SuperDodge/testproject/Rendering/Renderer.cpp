#include "Renderer.hpp"

#include "../Core/GameConstants.hpp"

#include <d3dcompiler.h>
#include <cstring>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

Renderer::~Renderer()
{
    Release();
}

bool Renderer::Initialize(HWND hwnd)
{
    _hwnd = hwnd;

    DXGI_SWAP_CHAIN_DESC swapDesc = {};
    swapDesc.BufferCount = 1;
    swapDesc.BufferDesc.Width = ScreenWidth;
    swapDesc.BufferDesc.Height = ScreenHeight;
    swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.OutputWindow = hwnd;
    swapDesc.SampleDesc.Count = 1;
    swapDesc.Windowed = TRUE;

    UINT flags = 0;

#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, nullptr, 0, D3D11_SDK_VERSION, &swapDesc, &_swapChain, &_device, &featureLevel, &_context);

#ifdef _DEBUG
    if (FAILED(hr) && (flags & D3D11_CREATE_DEVICE_DEBUG))
    {
        flags &= ~D3D11_CREATE_DEVICE_DEBUG;
        hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, nullptr, 0, D3D11_SDK_VERSION, &swapDesc, &_swapChain, &_device, &featureLevel, &_context);
    }
#endif

    if (FAILED(hr)) return false;

    ID3D11Texture2D* backBuffer = nullptr;
    hr = _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
    if (FAILED(hr)) return false;

    hr = _device->CreateRenderTargetView(backBuffer, nullptr, &_renderTargetView);
    backBuffer->Release();
    if (FAILED(hr)) return false;

    _context->OMSetRenderTargets(1, &_renderTargetView, nullptr);

    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(ScreenWidth);
    viewport.Height = static_cast<float>(ScreenHeight);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    _context->RSSetViewports(1, &viewport);

    return CreateShaders() && CreateVertexBuffer();
}

void Renderer::Clear(float r, float g, float b)
{
    float clearColor[4] = { r, g, b, 1.0f };
    _context->ClearRenderTargetView(_renderTargetView, clearColor);
}

void Renderer::Present()
{
    _swapChain->Present(1, 0);
}

void Renderer::DrawRect(const Vector2& center, const Vector2& size, const Color& color)
{
    float left = PixelToNdcX(center.x - size.x * 0.5f);
    float right = PixelToNdcX(center.x + size.x * 0.5f);
    float top = PixelToNdcY(center.y - size.y * 0.5f);
    float bottom = PixelToNdcY(center.y + size.y * 0.5f);

    DirectX::XMFLOAT4 c(color.r, color.g, color.b, color.a);

    Vertex vertices[6] =
    {
        {{ left, top, 0.0f }, c},
        {{ right, bottom, 0.0f }, c},
        {{ left, bottom, 0.0f }, c},

        {{ left, top, 0.0f }, c},
        {{ right, top, 0.0f }, c},
        {{ right, bottom, 0.0f }, c},
    };

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    _context->Map(_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, vertices, sizeof(vertices));
    _context->Unmap(_vertexBuffer, 0);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    _context->IASetInputLayout(_inputLayout);
    _context->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
    _context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _context->VSSetShader(_vertexShader, nullptr, 0);
    _context->PSSetShader(_pixelShader, nullptr, 0);
    _context->Draw(6, 0);
}

bool Renderer::CreateShaders()
{
    const char* shaderCode = R"(
        struct VS_INPUT
        {
            float3 position : POSITION;
            float4 color : COLOR;
        };

        struct PS_INPUT
        {
            float4 position : SV_POSITION;
            float4 color : COLOR;
        };

        PS_INPUT VSMain(VS_INPUT input)
        {
            PS_INPUT output;
            output.position = float4(input.position, 1.0f);
            output.color = input.color;
            return output;
        }

        float4 PSMain(PS_INPUT input) : SV_TARGET
        {
            return input.color;
        }
    )";

    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    HRESULT hr = D3DCompile(shaderCode, strlen(shaderCode), nullptr, nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vsBlob, &errorBlob);
    if (FAILED(hr))
    {
        if (errorBlob) errorBlob->Release();
        return false;
    }

    hr = D3DCompile(shaderCode, strlen(shaderCode), nullptr, nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &psBlob, &errorBlob);
    if (FAILED(hr))
    {
        if (vsBlob) vsBlob->Release();
        if (errorBlob) errorBlob->Release();
        return false;
    }

    hr = _device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &_vertexShader);
    if (FAILED(hr))
    {
        vsBlob->Release();
        psBlob->Release();
        return false;
    }

    hr = _device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &_pixelShader);
    if (FAILED(hr))
    {
        vsBlob->Release();
        psBlob->Release();
        return false;
    }

    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = _device->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &_inputLayout);

    vsBlob->Release();
    psBlob->Release();

    return SUCCEEDED(hr);
}

bool Renderer::CreateVertexBuffer()
{
    D3D11_BUFFER_DESC desc = {};
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth = sizeof(Vertex) * 6;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    return SUCCEEDED(_device->CreateBuffer(&desc, nullptr, &_vertexBuffer));
}

float Renderer::PixelToNdcX(float x) const
{
    return x / ScreenWidth * 2.0f - 1.0f;
}

float Renderer::PixelToNdcY(float y) const
{
    return 1.0f - y / ScreenHeight * 2.0f;
}

void Renderer::Release()
{
    if (_vertexBuffer) _vertexBuffer->Release();
    if (_inputLayout) _inputLayout->Release();
    if (_pixelShader) _pixelShader->Release();
    if (_vertexShader) _vertexShader->Release();
    if (_renderTargetView) _renderTargetView->Release();
    if (_swapChain) _swapChain->Release();
    if (_context) _context->Release();
    if (_device) _device->Release();

    _vertexBuffer = nullptr;
    _inputLayout = nullptr;
    _pixelShader = nullptr;
    _vertexShader = nullptr;
    _renderTargetView = nullptr;
    _swapChain = nullptr;
    _context = nullptr;
    _device = nullptr;
}