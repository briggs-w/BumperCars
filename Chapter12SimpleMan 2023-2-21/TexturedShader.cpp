#include "TexturedShader.h"

#include <iostream>
#include <fstream>

namespace AssimpPlus
{
	AssimpPlus::TexturedShader::Texture::Texture(ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context, const std::vector<unsigned char>& rawData, std::uint32_t width, std::uint32_t height)
		: Buffer(nullptr)
		, SRV(nullptr)
	{
		D3D11_TEXTURE2D_DESC dscTexture = {};
		dscTexture.Height = height;
		dscTexture.Width = width;
		dscTexture.MipLevels = 1;
		dscTexture.ArraySize = 1;
		dscTexture.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		dscTexture.SampleDesc.Count = 1;
		dscTexture.SampleDesc.Quality = 0;
		dscTexture.Usage = D3D11_USAGE_DEFAULT;
		dscTexture.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET; // TODO SESS: Need render target?
		dscTexture.CPUAccessFlags = 0x00;
		dscTexture.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		HRESULT hr = {};

		D3D11_SUBRESOURCE_DATA initialData = {};
		initialData.pSysMem = &rawData[0];
		initialData.SysMemPitch = width * 4u * sizeof(unsigned char); // unsigned char is always 1 byte, I just like to illustrate better
		//initialData.SysMemSlicePitch = width * height * 4u * sizeof(unsigned char);

		hr = device->CreateTexture2D(&dscTexture, &initialData, &Buffer);
		if (FAILED(hr))
		{
			std::cerr << "Failed to create Texture2D - object will not properly initialize!" << std::endl;
			return;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC dscSRV = {};
		dscSRV.Format = dscTexture.Format;
		dscSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		dscSRV.Texture2D.MostDetailedMip = 0;
		dscSRV.Texture2D.MipLevels = -1;
		hr = device->CreateShaderResourceView(Buffer.Get(), &dscSRV, &SRV);
		if (FAILED(hr))
		{
			std::cerr << "Failed to create SRV for texture - object will not properly initialize!" << std::endl;
			return;
		}

		context->GenerateMips(SRV.Get());
	}

	//added to remove the device pass, think it will still work, just without directx11 gui errors
	AssimpPlus::TexturedShader::Texture::Texture(const std::vector<unsigned char>& rawData, std::uint32_t width, std::uint32_t height)
		: Buffer(nullptr)
		, SRV(nullptr)
	{
		D3D11_TEXTURE2D_DESC dscTexture = {};
		dscTexture.Height = height;
		dscTexture.Width = width;
		dscTexture.MipLevels = 1;
		dscTexture.ArraySize = 1;
		dscTexture.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		dscTexture.SampleDesc.Count = 1;
		dscTexture.SampleDesc.Quality = 0;
		dscTexture.Usage = D3D11_USAGE_DEFAULT;
		dscTexture.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET; // TODO SESS: Need render target?
		dscTexture.CPUAccessFlags = 0x00;
		dscTexture.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		HRESULT hr = {};

		D3D11_SUBRESOURCE_DATA initialData = {};
		initialData.pSysMem = &rawData[0];
		initialData.SysMemPitch = width * 4u * sizeof(unsigned char); // unsigned char is always 1 byte, I just like to illustrate better
		//initialData.SysMemSlicePitch = width * height * 4u * sizeof(unsigned char);



		D3D11_SHADER_RESOURCE_VIEW_DESC dscSRV = {};
		dscSRV.Format = dscTexture.Format;
		dscSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		dscSRV.Texture2D.MostDetailedMip = 0;
		dscSRV.Texture2D.MipLevels = -1;
	}

	std::future<bool> TexturedShader::Initialize(ComPtr<ID3D11Device> device)
	{
		// Async/await pattern in C++. I love it. Pass a function as the second parameter and specify
		//  async (as opposed to deferred) for the first, and it returns a std::future.
		// The function given will execute on a new thread, in parallel with the calling thread.
		// When the .get() method is called on the future returned, the calling thread will
		//  block until the function provided finishes, and return the result.
		// Fantastic pattern, totally unnecessary here.
		return std::async(std::launch::async, [this, device]() -> bool {
			const char* vsFname = "./cso/TexturedShader.vs.cso";
			const char* psFname = "./cso/TexturedShader.ps.cso";

			std::uint32_t vsDataLength;
			std::uint32_t psDataLength;

			HRESULT hr = {};

			D3D11_INPUT_ELEMENT_DESC inputLayout[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
			};
			std::uint32_t numElements = _countof(inputLayout);

			// Using asynchronous programming to get the vertex and pixel bytecode from the file.
			//  Other things can happen while this is happening, so don't join until the data is needed
			std::future<std::vector<char>> vsData = std::async(std::launch::async, [this, &vsDataLength, vsFname] {
				std::vector<char> vsBytecode(0u);
				std::ifstream vin(vsFname, std::ios::binary);
				if (!vin)
				{
					std::cerr << "Failed to open vertex shader file for reading." << std::endl;
					return vsBytecode;
				}

				vin.seekg(0, std::ios::end);
				vsDataLength = (std::uint32_t)vin.tellg();
				vin.seekg(0, std::ios::beg);
				vsBytecode.resize(vsDataLength);
				vin.read(&vsBytecode[0], vsDataLength);

				return vsBytecode;
				});

			std::future<std::vector<char>> psData = std::async(std::launch::async, [this, &psDataLength, psFname] {
				std::vector<char> psBytecode(0u);

				std::ifstream pin(psFname, std::ios::binary);
				if (!pin)
				{
					std::cerr << "Failed to open pixel shader file for reading." << std::endl;
					return psBytecode;
				}

				pin.seekg(0, std::ios::end);
				psDataLength = (std::uint32_t)pin.tellg();
				pin.seekg(0, std::ios::beg);
				psBytecode.resize(psDataLength);
				pin.read(&psBytecode[0], psDataLength);

				return psBytecode;
				});

			// We have to wait for the vertex and pixel buffers to load, so this next part is pretty much free
			D3D11_BUFFER_DESC bufferDesc = {};
			bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bufferDesc.MiscFlags = 0x00;
			bufferDesc.StructureByteStride = 0x00;

			bufferDesc.ByteWidth = sizeof(TexturedShader::DVSC_PerObject.VSC_PerObject);
			hr = device->CreateBuffer(&bufferDesc, nullptr, &vsc_object_);
			if (FAILED(hr))
			{
				std::cerr << "Failed to create vertex shader per-object constant buffer" << std::endl;
				return false;
			}

			bufferDesc.ByteWidth = sizeof(TexturedShader::DVSC_PerFrame.VSC_PerFrame);
			hr = device->CreateBuffer(&bufferDesc, nullptr, &vsc_frame_);
			if (FAILED(hr))
			{
				std::cerr << "Failed to create vertex shader per-frame constant buffer" << std::endl;
				return false;
			}

			bufferDesc.ByteWidth = sizeof(TexturedShader::DPSC_PerObject.PSC_PerObject);
			hr = device->CreateBuffer(&bufferDesc, nullptr, &psc_object_);
			if (FAILED(hr))
			{
				std::cerr << "Failed to create pixel shader per-object constant buffer" << std::endl;
				return false;
			}

			bufferDesc.ByteWidth = sizeof(TexturedShader::DPSC_PerFrame.PSC_PerFrame);
			hr = device->CreateBuffer(&bufferDesc, nullptr, &psc_frame_);
			if (FAILED(hr))
			{
				std::cerr << "Failed to create pixel shader per-frame constant buffer" << std::endl;
				return false;
			}

			bufferDesc.ByteWidth = sizeof(TexturedShader::DPSC_PerScene.PSC_PerScene);
			hr = device->CreateBuffer(&bufferDesc, nullptr, &psc_scene_);
			if (FAILED(hr))
			{
				std::cerr << "Failed to create pixel shader per-scene constant buffer" << std::endl;
				return false;
			}

			// Create vertex shader
			std::vector<char> vsBytecode = vsData.get();
			if (vsBytecode.size() == 0u)
			{
				return false;
			}

			hr = device->CreateVertexShader(&vsBytecode[0], vsDataLength, nullptr, &vertexShader_);
			if (FAILED(hr))
			{
				std::cerr << "Failed to create vertex shader: " << hr << std::endl;
				return false;
			}

			hr = device->CreateInputLayout(inputLayout, numElements, &vsBytecode[0], vsDataLength, &inputLayout_);
			if (FAILED(hr))
			{
				std::cerr << "Failed to create input layout for vertex shader: " << hr << std::endl;
				return false;
			}

			std::vector<char> psBytecode = psData.get();
			if (psBytecode.size() == 0u)
			{
				return false;
			}

			hr = device->CreatePixelShader(&psBytecode[0], psDataLength, nullptr, &pixelShader_);
			if (FAILED(hr))
			{
				std::cerr << "Failed to create pixel shader: " << hr << std::endl;
				return false;
			}

			return true;
			});
	}

	// Make a draw call. The context is needed to do drawing things, and the call actually
	//  contains the specific information we need.
	// All globals (GL folks, uniforms) should have been set before this is called, because
	//  the call does not contain that information
	// This is not thread-safe, since it deals with the ID3D11DeviceContext
	bool TexturedShader::Render(const RenderCall& call)
	{
		HRESULT hr = {};

		context_->IASetInputLayout(inputLayout_.Get());
		context_->VSSetShader(vertexShader_.Get(), nullptr, 0);
		context_->PSSetShader(pixelShader_.Get(), nullptr, 0);

		// Update constant buffers. This involves mapping a chunk of host-side (CPU) memory
		//  to the constant buffer, writing to that memory, and then uploading the chunk
		//  to the graphics card. Mapping is done with "map", uploading with "unmap"
		if (DVSC_PerFrame.isDirty)
		{
			D3D11_MAPPED_SUBRESOURCE vscb;

			hr = context_->Map(vsc_frame_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0x00, &vscb);
			if (FAILED(hr))
			{
				std::cerr << "Material only shader render: Failed to map vertex per-frame buffer for CPU writing" << std::endl;
				return false;
			}

			memcpy(vscb.pData, &(DVSC_PerFrame.VSC_PerFrame), sizeof(DVSC_PerFrame.VSC_PerFrame));
			context_->Unmap(vsc_frame_.Get(), 0);

			DVSC_PerFrame.isDirty = false;
		}

		if (DVSC_PerObject.isDirty)
		{
			D3D11_MAPPED_SUBRESOURCE vscb;

			hr = context_->Map(vsc_object_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0x00, &vscb);
			if (FAILED(hr))
			{
				std::cerr << "Material only shader render: Failed to map vertex per-object buffer for CPU writing" << std::endl;
				return false;
			}

			memcpy(vscb.pData, &(DVSC_PerObject.VSC_PerObject), sizeof(DVSC_PerObject.VSC_PerObject));
			context_->Unmap(vsc_object_.Get(), 0);

			DVSC_PerObject.isDirty = false;
		}

		if (DPSC_PerFrame.isDirty)
		{
			D3D11_MAPPED_SUBRESOURCE pscb;

			hr = context_->Map(psc_frame_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0x00, &pscb);
			if (FAILED(hr))
			{
				std::cerr << "Material only shader render: Failed to map pixel per-frame buffer for CPU writing" << std::endl;
				return false;
			}

			memcpy(pscb.pData, &(DPSC_PerFrame.PSC_PerFrame), sizeof(DPSC_PerFrame.PSC_PerFrame));
			context_->Unmap(psc_frame_.Get(), 0);

			DPSC_PerFrame.isDirty = false;
		}

		if (DPSC_PerObject.isDirty)
		{
			D3D11_MAPPED_SUBRESOURCE pscb;

			hr = context_->Map(psc_object_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0x00, &pscb);
			if (FAILED(hr))
			{
				std::cerr << "Material only shader render: Failed to map pixel per-object buffer for CPU writing" << std::endl;
				return false;
			}

			memcpy(pscb.pData, &(DPSC_PerObject.PSC_PerObject), sizeof(DPSC_PerObject.PSC_PerObject));
			context_->Unmap(psc_object_.Get(), 0);

			DPSC_PerObject.isDirty = false;
		}

		if (DPSC_PerScene.isDirty)
		{
			D3D11_MAPPED_SUBRESOURCE pscb;

			hr = context_->Map(psc_scene_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0x00, &pscb);
			if (FAILED(hr))
			{
				std::cerr << "Material only shader render: Failed to map pixel per-scene buffer for CPU writing" << std::endl;
				return false;
			}

			memcpy(pscb.pData, &(DPSC_PerScene.PSC_PerScene), sizeof(DPSC_PerScene.PSC_PerScene));
			context_->Unmap(psc_scene_.Get(), 0);

			DPSC_PerScene.isDirty = false;
		}

		// Set the constant buffers to the pipeline
		ID3D11Buffer* vsCBuffers[] = { vsc_object_.Get(), vsc_frame_.Get() };
		context_->VSSetConstantBuffers(0, _countof(vsCBuffers), vsCBuffers);

		ID3D11Buffer* psCBuffers[] = { psc_object_.Get(), psc_frame_.Get(), psc_scene_.Get() };
		context_->PSSetConstantBuffers(0, _countof(psCBuffers), psCBuffers);

		// Set the input vertex buffer
		std::uint32_t stride = sizeof(AssimpPlus::Vertex);
		std::uint32_t offset = 0u;
		// Set the vertex buffer as input to the graphics pipeline. Only using one.
		context_->IASetVertexBuffers(0, 1, call.VertexBuffer.GetAddressOf(), &stride, &offset);

		// Set the index buffer as input to the graphics pipeline. Specify that we're using 32 bit unsigned integers
		//  DXGI is weird about formats, R32 means "one component, having 32 bits". The "R" stands for "red"
		//  A set of three values, 32 bit uints each, would be DXGI_FORMAT_R32G32B32_UINT, for example.
		context_->IASetIndexBuffer(call.IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);

		// We're drawing triangles...
		context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Set the texture we want to use...
		context_->PSSetShaderResources(0, 1, boundSRV.GetAddressOf());

		// Draw! Draw! Draw!
		context_->DrawIndexed(call.NumberOfIndices, 0u, 0u);
		//2023-02-02 Started seeing this warning:
		//D3D11 WARNING : ID3D11DeviceContext::DrawIndexed : The Pixel Shader unit expects a Sampler to be set at Slot 0, but none is bound.This is perfectly valid, as a NULL Sampler maps to default Sampler state.However, the developer may not want to rely on the defaults.[EXECUTION WARNING #352: DEVICE_DRAW_SAMPLER_NOT_SET]

		return true;
	}
}
