#pragma once

#include <string>
#include "Aurora/Core/Library.hpp"

namespace Aurora
{
	class AU_API GLRenderGroupScope
	{
	public:
		explicit GLRenderGroupScope(const char* name);
		explicit GLRenderGroupScope(const std::string& name);
		~GLRenderGroupScope();
	};
}