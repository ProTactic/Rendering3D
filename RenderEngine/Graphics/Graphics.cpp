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

    m_ImmediateDeviceContext->OMSetRenderTargets(1, m_renderTargetsView.GetAddressOf(), nullptr);

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

void Graphics::UpdatePipeline()
{
    red += colormodr * 0.00005f;
    green += colormodg * 0.00002f;
    blue += colormodb * 0.00001f;
    
    if (red >= 1.0f || red <= 0.0f)
        colormodr *= -1;
    if (green >= 1.0f || green <= 0.0f)
        colormodg *= -1;
    if (blue >= 1.0f || blue <= 0.0f)
        colormodb *= -1;
}

void Graphics::Render()
{

    UpdatePipeline();

    //Clear our backbuffer to the updated color
    float bgColor[4] = { red, green, blue, 1.0f };

    m_ImmediateDeviceContext->ClearRenderTargetView(m_renderTargetsView.Get(), bgColor);

    //Present the backbuffer to the screen
    m_swapChain->Present(0, 0);

}

void Graphics::CleanUp()
{
}
