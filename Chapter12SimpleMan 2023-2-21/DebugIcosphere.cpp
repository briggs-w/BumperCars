#include "DebugIcosphere.h"

//Next includes are from Madhav, for Draw


#include "Mesh.h"
#include "Game.h"

namespace AssimpPlus
{

	// What, you DON'T have the vertices for a simple icosphere memorized?
	float verts[] =
	{
		0, 0, -1,							0.7236, -0.52572, -0.447215,
		-0.276385, -0.85064, -0.447215,		0.7236, -0.52572, -0.447215,
		0, 0, -1,							0.7236, 0.52572, -0.447215,
		0, 0, -1,							-0.276385, -0.85064, -0.447215,
		-0.894425, 0, -0.447215,			0, 0, -1,
		-0.894425, 0, -0.447215,			-0.276385, 0.85064, -0.447215,
		0, 0, -1,							-0.276385, 0.85064, -0.447215,
		0.7236, 0.52572, -0.447215,			0.7236, -0.52572, -0.447215,
		0.7236, 0.52572, -0.447215,			0.894425, 0, 0.447215,
		-0.276385, -0.85064, -0.447215,		0.7236, -0.52572, -0.447215,
		0.276385, -0.85064, 0.447215,		-0.894425, 0, -0.447215,
		-0.276385, -0.85064, -0.447215,		-0.7236, -0.52572, 0.447215,
		-0.276385, 0.85064, -0.447215,		-0.894425, 0, -0.447215,
		-0.7236, 0.52572, 0.447215,			0.7236, 0.52572, -0.447215,
		-0.276385, 0.85064, -0.447215,		0.276385, 0.85064, 0.447215,
		0.7236, -0.52572, -0.447215,		0.894425, 0, 0.447215,
		0.276385, -0.85064, 0.447215,		-0.276385, -0.85064, -0.447215,
		0.276385, -0.85064, 0.447215,		-0.7236, -0.52572, 0.447215,
		-0.894425, 0, -0.447215,			-0.7236, -0.52572, 0.447215,
		-0.7236, 0.52572, 0.447215,			-0.276385, 0.85064, -0.447215,
		-0.7236, 0.52572, 0.447215,			0.276385, 0.85064, 0.447215,
		0.7236, 0.52572, -0.447215,			0.276385, 0.85064, 0.447215,
		0.894425, 0, 0.447215,				0.276385, -0.85064, 0.447215,
		0.894425, 0, 0.447215,				0, 0, 1,
		-0.7236, -0.52572, 0.447215,		0.276385, -0.85064, 0.447215,
		0, 0, 1,							-0.7236, 0.52572, 0.447215,
		-0.7236, -0.52572, 0.447215,		0, 0, 1,
		0.276385, 0.85064, 0.447215,		-0.7236, 0.52572, 0.447215,
		0, 0, 1,							0.894425, 0, 0.447215,
		0.276385, 0.85064, 0.447215,		0, 0, 1
	};

