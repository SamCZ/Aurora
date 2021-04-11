#pragma once

#include <Shader.h>
#include <RefCntAutoPtr.hpp>
#include "ResourceObject.hpp"

using namespace Diligent;

namespace Aurora
{
	typedef std::map<String, String> ShaderMacros_t;

	struct ShaderCompileState
	{
		bool Compiled;
		std::vector<std::pair<int, String>> LineErrors;
		RefCntAutoPtr<IShader> Shader;
	};

	AU_CLASS(ShaderResourceObject) : public ResourceObject
	{
	public:
		friend class AssetManager;
	private:
		String m_ShaderSource;
		SHADER_SOURCE_LANGUAGE m_SourceLanguage;
		SHADER_TYPE m_Type;
		RefCntAutoPtr<IShader> m_Shader;
	private:
		ShaderResourceObject(const Path& path, const SHADER_SOURCE_LANGUAGE& shaderSourceLanguage, const SHADER_TYPE& type);
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

		[[nodiscard]] const RefCntAutoPtr<IShader>& GetShader() const noexcept { return m_Shader; }
		[[nodiscard]] const SHADER_TYPE& GetShaderType() const { return m_Type; }
	};
}