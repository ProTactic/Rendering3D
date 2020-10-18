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

    D3D11_BUFFER_DESC bufferDesc;

    Vertex v[] = {
        Vertex(-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f),
        Vertex(-1.0f, +1.0f, -1.0f, 1.0f, 0.0f, 1.0f),
        Vertex(+1.0f, +1.0f, -1.0f, 0.0f, 1.0f, 1.0f),
        Vertex(+1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f),
        Vertex(-1.0f, -1.0f, +1.0f, 1.0f, 1.0f, 1.0f),
        Vertex(-1.0f, +1.0f, +1.0f, 1.0f, 1.0f, 1.0f),
        Vertex(+1.0f, +1.0f, +1.0f, 0.0f, 1.0f, 1.0f),
        Vertex(+1.0f, -1.0f, +1.0f, 0.0f, 0.0f, 1.0f),
    };


    DWORD indices[] = {
        // front face
        0, 1, 2,
        0, 2, 3,

        // back face
        4, 6, 5,
        4, 7, 6,

        // left face
        4, 5, 1,
        4, 1, 0,

        // right face
        3, 2, 6,
        3, 6, 7,

        // top face
        1, 5, 6,
        1, 6, 2,

        // bottom face
        4, 0, 3,
        4, 3, 7
    };

    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
    bufferDesc.ByteWidth = sizeof(DWORD) * ARRAYSIZE(indices);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vertexIndexSubResourceDesc;
    ZeroMemory(&vertexIndexSubResourceDesc, sizeof(D3D11_SUBRESOURCE_DATA));
    vertexIndexSubResourceDesc.pSysMem = indices;
    ThrowIfFailed(m_D3dDevice->CreateBuffer(&bufferDesc, &vertexIndexSubResourceDesc, &m_vertexIndexBuffer));

    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
    bufferDesc.ByteWidth = sizeof(Vertex) * ARRAYSIZE(v);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vertexSubResourceDesc;
    ZeroMemory(&vertexSubResourceDesc, sizeof(D3D11_SUBRESOURCE_DATA));
    vertexSubResourceDesc.pSysMem = v;
    ThrowIfFailed(m_D3dDevice->CreateBuffer(&bufferDesc, &vertexSubResourceDesc, &m_vertexBuffer));

    m_ImmediateDeviceContext->IASetInputLayout(m_vertexInputLayout.Get());
    m_ImmediateDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_ImmediateDeviceContext->VSSetShader(m_vertexShader.Get(), NULL, 0);
    m_ImmediateDeviceContext->PSSetShader(m_pixelShader.Get(), NULL, 0);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    m_ImmediateDeviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    m_ImmediateDeviceContext->IASetIndexBuffer(m_vertexIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    //create the constat buffer
    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
    bufferDesc.ByteWidth = sizeof(ConstantBufferMatrix);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    ThrowIfFailed(m_D3dDevice->CreateBuffer(&bufferDesc, NULL, &m_constantBuffer4x4Matrix));

    camPosition = dx::XMVectorSet(0.0f, 3.0f, -8.0f, 0.0f);
    camTarget = dx::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    camUp = dx::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    camView = dx::XMMatrixLookAtLH(camPosition, camTarget, camUp);
    camProjection = dx::XMMatrixPerspectiveFovLH(0.4f * 3.14f, (float)m_width / m_height, 1.0f, 1000.0f);

}

void Graphics::UpdateSence()
{
    static float rot = 0.01f;
    rot += .01f;
    if (rot > 6.28f)
        rot = 0.0f;

    World = dx::XMMatrixIdentity();

    //Define cube1's world space matrix
    dx::XMVECTOR rotaxis = dx::XMVectorSet(1.0f, 1.0f, 0.0f, 0.0f);
    Rotation = dx::XMMatrixRotationAxis(rotaxis, rot);
    Translation = dx::XMMatrixTranslation(0.0f, 0.0f, 4.0f);

    //Set cube1's world space using the transformations
    World = Translation * Rotation;

}

void Graphics::Render()
{

    UpdateSence();

    float bgColor[4] = { 0.5f, 0.5f, 0.5f, 0.5f };
    ConstantBufferMatrix cbm = {};

    m_ImmediateDeviceContext->ClearRenderTargetView(m_renderTargetsView.Get(), bgColor);
    m_ImmediateDeviceContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


    WVP = World * camView * camProjection;

    cbm.matrix = XMMatrixTranspose(WVP);
    m_ImmediateDeviceContext->UpdateSubresource(m_constantBuffer4x4Matrix.Get(), 0, NULL, &cbm, 0, 0);
    m_ImmediateDeviceContext->VSSetConstantBuffers(0, 1, m_constantBuffer4x4Matrix.GetAddressOf());

    m_ImmediateDeviceContext->DrawIndexed(36, 0, 0);
    //Present the backbuffer to the screen
    m_swapChain->Present(0, 0);

}

void Graphics::CleanUp()
{
}
