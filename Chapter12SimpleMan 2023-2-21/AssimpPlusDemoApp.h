#pragma once

// Base class for a demo application. Since this is a tutorial series on animation specifically,
//  I won't be doing much consideration around color buffer formats, sizes, anti-aliasing, etc.

#define WIN32_LEAN_AND_MEAN
#include <d3d11.h>
#include <dxgi1_4.h>
#include <Windows.h>
#include <wrl.h>
#include <chrono>
#include "FreeCamera.h"
#include "SimpleManActor.h"
#include "RoadActor.h"
#include "DebugIcosphere.h"
#include "Math.h"

using Microsoft::WRL::ComPtr;

namespace AssimpPlus
{
	class AssimpPlusDemoApp
	{
	public:
		AssimpPlusDemoApp(HINSTANCE hInstance);

		bool InitializeWin32();
		bool InitializeD3D();
	
		ComPtr<ID3D11Device> device_;
		ComPtr<ID3D11DeviceContext> context_;
		ComPtr<IDXGISwapChain1> swapChain_;
		D3D11_VIEWPORT viewport_;
		ComPtr<ID3D11RenderTargetView> renderTargetView_;
		ComPtr<ID3D11DepthStencilState> depthStencilState_;
		ComPtr<ID3D11RasterizerState> rasterizerState_;
		ComPtr<ID3D11Texture2D> depthStencilBuffer_;
		ComPtr<ID3D11DepthStencilView> depthStencilView_;

		// Win32 Stuff
		HINSTANCE hInstance_; // Handle to the application
		HWND hWnd_; // Handle to the window
		RECT windowSize_; // Size of the window
		LPCWSTR appName_; // Name of the application
	};
};