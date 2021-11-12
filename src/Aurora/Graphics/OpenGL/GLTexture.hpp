#pragma once

#include "GL.hpp"

#include "../Base/Texture.hpp"
#include "GLFormatMapping.hpp"

namespace Aurora
{
	class GLTexture : public ITexture
	{
	public:
		friend class GLRenderDevice;
	private:
		TextureDesc m_Desc;
		FormatMapping m_FormatMapping;
		GLuint m_Handle;
		GLuint m_SRGBHandle;

		GLuint64 m_BindlessHandle;
		GLuint64 m_BindlessSrgbHandle;

		GLenum m_BindTarget;
		bool m_UsedInFrameBuffers;
	public:
		GLTexture(TextureDesc desc, FormatMapping formatMapping, GLuint handle, GLuint srgbHandle, GLenum bindTarget, GLuint64 bindlessHandle, GLuint64 bindlessSrgbHandle);
		~GLTexture() override;

		[[nodiscard]] inline const FormatMapping& Format() const noexcept { return m_FormatMapping; }
		[[nodiscard]] inline GLuint Handle() const noexcept { return m_Handle; }
		[[nodiscard]] inline GLuint SRGBHandle() const noexcept { return m_SRGBHandle; }
		[[nodiscard]] inline GLenum BindTarget() const noexcept { return m_BindTarget; }
	public:
		[[nodiscard]] inline const TextureDesc& GetDesc() const override { return m_Desc; }
	};
}