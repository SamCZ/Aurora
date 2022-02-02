#pragma once

#include "../PostProcessEffect.hpp"
#include "Aurora/Graphics/Material/Material.hpp"

namespace Aurora
{
	class AU_API ToneMappingMaterial : public Material
	{
	private:
		Shader_ptr m_Shader;
	public:
		friend class ToneMappingEffect;
		CLASS_OBJ(ToneMappingMaterial, Material);
		AU_CLASS_BODY(ToneMappingMaterial);
		ToneMappingMaterial();

		DEFINE_PARAM(MP_LOOKUP);

		void OnShaderReload(ResourceManager* rsm) override;
	};

	class AU_API ToneMappingEffect : public PostProcessEffect
	{
	public:
		CLASS_OBJ(ToneMappingEffect, PostProcessEffect);
		AU_CLASS_BODY(ToneMappingEffect);

		[[nodiscard]] bool CanRender() const override;

		void Init() override;
		void Render(const Texture_ptr& input, const Texture_ptr& output) override;
	};
}
