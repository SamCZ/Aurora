#pragma once

#include <TextureLoader.h>
#include <Image.h>
#include <nlohmann/json.hpp>
#include <Aurora/Assets/Resources/ShaderResourceObject.hpp>


#include "Aurora/Core/Common.hpp"
#include "Aurora/Graphics/ShaderCollection.hpp"
#include "FilePackager.hpp"

#include "Resources/ShaderResourceObject.hpp"

using namespace Diligent;

namespace Aurora
{
	struct ShaderLoadDesc
	{
		SHADER_TYPE ShaderType;
		SHADER_SOURCE_LANGUAGE SourceLanguage;
		Path FilePath;
		String Source;
		ShaderMacro Macros;
	};

	AU_CLASS(AssetManager)
	{
	private:
		std::map<Path, RefCntAutoPtr<IShader>> m_LoadedShaders;
		std::map<Path, std::pair<Path, FileHeader>> m_AssetPackageFiles;
		std::map<Path, std::vector<Path>> m_AssetPackageFolders;
		std::map<Path, RefCntAutoPtr<ITexture>> m_LoadedTextures;

		std::map<Path, ShaderResourceObject_ptr> m_ShaderResources;
	public:
		AssetManager();

		RefCntAutoPtr<IDataBlob> LoadFile(const Path& path, bool* isFromAssetPackage = nullptr);
		[[nodiscard]] bool FileExists(const Path& path, bool* isFromAssetPackage = nullptr) const;
		String LoadFileToString(const Path& path, bool* isFromAssetPackage = nullptr);

		void LoadPackageFile(const Path& path);
		ShaderCollection_ptr LoadShaders(const std::vector<ShaderLoadDesc>& shaderLoadDescriptions, const std::map<String, String>& macros = {});
		ShaderCollection_ptr LoadShaders(const Path& shaderFolder, const std::map<String, String>& macros = {});

		RefCntAutoPtr<IShader> LoadShader(const Path& path, const SHADER_TYPE& type, const std::map<String, String>& macros = {});
		const ShaderResourceObject_ptr& LoadShaderResource(const Path& path, const SHADER_SOURCE_LANGUAGE& shaderSourceLanguage, const SHADER_TYPE& type);
		std::vector<ShaderResourceObject_ptr> LoadShaderResourceFolder(const Path& path);

		RefCntAutoPtr<ITexture> LoadTexture(const Path& path, const TextureLoadInfo& textureLoadInfo = {});
		RefCntAutoPtr<ITexture> LoadTexture(const String& filename, IDataBlob* fileData, const TextureLoadInfo& textureLoadInfo = {});

		bool LoadJson(const Path& path, nlohmann::json& json);
	private:
		static IMAGE_FILE_FORMAT CreateImageFromDataBlob(const String& filename, IDataBlob* dataBlob, Image** ppImage);

		std::vector<Path> ListFiles(const Path &path);
	};
}
