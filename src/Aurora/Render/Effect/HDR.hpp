#pragma once

#include "../PostProcessEffect.hpp"
#include "Aurora/Graphics/Material/Material.hpp"

namespace Aurora
{
	class HDREffect : public PostProcessEffect
	{
	public:
		CLASS_OBJ(HDREffect, PostProcessEffect);
		AU_CLASS_BODY(HDREffect);

		[[nodiscard]] bool CanRender() const override;

		void Init() override;
		void Render(const Texture_ptr& input, const Texture_ptr& output) override;
	};
}
