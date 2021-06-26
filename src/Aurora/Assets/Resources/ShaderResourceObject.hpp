#pragma once

#include "Aurora/Graphics/Base/ShaderBase.hpp"
#include "ResourceObject.hpp"

namespace Aurora
{
	typedef std::map<String, String> ShaderMacros_t;

	struct ShaderCompileState
	{
		bool Compiled;
		std::vector<std::pair<int, String>> LineErrors;
		Shader_ptr Shader;
	};

	AU_CLASS(ShaderResourceObject) : public ResourceObject
	{
	public:
		friend class AssetManager;
	private:
		String m_ShaderSource;
		EShaderType m_Type;
		Shader_ptr m_Shader;
	private:
		ShaderResourceObject(const Path& path, const EShaderType& type);
	public:
		bool Load(bool forceReload) override;
		bool Save() override;

		ShaderCompileState Compile(const String& shaderSource, const ShaderMacros_t& macros = {});
		ShaderCompileState GetOrCompile(const ShaderMacros_t& macros = {});
	public:
		const String& GetShaderSource() { return m_ShaderSource; }
		void SetShaderSource(const String& source)
		{
			if(m_ShaderSource != source) {
				m_ShaderSource = source;

				//TODO: mark shaders dirty

				m_ResourceChangedEvents.Invoke(this);
			}
		}

		[[nodiscard]] Shader_ptr GetShader() const noexcept { return m_Shader; }
		[[nodiscard]] const EShaderType& GetShaderType() const { return m_Type; }
	};
}