#pragma once

#include "../PostProcessEffect.hpp"
#include "Aurora/Graphics/Material/Material.hpp"

namespace Aurora
{
	class FXAAMaterial : public Material
	{
	public:
		CLASS_OBJ(FXAAMaterial, Material)
		FXAAMaterial();

		void BeginPass(DrawCallState& drawState, EPassType passType) const override;
	};

	class FXAAEffect : public PostProcessEffect
	{
	public:
		CLASS_OBJ(FXAAEffect, PostProcessEffect)

		void Init() override;
		void Render() override;
	};
}
