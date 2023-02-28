#pragma once

#include "Transform.h"
#include <vector>
#include <memory>

#include "MaterialOnlyShader.h"
#include "DirectXThing.h"
#include "Actor.h"
#include "MeshComponent.h"

class Shader;
class Game;

namespace AssimpPlus
{
	class RoadActor: public DirectXThing, public Actor
	{
	public:
		RoadActor(class ::Game* game);
		RoadActor(const RoadActor&) = delete;
		~RoadActor() = default;

		bool Update(float dt);
		bool Render(MaterialOnlyShader* shader) const;
		void Draw(Shader* shader);

		static std::shared_ptr<RoadActor> LoadFromFile(Game* game, const char* fName);

		ComPtr<ID3D11DeviceContext> context_;
		std::vector<AssimpPlus::Mesh*>	meshes_;
		std::vector<MeshComponent*> meshComponents_;
	};

	//inline void SetRoadModelRotation(RoadActor& m, const ::Quaternion& q) { m.SetRotation(q); }
};