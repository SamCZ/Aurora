#pragma once

#include "../Base/Buffer.hpp"
#include "GL.hpp"

namespace Aurora
{
	class AU_API GLBuffer : public IBuffer
	{
	private:
		BufferDesc m_Desc;
		GLuint m_Handle;
		GLenum m_BindTarget;
		GLenum m_Usage;
	public:
		uint8_t* m_MappedData = nullptr;

		GLBuffer(BufferDesc desc, GLuint handle, GLenum bindTarget, GLenum usage);
		~GLBuffer() override;

		[[nodiscard]] inline GLuint Handle() const noexcept { return m_Handle; }
		[[nodiscard]] inline GLenum BindTarget() const noexcept { return m_BindTarget; }
		[[nodiscard]] inline GLenum Usage() const noexcept { return m_Usage; }
		[[nodiscard]] const BufferDesc& GetDesc() const noexcept override { return m_Desc; }
	};
}