	float normals[] =
	{
		0.187597, -0.577354, -0.794651,		0.187597, -0.577354, -0.794651,
		0.187597, -0.577354, -0.794651,		0.607065, 0, -0.794652,
		0.607065, 0, -0.794652,				0.607065, 0, -0.794652,
		-0.491122, -0.356829, -0.794652,	-0.491122, -0.356829, -0.794652,
		-0.491122, -0.356829, -0.794652,	-0.491122, 0.356829, -0.794652,
		-0.491122, 0.356829, -0.794652,		-0.491122, 0.356829, -0.794652,
		0.187597, 0.577354, -0.794651,		0.187597, 0.577354, -0.794651,
		0.187597, 0.577354, -0.794651,		0.982246, 0, -0.187597,
		0.982246, 0, -0.187597,				0.982246, 0, -0.187597,
		0.303536, -0.934171, -0.187589,		0.303536, -0.934171, -0.187589,
		0.303536, -0.934171, -0.187589,		-0.794649, -0.577359, -0.187587,
		-0.794649, -0.577359, -0.187587,	-0.794649, -0.577359, -0.187587,
		-0.794649, 0.577359, -0.187587,		-0.794649, 0.577359, -0.187587,
		-0.794649, 0.577359, -0.187587,		0.303536, 0.934171, -0.187589,
		0.303536, 0.934171, -0.187589,		0.303536, 0.934171, -0.187589,
		0.794649, -0.577359, 0.187587,		0.794649, -0.577359, 0.187587,
		0.794649, -0.577359, 0.187587,		-0.303536, -0.934171, 0.187589,
		-0.303536, -0.934171, 0.187589,		-0.303536, -0.934171, 0.187589,
		-0.982246, 0, 0.187597,				-0.982246, 0, 0.187597,
		-0.982246, 0, 0.187597,				-0.303536, 0.934171, 0.187589,
		-0.303536, 0.934171, 0.187589,		-0.303536, 0.934171, 0.187589,
		0.794649, 0.577359, 0.187587,		0.794649, 0.577359, 0.187587,
		0.794649, 0.577359, 0.187587,		0.491122, -0.356829, 0.794652,
		0.491122, -0.356829, 0.794652,		0.491122, -0.356829, 0.794652,
		-0.187596, -0.577354, 0.794651,		-0.187596, -0.577354, 0.794651,
		-0.187596, -0.577354, 0.794651,		-0.607065, 0, 0.794652,
		-0.607065, 0, 0.794652,				-0.607065, 0, 0.794652,
		-0.187597, 0.577354, 0.794651,		-0.187597, 0.577354, 0.794651,
		-0.187597, 0.577354, 0.794651,		0.491122, 0.356829, 0.794652,
		0.491122, 0.356829, 0.794652,		0.491122, 0.356829, 0.794652
	};

	std::uint32_t g_indices[] =
	{
		0,1,2,		3,4,5,		6,7,8,
		9,10,11,	12,13,14,	15,16,17,
		18,19,20,	21,22,23,	24,25,26,
		27,28,29,	30,31,32,	33,34,35,
		36,37,38,	39,40,41,	42,43,44,
		45,46,47,	48,49,50,	51,52,53,
		54,55,56,	57,58,59
	};

	void IcosphereActor::CreateMesh()
	{
		mMesh = new ::Mesh; //2023-2-7 WSB be sure somebody deletes this eventually

		mMesh->mShaderName = "BasicMesh";

		// Set the vertex layout/size based on the format in the file
		VertexArray::Layout layout = VertexArray::PosNormTex;
		size_t vertSize = 8;
		mMesh->mSpecPower = 100.0;

		//DebugIcosophere isn't supposed to have textures. If I get a crash, add this default one.
		Texture* t = GetGame ()->GetRenderer()->GetTexture("Assets/Sphere.png");
		mMesh->mTextures.emplace_back(t);
		
		// Load in the vertices
		std::vector<::Vertex> vertices;
		vertices.reserve((_countof(verts)/3)*8); //verts/3 is # of vertices. For each vertex, 8 floats: 3 for vert, 3 for index, 2 for text coords
		float* currentVertex = verts; 
		float* currentNormal = normals;

		mMesh->mRadius = 0.0f;
		for (std::uint32_t i = 0; i < _countof(verts)/3; i++)
		{
			::Vertex v;

			//Keep mRadius and mBox updated: for each new vertex, if this one's further out, remember it
			Vector3 pos(currentVertex[0], currentVertex[1], currentVertex[2]);
			mMesh->mRadius = Math::Max(mMesh->mRadius, pos.LengthSq());
			mMesh->mBox.UpdateMinMax(pos);

			// Add the floats
			for (int j = 0; j < 3; j++) { v.f = currentVertex[j]; vertices.emplace_back(v); } currentVertex += 3;
			for (int j = 0; j < 3; j++) { v.f = currentNormal[j]; vertices.emplace_back(v); } currentNormal += 3;
			for (int j = 0; j < 2; j++) { v.f = 0.0f;             vertices.emplace_back(v); } //2 dummy tex coords
		}

		// We were computing length squared earlier
		mMesh->mRadius = Math::Sqrt(mMesh->mRadius);

		std::vector<unsigned int> indices;
		std::uint32_t* currentIndex = g_indices;
		indices.reserve(_countof(g_indices));
		for (int i = 0; i < _countof(g_indices)/3; i++)
		{
			indices.emplace_back(currentIndex[0]);
			indices.emplace_back(currentIndex[1]);
			indices.emplace_back(currentIndex[2]);
			currentIndex += 3;
		}

		// Now create a vertex array
		mMesh->mVertexArray = new VertexArray(vertices.data(), static_cast<unsigned>(vertices.size()) / vertSize,
			layout, indices.data(), static_cast<unsigned>(indices.size()));

		mMesh->whoAmI_ = "IcosphereMesh"; //for debugging
	}

