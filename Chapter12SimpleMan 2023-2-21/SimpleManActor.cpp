//taken from indego code
#include "SimpleManActor.h"


//need to fix these includes
//should be something like #include <../External/Assimp/include/assimpcimport.h>
//think this might be fixed in linker properties
#include "lodepng.h" 

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

#include <iostream>
#include "Game.h"
#include "MeshComponentDirectX.h"
#include "Mesh.h"

namespace AssimpPlus
{
	void GenerateDirectXMeshes(::Game* game, const char* fName, std::vector<AssimpPlus::Mesh*>& meshes);
	void GenerateMeshes(::Game* game, const char* fName, ::Texture* t, std::vector<::Mesh*>& meshes); //Madhav

	TexturedShader::Texture flooboggle //What's with the funky name? VS wouldn't let me call it loadTexture
		(const char* textureFilename, ComPtr<ID3D11Device> d3dDevice, ComPtr<ID3D11DeviceContext> d3dDeviceContext)
		;
	SimpleManActor::SimpleManActor(::Game* game)
		: Actor(game)
		, texture_(flooboggle("Assets/man-skin.png", game->appPtr_->device_, game->appPtr_->context_))
		, rotationSpeed_(1.1f)
		, rotationAngle_(0.0f)
	{
		SetScale   (0.55f);

		GenerateDirectXMeshes(game, "Assets/SimpleMan2.6.fbx", meshes_);      //<--John: isn't meshes_ empty?

		for (auto&& mesh : meshes_)                                           //<--John
			meshComponents_.push_back(new MeshComponentDirectX(this, mesh, false));

		//Now, Madhav
		::Texture* texture = new ::Texture; texture->Load("Assets/man-skin.png");
		std::vector<::Mesh*> madhavMeshes;
		GenerateMeshes(game, "Assets/SimpleMan2.6.fbx", texture, madhavMeshes);
		for (auto&& mesh : madhavMeshes)
		{
			MeshComponent* mc = new MeshComponent(this); mc->SetMesh(mesh);
			meshComponents_.push_back(mc);
		}
	}

	static void GenerateMeshes(::Game* game, const char* fName, ::Texture* t, std::vector<::Mesh*>& meshes)
	{
		meshes.clear();

		const aiScene* scene = aiImportFile(fName, aiProcessPreset_TargetRealtime_MaxQuality);

		if (!scene)
		{
			std::cerr << "Could not load file " << fName << std::endl;
			throw "Couldn't load file in SimpleMan::LoadFromFile"; //fix exception later if this function survives
		}
		else
		{
			// Load all meshes and whatnot
			meshes.reserve(scene->mNumMeshes);
			for (std::uint32_t meshIdx = 0u; meshIdx < scene->mNumMeshes; meshIdx++)
			{
				aiMesh* mesh = scene->mMeshes[meshIdx];

				::Mesh* mMesh = new ::Mesh; //2023-2-7 WSB be sure somebody deletes this eventually

				mMesh->mShaderName = "BasicMesh";

				// Set the vertex layout/size based on the format in the file
				VertexArray::Layout layout = VertexArray::PosNormTex;
				size_t vertSize = 8;
				mMesh->mSpecPower = 100.0;

				//Associate the loaded texture
				mMesh->mTextures.emplace_back(t);

				// Load in the vertices
				std::vector<::Vertex> vertices;
				vertices.reserve(mesh->mNumVertices); //((_countof(verts) / 3) * 8); //verts/3 is # of vertices. For each vertex, 8 floats: 3 for vert, 3 for index, 2 for text coords

				mMesh->mRadius = 0.0f;
				for (std::uint32_t i = 0; i < mesh->mNumVertices; i++)
				{
					aiVector3D vert = mesh->mVertices[i];
					aiVector3D norm = mesh->mNormals[i];
					aiVector3D uv(0.f, 0.f, 0.f);
					if (mesh->mTextureCoords[0]) //if there ARE texture coords
						uv = mesh->mTextureCoords[0][i]; //WSB 2023-02-21 Is there possibly a z component? Why use a 3d vector?

					//Keep mRadius and mBox updated: for each new vertex, if this one's further out, remember it
					Vector3 pos(vert[0], vert[1], vert[2]);
					mMesh->mRadius = Math::Max(mMesh->mRadius, pos.LengthSq());
					mMesh->mBox.UpdateMinMax(pos);

					::Vertex v;

					// Add the floats
					for (int j = 0; j < 3; j++) { v.f = vert[j]; vertices.emplace_back(v); } 
					for (int j = 0; j < 3; j++) { v.f = norm[j]; vertices.emplace_back(v); } 
					for (int j = 0; j < 2; j++) { v.f = uv  [j]; vertices.emplace_back(v); } 
				}

				// We were computing length squared earlier
				mMesh->mRadius = Math::Sqrt(mMesh->mRadius);

				std::vector<unsigned int> indices;
				indices.reserve(mesh->mNumFaces * 3u);
				for (unsigned int i = 0; i < mesh->mNumFaces; i++)
				{
					indices.emplace_back(mesh->mFaces[i].mIndices[0u]);
					indices.emplace_back(mesh->mFaces[i].mIndices[1u]);
					indices.emplace_back(mesh->mFaces[i].mIndices[2u]);
				}

				//WSB 2023-02-21: aren't we supposed to have an mBox?
				
				// Now create a vertex array
				mMesh->mVertexArray = new VertexArray(vertices.data(), static_cast<unsigned>(vertices.size()) / vertSize,
					layout, indices.data(), static_cast<unsigned>(indices.size()));

				mMesh->whoAmI_ = "SimpleMan"; //for debugging

				//meshes.push_back (mMesh); //<-- John -- this isn't actually being done yet!
			}
		}
	}

