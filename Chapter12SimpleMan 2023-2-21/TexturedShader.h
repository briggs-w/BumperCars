//from indego code
//need to rewrite and clean up/organize

#pragma once

#include "AssimpPlusShader.h"

// This class interfaces between the logical code shown in the tutorials on YouTube, and
//  the Direct3D 11 code that I'm actually using to draw things.
// For more details, see the MaterialOnlyShader class - it is much more thoroughly commented.

namespace AssimpPlus
{
	class TexturedShader: public AssimpPlusShader
	{
	public:
		TexturedShader(ComPtr<ID3D11DeviceContext> context)	: 
			AssimpPlusShader(context), boundSRV(nullptr)
		{}
		TexturedShader(const TexturedShader&) = delete;
		~TexturedShader() = default;

		std::future<bool> Initialize(ComPtr<ID3D11Device> device);

		bool Render(const RenderCall & call);
		void SetTexture(const Texture& texture)		{			boundSRV = texture.SRV;		}

		// Bridging stuff. The implementations of these are graphics AIP dependent, but the overall
		//  structure would be very similar in OGL/VK

	protected:
		ComPtr<ID3D11ShaderResourceView> boundSRV;
	};
}

