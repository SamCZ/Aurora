#pragma once

#include <string>

namespace Aurora
{
	class GLRenderGroupScope
	{
	public:
		explicit GLRenderGroupScope(const char* name);
		explicit GLRenderGroupScope(const std::string& name);
		~GLRenderGroupScope();
	};
}