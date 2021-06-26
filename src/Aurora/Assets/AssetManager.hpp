#pragma once

#include <nlohmann/json.hpp>
#include <Aurora/Assets/Resources/ShaderResourceObject.hpp>


#include "Aurora/Core/Common.hpp"
#include "Aurora/Graphics/ShaderCollection.hpp"
#include "Aurora/Graphics/Base/IRenderDevice.hpp"
#include "FilePackager.hpp"

#include "Resources/ShaderResourceObject.hpp"

namespace Aurora
{
	struct TextureLoadInfo
	{

	};

	struct ShaderLoadDesc
	{
		EShaderType ShaderType;
		Path FilePath;
		String Source;
		ShaderMacros Macros;
	};

	AU_CLASS(AssetManager)
	{
	private:
		std::map<Path, Shader_ptr> m_LoadedShaders;
		std::map<Path, std::pair<Path, FileHeader>> m_AssetPackageFiles;
		std::map<Path, std::vector<Path>> m_AssetPackageFolders;
		std::map<Path, Texture_ptr> m_LoadedTextures;

		std::map<Path, ShaderResourceObject_ptr> m_ShaderResources;
		std::map<Path, Shader_ptr> m_ShaderPrograms;
	public:
		AssetManager();

		DataBlob LoadFile(const Path& path, bool* isFromAssetPackage = nullptr);
		[[nodiscard]] bool FileExists(const Path& path, bool* isFromAssetPackage = nullptr) const;
		String LoadFileToString(const Path& path, bool* isFromAssetPackage = nullptr);

		void LoadPackageFile(const Path& path);

		/*const ShaderResourceObject_ptr& LoadShaderResource(const Path& path, const ShaderType& type);
		std::vector<ShaderResourceObject_ptr> LoadShaderResourceFolder(const Path& path);*/

		Shader_ptr LoadShaderFolder(const Path& path, const ShaderMacros& macros = {});
		Shader_ptr LoadComputeShader(const Path& path, const ShaderMacros& macros = {});

		Texture_ptr LoadTexture(const Path& path, const TextureLoadInfo& textureLoadInfo = {});
		Texture_ptr LoadTexture(const String& filename, const DataBlob& fileData, const TextureLoadInfo& textureLoadInfo = {});

		bool LoadJson(const Path& path, nlohmann::json& json);
	private:
		//static IMAGE_FILE_FORMAT CreateImageFromDataBlob(const String& filename, IDataBlob* dataBlob, Image** ppImage);

		std::vector<Path> ListFiles(const Path &path);
	};
}
