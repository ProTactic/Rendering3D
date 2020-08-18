#include "Graphics.h"
#include <cassert>

Graphics::Graphics() : m_width(0), m_height(0), m_hwnd(NULL)
{
}

Graphics::~Graphics()
{
	CleanUp();
}

bool Graphics::Initialize(HWND hwnd, UINT windowWidth, UINT windowHeight)
{

    m_hwnd = hwnd;
    m_width = windowWidth;
    m_height = windowHeight;

    ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory)));

    SetDeviceAndHardwareAdapter();
    SetSwapChain();

    //create the render taget new and bind it to the om
    ID3D11Texture2D* backBuffer;
    ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));
    ThrowIfFailed(m_D3dDevice->CreateRenderTargetView(backBuffer, 0, &m_renderTargetsView));
    backBuffer->Release();

    SetDSV();

    m_ImmediateDeviceContext->OMSetRenderTargets(1, m_renderTargetsView.GetAddressOf(), m_depthStencilView.Get());

   D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.Width = m_width;
    viewport.Height = m_height;

    //Set the Viewport
    m_ImmediateDeviceContext->RSSetViewports(1, &viewport);

    InitShaders();
    InitSence();

	return true;
}

/*
    DXGIFactory need to be initialized
*/
void Graphics::SetDeviceAndHardwareAdapter()
{
    assert(m_dxgiFactory.Get());

    UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    UINT adapterIndex = 0;
    HRESULT hr;

    D3D_FEATURE_LEVEL featureLevel;
    ThrowIfFailed(D3D11CreateDevice(0,
        D3D_DRIVER_TYPE_HARDWARE,
        0,
        createDeviceFlags,
        nullptr, 0,
        D3D11_SDK_VERSION,
        &m_D3dDevice,
        &featureLevel,
        &m_ImmediateDeviceContext));

        if (featureLevel != D3D_FEATURE_LEVEL_11_0) {
            //Thorw an error there isnt any feated adapter
            throw HrException(UNSUPPORTED_GRAPHIC_ADAPTER);
        }
}

/*
    ID3D12CommandQueue must be initialized
*/
void Graphics::SetSwapChain()
{

    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    swapChainDesc.BufferDesc.Width = m_width;
    swapChainDesc.BufferDesc.Height = m_height;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.SampleDesc.Count = 1;

    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = m_frameBackBuffers;
    swapChainDesc.OutputWindow = m_hwnd;
    swapChainDesc.Windowed = true; //Currently not support fullscreen
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = 0;

    ThrowIfFailed(m_dxgiFactory->CreateSwapChain(m_D3dDevice.Get(), &swapChainDesc, &m_swapChain));
}

void Graphics::SetDSV()
{
    D3D11_TEXTURE2D_DESC depthStencilDesc;
    depthStencilDesc.Width = m_width;
    depthStencilDesc.Height = m_height;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    ThrowIfFailed(m_D3dDevice->CreateTexture2D(&depthStencilDesc, NULL, &m_DSVBuffer));
    ThrowIfFailed(m_D3dDevice->CreateDepthStencilView(m_DSVBuffer.Get(), NULL, &m_depthStencilView));
}

void Graphics::InitShaders()
{
    
    std::wstring shaderfolder = L"";
#pragma region DetermineShaderPath
    if (IsDebuggerPresent() == TRUE)
    {
#ifdef _DEBUG //Debug Mode
    #ifdef _WIN64 //x64
            shaderfolder = L"..\\x64\\Debug\\";
    #else  //x86 (Win32)
            shaderfolder = L"..\\Debug\\";
    #endif
    #else //Release Mode
    #ifdef _WIN64 //x64
            shaderfolder = L"..\\x64\\Release\\";
    #else  //x86 (Win32)
            shaderfolder = L"..\\Release\\";
    #endif
#endif
    }

    std::wstring fullPath = shaderfolder + L"VertexShader.cso";
    ThrowIfFailed(D3DReadFileToBlob(fullPath.c_str(), &m_vertexShaderByteCode));
    ThrowIfFailed(m_D3dDevice->CreateVertexShader(m_vertexShaderByteCode->GetBufferPointer(), m_vertexShaderByteCode->GetBufferSize(), NULL, &m_vertexShader));
    
    fullPath = shaderfolder + L"PixelShader.cso";
    ThrowIfFailed(D3DReadFileToBlob(fullPath.c_str(), &m_pixelShaderByteCode));
    ThrowIfFailed(m_D3dDevice->CreatePixelShader(m_pixelShaderByteCode->GetBufferPointer(), m_pixelShaderByteCode->GetBufferSize(), NULL, &m_pixelShader));

    m_D3dDevice->CreateInputLayout(layout, numElements, m_vertexShaderByteCode->GetBufferPointer(), m_vertexShaderByteCode->GetBufferSize(), &m_vertexInputLayout);
}

void Graphics::InitSence()
{

    Vertex v[] = {
        Vertex(-0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 1.0f),     //Bottom Left White Point
        Vertex(-0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f),     //Top Left White Point
        Vertex(0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f),       //Top Right Red Point
        Vertex(0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f),      //Bottom Right White Point
    };

    DWORD indices[] = {
        0, 1, 2,
        0, 2, 3,
    };

    D3D11_BUFFER_DESC vertexIndexBufferDesc;
    ZeroMemory(&vertexIndexBufferDesc, sizeof(D3D11_BUFFER_DESC));
    vertexIndexBufferDesc.ByteWidth = sizeof(DWORD) * ARRAYSIZE(indices);
    vertexIndexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexIndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    vertexIndexBufferDesc.CPUAccessFlags = 0;
    vertexIndexBufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA vertexIndexSubResourceDesc;
    ZeroMemory(&vertexIndexSubResourceDesc, sizeof(D3D11_SUBRESOURCE_DATA));
    vertexIndexSubResourceDesc.pSysMem = indices;

    ThrowIfFailed(m_D3dDevice->CreateBuffer(&vertexIndexBufferDesc, &vertexIndexSubResourceDesc, &m_vertexIndexBuffer));

    D3D11_BUFFER_DESC vertexBufferDesc;
    ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
    vertexBufferDesc.ByteWidth = sizeof(Vertex) * ARRAYSIZE(v);
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA vertexSubResourceDesc;
    ZeroMemory(&vertexSubResourceDesc, sizeof(D3D11_SUBRESOURCE_DATA));
    vertexSubResourceDesc.pSysMem = v;

    ThrowIfFailed(m_D3dDevice->CreateBuffer(&vertexBufferDesc, &vertexSubResourceDesc, &m_vertexBuffer));

    m_ImmediateDeviceContext->IASetInputLayout(m_vertexInputLayout.Get());
    m_ImmediateDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_ImmediateDeviceContext->VSSetShader(m_vertexShader.Get(), NULL, 0);
    m_ImmediateDeviceContext->PSSetShader(m_pixelShader.Get(), NULL, 0);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    m_ImmediateDeviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    m_ImmediateDeviceContext->IASetIndexBuffer(m_vertexIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

}

void Graphics::UpdateSence()
{
}

void Graphics::Render()
{

    UpdateSence();

    float bgColor[4] = { 0.5f, 0.5f, 0.5f, 0.5f };

    m_ImmediateDeviceContext->ClearRenderTargetView(m_renderTargetsView.Get(), bgColor);
    m_ImmediateDeviceContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    m_ImmediateDeviceContext->DrawIndexed(6, 0, 0);
    //Present the backbuffer to the screen
    m_swapChain->Present(0, 0);

}

void Graphics::CleanUp()
{
}
