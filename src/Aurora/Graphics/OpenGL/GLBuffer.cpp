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
			glUnmapNamedBuffer(m_Handle);
		}
		glDeleteBuffers(1, &m_Handle);
	}
}