	static void GenerateDirectXMeshes(::Game* game, const char* fName, std::vector<AssimpPlus::Mesh*>& meshes)
	{
		meshes.clear();

		const aiScene* scene = aiImportFile(fName, aiProcessPreset_TargetRealtime_MaxQuality);

		if (!scene)
		{
			std::cerr << "Could not load file " << fName << std::endl;
			throw "Couldn't load file in SimpleMan::LoadFromFile"; //fix exception later if this function survives
		}
		else
		{
			// Load all meshes and whatnot
			meshes.reserve(scene->mNumMeshes);
			for (std::uint32_t meshIdx = 0u; meshIdx < scene->mNumMeshes; meshIdx++)
			{
				aiMesh* mesh = scene->mMeshes[meshIdx];

				std::vector<AssimpPlus::Vertex> verts;
				verts.reserve(mesh->mNumVertices);

				aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
				aiColor4D specularColor;
				aiColor4D diffuseColor;
				aiColor4D ambientColor;
				float shininess;

				aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specularColor);
				aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuseColor);
				aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambientColor);
				aiGetMaterialFloat(material, AI_MATKEY_SHININESS, &shininess);

				AssimpPlus::Material meshMaterial
				(
					Material::Color(specularColor.r, specularColor.g, specularColor.b, shininess), // Specular
					Material::Color(diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a), // Diffuse
					Material::Color(ambientColor.r, ambientColor.g, ambientColor.b, ambientColor.a) // Ambient
				);

				for (std::uint32_t vertIdx = 0u; vertIdx < mesh->mNumVertices; vertIdx++)
				{
					aiVector3D vert = mesh->mVertices[vertIdx];
					aiVector3D norm = mesh->mNormals[vertIdx];
					aiVector3D uv(0.f, 0.f, 0.f);
					if (mesh->mTextureCoords[0]) //if there ARE texture coords
						uv = mesh->mTextureCoords[0][vertIdx];

					verts.push_back
					(
						AssimpPlus::Vertex
						(
							Vector3(vert.x, vert.y, vert.z),
							Vector3(norm.x, norm.y, norm.z),
							uv.x, uv.y
						)
					);
				}

				std::vector<std::uint32_t> indices;
				indices.reserve(mesh->mNumFaces * 3u);
				for (std::uint32_t faceIdx = 0u; faceIdx < mesh->mNumFaces; faceIdx++)
				{
					indices.push_back(mesh->mFaces[faceIdx].mIndices[0u]);
					indices.push_back(mesh->mFaces[faceIdx].mIndices[1u]);
					indices.push_back(mesh->mFaces[faceIdx].mIndices[2u]);
				}

				RenderCall call(game->appPtr_->device_, verts, indices);

				meshes.push_back(new AssimpPlus::Mesh(call, meshMaterial));
			}
		}
	}

	TexturedShader::Texture flooboggle //What's with the funky name? VS wouldn't let me call it loadTexture
			(const char* textureFilename, ComPtr<ID3D11Device> d3dDevice, ComPtr<ID3D11DeviceContext> d3dDeviceContext)
	{
		//
		// Load image with LodePNG
		//
		std::vector<unsigned char> textureData; // Raw pixel data
		std::uint32_t imageWidth, imageHeight; // Image metadata

		// Decode image with LodePNG
		std::uint32_t decodeError = lodepng::decode(textureData, imageWidth, imageHeight, textureFilename);

		// Flip all values on Y
		for (unsigned int row = 0; row < imageHeight / 2u; row++)
		{
			for (unsigned int col = 0; col < imageWidth; col++)
			{
				int topPixelId = row * imageWidth + col;
				int botPixelId = (imageHeight - row - 1) * imageWidth + col;
				for (int component = 0; component < 4; component++)
				{
					unsigned char temp = textureData[topPixelId * 4 + component];
					textureData[topPixelId * 4 + component] = textureData[botPixelId * 4 + component];
					textureData[botPixelId * 4 + component] = temp;
				}
			}
		}

		//
		// ... Done loading image. Lode Vandevenne, you're AWESOME dude
		//
		
		return TexturedShader::Texture (d3dDevice, d3dDeviceContext, textureData, imageWidth, imageHeight);
	}

	bool SimpleManActor::Update(float dt)
	{
		rotationAngle_ += rotationSpeed_ * dt;
		while (rotationAngle_ > 2.f * Math::Pi)
		{
			rotationAngle_ -= 2.f * Math::Pi;
		}

		::Quaternion temp = 
					::Quaternion::Concatenate(::Quaternion(Vector3::UnitY, Math::ToRadians(180.f)),
					::Quaternion(Vector3::UnitX, Math::ToRadians(90.f)));
		SetRotation(::Quaternion::Concatenate(temp, ::Quaternion(Vector3::UnitY, rotationAngle_)));
		//SetRotation(::Quaternion()); //just for debug

		return true;
	}

	bool SimpleManActor::Render(TexturedShader* shader) 
	{
		shader->SetModelTransform(GetWorldTransform().Transpose()); //Madhav has world transform matrix transposed relative to assimp
																	// and other sources I find 2023-01-31
		shader->SetTexture(texture_);

		for (auto&& mesh : meshes_)
		{
			shader->SetObjectMaterial(mesh->Material);
			shader->Render(mesh->Call);
		}

		return true;
	}
}
