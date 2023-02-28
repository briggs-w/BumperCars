#include "AssimpPlusDemoApp.h"

#include <chrono>
#include <iostream>

namespace AssimpPlus
{
	AssimpPlusDemoApp::AssimpPlusDemoApp(HINSTANCE hInstance)
		: device_(nullptr)
		, context_(nullptr)
		, swapChain_(nullptr)
		, viewport_({ 0.f, 0.f, 800.f, 600.f, 0.f, 1.f })
		, renderTargetView_(nullptr)
		, depthStencilState_(nullptr)
		, rasterizerState_(nullptr)
		, depthStencilBuffer_(nullptr)
		, hInstance_(hInstance)
		, hWnd_(nullptr)
		, windowSize_({ 100, 100, 900, 700 })
		, appName_(L"Demo - Drawing with Materials Only")
	{
	}

	bool AssimpPlusDemoApp::InitializeWin32()
	{
		WNDCLASSEX wc = {};
		wc.cbSize = sizeof(wc);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = DefWindowProc; // WndProc;
		wc.lpszClassName = appName_;
		wc.hInstance = hInstance_;
		wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
		wc.hIconSm = wc.hIcon;

		RegisterClassEx(&wc);

		hWnd_ = CreateWindow
		(
			appName_, appName_,
			WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX ^ WS_MINIMIZEBOX,
			windowSize_.left, windowSize_.top,
			(windowSize_.right - windowSize_.left),
			(windowSize_.bottom - windowSize_.top),
			nullptr, nullptr, hInstance_,
			nullptr
		);

		if (!hWnd_)
		{
			std::cerr << "Failed to create window!" << std::endl;  
			std::cerr << GetLastError() << std::endl;
			return false;
		}

		ShowWindow(hWnd_, SW_SHOWDEFAULT);
		return true;
	}

	bool AssimpPlusDemoApp::InitializeD3D()
	{
		UINT creationFlags = 0x00;
#if defined(DEBUG) | defined(_DEBUG)
		// Set an extra flag used in creating the device that specifies to DirectX
		//  to use debug information. This is used in conjunction with the debug
		//  interface seen below. This includes reference counting for COM objects,
		//  and more advanced output regarding pipeline errors, bindings, etc.
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		HRESULT hr = {};
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
		hr = D3D11CreateDevice
		(
			nullptr, D3D_DRIVER_TYPE_HARDWARE,
			nullptr, creationFlags,
			&featureLevel, 1u, D3D11_SDK_VERSION,
			&device_, nullptr, &context_
		);

		if (FAILED(hr))
		{
			std::cerr << "Failed to create D3D11 device! Error code: " << hr << std::endl;
			return false;
		}

#if defined(DEBUG) | defined(_DEBUG)
		// Moar debug (see above comments)
		ComPtr<ID3D11Debug> debugInterface;
		hr = device_.Get()->QueryInterface(IID_PPV_ARGS(&debugInterface));
		if (FAILED(hr))
		{
			// Inability to use the debug interface is not a fatal error - we can continue
			//  without it (hopefully - this is disabled on release builds!)
			std::cout << "Unable to use debug interface, even on debug build. Sorry." << std::endl;
		}
#endif

		//
		// Swap chain time!
		//
		{
			// DXGI objects must be obtained through a factory. There is a static C-style
			//  method that can get a factory for us. A factory factory, if you will.
			// Java? Is that you? What are you doing here in C++?
			ComPtr<IDXGIFactory2> factory;
			hr = CreateDXGIFactory2(0x00, IID_PPV_ARGS(&factory));
			if (FAILED(hr))
			{
				std::cerr << "Failed to create DXGI factory, error code: " << hr;
				return false;
			}

			DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
			swapChainDesc.Width = (windowSize_.right - windowSize_.left);
			swapChainDesc.Height = (windowSize_.bottom - windowSize_.top);
			swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDesc.Stereo = false; // No stereoscopy here!
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.SampleDesc.Quality = 0; // Or anti-aliasing
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // We want to use this swap chain for rendering
			swapChainDesc.BufferCount = 2; // Two buffers - one for looking at, one for drawing to
			swapChainDesc.Scaling = DXGI_SCALING_STRETCH; // Squash or stretch image to fit the window
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
			swapChainDesc.Flags = 0x00;

			// This method is the reason I'm using IDXGISwapChain1 instead of IDXGISwapChain,
			//  and so forth - a swap chain can be created directly for a window, without having
			//  to query all the resolutions and whatnot. A minor convenience, but when you're
			//  writing ton of code for a tutorial that's not even about Win32 programming or
			//  even directly about DirectX programming... is nice.
			hr = factory->CreateSwapChainForHwnd
			(
				device_.Get(), hWnd_, &swapChainDesc,
				nullptr, nullptr, &swapChain_
			);
			if (FAILED(hr))
			{
				std::cerr << "Failed to create swap chain! Error code: " << hr;
				return false;
			}
		}

		//
		// Create the render target view
		//
		{
			ComPtr<ID3D11Texture2D> backBuffer;
			hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
			if (FAILED(hr))
			{
				std::cerr << "Failed to get back buffer pointer, error code: " << hr << std::endl;
				return false;
			}

			hr = device_->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetView_);
			if (FAILED(hr))
			{
				std::cerr << "Failed to create render target view for back buffer. Error: " << hr << std::endl;
				return false;
			}
		}

