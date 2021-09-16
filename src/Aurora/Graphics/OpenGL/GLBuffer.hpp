#pragma once

#include "../Base/Buffer.hpp"
#include "GL.hpp"

namespace Aurora
{
	class GLBuffer : public IBuffer
	{
	private:
		BufferDesc m_Desc;
		GLuint m_Handle;
		GLenum m_BindTarget;
		GLenum m_Usage;
	public:
		GLBuffer(BufferDesc desc, GLuint handle, GLenum bindTarget, GLenum usage);
		~GLBuffer() override;

		[[nodiscard]] inline GLuint Handle() const noexcept { return m_Handle; }
		[[nodiscard]] inline GLenum BindTarget() const noexcept { return m_BindTarget; }
		[[nodiscard]] const BufferDesc& GetDesc() const noexcept override { return m_Desc; }
	};
}