#pragma once

#include <TextureLoader.h>
#include <Image.h>
#include <nlohmann/json.hpp>

#include "Aurora/Core/Common.hpp"
#include "Aurora/Graphics/ShaderCollection.hpp"
#include "FilePackager.hpp"

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
		std::map<Path, RefCntAutoPtr<ITexture>> m_LoadedTextures;
	public:
		AssetManager();

		RefCntAutoPtr<IDataBlob> LoadFile(const Path& path);
		[[nodiscard]] bool FileExists(const Path& path) const;
		String LoadFileToString(const Path& path);

		void LoadPackageFile(const Path& path);
		ShaderCollection_ptr LoadShaders(const std::vector<ShaderLoadDesc>& shaderLoadDescriptions);
		ShaderCollection_ptr LoadShaders(const Path& shaderFolder);

		RefCntAutoPtr<ITexture> LoadTexture(const Path& path, const TextureLoadInfo& textureLoadInfo = {});

		bool LoadJson(const Path& path, nlohmann::json& json);
	private:
		static IMAGE_FILE_FORMAT CreateImageFromDataBlob(const String& filename, IDataBlob* dataBlob, Image** ppImage);
	};
}
