#pragma once
//2022-5-3 This is to take things common to TexturedShader and MaterialOnlyShader and
// put 'em to gether, for simplicity
//Will Briggs

#include <d3d11.h>
#include <wrl.h>
#include <future>
#include <vector>
#include "Renderer.h" //for DirectionalLight
#include "math.h"
#include "Shader.h"

using Microsoft::WRL::ComPtr;

namespace AssimpPlus
{
	// Data structures used by this shader (GRAPIHCS API AGNOSTIC)
	struct Vertex
	{
		Vector3 Position;
		float __one;
		Vector3 Normal;
		float __zero;
		float U; // These could be put in a Vec2 (and will be in the shader itself), but it would
		float V; //  only be for grouping. I'll just leave them separate like this.
		//WSB: U and V are only needed for the TexturedShader, but let's be consistent

	public:
		Vertex(const Vector3& pos, const Vector3& norm, float u = 0.0f, float v = 0.0f)
			: Position(pos), __one(1.f), Normal(norm), __zero(0.f), U(u), V(v)
		{}
	};

	// Input of the render function - contains D3D11 buffers storing geometry data
	class RenderCall
	{
	public:
		RenderCall(ComPtr<ID3D11Device> device, const std::vector<Vertex>& vertices, const std::vector<std::uint32_t>& indices);
		//without passed dx11 device
		RenderCall(const std::vector<Vertex>& vertices, const std::vector<std::uint32_t>& indices);

	public:
		ComPtr<ID3D11Buffer> VertexBuffer;
		ComPtr<ID3D11Buffer> IndexBuffer;
		std::uint32_t NumberOfIndices;
	};

	// Material color will be multiplied with texture color for ambient and diffuse color, specular
	//  color will be applied as is. There are other approaches that could be taken, but I think this
	//  one finds a good balance between accurate and cheap.
	// This tutorial is about loading/using textures, not about advanced rendering techniques :-)

		// Both lights and materials have three color components - ambient, specular, diffuse
		// "Ambient" light/color refers to what color a surface has when it is not exposed directly
		//  to light from the object. Think of an overcast day - everything in this case is lit by
		//  ambient lighting alone. Things in shadow are also lit by ambient lights alone.
		// "Diffuse" light/color refers to the color when a light hits a surface, illuminating it.
		//  Most full-on light is visible as diffuse light.
		// "Specular" light/color refers to the shiny spots on things like water and metallic surfaces
		//  When you look across a lake in the sunset, there's super bright white spots that appear
		//  where the reflection of the sun is. Those are lit primarily by specular light.
		// For further information, read up on the Phong lighting model.

	struct Material
	{
		class Color
		{
		public:
			Color(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f) : _r(r), _g(g), _b(b), _a(a) {}
			Color(const Color&) = default;
			Color(const Vector3& v) : _r(v.x), _g(v.y), _b(v.z), _a(1.0f) {}

			float a() const { return _a; }
			float r() const { return _r; }
			float g() const { return _g; }
			float b() const { return _b; }

		protected:
			float _r, _g, _b, _a;
		};

		Material(const Color& specular, const Color& diffuse, const Color& ambient)
			: Specular(specular), Diffuse(diffuse), Ambient(ambient)
		{}
		Color Specular;
		Color Diffuse;
		Color Ambient;
	};

	struct DirectionalLight
	{
		//When I can, dump this ctor: DirectionalLight should ONLY be used inside
		//AssimpPlusShader
		DirectionalLight(const Vector3& direction, const Material::Color& specular, const Material::Color& diffuse, const Material::Color& ambient)
			: Direction(direction), __pad(0.f), Specular(specular), Diffuse(diffuse), Ambient(ambient)
		{}
		DirectionalLight(const ::DirectionalLight& light) :
			Direction(light.mDirection), __pad(0.f), Specular(light.mSpecColor), 
			Diffuse(light.mDiffuseColor),
			Ambient (light.mDiffuseColor) //mAmbient is not specified in Madhav, so I'll guess
		{}

		Vector3 Direction;
		float __pad; // To keep 16-byte aligned-ness of constant buffer
		Material::Color Specular;
		Material::Color Diffuse;
		Material::Color Ambient;
	};

	class Mesh
	{
	public:
		Mesh(RenderCall& c, AssimpPlus::Material& m) : Call(c), Material(m) {}
		RenderCall Call;
		AssimpPlus::Material Material;
	};

	class AssimpPlusShader: public Shader
	{
	public:
		AssimpPlusShader(ComPtr<ID3D11DeviceContext> context);

		// Calls used by this shader to set shader variables (GRAPHICS API AGNOSTIC)
		//  If you want to change any of these values between render calls, these functions
		//  must be called. The render call itself has no notion of any of these things.
		// That also helps in optimizations - DX11/GL/VK all have optimizations that can be
		//  applied by using this sort of pattern. Different optimizations, but optimizations.

		void setContext(ComPtr<ID3D11DeviceContext> context) { context_ = context; }

		void SetModelTransform(const Matrix4& modelTransformation)
		{
			DVSC_PerObject.VSC_PerObject.Model = modelTransformation;
			DVSC_PerObject.isDirty = true;
		}

		void SetViewTransform(const Matrix4& viewTransform)
		{
			DVSC_PerFrame.VSC_PerFrame.View = viewTransform.Transpose();
			DVSC_PerFrame.isDirty = true;
		}

		void SetProjectionTransform(const Matrix4& projTransform)
		{
			DVSC_PerFrame.VSC_PerFrame.Proj = projTransform.Transpose();
			DVSC_PerFrame.isDirty = true;
		}

