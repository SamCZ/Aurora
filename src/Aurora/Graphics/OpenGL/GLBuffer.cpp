#include "GLBuffer.hpp"

namespace Aurora
{

	GLBuffer::GLBuffer(BufferDesc desc, GLuint handle, GLenum bindTarget, GLenum usage)
	: m_Desc(std::move(desc)), m_Handle(handle), m_BindTarget(bindTarget), m_Usage(usage)
	{

	}

	GLBuffer::~GLBuffer()
	{
		if(m_Desc.IsDMA)
		{
			GLint lastBuffer = 0;
			glGetIntegerv(GL_BUFFER_BINDING, &lastBuffer);

			glBindBuffer(m_BindTarget, m_Handle);
			glUnmapBuffer(m_BindTarget);

			glBindBuffer(m_BindTarget, lastBuffer);
		}
		glDeleteBuffers(1, &m_Handle);
	}
}