#pragma once

#include "../Base/ShaderBase.hpp"
#include <glad/glad.h>
#include "GLShaderResources.hpp"

namespace Aurora
{
	class OpenGLShader : public ShaderBase
	{
	private:
		const ShaderDesc m_Desc;
		const GLuint m_Handle;
		GLShaderResources m_Resources;
	public:
		OpenGLShader(GLuint handle, const ShaderDesc& desc);
		~OpenGLShader();
	public:
		[[nodiscard]] const ShaderDesc& GetDesc() const override { return m_Desc; }
		[[nodiscard]] std::vector<ShaderResourceDesc> GetResources(const ShaderResourceType& resourceType) override;
	public:
		[[nodiscard]] GLuint Handle() const noexcept { return m_Handle; }
		[[nodiscard]] const GLShaderResources& GetGLResource() const { return m_Resources; }
	};
}