		//
		// Depth stencil state
		//
		{
			D3D11_DEPTH_STENCIL_DESC depthStencilDesc = { 0 };
			depthStencilDesc.DepthEnable = true;
			depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
			depthStencilDesc.StencilEnable = false;
			depthStencilDesc.StencilWriteMask = 0xFFu;
			depthStencilDesc.StencilReadMask = 0xFFu;

			depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
			depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

			depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
			depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

			hr = device_->CreateDepthStencilState(&depthStencilDesc, &depthStencilState_);
			if (FAILED(hr))
			{
				std::cerr << "Failed to create depth stencil state! " << hr << std::endl;
				return false;
			}

			context_->OMSetDepthStencilState(depthStencilState_.Get(), 1);
		}

		//
		// Depth stencil buffer
		//
		D3D11_TEXTURE2D_DESC depthBufferDesc = { 0 };
		depthBufferDesc.Width = (windowSize_.right - windowSize_.left);
		depthBufferDesc.Height = (windowSize_.bottom - windowSize_.top);
		depthBufferDesc.MipLevels = 1;
		depthBufferDesc.ArraySize = 1;
		depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthBufferDesc.SampleDesc.Count = 1;
		depthBufferDesc.SampleDesc.Quality = 0;
		depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthBufferDesc.CPUAccessFlags = 0x00;
		depthBufferDesc.MiscFlags = 0x00;

		hr = device_->CreateTexture2D(&depthBufferDesc, nullptr, &depthStencilBuffer_);
		if (FAILED(hr))
		{
			std::cerr << "Failed to create depth stencil buffer " << hr << std::endl;
			return false;
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		depthStencilViewDesc.Flags = 0x00;
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		hr = device_->CreateDepthStencilView(depthStencilBuffer_.Get(), &depthStencilViewDesc, &depthStencilView_);
		if (FAILED(hr))
		{
			std::cerr << "Failed to create depth stencil view: " << hr << std::endl;
			return false;
		}

		//
		// Create the rasterizer state
		//
		{
			D3D11_RASTERIZER_DESC rasterDesc = {};
			rasterDesc.AntialiasedLineEnable = false;
			rasterDesc.CullMode = D3D11_CULL_BACK;
			rasterDesc.FillMode = D3D11_FILL_SOLID;
			rasterDesc.DepthClipEnable = true;
			rasterDesc.FrontCounterClockwise = false;
			rasterDesc.MultisampleEnable = false;
			rasterDesc.ScissorEnable = false;

			hr = device_->CreateRasterizerState(&rasterDesc, &rasterizerState_);
			if (FAILED(hr))
			{
				std::cerr << "Failed to create rasterizer state! Error: " << hr << std::endl;
				return false;
			}

			context_->RSSetState(rasterizerState_.Get());
		}

		return true;
	}
};