#include "ToneMapping.hpp"

#include "Aurora/Engine.hpp"
#include "Aurora/Resource/ResourceManager.hpp"

namespace Aurora
{
	DECLARE_PARAM(ToneMappingMaterial, MP_LOOKUP);

	ToneMappingMaterial::ToneMappingMaterial()
	{
		MP_LOOKUP = CreateTextureParam("Lookup texture", GetEngine()->GetResourceManager()->LoadTexture("Assets/Textures/tone_base.png", GraphicsFormat::SRGBA8_UNORM, {}));
	}

	void ToneMappingMaterial::OnShaderReload(ResourceManager *rsm)
	{
		m_Shader = rsm->LoadShader("FXAA", {
			{EShaderType::Vertex, "Assets/Shaders/fs_quad.vss"},
			{EShaderType::Pixel, "Assets/Shaders/PostProcess/tone_mapping.fss"},
		});
	}

	bool ToneMappingEffect::CanRender() const
	{
		bool canRender = PostProcessEffect::CanRender();

		if(canRender && GetMaterial() != nullptr && GetMaterial()->GetTypeID() == ToneMappingMaterial::TypeID())
		{
			return true;
		}

		return false;
	}

	void ToneMappingEffect::Init()
	{

	}

	void ToneMappingEffect::Render(const Texture_ptr &input, const Texture_ptr &output)
	{
		auto mat = ToneMappingMaterial::SafeCast(GetMaterial());

		DrawCallState drawState = PrepareState(mat->m_Shader);
		drawState.ViewPort = output->GetDesc().GetSize();

		drawState.BindTexture("SceneTarget", input);
		drawState.BindSampler("SceneTarget", Samplers::ClampClampNearestNearest);

		drawState.BindTexture("LutTarget", mat->GetParamTexture(ToneMappingMaterial::MP_LOOKUP));
		drawState.BindSampler("LutTarget", Samplers::ClampClampNearestNearest);

		drawState.BindTarget(0, output);

		RenderState(drawState);
	}
}