#pragma once

//Windows
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")



#include <dxgi.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>

#include<wrl/client.h>

#include "DXUtils.h"

namespace dx = DirectX;

class Graphics
{

private:

	static const UINT m_frameBackBuffers = 1;

	Microsoft::WRL::ComPtr<IDXGIFactory> m_dxgiFactory;

	Microsoft::WRL::ComPtr<ID3D11Device> m_D3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_ImmediateDeviceContext;
	
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetsView;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> m_vertexShaderByteCode;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
	Microsoft::WRL::ComPtr<ID3DBlob> m_pixelShaderByteCode;


	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_vertexInputLayout;



	void SetDeviceAndHardwareAdapter();
	void SetSwapChain();

	void InitShaders();
	void InitSence();
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

	struct Vertex	//Overloaded Vertex Structure
	{
		Vertex(float x, float y, float r, float g, float b)
			: pos(x, y), color(r, g, b) {}

		dx::XMFLOAT2 pos;
		dx::XMFLOAT3 color;
	};

	D3D11_INPUT_ELEMENT_DESC layout[2] =
	{
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"COLOR", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
	};

	UINT numElements = ARRAYSIZE(layout);
};