	const AssimpPlus::Material::Color Red(0xa6u / float(0xffu), 0x3du / float(0xffu), 0x40u / float(0xffu), 0xffu / float(0xffu)); // #a63d40
	const AssimpPlus::Material::Color DUSKY_RED(0.9f * 0xa6u / float(0xffu), 0.9f * 0x3du / float(0xffu), 0.9f * 0x40u / float(0xffu), 0xffu / float(0xffu)); // #a63d40
	const AssimpPlus::Material::Color SOFT_RED(0xa6u / float(0xffu), 0x4du / float(0xffu), 0x50u / float(0xffu), 0xffu / float(0xffu)); // #a63d40

	IcosphereActor::IcosphereActor (::Game* game)
		: Actor(game)
		, call_(nullptr)
		, context_ (game->appPtr_->context_)
		, material_(AssimpPlus::Material(SOFT_RED, Red, DUSKY_RED))
		, rotationSpeed_(1.1f)
		, rotationAngle_(0.f)
	{
		std::vector<AssimpPlus::Vertex> vertices;
		vertices.reserve(_countof(verts));
		for (std::uint32_t idx = 0u; idx < _countof(verts); idx += 3)
		{
			vertices.push_back
			(
				AssimpPlus::Vertex
				(
					Vector3(verts[idx], verts[idx + 1u], verts[idx + 2u]),
					Vector3(normals[idx], normals[idx + 1u], normals[idx + 2u])
				)
			);
		}

		std::vector<std::uint32_t> indices;
		indices.reserve(_countof(g_indices));
		for (std::uint32_t idx = 0u; idx < _countof(g_indices); idx++)
		{
			indices.push_back(g_indices[idx]);
		}

		call_ = std::make_shared<RenderCall>(game->appPtr_->device_, vertices, indices);

		//Now, the Madhav stuff: Can't do it yet because I need OpenGL initialized first
		// so move that around and try again.
	
		CreateMesh();

		MeshComponent* mc = new MeshComponent(this); mc->SetMesh(mMesh);
	}

	bool IcosphereActor::Update(float dt)
	{
		rotationAngle_ += rotationSpeed_ * dt;
		while (rotationAngle_ > 2.f * Math::Pi)
			rotationAngle_ -= 2.f * Math::Pi;

		SetRotation(::Quaternion(Vector3::UnitY, rotationAngle_));
		
		return true;
	}

	bool IcosphereActor::Render(MaterialOnlyShader* shader) const
	{
		shader->SetObjectMaterial(material_);
		shader->SetModelTransform(GetWorldTransform().Transpose()); 
		shader->Render(*call_);

		return true;
	}

	//void IcosphereActor::Draw(Shader* shader) //Not currently used. Will it work? Not yet!
	//{
		//if (mMesh)
		//{
		//	// Set the world transform
		//	shader->SetMatrixUniform("uWorldTransform",	GetWorldTransform());
		//	// Set specular power
		//	shader->SetFloatUniform("uSpecPower", mMesh->mSpecPower);
		//	
		//	//do I need to tell it about the material?
		//	
		//	// Set the mesh's vertex array as active
		//	VertexArray* va = mMesh->mVertexArray;
		//	va->SetActive();
		//	// Draw
		//	glDrawElements(GL_TRIANGLES, va->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
		//}
	//}

}