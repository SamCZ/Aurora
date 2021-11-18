#pragma once

#include "../PostProcessEffect.hpp"
#include "Aurora/Graphics/Material/Material.hpp"

namespace Aurora
{
	class SSAOEffect : public PostProcessEffect
	{
	public:
		CLASS_OBJ(SSAOEffect, PostProcessEffect);
		AU_CLASS_BODY(SSAOEffect);

		bool CanRender() const override;

		void Init() override;
		void Render(const Texture_ptr& input, const Texture_ptr& output) override;
	};
}
