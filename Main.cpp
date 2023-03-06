#include "mainInclude.h"


int WINAPI WinMain(HINSTANCE thisInstance, HINSTANCE preInstance, LPSTR cmdLine, int cmdShow)
{
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

	//CD3DX12_VIEWPORT setViewport(0.0f, 0.0f, static_cast<float>(iwidth), static_cast<float>(iheight));
	CD3DX12_RECT setScissorRect(0, 0, static_cast<LONG>(iwidth), static_cast<LONG>(iheight));

	ComPtr<IDXGIFactory5> pIDXGIFactory5;
	ComPtr<IDXGIAdapter1> pIDXGIAdapter;
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

	//注册和创建窗口
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
		nullptr
	);

	if (!hwnd) {
		MessageBox(NULL, L"create window failed", L"Error", MB_OK);
		return 1;
	}

	while (true)
	{
		ShowWindow(hwnd, SW_SHOW);
		UpdateWindow(hwnd);
	}

	return 0;
}