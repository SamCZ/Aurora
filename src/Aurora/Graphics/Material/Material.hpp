#pragma once
#include "MaterialBase.hpp"

namespace Aurora
{
	class Material : public MaterialBase
	{
	private:

	public:
		virtual ~Material() override = default;

		void SetShaders();

		void SetShaderVars();

		void Render();
		void RenderInstanced();
	};
}
