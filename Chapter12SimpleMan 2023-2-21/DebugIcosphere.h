#pragma once

// Debug icosphere. Does not load froma file, simply shows how to render stuff.
#include "MaterialOnlyShader.h"

#include "Transform.h"

#include <d3d11.h>
#include <wrl.h>
#include <memory>
#include "DirectXThing.h"
#include "Actor.h"
#include "Collision.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"

using Microsoft::WRL::ComPtr;

class Shader;
class Mesh;

namespace AssimpPlus
{
	class IcosphereActor: public DirectXThing, public Actor
	{
	public:
		IcosphereActor(::Game* game);
		IcosphereActor(const IcosphereActor&) = delete;
		~IcosphereActor() = default;

		bool Update(float dt);
		bool Render(MaterialOnlyShader* shader) const;
		//void Draw (Shader* shader);
		
		void CreateMesh();

	protected:
		ComPtr<ID3D11DeviceContext> context_;
		std::shared_ptr<RenderCall> call_;
		AssimpPlus::Material material_;

		float rotationSpeed_;
		float rotationAngle_;

		::Mesh* mMesh = nullptr;
	};

}

