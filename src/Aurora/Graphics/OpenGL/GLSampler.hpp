#pragma once

#include "../Base/Sampler.hpp"
#include "GL.hpp"

namespace Aurora
{
	class GLSampler : public ISampler
	{
	private:
		SamplerDesc m_Desc;
		GLuint m_Handle;
	public:
		GLSampler(SamplerDesc desc, GLuint handle);
		~GLSampler() override;

		[[nodiscard]] inline GLuint Handle() const noexcept { return m_Handle; }
		[[nodiscard]] const SamplerDesc& GetDesc() const noexcept override { return m_Desc; }
	};
}