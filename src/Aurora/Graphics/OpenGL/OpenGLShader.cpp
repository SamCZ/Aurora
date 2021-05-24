#include "OpenGLShader.hpp"

namespace Aurora
{
	OpenGLShader::OpenGLShader(GLuint handle, const ShaderDesc &desc) : m_Desc(desc), m_Handle(handle), m_Resources()
	{
		m_Resources.LoadUniforms(m_Desc.Type, this);

		// TODO: Complete resources
	}

	OpenGLShader::~OpenGLShader()
	{
		if(m_Handle) {
			glDeleteProgram(m_Handle);
		}
	}

	std::vector<ShaderResourceDesc> OpenGLShader::GetResources(const ShaderResourceType& resourceType)
	{
		return std::vector<ShaderResourceDesc>();
	}
}