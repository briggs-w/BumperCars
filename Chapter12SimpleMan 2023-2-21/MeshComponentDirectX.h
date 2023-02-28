//Class to be a MeshComponent AND to use DirectX.
//This is a temporary measure!
//Will Briggs 2023-01-31

#pragma once
#include "MeshComponent.h"
#include "AssimpPlusShader.h"

namespace AssimpPlus
{
	class MaterialOnlyShader;
};

class MeshComponentDirectX : public MeshComponent
{
public:
	MeshComponentDirectX(class Actor* owner, AssimpPlus::Mesh* mesh, bool isSkeletal = false) 
		: MeshComponent(owner, isSkeletal), mesh_(mesh)
	{
	}

	// Draw this mesh component
	virtual void Draw(class Shader* shader) override; //This is for Madhav's world
	bool Render(AssimpPlus::MaterialOnlyShader* shader) const; //This is for DirectX's world //<-- John
	// Set the mesh/texture index used by mesh component
	virtual void SetMesh(class AssimpPlus::Mesh* mesh) { mesh_ = mesh; }

private:
	AssimpPlus::Mesh* mesh_;
};

