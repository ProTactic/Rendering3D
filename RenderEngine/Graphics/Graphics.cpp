#include "Graphics.h"
#include <cassert>

Graphics::Graphics() : m_width(0), m_height(0)
{
}

Graphics::~Graphics()
{
	WaitFenceEvent();
	CloseHandle(m_fenceEvent);
	CleanUp();
}

bool Graphics::Initialize(HWND hwnd, UINT windowWidth, UINT windowHeight)
{

#if defined(DEBUG) || defined(_DEBUG) 
    // Enable the D3D12 debug layer.
    {
        Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
        ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
        debugController->EnableDebugLayer();
    }
#endif

    m_hwnd = hwnd;
    m_width = windowWidth;
    m_height = windowHeight;

    ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory)));

    SetDeviceAndHardwareAdapter();
    SetCommandsD3D12();
    SetSwapChain();

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heapDesc.NumDescriptors = m_frameBuffers;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    heapDesc.NodeMask = 0;

    ThrowIfFailed(m_D3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_RTVdescriptorHeap)));
    rtvDescriptorSize = m_D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVdescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    for (int i = 0; i < m_frameBuffers; i++)
    {
        ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])));
        m_D3dDevice->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, rtvDescriptorSize);
    }

    //      Create the fence     //
    for (int i = 0; i < m_frameBuffers; i++)
    {
        ThrowIfFailed(m_D3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fencesArr[i])));
        m_fenceValues[i] = 0;
    }

    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_fenceEvent == NULL) {
        return false;
    }

	/*HRESULT hr;

	// -- Create the Device -- //

	IDXGIFactory4* dxgiFactory;
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
	if (FAILED(hr))
	{
		return false;
	}

	IDXGIAdapter1* adapter; // adapters are the graphics card (this includes the embedded graphics on the motherboard)

	int adapterIndex = 0; // we'll start looking for directx 12  compatible graphics devices starting at index 0

	bool adapterFound = false; // set this to true when a good one was found

							   // find first hardware gpu that supports d3d 12
	while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// we dont want a software device
			continue;
		}

		// we want a device that is compatible with direct3d 12 (feature level 11 or higher)
		hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(hr))
		{
			adapterFound = true;
			break;
		}

		adapterIndex++;
	}

	if (!adapterFound)
	{
		return false;
	}

	// Create the device
	hr = D3D12CreateDevice(
		adapter,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&m_D3dDevice)
	);
	if (FAILED(hr))
	{
		return false;
	}

	// -- Create a direct command queue -- //

	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // direct means the gpu can directly execute this command queue

	hr = m_D3dDevice->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&m_commandQueue)); // create the command queue
	if (FAILED(hr))
	{
		return false;
	}

	// -- Create the Swap Chain (double/tripple buffering) -- //

	DXGI_MODE_DESC backBufferDesc = {}; // this is to describe our display mode
	backBufferDesc.Width = m_width; // buffer width
	backBufferDesc.Height = m_height; // buffer height
	backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the buffer (rgba 32 bits, 8 bits for each chanel)

														// describe our multi-sampling. We are not multi-sampling, so we set the count to 1 (we need at least one sample of course)
	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1; // multisample count (no multisampling, so we just put 1, since we still need 1 sample)

						  // Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = m_frameBuffers; // number of buffers we have
	swapChainDesc.BufferDesc = backBufferDesc; // our back buffer description
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // this says the pipeline will render to this swap chain
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // dxgi will discard the buffer (data) after we call present
	swapChainDesc.OutputWindow = hwnd; // handle to our window
	swapChainDesc.SampleDesc = sampleDesc; // our multi-sampling description
	swapChainDesc.Windowed = true; // set to true, then if in fullscreen must call SetFullScreenState with true for full screen to get uncapped fps

	IDXGISwapChain* tempSwapChain;

	dxgiFactory->CreateSwapChain(
		m_commandQueue.Get(), // the queue will be flushed once the swap chain is created
		&swapChainDesc, // give it the swap chain description we created above
		&tempSwapChain // store the created swap chain in a temp IDXGISwapChain interface
	);

	m_swapChain = static_cast<IDXGISwapChain3*>(tempSwapChain);

	m_frameBufferCurrentIndex = m_swapChain->GetCurrentBackBufferIndex();

	// -- Create the Back Buffers (render target views) Descriptor Heap -- //

	// describe an rtv descriptor heap and create
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = m_frameBuffers; // number of descriptors for this heap.
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // this heap is a render target view heap

													   // This heap will not be directly referenced by the shaders (not shader visible), as this will store the output from the pipeline
													   // otherwise we would set the heap's flag to D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = m_D3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RTVdescriptorHeap));
	if (FAILED(hr))
	{
		return false;
	}

	// get the size of a descriptor in this heap (this is a rtv heap, so only rtv descriptors should be stored in it.
	// descriptor sizes may vary from device to device, which is why there is no set size and we must ask the 
	// device to give us the size. we will use this size to increment a descriptor handle offset
	rtvDescriptorSize = m_D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// get a handle to the first descriptor in the descriptor heap. a handle is basically a pointer,
	// but we cannot literally use it like a c++ pointer.
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVdescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// Create a RTV for each buffer (double buffering is two buffers, tripple buffering is 3).
	for (int i = 0; i < m_frameBuffers; i++)
	{
		// first we get the n'th buffer in the swap chain and store it in the n'th
		// position of our ID3D12Resource array
		hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]));
		if (FAILED(hr))
		{
			return false;
		}

		// the we "create" a render target view which binds the swap chain buffer (ID3D12Resource[n]) to the rtv handle
		m_D3dDevice->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);

		// we increment the rtv handle by the rtv descriptor size we got above
		rtvHandle.Offset(1, rtvDescriptorSize);
	}

	// -- Create the Command Allocators -- //

	for (int i = 0; i < m_frameBuffers; i++)
	{
		hr = m_D3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocatorArr[i]));
		if (FAILED(hr))
		{
			return false;
		}
	}

	// -- Create a Command List -- //

	// create the command list with the first allocator
	hr = m_D3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocatorArr[0].Get(), NULL, IID_PPV_ARGS(&m_commandList));
	if (FAILED(hr))
	{
		return false;
	}

	// command lists are created in the recording state. our main loop will set it up for recording again so close it now
	m_commandList->Close();

	// -- Create a Fence & Fence Event -- //

	// create the fences
	for (int i = 0; i < m_frameBuffers; i++)
	{
		hr = m_D3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fencesArr[i]));
		if (FAILED(hr))
		{
			return false;
		}
		m_fenceValues[i] = 0; // set the initial fence value to 0
	}

	// create a handle to a fence event
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_fenceEvent == nullptr)
	{
		return false;
	}*/

	return true;
}

