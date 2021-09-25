#include "ResourceManager.hpp"

#include <fstream>
#include <regex>
#include "Aurora/Core/FileSystem.hpp"

namespace Aurora
{

	ResourceManager::ResourceManager(IRenderDevice* renderDevice) : m_RenderDevice(renderDevice)
	{

	}

	void ResourceManager::AddFileSearchPath(const String &path)
	{
		m_FileSearchPaths.emplace_back(path);
	}

	void ResourceManager::LoadPackageFile(const Path& path)
	{
		auto map = AssetBank::ReadHeadersFromPackage(path);

		for(auto& it : map) {
			if(m_AssetPackageFiles.find(it.first) != m_AssetPackageFiles.end()) {
				std::cerr << "Found duplicated file: " << it.first << " in package:" << path << std::endl;
				continue;
			}

			m_AssetPackageFiles[it.first] = std::pair<Path, ABankHeader>(path, it.second);

			Path fileFolder = it.first.parent_path();

			m_AssetPackageFolders[fileFolder].push_back(it.first);
		}
	}

	String ResourceManager::LoadFileToString(const Path& path, bool* isFromAssetPackage)
	{
		auto blob = LoadFile(path, isFromAssetPackage);
		const char* str = reinterpret_cast<const char*>(blob.data());
		return std::string(str, str + blob.size());
	}

	DataBlob ResourceManager::LoadFile(const Path& path, bool* isFromAssetPackage)
	{
		if(m_AssetPackageFiles.find(path) != m_AssetPackageFiles.end()) {
			if(isFromAssetPackage != nullptr) {
				*isFromAssetPackage = true;
			}

			auto& packageDesc = m_AssetPackageFiles[path];
			return AssetBank::ReadFileFromPackage(packageDesc.first, packageDesc.second);
		}

		if(isFromAssetPackage != nullptr) {
			*isFromAssetPackage = false;
		}

		for (const auto &item : m_FileSearchPaths)
		{
			Path newPath = item / path;

			if(FS::FileExists(newPath))
			{
				return FS::LoadFile(newPath);
			}
		}

		return FS::LoadFile(path);
	}

	bool ResourceManager::FileExists(const Path& path, bool* isFromAssetPackage) const
	{
		if(m_AssetPackageFiles.find(path) != m_AssetPackageFiles.end()) {
			if(isFromAssetPackage != nullptr) {
				*isFromAssetPackage = true;
			}
			return true;
		}

		if(isFromAssetPackage != nullptr) {
			*isFromAssetPackage = false;
		}

		for (const auto &item : m_FileSearchPaths)
		{
			Path newPath = item / path;

			if(FS::FileExists(newPath))
			{
				return true;
			}
		}

		return FS::FileExists(path);
	}

	String ResourceManager::ReadShaderSource(const Path &path, std::vector<Path>& alreadyIncluded)
	{
		String shaderSource = LoadFileToString(path);
		if(path.filename() != "common.h") {
			static const std::regex re("^[ ]*#[ ]*include[ ]+[\"<](.*)[\">].*");

			std::stringstream input;
			std::stringstream output;
			input << shaderSource;

			std::smatch matches;

			std::string line;
			while(std::getline(input,line))
			{
				if (std::regex_search(line, matches, re))
				{
					std::string include_file = matches[1];
					Path includePath = path.parent_path() / include_file;

					if(std::find(alreadyIncluded.begin(), alreadyIncluded.end(), includePath.filename()) != alreadyIncluded.end())
					{
						continue;
					}

					if(!FileExists(includePath))
					{
						AU_LOG_FATAL("File " , includePath.string(), " not found !");
					}

					//std::cout << "including " << includePath.string() << " (" << includePath.filename() << ") " << " in " << path.string() << std::endl;

					alreadyIncluded.push_back(includePath.filename());
					output << ReadShaderSource(includePath, alreadyIncluded) << std::endl;
				} else {
					output << line << std::endl;
				}
			}

			shaderSource = output.str();
		}

		return shaderSource;
	}

	Shader_ptr ResourceManager::LoadComputeShader(const Path &path, const ShaderMacros &macros)
	{
		auto it = m_ShaderPrograms.find(path);

		if(it != m_ShaderPrograms.end()) {
			return m_ShaderPrograms[path];
		}

		String shaderSource = ReadShaderSource(path);
		ShaderProgramDesc shaderProgramDesc(path.string());
		shaderProgramDesc.AddShader(EShaderType::Compute, shaderSource, macros);

		auto shaderProgram = m_RenderDevice->CreateShaderProgram(shaderProgramDesc);

		if(shaderProgram == nullptr) {
			return nullptr;
		}

		m_ShaderPrograms[path] = shaderProgram;

		return shaderProgram;
	}

	Shader_ptr ResourceManager::LoadShader(const String& name, const std::map<EShaderType, Path>& shaderTypesPaths, const ShaderMacros &macros)
	{
		std::string multipath = "";

		for (const auto &item : shaderTypesPaths)
		{
			multipath += item.second.string() + ";";
		}

		multipath = multipath.substr(multipath.length() - 1);

		auto it = m_ShaderPrograms.find(multipath);

		if(it != m_ShaderPrograms.end()) {
			return m_ShaderPrograms[multipath];
		}

		ShaderProgramDesc shaderProgramDesc(name);

		for (const auto &item : shaderTypesPaths)
		{
			if(item.first == EShaderType::Compute && shaderTypesPaths.size() > 1)
			{
				AU_LOG_FATAL("You cannot mix compute shader with others !");
			}

			Path filePath = item.second;

			if(!FileExists(filePath))
			{
				AU_LOG_FATAL("File " , filePath.string(), " not found !");
			}

			String shaderSource = ReadShaderSource(filePath);
			shaderProgramDesc.AddShader(item.first, shaderSource, macros);
		}

		auto shaderProgram = m_RenderDevice->CreateShaderProgram(shaderProgramDesc);

		if(shaderProgram == nullptr) {
			return nullptr;
		}

		m_ShaderPrograms[multipath] = shaderProgram;

		return shaderProgram;
	}
}