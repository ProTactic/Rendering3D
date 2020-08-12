#include "Graphics.h"
#include <cassert>

Graphics::Graphics()
{
}

Graphics::~Graphics()
{
}

bool Graphics::Initialize()
{

    ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory)));

    SetDeviceAndHardwareAdapter();

    return true;
}

/*
    DXGIFactory need to be initialized
*/
void Graphics::SetDeviceAndHardwareAdapter()
{
    assert(m_dxgiFactory.Get());

    UINT adapterIndex = 0;
    Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
    HRESULT hr;

    while (m_dxgiFactory->EnumAdapters1(adapterIndex, &m_dxgiAdapter) != DXGI_ERROR_NOT_FOUND) {
        hr = D3D12CreateDevice(
            m_dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_0, 
            _uuidof(ID3D12Device), nullptr);

        if (SUCCEEDED(hr)) {
            //Just to make sure, although it succeeded
            ThrowIfFailed(D3D12CreateDevice(
                m_dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_0,
                _uuidof(ID3D12Device), &m_D3dDevice));
            return;
        }
    }
    
    //Thorw an error there isnt any feated adapter
    throw HrException(UNSUPPORTED_GRAPHIC_ADAPTER);
}
