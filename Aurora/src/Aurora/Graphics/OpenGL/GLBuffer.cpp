#include "GLBuffer.hpp"
#include "Aurora/Engine.hpp"
#include "GLRenderDevice.hpp"

namespace Aurora
{

	GLBuffer::GLBuffer(BufferDesc desc, GLuint handle, GLenum bindTarget, GLenum usage)
	: m_Desc(std::move(desc)), m_Handle(handle), m_BindTarget(bindTarget), m_Usage(usage)
	{

	}

	GLBuffer::~GLBuffer()
	{
		if (GEngine->GetRenderDevice())
		{
			static_cast<GLRenderDevice*>(GEngine->GetRenderDevice())->NotifyBufferDestroy(this); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
		}

		if (m_Desc.IsDMA)
		{
			glUnmapNamedBuffer(m_Handle);
		}
		glDeleteBuffers(1, &m_Handle);
	}
}