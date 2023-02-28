#include "RoadActor.h"

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

#include <iostream>

//From Madhav
#include "Texture.h"
#include "Shader.h"
#include "VertexArray.h"
#include "Game.h"
#include "MeshComponentDirectX.h"
#include "BoxComponent.h"

namespace AssimpPlus
{
	void GenerateDirectXMeshes(::Game* game, const char* fName, std::vector<AssimpPlus::Mesh*>& meshes);
	void GenerateMeshes(::Game* game, const char* fName, std::vector<::Mesh*>& meshes); //Madhav meshes

	RoadActor::RoadActor(Game* game)
		: Actor(game)
		, context_ (game->appPtr_->context_)
	{
		GenerateDirectXMeshes(game, "Assets/road.fbx", meshes_);           

		//for (auto&& mesh : meshes_)                                      //<-- John: here's what to do next
		//	meshComponents_.push_back(new MeshComponentDirectX(this, mesh, false)); 

		//This is for Madhav code (doesn't do anything yet)
		std::vector<::Mesh*> meshesForMadhav;
		GenerateMeshes(game, "Assets/road.fbx", meshesForMadhav);

		for (auto&& mesh : meshesForMadhav)
		{
			::MeshComponent* mc = new MeshComponent(this);
			mc->SetMesh(mesh);
			meshComponents_.push_back(mc);
		}
	}

	static void GenerateMeshes(::Game* game, const char* fName, std::vector<::Mesh*>& meshes)
	{
		//don't know how yet!
	}

	static void GenerateDirectXMeshes(::Game* game, const char* fName, std::vector<AssimpPlus::Mesh*>& meshes)
	{
		meshes.clear();

		const aiScene* scene = aiImportFile(fName, aiProcessPreset_TargetRealtime_MaxQuality);

		if (!scene)
		{
			std::cerr << "Could not load file " << fName << std::endl;
			throw "Couldn't load file in RoadActor::LoadFromFile"; //fix exception later if this function survives
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

	bool RoadActor::Update(float dt) {	return true; }

	bool RoadActor::Render(MaterialOnlyShader* shader) const
	{
		//shader->SetModelTransform(getTransformMatrix(GetPosition(), GetRotation(), GetScale())); 
		shader->SetModelTransform(GetWorldTransform().Transpose()); 
					//Madhav has world transform matrix transposed relative to assimp
					// and other sources I find 2023-01-31

		for (auto&& mesh : meshes_)
		{
			shader->SetObjectMaterial(mesh->Material);
			shader->Render(mesh->Call);
		}

		return true;
	}

	void RoadActor::Draw(Shader* shader)
	{
		for (auto&& mesh: meshes_)
		{
		//	// Set the world transform
		//	shader->SetMatrixUniform("uWorldTransform",
		//		mOwner->GetWorldTransform());
		//	// Set specular power
		//	shader->SetFloatUniform("uSpecPower", mMesh->GetSpecPower());
		//	// Set the active texture
		//	Texture* t = mMesh->GetTexture(mTextureIndex);
		//	if (t)
		//	{
		//		t->SetActive();
		//	}
		//	// Set the mesh's vertex array as active
		//	VertexArray* va = mMesh->GetVertexArray();
		//	va->SetActive();
		//	// Draw
		//	glDrawElements(GL_TRIANGLES, va->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
		}
	}

};