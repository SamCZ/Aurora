#include "AssetManager.hpp"

#include <BasicFileSystem.hpp>
#include <BasicFileStream.hpp>
#include <StringTools.hpp>
#include <ShaderMacroHelper.hpp>

#include "Aurora/AuroraEngine.hpp"
#include "Aurora/Core/FileSystem.hpp"

#include <fstream>
#include <regex>

namespace Aurora
{
	AssetManager::AssetManager() = default;

	void AssetManager::LoadPackageFile(const Path& path)
	{
		auto map = FilePackager::ReadHeadersFromPackage(path);

		for(auto& it : map) {
			if(m_AssetPackageFiles.find(it.first) != m_AssetPackageFiles.end()) {
				std::cerr << "Found duplicated file: " << it.first << " in package:" << path << std::endl;
				continue;
			}

			m_AssetPackageFiles[it.first] = std::pair<Path, FileHeader>(path, it.second);

			Path fileFolder = it.first.parent_path();

			m_AssetPackageFolders[fileFolder].push_back(it.first);
		}
	}

	ShaderCollection_ptr AssetManager::LoadShaders(const std::vector<ShaderLoadDesc>& shaderLoadDescriptions, const std::map<String, String>& macros)
	{
		ShaderCollection_ptr collection = std::make_shared<ShaderCollection>();

		for(auto& desc : shaderLoadDescriptions) {
			RefCntAutoPtr<IShader> shader;

			if(m_LoadedShaders.find(desc.FilePath) != m_LoadedShaders.end() && desc.Source.length() == 0 && macros.empty()) {
				shader = m_LoadedShaders[desc.FilePath];
			} else {
				if(!FileExists(desc.FilePath) && desc.Source.length() == 0) {
					std::cerr << "Shader not found: " << desc.FilePath << std::endl;
					continue;
				}
				ShaderCreateInfo ShaderCI = {};
				ShaderCI.SourceLanguage = desc.SourceLanguage;
				ShaderCI.UseCombinedTextureSamplers = true;
				ShaderCI.Desc.ShaderType = desc.ShaderType;
				ShaderCI.EntryPoint      = "main";
				ShaderCI.Desc.Name       = "...";

				ShaderMacroHelper Macros;
				for(auto& it : macros) {
					Macros.AddShaderMacro(it.first.c_str(), it.second);
				}

				ShaderCI.Macros = Macros;

				String source;
				if(desc.Source.length() > 0) {
					ShaderCI.Source          = desc.Source.c_str();
				} else {
					source = LoadFileToString(desc.FilePath);
					ShaderCI.Source          = source.c_str();
				}

				AuroraEngine::RenderDevice->CreateShader(ShaderCI, &shader);

				if(desc.Source.length() == 0) {
					m_LoadedShaders[desc.FilePath] = shader;
				}
			}

			switch (desc.ShaderType) {
				case SHADER_TYPE_VERTEX:
					collection->Vertex = shader;
					break;
				case SHADER_TYPE_PIXEL:
					collection->Pixel = shader;
					break;
				case SHADER_TYPE_GEOMETRY:
					collection->Geometry = shader;
					break;
				case SHADER_TYPE_HULL:
					collection->Hull = shader;
					break;
				case SHADER_TYPE_DOMAIN:
					collection->Domain = shader;
					break;
				case SHADER_TYPE_AMPLIFICATION:
					collection->Amplification = shader;
					break;
				case SHADER_TYPE_MESH:
					collection->Mesh = shader;
					break;
				default:
					break;
			}
		}

		return collection;
	}

	ShaderCollection_ptr AssetManager::LoadShaders(const Path &shaderFolder, const std::map<String, String>& macros)
	{
		std::vector<ShaderLoadDesc> descriptors;

		Path vertex = shaderFolder / "vertex.glsl";
		Path fragment = shaderFolder / "fragment.glsl";

		if(FileExists(vertex)) {
			descriptors.push_back({SHADER_TYPE_VERTEX, SHADER_SOURCE_LANGUAGE_GLSL, "", LoadFileToString(vertex), {}});
		}

		if(FileExists(fragment)) {
			descriptors.push_back({SHADER_TYPE_PIXEL, SHADER_SOURCE_LANGUAGE_GLSL, "", LoadFileToString(fragment), {}});
		}

		if(descriptors.empty()) {
			std::cerr << "Could not find any shader in " << shaderFolder << std::endl;
			exit(1);
		}

		return LoadShaders(descriptors, macros);
	}

