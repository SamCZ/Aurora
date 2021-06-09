#include "GLTexture.hpp"

namespace Aurora
{

	GLTexture::GLTexture(TextureDesc desc, FormatMapping formatMapping, GLuint handle, GLuint srgbView, GLenum bindTarget)
	: m_Desc(std::move(desc)), m_FormatMapping(formatMapping), m_Handle(handle), m_SrgbView(srgbView), m_BindTarget(bindTarget), m_UsedInFrameBuffers(false)
	{

	}

	GLTexture::~GLTexture()
	{
		if(m_Handle) {
			glDeleteTextures(1, &m_Handle);
		}

		if(m_SrgbView) {
			glDeleteTextures(1, &m_SrgbView);
		}
	}
}