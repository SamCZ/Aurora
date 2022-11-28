#include "GLSampler.hpp"

namespace Aurora
{

	GLSampler::GLSampler(SamplerDesc desc, GLuint handle) : m_Desc(std::move(desc)), m_Handle(handle)
	{

	}

	GLSampler::~GLSampler()
	{
		glDeleteSamplers(1, &m_Handle);
	}
}