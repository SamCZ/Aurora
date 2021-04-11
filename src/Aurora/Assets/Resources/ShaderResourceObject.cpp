#include "ShaderResourceObject.hpp"

#include <fstream>
#include <regex>

#include <ShaderMacroHelper.hpp>

#include "../AssetManager.hpp"
#include "../../AuroraEngine.hpp"

namespace Aurora
{
	ShaderResourceObject::ShaderResourceObject(const Path &path, const SHADER_SOURCE_LANGUAGE& shaderSourceLanguage, const SHADER_TYPE &type)
	: ResourceObject(path, false), m_SourceLanguage(shaderSourceLanguage), m_Type(type), m_Shader(nullptr)
	{

	}

	bool ShaderResourceObject::Load(bool forceReload)
	{
		if(m_IsLoaded && !forceReload) {
			return true;
		}

		auto data = AuroraEngine::AssetManager->LoadFile(GetPath(), &m_FromAssetPackage);

		if(data == nullptr) {
			m_IsLoaded = false;
			throw std::runtime_error("Cannot load shader: " + m_Path.string());
			return false;
		}

		const char* str = reinterpret_cast<const char*>(data->GetConstDataPtr());
		m_ShaderSource = String(str, str + data->GetSize());

		if(m_IsLoaded) {
			m_ResourceChangedEvents.Invoke(this);
		}

		m_IsLoaded = true;
		return true;
	}

	bool ShaderResourceObject::Save()
	{
		if(m_FromAssetPackage) {
			std::cout << "Cannot save " << m_Path << " because it was loaded from asset package !" << std::endl;
			return false;
		}

		std::ofstream out(m_Path);

		if(out.is_open()) {
			out << m_ShaderSource;
			out.close();
			return true;
		} else {
			return false;
		}
	}

	int CountLines(const String& str)
	{
		int lineCount = 0;

		for (int i = 0; i < str.length(); ++i) {
			if(i != str.length() - 1 && str[i] == '\r' && str[i + 1] == '\n') {
				lineCount++;
				i++;
			} else if(str[i] == '\n') {
				lineCount++;
			}
		}

		return lineCount;
	}

	ShaderCompileState ShaderResourceObject::Compile(const String& shaderSource, const ShaderMacros_t& macros)
	{
		ShaderCompileState compileState = {};
		compileState.Shader = nullptr;

		ShaderCreateInfo ShaderCI = {};
		ShaderCI.SourceLanguage = m_SourceLanguage;
		ShaderCI.UseCombinedTextureSamplers = true;
		ShaderCI.Desc.ShaderType = m_Type;
		ShaderCI.EntryPoint      = "main";
		ShaderCI.Desc.Name       = m_Path.string().c_str();

		ShaderMacroHelper Macros;
		for(auto& it : macros) {
			Macros.AddShaderMacro(it.first.c_str(), it.second);
		}

		ShaderCI.Macros = Macros;
		ShaderCI.Source = shaderSource.c_str();

		IDataBlob* outputLog = nullptr;
		ShaderCI.ppCompilerOutput = &outputLog;

		RefCntAutoPtr<IShader> shader;
		AuroraEngine::RenderDevice->CreateShader(ShaderCI, &shader);

		if(outputLog != nullptr) {
			const char* str = reinterpret_cast<const char*>(outputLog->GetConstDataPtr());
			auto out = std::string(str, str + outputLog->GetSize());

			int lineOffset = 0;

			auto shaderStringOffset = String::npos;

			// Find line offset
			for (int i = 0; i < out.length(); ++i) {
				if(i == 0) continue;

				if(out[i] == '\n' && out[i - 1] == '\n') {
					shaderStringOffset = i + 1;
					break;
				}
			}

			// Count final shader string line numbers
			if(shaderStringOffset != String::npos) {
				String finalShaderString = out.substr(shaderStringOffset);
				lineOffset = CountLines(finalShaderString) - CountLines(shaderSource);
			}

			// Find errors
			std::regex findErrRegex("ERROR: (.*)[^\\r\\n]*");
			std::smatch matches;

			String::const_iterator searchStart(out.cbegin());
			while(std::regex_search(searchStart, out.cend(), matches, findErrRegex))
			{
				searchStart = matches.suffix().first;
				String error_string = matches[1];

				auto splitIndex = error_string.find(": ");

				if(splitIndex == std::string::npos) {
					continue;
				}

				String first = error_string.substr(0, splitIndex);
				String lineMessage = error_string.substr(splitIndex + 2);

				auto startFirstIndex = first.find("0:");

				if(startFirstIndex == std::string::npos) {
					throw std::runtime_error("Find unknown state in error!");
				}

				int lineNumber = std::stoi(first.substr(startFirstIndex + 2));

				lineNumber -= lineOffset;

				compileState.LineErrors.emplace_back(lineNumber, lineMessage);
				compileState.Compiled = false;
			}
		} else {
			compileState.Compiled = true;
			compileState.Shader = shader;
		}

		return compileState;
	}

	ShaderCompileState ShaderResourceObject::GetOrCompile(const ShaderMacros_t &macros)
	{
		ShaderCompileState compileState = {};

		if(false) {

		} else {
			compileState = Compile(m_ShaderSource, macros);
		}

		return compileState;
	}
}