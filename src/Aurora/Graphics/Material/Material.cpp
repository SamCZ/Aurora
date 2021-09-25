#include "Material.hpp"

namespace Aurora
{

	void Material::BeginPass(DrawCallState &drawState, EPassType passType) const
	{
		drawState.Shader = GetShader(passType);
		au_assert(drawState.Shader);
	}
}