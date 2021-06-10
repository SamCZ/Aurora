#include "GLBuffer.hpp"

namespace Aurora
{

	GLBuffer::GLBuffer(BufferDesc desc, GLuint handle, GLenum bindTarget, GLenum usage)
	: m_Desc(std::move(desc)), m_Handle(handle), m_BindTarget(bindTarget), m_Usage(usage)
	{

	}

	GLBuffer::~GLBuffer()
	{
		glDeleteBuffers(1, &m_Handle);
	}
}