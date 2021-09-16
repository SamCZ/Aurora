#pragma once

#include <cstdint>
#include "Aurora/Core/Common.hpp"

namespace Aurora
{
	struct SwapChainDesc
	{

	};

	AU_CLASS(ISwapChain)
	{
	public:
		virtual void Present(int syncInterval) = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		[[nodiscard]] virtual const SwapChainDesc& GetDesc() const noexcept = 0;
	};
}