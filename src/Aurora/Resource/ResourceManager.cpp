#include "ResourceManager.hpp"

#include <fstream>
#include <regex>
#include "Aurora/Core/FileSystem.hpp"

#include <stb_image.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize.h"

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
		{
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
					if(path.filename() == "common.h")
					{
						continue;
					}

					std::string include_file = matches[1];
					Path includePath = path.parent_path() / include_file;

					// If path contains / at first character, we just absolute path
					if(include_file.starts_with('/'))
					{
						includePath = include_file.substr(1);
					}

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
		auto it = m_ShaderPrograms.find(path.string());

		if(it != m_ShaderPrograms.end()) {
			return m_ShaderPrograms[path.string()];
		}

		String shaderSource = ReadShaderSource(path);
		ShaderProgramDesc shaderProgramDesc(path.string());
		shaderProgramDesc.AddShader(EShaderType::Compute, shaderSource, macros);

		auto shaderProgram = m_RenderDevice->CreateShaderProgram(shaderProgramDesc);

		if(shaderProgram == nullptr) {
			return nullptr;
		}

		m_ShaderPrograms[path.string()] = shaderProgram;

		return shaderProgram;
	}

	Shader_ptr ResourceManager::LoadShader(const String& name, const std::map<EShaderType, Path>& shaderTypesPaths, const ShaderMacros &macros)
	{
		/*std::stringstream ss;

		for (const auto &item : shaderTypesPaths)
		{
			ss << item.second.string() << ";";
		}

		String multipath = ss.str();

		auto it = m_ShaderPrograms.find(multipath);

		if(it != m_ShaderPrograms.end()) {
			return m_ShaderPrograms[multipath];
		}*/

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

		//m_ShaderPrograms[multipath] = shaderProgram;

		AU_LOG_INFO("Loaded shader: ", name);

		return shaderProgram;
	}

	bool ResourceManager::LoadJson(const Path &path, nlohmann::json &json)
	{
		auto file = LoadFile(path);

		if(file.empty()) {
			return false;
		}

		auto* data = reinterpret_cast<uint8_t*>(file.data());
		auto dataLen = file.size();

		json = nlohmann::json::parse(data, data + dataLen);

		return true;
	}

	Texture_ptr ResourceManager::LoadTexture(const Path &path, GraphicsFormat format, const TextureLoadInfo &textureLoadInfo)
	{
		if(m_LoadedTextures.find(path) != m_LoadedTextures.end()) {
			return m_LoadedTextures[path];
		}

		auto fileData = LoadFile(path);

		if(fileData.empty()) {
			return nullptr;
		}

		Texture_ptr texture = nullptr;

		int width,height,channels_in_file;
		unsigned char *data = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(fileData.data()), fileData.size(), &width, &height, &channels_in_file, STBI_rgb_alpha);
		if(!data) {
			AU_LOG_ERROR("Cannot load texture !", path.string());
			return nullptr;
		}

		TextureDesc textureDesc;
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = textureDesc.GetMipLevelCount();
		textureDesc.ImageFormat = format;
		textureDesc.Name = path.string();
		textureDesc.UseAsBindless = textureLoadInfo.Bindless;
		texture = m_RenderDevice->CreateTexture(textureDesc, nullptr);

		for (unsigned int mipLevel = 0; mipLevel < textureDesc.MipLevels; mipLevel++)
		{
			m_RenderDevice->WriteTexture(texture, mipLevel, 0, data);

			if (mipLevel < textureDesc.MipLevels - 1u)
			{
				int newWidth = std::max<int>(1, width >> 1);
				int newHeight = std::max<int>(1, height >> 1);

				auto* resizedData = new uint8_t[newWidth * newHeight * STBI_rgb_alpha];
				stbir_resize_uint8(data, width, height, 0, resizedData, newWidth, newHeight, 0, STBI_rgb_alpha);

				stbi_image_free(data);
				data = resizedData;
				width = newWidth;
				height = newHeight;
			}
		}

		stbi_image_free(data);

		m_LoadedTextures[path] = texture;

		return texture;
	}

	Texture_ptr ResourceManager::LoadLutTexture(const Path& path)
	{
		//Path hashPath = path / ".lut";

		if(m_LoadedTextures.find(path) != m_LoadedTextures.end()) {
			return m_LoadedTextures[path];
		}

		auto fileData = LoadFile(path);

		if(fileData.empty()) {
			return nullptr;
		}

		Texture_ptr texture = nullptr;

		int width,height,channels_in_file;
		uint8_t* data = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(fileData.data()), fileData.size(), &width, &height, &channels_in_file, STBI_rgb_alpha);
		if(!data) {
			AU_LOG_ERROR("Cannot load Lut texture ! ", path.string());
			return nullptr;
		}

		int volumeWidth = width / height;
		int volumeHeight = height;
		int volumeDepth = volumeWidth;

		if(width % height > 0)
		{
			AU_LOG_ERROR("Texture needs to have depth slices multiply of height of the texture ! ", path.string());
			return nullptr;
		}

		TextureDesc textureDesc;
		textureDesc.Width = volumeWidth;
		textureDesc.Height = volumeHeight;
		textureDesc.DepthOrArraySize = volumeDepth;
		textureDesc.DimensionType = EDimensionType::TYPE_3D;
		textureDesc.MipLevels = 1;
		textureDesc.ImageFormat = GraphicsFormat::RGBA8_UNORM;
		textureDesc.Name = path.string();
		texture = m_RenderDevice->CreateTexture(textureDesc, nullptr);

		auto* slice = new uint8_t[volumeWidth * volumeHeight * 4];

		for (int d = 0; d < volumeDepth; ++d)
		{
			for (int y = 0; y < volumeHeight; ++y)
			{
				for (int x = 0; x < volumeWidth; ++x)
				{
					for (int c = 0; c < 4; ++c)
					{
						slice[(x + y * volumeWidth) * 4 + c] = data[((x + d * volumeDepth) + y * width) * 4 + c];
					}
				}
			}

			m_RenderDevice->WriteTexture(texture, 0, d, slice);
		}

		delete[] slice;

		m_LoadedTextures[path] = texture;
		return texture;
	}
}