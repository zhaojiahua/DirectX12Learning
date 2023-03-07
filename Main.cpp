#include "mainInclude.h"


int WINAPI WinMain(HINSTANCE thisInstance, HINSTANCE preInstance, LPSTR cmdLine, int cmdShow)
{
	//////////////////////////////////////////////////////////////////////////////////////////////////预备一些变量
	const UINT nFrameBackBufferCount = 3u;

	int iwidth = 1280; int iheight = 720;
	UINT nframeIndex = 0; UINT nframe = 0;

	UINT nDXGIFactoryFlags = 0u;
	UINT nRTVDescriptionSize = 0u;

	HWND hwnd = nullptr;
	MSG msg = {};

	float fAspectRatio = 3.0f;
	D3D12_VERTEX_BUFFER_VIEW setVertexBufferView = {};

	UINT64 n64FenceValue = 0ui64;
	HANDLE hFenceEvent = nullptr;

	CD3DX12_VIEWPORT setViewport(0.0f, 0.0f, static_cast<float>(iwidth), static_cast<float>(iheight));
	CD3DX12_RECT setScissorRect(0, 0, static_cast<LONG>(iwidth), static_cast<LONG>(iheight));

	ComPtr<IDXGIFactory5> pIDXGIFactory5;
	ComPtr<IDXGIAdapter1> pIAdapter;
	ComPtr<ID3D12Device4> pIDXGIDevice;
	ComPtr<ID3D12CommandQueue> pICommandQueue;
	ComPtr<IDXGISwapChain1> pISwapChain1;
	ComPtr<IDXGISwapChain3> pISwapChain3;
	ComPtr<ID3D12DescriptorHeap> pIRTVHeap;
	ComPtr<ID3D12Resource> pIARenderTargets[nFrameBackBufferCount];
	ComPtr<ID3D12CommandAllocator> pICommandAllocator;
	ComPtr<ID3D12RootSignature> pIRootSignature;
	ComPtr<ID3D12PipelineState> pIPipelineState;
	ComPtr<ID3D12GraphicsCommandList> pICommandList;
	ComPtr<ID3D12Resource> pIVertexBuffer;
	ComPtr<ID3D12Fence> pIFence;

	//////////////////////////////////////////////////////////////////////////////////////////////////注册和创建窗口
	WNDCLASSEX wcex;
	wcex.cbClsExtra = 0; wcex.cbWndExtra = 0;
	wcex.lpszClassName = GRS_WND_CLASS_NAME;
	wcex.hInstance = thisInstance;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);	//防止无聊的重绘
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hIcon = nullptr;
	wcex.hIconSm = NULL;
	wcex.lpszMenuName = nullptr;
	wcex.style = CS_GLOBALCLASS;	//去掉redraw类型
	wcex.lpfnWndProc = (WNDPROC)WindProc;

	ATOM registerAtom = RegisterClassEx(&wcex);
	if (!registerAtom) {
		MessageBox(NULL, L"register window failed", L"Error", MB_OK);
		return 1;
	}
	RECT rect = { 0,0,iwidth,iheight };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, NULL);

	hwnd = CreateWindowEx(
		NULL,
		GRS_WND_CLASS_NAME,
		GRS_WND_TITLE,
		WS_OVERLAPPED | WS_SYSMENU,
		100, 100,
		iwidth, iheight,
		nullptr,
		nullptr,
		preInstance,
		nullptr);

	if (!hwnd) {
		MessageBox(NULL, L"create window failed", L"Error", MB_OK);
		return 1;
	}
	
	while (msg.message != WM_QUIT)
	{
		//有信息就处理信息,处理完信息再渲染窗口
		if (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			UpdateWindow(hwnd);
			ShowWindow(hwnd, SW_SHOW);
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////创建DXGI
	CreateDXGIFactory2(nDXGIFactoryFlags, IID_PPV_ARGS(&pIDXGIFactory5));
	GRS_THROW_IF_FAILED(pIDXGIFactory5->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));

	//枚举选择一个合适的显示适配器(显卡)来创建D3D设备接口
	for (UINT adapterIndex = 1; DXGI_ERROR_NOT_FOUND != pIDXGIFactory5->EnumAdapters1(adapterIndex, &pIAdapter); ++adapterIndex) {
		DXGI_ADAPTER_DESC1 desc = {};
		pIAdapter->GetDesc1(&desc);
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pIAdapter.Get(), D3D_FEATURE_LEVEL_12_1, _uuidof(ID3D12Device), nullptr))) {
			//MessageBox(hwnd, L"Device create succesful", L"info", MB_OK);
			break;
		}
	}
	//创建D3D12.1的设备
	GRS_THROW_IF_FAILED(D3D12CreateDevice(pIAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&pIDXGIDevice)));
	//创建命令队列(一种3D命令引擎)
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;//这种类型的命令队列(引擎)可以执行几乎所有的命令,包括图形命令计算命令复制命令视频解码命令等,基本代表了整个GPU的执行力
	GRS_THROW_IF_FAILED(pIDXGIDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&pICommandQueue)));
	//创建交换链(创建交换链时就需要指定命令队列,交换链在最终呈现画面之前需要确认绘制操作是否完成完成了,需要用到这个命令队列flush一下)
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.BufferCount = nFrameBackBufferCount;
	swapchainDesc.Width = iwidth; swapchainDesc.Height = iheight;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.SampleDesc.Count = 1;//未开启多采样
	GRS_THROW_IF_FAILED(pIDXGIFactory5->CreateSwapChainForHwnd(pICommandQueue.Get(), hwnd, &swapchainDesc, nullptr, nullptr, &pISwapChain1));
	//获取后缓冲序号
	GRS_THROW_IF_FAILED(pISwapChain1.As(&pISwapChain3));
	nframeIndex = pISwapChain3->GetCurrentBackBufferIndex();
	//创建RTV描述符和堆
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = nFrameBackBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	GRS_THROW_IF_FAILED(pIDXGIDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&pIRTVHeap)));
	nRTVDescriptionSize = pIDXGIDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);//每个描述符堆的大小
	//创建堆
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvhandle(pIRTVHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < nFrameBackBufferCount; i++) {
		GRS_THROW_IF_FAILED(pISwapChain3->GetBuffer(i, IID_PPV_ARGS(&pIARenderTargets[i])));
		pIDXGIDevice->CreateRenderTargetView(pIARenderTargets[i].Get(), nullptr, rtvhandle);
		rtvhandle.Offset(1, nRTVDescriptionSize);
	}
	//创建根签名对象接口
	CD3DX12_ROOT_SIGNATURE_DESC rootsignatureDesc;
	rootsignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	GRS_THROW_IF_FAILED(D3D12SerializeRootSignature(&rootsignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
	GRS_THROW_IF_FAILED(pIDXGIDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&pIRootSignature)));
	//编译Shader
	ComPtr<ID3DBlob> vertexShader;
	ComPtr<ID3DBlob> pixelShader;
