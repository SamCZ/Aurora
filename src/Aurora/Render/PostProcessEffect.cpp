#include "PostProcessEffect.hpp"

namespace Aurora
{

	/*TemporalRenderTarget& PostProcessEffect::CopyCurrentRT(uint index) const
	{
		return <#initializer#>;
	}*/
	bool PostProcessEffect::CanRender() const
	{
		return Enabled();
	}
}