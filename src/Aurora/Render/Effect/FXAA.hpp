#pragma once

#include "../PostProcessEffect.hpp"
#include "Aurora/Graphics/Material/Material.hpp"

namespace Aurora
{
	class FXAAMaterial : public Material
	{
	private:
		Shader_ptr m_Shader;
	public:
		friend class FXAAEffect;
		CLASS_OBJ(FXAAMaterial, Material);
		AU_CLASS_BODY(FXAAMaterial);
		FXAAMaterial();

		void OnShaderReload(ResourceManager* rsm) override;
		void BeginPass(DrawCallState& drawState, EPassType passType) const override;
	};

	class FXAAEffect : public PostProcessEffect
	{
	public:
		CLASS_OBJ(FXAAEffect, PostProcessEffect);
		AU_CLASS_BODY(FXAAEffect);

		bool CanRender() const override;

		void Init() override;
		void Render(const Texture_ptr& input, const Texture_ptr& output) override;
	};
}