	RefCntAutoPtr<IShader> AssetManager::LoadShader(const Path& path, const SHADER_TYPE& type, const std::map<String, String>& macros)
	{
		RefCntAutoPtr<IShader> shader;

		SHADER_SOURCE_LANGUAGE language = SHADER_SOURCE_LANGUAGE_DEFAULT;

		String extension = path.extension().string();

		if(extension == ".glsl") {
			language = SHADER_SOURCE_LANGUAGE_GLSL;
		} else if(extension == ".hlsl") {
			language = SHADER_SOURCE_LANGUAGE_HLSL;
		} else {
			AU_LOG_ERROR("Unknown shader extension ", extension, " for ", path);
		}

		IDataBlob* outputLog = nullptr;

		ShaderCreateInfo ShaderCI = {};
		ShaderCI.SourceLanguage = language;
		ShaderCI.UseCombinedTextureSamplers = true;
		ShaderCI.Desc.ShaderType = type;
		ShaderCI.EntryPoint      = "main";
		ShaderCI.Desc.Name       = path.string().c_str();
		String source = LoadFileToString(path);
		ShaderCI.Source          = source.c_str();
		ShaderCI.ppCompilerOutput = &outputLog;

		ShaderMacroHelper Macros;
		for(auto& it : macros) {
			Macros.AddShaderMacro(it.first.c_str(), it.second);
		}

		ShaderCI.Macros = Macros;

		AuroraEngine::RenderDevice->CreateShader(ShaderCI, &shader);

		if(outputLog != nullptr) {

		}

		return shader;
	}

	String AssetManager::LoadFileToString(const Path& path, bool* isFromAssetPackage)
	{
		auto blob = LoadFile(path, isFromAssetPackage);
		const char* str = reinterpret_cast<const char*>(blob->GetConstDataPtr());
		return std::string(str, str + blob->GetSize());
	}

	RefCntAutoPtr<IDataBlob> AssetManager::LoadFile(const Path& path, bool* isFromAssetPackage)
	{
		if(m_AssetPackageFiles.find(path) != m_AssetPackageFiles.end()) {
			if(isFromAssetPackage != nullptr) {
				*isFromAssetPackage = true;
			}

			auto& packageDesc = m_AssetPackageFiles[path];
			return FilePackager::ReadFileFromPackage(packageDesc.first, packageDesc.second);
		}

		if(isFromAssetPackage != nullptr) {
			*isFromAssetPackage = false;
		}

		/*auto blob = FS::LoadFile(path);

		RefCntAutoPtr<IDataBlob> pFileData(MakeNewRCObj<DataBlobImpl>()(blob.size()));
		memcpy(pFileData->GetDataPtr(), blob.data(), blob.size());*/

		RefCntAutoPtr<BasicFileStream> pFileStream(MakeNewRCObj<BasicFileStream>()((const Char*)path.string().c_str(), EFileAccessMode::Read));
		if (!pFileStream->IsValid())
			LOG_ERROR_AND_THROW("Failed to open file \"", path, '\"');

		RefCntAutoPtr<IDataBlob> pFileData(MakeNewRCObj<DataBlobImpl>()(0));
		pFileStream->ReadBlob(pFileData);

		return pFileData;
	}

	bool AssetManager::FileExists(const Path& path, bool* isFromAssetPackage) const
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