/*
    DXGIFactory need to be initialized
*/
void Graphics::SetDeviceAndHardwareAdapter()
{
    assert(m_dxgiFactory.Get());

    UINT adapterIndex = 0;
    HRESULT hr;

    while (m_dxgiFactory->EnumAdapters1(adapterIndex, &m_dxgiAdapter) != DXGI_ERROR_NOT_FOUND) {

        //Check if it is not software device
        DXGI_ADAPTER_DESC1 desc;
        m_dxgiAdapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
            continue;
        }

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

/*
    ID3D12Device need to be initialized
*/
void Graphics::SetCommandsD3D12()
{

    assert(m_D3dDevice.Get());

    D3D12_COMMAND_LIST_TYPE commandListType = D3D12_COMMAND_LIST_TYPE_DIRECT;

    D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
    commandQueueDesc.Type = commandListType;
    commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    commandQueueDesc.NodeMask = 0;
    ThrowIfFailed(m_D3dDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&m_commandQueue)));

    for (UINT i = 0; i < m_frameBuffers; i++)
    {
        ThrowIfFailed(m_D3dDevice->CreateCommandAllocator(commandListType, IID_PPV_ARGS(&m_commandAllocatorArr[i])));
    }

    ThrowIfFailed(m_D3dDevice->CreateCommandList(0, commandListType, m_commandAllocatorArr[0].Get(), NULL, IID_PPV_ARGS(&m_commandList)));
    m_commandList->Close();
}


/*
    ID3D12CommandQueue must be initialized
*/
void Graphics::SetSwapChain()
{

    assert(m_commandQueue.Get());

    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    swapChainDesc.BufferDesc.Width = m_width;
    swapChainDesc.BufferDesc.Height = m_height;
    swapChainDesc.BufferDesc.RefreshRate = {};
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    swapChainDesc.SampleDesc = {};
    swapChainDesc.SampleDesc.Count = 1;

    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = m_frameBuffers;
    swapChainDesc.OutputWindow = m_hwnd;
    swapChainDesc.Windowed = true; //Currently not support fullscreen
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.Flags = 0;

    Microsoft::WRL::ComPtr<IDXGISwapChain> tmpSwapChain;
    ThrowIfFailed(m_dxgiFactory->CreateSwapChain(m_commandQueue.Get(), &swapChainDesc, &tmpSwapChain));
    m_swapChain = static_cast<IDXGISwapChain3*>(tmpSwapChain.Get());
    m_frameBufferCurrentIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void Graphics::UpdatePipeline()
{

    WaitFenceEvent();

    ThrowIfFailed(m_commandAllocatorArr[m_frameBufferCurrentIndex]->Reset());
    ThrowIfFailed(m_commandList->Reset(m_commandAllocatorArr[m_frameBufferCurrentIndex].Get(), NULL));

    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameBufferCurrentIndex].Get(),
                                        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVdescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_frameBufferCurrentIndex, rtvDescriptorSize);

    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameBufferCurrentIndex].Get(),
                                        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(m_commandList->Close());

}

void Graphics::WaitFenceEvent()
{

    if (m_fencesArr[m_frameBufferCurrentIndex]->GetCompletedValue() < m_fenceValues[m_frameBufferCurrentIndex])
    {
        ThrowIfFailed(m_fencesArr[m_frameBufferCurrentIndex]->SetEventOnCompletion(m_fenceValues[m_frameBufferCurrentIndex], m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }

    m_fenceValues[m_frameBufferCurrentIndex]++;
    m_frameBufferCurrentIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void Graphics::Render()
{

    UpdatePipeline();

    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };

    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    ThrowIfFailed(m_commandQueue->Signal(m_fencesArr[m_frameBufferCurrentIndex].Get(), m_fenceValues[m_frameBufferCurrentIndex]));
    ThrowIfFailed(m_swapChain->Present(0, 0));

}

void Graphics::CleanUp()
{
    for (int i = 0; i < m_frameBuffers; ++i)
    {
        m_frameBufferCurrentIndex = i;
        WaitFenceEvent();
    }

}
