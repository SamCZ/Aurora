#include "GLTexture.hpp"

#include "GLRenderDevice.hpp"
#include "Aurora/AuroraEngine.hpp"

namespace Aurora
{

	GLTexture::GLTexture(TextureDesc desc, FormatMapping formatMapping, GLuint handle, GLuint srgbHandle, GLenum bindTarget)
	: m_Desc(std::move(desc)), m_FormatMapping(formatMapping), m_Handle(handle), m_SRGBHandle(srgbHandle), m_BindTarget(bindTarget), m_UsedInFrameBuffers(false)
	{

	}

	GLTexture::~GLTexture()
	{
		if(m_UsedInFrameBuffers) {
			static_cast<GLRenderDevice*>(RD)->NotifyTextureDestroy(this); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
		}

		if(m_Handle) {
			glDeleteTextures(1, &m_Handle);
		}

		if(m_SRGBHandle) {
			glDeleteTextures(1, &m_SRGBHandle);
		}
	}
}