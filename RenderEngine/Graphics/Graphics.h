#pragma once

//Windows
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")


#include <dxgi.h>
#include <d3d11.h>

#include<wrl/client.h>

#include "DXUtils.h"

class Graphics
{

private:

	static const UINT m_frameBackBuffers = 1;

	Microsoft::WRL::ComPtr<IDXGIFactory> m_dxgiFactory;

	Microsoft::WRL::ComPtr<ID3D11Device> m_D3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_ImmediateDeviceContext;
	
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetsView;


	void SetDeviceAndHardwareAdapter();
	void SetSwapChain();

	void UpdatePipeline();
	void CleanUp();

protected:
	UINT m_width;
	UINT m_height;
	HWND m_hwnd;

	//tmp
	float red = 0.0f;
	float green = 0.0f;
	float blue = 0.0f;
	int colormodr = 1;
	int colormodg = 1;
	int colormodb = 1;

public:
	Graphics();
	~Graphics();

	bool Initialize(HWND hwnd, UINT windowWidth, UINT windowHeight);
	void Render();
};

