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

	return 0;
}