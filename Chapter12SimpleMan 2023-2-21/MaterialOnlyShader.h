#pragma once

// IMPORTANT! If you're reading this, pay attention to the comments! The code is carefully
//  separated SPATIALLY according to if it's DirectX stuff or general shader stuff.
// There's a TON of boilerplate code here, and some of it is more complicated than it needs to be.
// The stuff near the top is what you should be concerned with

// This class forms a bridge between the logic and rendering.
//  The method that will be used externally is just the "Render" method,
//  which will VERY CAREFULLY only take in data that is useful
//  across DirectX 9/10/11, OpenGL 2/3/4, and Vulkan

// This shader specifically is for use objects that only have material properties defined.
// Each vertex will have:
// - Position
// - Normal
// Each object (draw call) will have:
// - Camera position
// - Material (diffuse, specular, ambient colors)

#include "AssimpPlusShader.h"

namespace AssimpPlus
{
	class MaterialOnlyShader: public AssimpPlusShader
	{
	public:
		MaterialOnlyShader(ComPtr<ID3D11DeviceContext> context): AssimpPlusShader(context) {}
		MaterialOnlyShader(const MaterialOnlyShader&) = delete;
		~MaterialOnlyShader() = default;

		std::future<bool> Initialize(ComPtr<ID3D11Device> device);

		bool Render(const RenderCall & call);		// Bridge stuff. The implementation here is graphics API dependent, but the
		//  creation and external use of it isn't.
	};

}

