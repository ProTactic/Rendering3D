#pragma once

//Windows
#include <dxgi1_4.h>
#include <d3d12.h>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#include "d3dx12.h"

#include<wrl/client.h>

#include "DXUtils.h"

class Graphics
{

private:

	static const UINT m_frameBuffers = 3;

	Microsoft::WRL::ComPtr<IDXGIFactory4> m_dxgiFactory;
	Microsoft::WRL::ComPtr<IDXGIAdapter1> m_dxgiAdapter;

	Microsoft::WRL::ComPtr<ID3D12Device> m_D3dDevice;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocatorArr[m_frameBuffers];
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
	
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RTVdescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[m_frameBuffers];

	Microsoft::WRL::ComPtr<ID3D12Fence> m_fencesArr[m_frameBuffers];
	UINT m_fenceValues[m_frameBuffers];
	HANDLE m_fenceEvent;


	void SetDeviceAndHardwareAdapter();
	void SetCommandsD3D12();
	void SetSwapChain();

	void UpdatePipeline();
	void WaitFenceEvent();
	void CleanUp();

protected:
	UINT m_width;
	UINT m_height;
	HWND m_hwnd;

	UINT m_frameBufferCurrentIndex;

	UINT rtvDescriptorSize;

public:
	Graphics();
	~Graphics();

	bool Initialize(HWND hwnd, UINT windowWidth, UINT windowHeight);
	void Render();
};