#if defined(__DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif
	TCHAR pszShaderFilename[] = _T("shaders/shader.hlsl");
	GRS_THROW_IF_FAILED(D3DCompileFromFile(pszShaderFilename, nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
	GRS_THROW_IF_FAILED(D3DCompileFromFile(pszShaderFilename, nullptr, nullptr, "PSMain", "vs_5_0", compileFlags, 0, &pixelShader, nullptr));
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"COLOR",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0}
	};
	//创建渲染管线pso
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs ,_countof(inputElementDescs) };
	psoDesc.pRootSignature = pIRootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	GRS_THROW_IF_FAILED(pIDXGIDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pIPipelineState)));
	//加载待渲染数据
	ZJH_Vertex triangleVertices[] = {//一个三角形数据
		{{0.0f,0.25f * fAspectRatio,0.0f},{1.0f,0.0f,0.0f,1.0f}},
		{{0.25f * fAspectRatio,-0.25f * fAspectRatio,0.0f},{0.0f,1.0f,0.0f,1.0f}},
		{{-0.25f * fAspectRatio,-0.25f * fAspectRatio,0.0f},{0.0f,0.0f,1.0f,1.0f}}
	};
	const UINT vertexBufferSize = sizeof(triangleVertices);
	GRS_THROW_IF_FAILED(pIDXGIDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&pIVertexBuffer)));
	UINT8* pvertexDataBegin = nullptr;
	CD3DX12_RANGE readRange(0, 0);
	GRS_THROW_IF_FAILED(pIVertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pvertexDataBegin)));
	memcpy(pvertexDataBegin, triangleVertices, vertexBufferSize);
	pIVertexBuffer->Unmap(0, nullptr);
	setVertexBufferView.BufferLocation = pIVertexBuffer->GetGPUVirtualAddress();
	setVertexBufferView.StrideInBytes = sizeof(ZJH_Vertex);
	setVertexBufferView.SizeInBytes = vertexBufferSize;

	return 0;

}