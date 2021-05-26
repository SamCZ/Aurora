#pragma once

#include "../Base/IRenderDevice.hpp"
#include <glad/glad.h>

namespace Aurora
{
	struct GLPipelineState
	{
		GLuint LastShaderHandle;
	};

	class GLRenderDevice : public IRenderDevice
	{
	private:
		GLPipelineState m_PipelineState;
	public:
		GLRenderDevice();
		~GLRenderDevice() override = default;
	public:
		void Init() override;
		// Shaders
		Shader_ptr CreateShaderProgram(const ShaderProgramDesc& desc) override;
		static GLuint CompileShaderRaw(const std::string& sourceString, const ShaderType& shaderType, std::string* errorOutput);
		void ApplyShader(const Shader_ptr& shader) override;
		//
	};
}