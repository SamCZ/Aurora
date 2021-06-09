#pragma once

#include <cstdint>

#include "GL.hpp"

namespace Aurora
{
	class GLContextState
	{
	private:
		uint32_t m_PendingMemoryBarriers = 0;
	public:
		void Invalidate();
	};
}