#pragma once
#include <sdkddkver.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <tchar.h>
//添加wrl支持,方便使用Com
#include <wrl.h>
using namespace Microsoft;
using namespace Microsoft::WRL;
#include <dxgi1_6.h>
#include <DirectXMath.h>
using namespace DirectX;

//for d3d
#include <d3d12.h>
#include <d3d12shader.h>
#include <d3dcompiler.h>

//linker
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"d3dcompiler.lib")

#if  defined(_DEBUG)
#include <dxgidebug.h>
#endif //  defined(_DEBUG)

#include "DirectX12/d3dx12.h"//微软提供的DX工具

#define GRS_WND_CLASS_NAME _T("Game Window Class")
#define GRS_WND_TITLE			_T("DirectX12 Trigger Sample")

#define GRS_THROW_IF_FAILED(hr) if (FAILED(hr)){throw CGRSCOMException(hr);}

//定义自己的3D数据结构
struct ZJH_Vertex
{
	XMFLOAT3 mPosition;
	XMFLOAT4 mColor;
};

//窗口回调函数
LRESULT WindProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam) {
	switch (msg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	default:
		break;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}