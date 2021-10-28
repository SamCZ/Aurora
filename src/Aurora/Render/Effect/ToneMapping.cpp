#include "ToneMapping.hpp"

#include "Aurora/Engine.hpp"
#include "Aurora/Resource/ResourceManager.hpp"

namespace Aurora
{
	DECLARE_PARAM(ToneMappingMaterial, MP_LOOKUP);

	ToneMappingMaterial::ToneMappingMaterial()
	{
		MP_LOOKUP = CreateTextureParam("Lookup texture", GetEngine()->GetResourceManager()->LoadLutTexture("Assets/Textures/default_lut.png"));
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

		const Texture_ptr& lutTexture = mat->GetParamTexture(ToneMappingMaterial::MP_LOOKUP);
		const TextureDesc& lutDesc = lutTexture->GetDesc();

		DrawCallState drawState = PrepareState(mat->m_Shader);
		drawState.ViewPort = output->GetDesc().GetSize();

		drawState.BindTexture("SceneTarget", input);
		drawState.BindSampler("SceneTarget", Samplers::ClampClampNearestNearest);

		drawState.BindTexture("LutTarget", lutTexture);
		drawState.BindSampler("LutTarget", Samplers::ClampClampClampLinearLinearLinear);
		drawState.BindTarget(0, output);

		float scale = (float)(lutDesc.Width - 1) / (float)lutDesc.Width;
		float offset = 0.5f / (float)lutDesc.Width;

		BEGIN_UB(Vector4, desc)
			*desc = Vector4(scale, offset, 0, 0);
		END_UB(ToneMappingDesc)

		RenderState(drawState);
	}
}