#include "AssimpPlusShader.h"
#include <iostream> //for cerr

namespace AssimpPlus
{
	static const Vector3 Black; //defaults to black
	static const Vector3 CreamIGuess(0xf9u / float(0xffu), 0xb8u / float(0xffu), 0x72u / float(0xffu)); // #e9b872

	AssimpPlusShader::AssimpPlusShader(ComPtr<ID3D11DeviceContext> context) :
		context_(context)
		, vertexShader_(nullptr)
		, pixelShader_(nullptr)
		, inputLayout_(nullptr)
		, vsc_object_(nullptr)
		, vsc_frame_(nullptr)
		, psc_object_(nullptr)
		, psc_frame_(nullptr)
		, psc_scene_(nullptr)
		, DVSC_PerObject(Matrix4::Identity)
		, DVSC_PerFrame(Matrix4::Identity, Matrix4::Identity)
		, DPSC_PerFrame(Vector3::Zero, 0.0f)
		, DPSC_PerObject(Material(Black, Black, Black))
		, DPSC_PerScene(::DirectionalLight(Vector3::UnitY, CreamIGuess, CreamIGuess))
	{}

	RenderCall::RenderCall(ComPtr<ID3D11Device> device, const std::vector<AssimpPlus::Vertex>& vertices, const std::vector<std::uint32_t>& indices)
		: VertexBuffer(nullptr)
		, IndexBuffer(nullptr)
		, NumberOfIndices(0u)
	{
		HRESULT hr = {};
		D3D11_BUFFER_DESC vbDesc = {};
		vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbDesc.CPUAccessFlags = 0x00;
		vbDesc.MiscFlags = 0x00;
		vbDesc.ByteWidth = sizeof(AssimpPlus::Vertex) * (UINT)vertices.size();
		vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vbDesc.StructureByteStride = 0x00;

		// Vector elements must be stored contiguously. As per the C++11 standard,
		//  they don't necessarily have to be stored as an array, but the identity
		//  &v[n] = &v[0] + n for all 0 <= n < v.size() must hold true.
		// http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#69
		D3D11_SUBRESOURCE_DATA vertexData = {};
		vertexData.pSysMem = &vertices[0];

		hr = device->CreateBuffer(&vbDesc, &vertexData, &VertexBuffer);
		if (FAILED(hr))
		{
			std::cerr << "Failed to allocate vertex buffer for render call! " << hr << std::endl;
			return;
		}

		D3D11_BUFFER_DESC ibDesc = {};
		ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibDesc.CPUAccessFlags = 0x00;
		ibDesc.MiscFlags = 0x00;
		ibDesc.ByteWidth = sizeof(std::uint32_t) * (UINT)indices.size();
		ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
		ibDesc.StructureByteStride = 0x00;

		D3D11_SUBRESOURCE_DATA indexData = {};
		indexData.pSysMem = &indices[0];

		hr = device->CreateBuffer(&ibDesc, &indexData, &IndexBuffer);

		NumberOfIndices = (std::uint32_t)indices.size();
	}

	////without dx11 device
//RenderCall::RenderCall(const std::vector<AssimpPlus::Vertex>& vertices, const std::vector<std::uint32_t>& indices)
//	: VertexBuffer(nullptr)
//	, IndexBuffer(nullptr)
//	, NumberOfIndices(0u)
//{
//	HRESULT hr = {};
//	D3D11_BUFFER_DESC vbDesc = {};
//	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//	vbDesc.CPUAccessFlags = 0x00;
//	vbDesc.MiscFlags = 0x00;
//	vbDesc.ByteWidth = sizeof(AssimpPlus::Vertex) * (UINT)vertices.size();
//	vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
//	vbDesc.StructureByteStride = 0x00;

//	// Vector elements must be stored contiguously. As per the C++11 standard,
//	//  they don't necessarily have to be stored as an array, but the identity
//	//  &v[n] = &v[0] + n for all 0 <= n < v.size() must hold true.
//	// http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#69
//	D3D11_SUBRESOURCE_DATA vertexData = {};
//	vertexData.pSysMem = &vertices[0];

//	D3D11_BUFFER_DESC ibDesc = {};
//	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
//	ibDesc.CPUAccessFlags = 0x00;
//	ibDesc.MiscFlags = 0x00;
//	ibDesc.ByteWidth = sizeof(std::uint32_t) * (UINT)indices.size();
//	ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
//	ibDesc.StructureByteStride = 0x00;

//	D3D11_SUBRESOURCE_DATA indexData = {};
//	indexData.pSysMem = &indices[0];

//	NumberOfIndices = (std::uint32_t)indices.size();
//}


}