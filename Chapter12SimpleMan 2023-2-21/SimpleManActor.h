#pragma once
//taken from indego code's example

#include "Transform.h"
#include "DirectXThing.h"
#include <vector>
#include <memory>

#include "TexturedShader.h"
#include "Actor.h"
#include "MeshComponent.h"

class Shader;
class Game;

namespace AssimpPlus
{
	class SimpleManActor: public DirectXThing, public Actor
	{
	public:
		SimpleManActor(::Game* game);
		SimpleManActor(const SimpleManActor&) = delete;
		~SimpleManActor() = default;

		bool Update(float dt);
		bool Render(TexturedShader* shader) ; //make const again when debugging's done
		void Draw(AssimpPlusShader* shader);

		//need to mess with this i think
		static std::shared_ptr<SimpleManActor> 
			LoadFromFile(Game* game, const char* fName, const char* textureFilename);
		TexturedShader::Texture loadTexture (const char* textureFilename,
			ComPtr<ID3D11Device> d3dDevice, 
			ComPtr<ID3D11DeviceContext> d3dDeviceContext);

	protected:
		float rotationSpeed_;
		float rotationAngle_;

		std::vector<AssimpPlus::Mesh*> meshes_;
		std::vector<MeshComponent*> meshComponents_;
		TexturedShader::Texture texture_;
	};
}