		void SetObjectMaterial(const Material& objectMaterial)
		{
			DPSC_PerObject.PSC_PerObject.ObjectMaterial = objectMaterial;
			DPSC_PerObject.isDirty = true;
		}

		void SetCameraPosition(const Vector3& cameraPosition)
		{
			DPSC_PerFrame.PSC_PerFrame.CameraPosition = cameraPosition;
			DPSC_PerFrame.PSC_PerFrame.__extra = 1.f;
			DPSC_PerFrame.isDirty = true;
		}

		void SetSunLight(const ::DirectionalLight& light)
		{
			DPSC_PerScene.PSC_PerScene.SunLight = light;
			DPSC_PerScene.isDirty = true;
		}

		//Texture definition is below - I want to keep this file organized
					   //  (1) graphics API agnostic, (2) implementation specific to D3D11,
					   //  (3) architecture and implementation specific to D3D11
					   // This class belongs in (2), but it is used by a call in (1)

					   // Wrapper around D3D11 texture -- used only by TexturedShader, but let's be consistent
		class Texture
		{
		public:
			Texture(ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context, const std::vector<unsigned char>& rawData, std::uint32_t width, std::uint32_t height);
			//removed device pass
			Texture(const std::vector<unsigned char>& rawData, std::uint32_t width, std::uint32_t height);
			Texture(const Texture&) = default;
			~Texture() = default;

			ComPtr<ID3D11Texture2D> Buffer;
			ComPtr<ID3D11ShaderResourceView> SRV;
		};

	protected:
		ComPtr<ID3D11DeviceContext> context_;

		// Members held by the shader. DirectX 11 specific, but also similar in OpenGL
		ComPtr<ID3D11VertexShader> vertexShader_;
		ComPtr<ID3D11PixelShader> pixelShader_;
		ComPtr<ID3D11InputLayout> inputLayout_;

		// D3D11 constant buffers. These buffers are where shader global variables are held,
		//  like the model/view/proj transformations. Since forming a new transfer to the
		//  graphics card is relatively expensive, it's good to group this data based on
		//  frequency of update and location.
		// In this case, the vertex shader requires information...
		//  - Per object: Model transformation
		//  - Per frame: View transformation, projection transformation (proj could also be per scene)
		// The fragment shader requires...
		//  - Per object: Material
		//  - Per frame: Camera position
		//  - Per scene: Sunlight position
		ComPtr<ID3D11Buffer> vsc_object_;
		ComPtr<ID3D11Buffer> vsc_frame_;
		ComPtr<ID3D11Buffer> psc_object_;
		ComPtr<ID3D11Buffer> psc_frame_;
		ComPtr<ID3D11Buffer> psc_scene_;

		// DirectX 11 specific optimizations. Similar ones could be used in OpenGL as well.
//  I don't actually know Vulkan well enough to comment on if it could be used there
		struct DVSC_PerObjectType // Constant buffer for the vertex shader, per object
		{
			DVSC_PerObjectType(const Matrix4& m) : VSC_PerObject(m), isDirty(true) {}
			struct VSC_PerObject_Type
			{
				VSC_PerObject_Type(const Matrix4& m) : Model(m) {}
				Matrix4 Model;
			} VSC_PerObject;
			bool isDirty;
		};

		DVSC_PerObjectType DVSC_PerObject;

		struct DVSC_PerFrameType // Constant buffer for the vertex shader, per frame
		{
			DVSC_PerFrameType(const Matrix4& v, const Matrix4& p) : VSC_PerFrame(v, p), isDirty(true) {}
			struct VSC_PerFrame_Type
			{
				VSC_PerFrame_Type(const Matrix4& v, const Matrix4& p) : View(v), Proj(p) {}
				Matrix4 View;
				Matrix4 Proj;
			} VSC_PerFrame;
			bool isDirty;
		};

		DVSC_PerFrameType DVSC_PerFrame;

		struct DPSC_PerObjectType // Constant buffer for the pixel shader, per object
		{
			DPSC_PerObjectType(const Material& m) : PSC_PerObject(m), isDirty(true) {}
			struct PSC_PerObject_Type
			{
				PSC_PerObject_Type(const Material& m) : ObjectMaterial(m) {}
				Material ObjectMaterial;
			} PSC_PerObject;
			bool isDirty;
		};

		DPSC_PerObjectType DPSC_PerObject;

		struct DPSC_PerFrameType // Constant buffer for the pixel shader, per frame
		{
			DPSC_PerFrameType(const Vector3& v, float e) : PSC_PerFrame(v, e), isDirty(true) {}
			struct PSC_PerFrame_Type
			{
				PSC_PerFrame_Type(const Vector3& v, float e) : CameraPosition(v), __extra(e) {}
				Vector3 CameraPosition;
				float __extra;
			} PSC_PerFrame;
			bool isDirty;
		};

		DPSC_PerFrameType DPSC_PerFrame;

		struct DPSC_PerSceneType // Constant buffer for the pixel shader, per scene
		{
			DPSC_PerSceneType(const ::DirectionalLight& light) : PSC_PerScene(light), isDirty(true) {}
			struct PSC_PerScene_Type
			{
				PSC_PerScene_Type(const ::DirectionalLight& light) : SunLight(light) {}

				DirectionalLight SunLight;
			} PSC_PerScene;
			bool isDirty;
		};

		DPSC_PerSceneType DPSC_PerScene;
	};
}