		return FS::FileExists(path);
	}

	RefCntAutoPtr<ITexture> AssetManager::LoadTexture(const Path& path, const TextureLoadInfo& textureLoadInfo)
	{
		if(m_LoadedTextures.find(path) != m_LoadedTextures.end()) {
			return m_LoadedTextures[path];
		}

		auto fileData = LoadFile(path);

		if(fileData == nullptr) {
			return RefCntAutoPtr<ITexture>(nullptr);
		}

		RefCntAutoPtr<ITexture> ppTexture;
		RefCntAutoPtr<Image> pImage;

		auto ImgFmt = CreateImageFromDataBlob(path.filename().string(), fileData, &pImage);

		if (pImage)
			CreateTextureFromImage(pImage, textureLoadInfo, AuroraEngine::RenderDevice, &ppTexture);
		else if (fileData)
		{
			if (ImgFmt == IMAGE_FILE_FORMAT_DDS)
				CreateTextureFromDDS(fileData, textureLoadInfo, AuroraEngine::RenderDevice,  &ppTexture);
			else if (ImgFmt == IMAGE_FILE_FORMAT_KTX)
				CreateTextureFromKTX(fileData, textureLoadInfo, AuroraEngine::RenderDevice,  &ppTexture);
			else
				UNEXPECTED("Unexpected format");
		}

		m_LoadedTextures[path] = ppTexture;

		return ppTexture;
	}

	RefCntAutoPtr<ITexture> AssetManager::LoadTexture(const String& filename, IDataBlob* fileData, const TextureLoadInfo &textureLoadInfo)
	{
		if(fileData == nullptr) {
			return RefCntAutoPtr<ITexture>(nullptr);
		}

		RefCntAutoPtr<ITexture> ppTexture;
		RefCntAutoPtr<Image> pImage;

		auto ImgFmt = CreateImageFromDataBlob(filename, fileData, &pImage);

		if (pImage)
			CreateTextureFromImage(pImage, textureLoadInfo, AuroraEngine::RenderDevice, &ppTexture);
		else if (fileData)
		{
			if (ImgFmt == IMAGE_FILE_FORMAT_DDS)
				CreateTextureFromDDS(fileData, textureLoadInfo, AuroraEngine::RenderDevice,  &ppTexture);
			else if (ImgFmt == IMAGE_FILE_FORMAT_KTX)
				CreateTextureFromKTX(fileData, textureLoadInfo, AuroraEngine::RenderDevice,  &ppTexture);
			else
				UNEXPECTED("Unexpected format");
		}

		return ppTexture;
	}

	IMAGE_FILE_FORMAT AssetManager::CreateImageFromDataBlob(const String& filename, IDataBlob* pFileData, Image** ppImage)
	{
		auto ImgFileFormat = IMAGE_FILE_FORMAT_UNKNOWN;

		try
		{
			ImgFileFormat = Image::GetFileFormat(reinterpret_cast<Uint8*>(pFileData->GetDataPtr()), pFileData->GetSize());
			if (ImgFileFormat == IMAGE_FILE_FORMAT_UNKNOWN)
			{
				//LOG_WARNING_MESSAGE("Unable to derive image format from the header for file \"", filename, "\". Trying to analyze extension.");

				// Try to use extension to derive format
				auto* pDotPos = strrchr(filename.c_str(), '.');
				/*if (pDotPos == nullptr)
					LOG_ERROR_AND_THROW("Unable to recognize file format: file name \"", filename, "\" does not contain extension");*/

				auto* pExtension = pDotPos + 1;
				/*if (*pExtension == 0)
					LOG_ERROR_AND_THROW("Unable to recognize file format: file name \"", filename, "\" contain empty extension");*/

				String Extension = StrToLower(pExtension);
				if (Extension == "png")
					ImgFileFormat = IMAGE_FILE_FORMAT_PNG;
				else if (Extension == "jpeg" || Extension == "jpg")
					ImgFileFormat = IMAGE_FILE_FORMAT_JPEG;
				else if (Extension == "tiff" || Extension == "tif")
					ImgFileFormat = IMAGE_FILE_FORMAT_TIFF;
				else if (Extension == "dds")
					ImgFileFormat = IMAGE_FILE_FORMAT_DDS;
				else if (Extension == "ktx")
					ImgFileFormat = IMAGE_FILE_FORMAT_KTX;
				else if (Extension == "tga")
					ImgFileFormat = IMAGE_FILE_FORMAT_TGA;
				else
					LOG_ERROR_AND_THROW("Unsupported file format ", Extension);
			}

			if (ImgFileFormat == IMAGE_FILE_FORMAT_PNG ||
				ImgFileFormat == IMAGE_FILE_FORMAT_JPEG ||
				ImgFileFormat == IMAGE_FILE_FORMAT_TIFF ||
				ImgFileFormat == IMAGE_FILE_FORMAT_TGA)
			{
				ImageLoadInfo ImgLoadInfo;
				ImgLoadInfo.Format = ImgFileFormat;
				Image::CreateFromDataBlob(pFileData, ImgLoadInfo, ppImage);
			}
		}
		catch (std::runtime_error& err)
		{
			LOG_ERROR("Failed to create image from file: ", err.what());
		}

		return ImgFileFormat;
	}

	bool AssetManager::LoadJson(const Path &path, nlohmann::json &json)
	{
		auto file = LoadFile(path);

		if(file == nullptr) {
			return false;
		}

		auto* data = reinterpret_cast<uint8_t*>(file->GetDataPtr());
		auto dataLen = file->GetSize();

		json = nlohmann::json::parse(data, data + dataLen);

		return true;
	}

	const ShaderResourceObject_ptr &AssetManager::LoadShaderResource(const Path &path, const SHADER_SOURCE_LANGUAGE& shaderSourceLanguage, const SHADER_TYPE &type)
	{
		if(m_ShaderResources.find(path) != m_ShaderResources.end()) {
			return m_ShaderResources[path];
		} else {
			m_ShaderResources[path] = ShaderResourceObject_ptr(new ShaderResourceObject(path, shaderSourceLanguage, type));
			auto& res = m_ShaderResources[path];
			res->Load(false);
			return res;
		}
	}

	static std::map<String, SHADER_TYPE> ShaderFileTypeNames = { // NOLINT(cert-err58-cpp)
			{"vertex", SHADER_TYPE_VERTEX},
			{"fragment", SHADER_TYPE_PIXEL},
			{"geometry", SHADER_TYPE_GEOMETRY},
			{"hull", SHADER_TYPE_HULL},
			{"domain", SHADER_TYPE_DOMAIN},
			{"compute", SHADER_TYPE_COMPUTE}
	};

	static std::map<String, SHADER_SOURCE_LANGUAGE> ShaderLanguageFileExt = { // NOLINT(cert-err58-cpp)
			{".glsl", SHADER_SOURCE_LANGUAGE_GLSL},
			{".hlsl", SHADER_SOURCE_LANGUAGE_HLSL}
	};

	std::vector<ShaderResourceObject_ptr> AssetManager::LoadShaderResourceFolder(const Path &path)
	{
		std::vector<ShaderResourceObject_ptr> shaders;

		for(auto& filePath: ListFiles(path)) {
			auto extension = filePath.extension().string();

			if(extension == ".disabled") continue;

			// Find shader language by file extension
			auto extTypeIter = ShaderLanguageFileExt.find(extension);

			if(extTypeIter == ShaderLanguageFileExt.end()) {
				std::cerr << "Unknown shader extension " << extension << std::endl;
				continue;
			}

			auto shaderLang = ShaderLanguageFileExt[extension];

			// Find shader type by filename

			auto filenameWithoutExtension = filePath.stem().string();
			auto shaderTypeIter = ShaderFileTypeNames.find(filenameWithoutExtension);

			if(shaderTypeIter == ShaderFileTypeNames.end()) {
				std::cerr << "Unknown shader type " << filenameWithoutExtension << std::endl;
				continue;
			}

			auto shaderType = ShaderFileTypeNames[filenameWithoutExtension];

			// Load resource
			shaders.push_back(LoadShaderResource(filePath, shaderLang, shaderType));
		}

		return shaders;
	}

	std::vector<Path> AssetManager::ListFiles(const Path &path)
	{
		std::vector<Path> files;

		if(m_AssetPackageFolders.contains(path)) {
			for(const auto& file : m_AssetPackageFolders[path]) {
				files.push_back(file);
			}
		} else {
			for(auto& file: std::filesystem::directory_iterator(path)) {
				files.push_back(file.path());
			}
		}

		return files;
	}
}