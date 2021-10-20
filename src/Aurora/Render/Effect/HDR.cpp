#include "HDR.hpp"

namespace Aurora
{

	bool HDREffect::CanRender() const
	{
		return PostProcessEffect::CanRender();
	}

	void HDREffect::Init()
	{

	}

	void HDREffect::Render(const Texture_ptr &input, const Texture_ptr &output)
	{

	}
}