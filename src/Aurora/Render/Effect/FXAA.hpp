#pragma once

#include "../PostProcessEffect.hpp"

namespace Aurora
{
	class FXAAEffect : public PostProcessEffect
	{
	public:
		CLASS_OBJ(FXAAEffect, PostProcessEffect)

		void Init() override;
		void Render() override;
	};
}
