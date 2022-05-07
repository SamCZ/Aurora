#pragma once

#include "../Base/ShaderBase.hpp"
#include "Aurora/Core/Hash.hpp"
#include "Aurora/Tools/robin_hood.h"
#include "GL.hpp"
#include "GLShaderResources.hpp"

namespace Aurora
{
	class AU_API GLShaderProgram : public IShaderProgram
	{
	private:
		const ShaderProgramDesc m_Desc;
		const GLuint m_Handle;
		GLShaderResources m_Resources;
		bool m_HasInputLayout : 1;
		ShaderInputVariables_t m_InputVariables;

		std::vector<ShaderResourceDesc> m_ConstantBufferDescriptorCache;
		bool m_ConstantBufferDescriptorCacheInitialized;

		std::vector<ShaderResourceDesc> m_SamplerDescriptorCache;
		bool m_SamplerDescriptorCacheInitialized;

		robin_hood::unordered_map<TTypeID, GLint> m_UniformLocationCache;
	public:
		GLShaderProgram(GLuint handle, ShaderProgramDesc desc);
		~GLShaderProgram();
	public:
		[[nodiscard]] const ShaderProgramDesc& GetDesc() const override { return m_Desc; }
		[[nodiscard]] std::vector<ShaderResourceDesc> GetResources(const ShaderResourceType& resourceType) override;

		[[nodiscard]] inline bool HasInputLayout() const noexcept override { return m_HasInputLayout; }
		[[nodiscard]] inline uint8_t GetInputVariablesCount() const noexcept override { return m_InputVariables.size(); }
		[[nodiscard]] inline const ShaderInputVariables_t& GetInputVariables() const noexcept override { return m_InputVariables; }
	public:
		[[nodiscard]] GLuint Handle() const noexcept { return m_Handle; }
		[[nodiscard]] const GLShaderResources& GetGLResource() const { return m_Resources; }

		GLint GetUniformLocation(TTypeID nameID) const;
		inline GLint GetUniformLocation(const std::string& name) const { return GetUniformLocation(Hash_djb2(name.c_str())); }
	};
}
