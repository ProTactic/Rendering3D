#pragma once

//Windows
#include <dxgi.h>
#include <d3d12.h>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")

#include<wrl/client.h>

#include "DXUtils.h"

class Graphics
{

private:

	Microsoft::WRL::ComPtr<IDXGIFactory1> m_dxgiFactory;
	Microsoft::WRL::ComPtr<IDXGIAdapter1> m_dxgiAdapter;

	Microsoft::WRL::ComPtr<ID3D12Device> m_D3dDevice;

	void SetDeviceAndHardwareAdapter();

public:
	Graphics();
	~Graphics();

	bool Initialize();
};

