#include "GLTexture.hpp"

#include "GLRenderDevice.hpp"

namespace Aurora
{

	GLTexture::GLTexture(TextureDesc desc, FormatMapping formatMapping, GLuint handle, GLuint srgbHandle, GLenum bindTarget, GLuint64 bindlessHandle, GLuint64 bindlessSrgbHandle)
	: m_Desc(std::move(desc)),
	m_FormatMapping(formatMapping),
	m_Handle(handle),
	m_SRGBHandle(srgbHandle),
	m_BindTarget(bindTarget),
	m_UsedInFrameBuffers(false),
	m_BindlessHandle(bindlessHandle),
	m_BindlessSrgbHandle(bindlessSrgbHandle)
	{

	}

	GLTexture::~GLTexture()
	{
		/*if(m_UsedInFrameBuffers && RD != nullptr) {
			static_cast<GLRenderDevice*>(RD)->NotifyTextureDestroy(this); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
		}*/

		if(m_Handle) {
			glDeleteTextures(1, &m_Handle);
		}

		if(m_SRGBHandle) {
			glDeleteTextures(1, &m_SRGBHandle);
		}
	}
}