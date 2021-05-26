#pragma once

#include "ShaderBase.hpp"

namespace Aurora
{
	class IRenderDevice
	{
	public:
		IRenderDevice() = default;
		IRenderDevice(const IRenderDevice& other) = delete;
		virtual ~IRenderDevice() = default;
	public:
		virtual void Init() = 0;

		virtual Shader_ptr CreateShaderProgram(const ShaderProgramDesc& desc) = 0;
		virtual void ApplyShader(const Shader_ptr& shader) = 0;
	};
}