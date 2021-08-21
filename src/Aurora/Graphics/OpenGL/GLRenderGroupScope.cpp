#include "GLRenderGroupScope.hpp"
#include "GL.hpp"
#include <cstring>

namespace Aurora
{

	GLRenderGroupScope::GLRenderGroupScope(const char *name)
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, static_cast<GLsizei>(strlen(name)), name);
	}

	GLRenderGroupScope::GLRenderGroupScope(const std::string &name)
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, static_cast<GLsizei>(name.length()), name.c_str());
	}

	GLRenderGroupScope::~GLRenderGroupScope()
	{
		glPopDebugGroup();
	}
}