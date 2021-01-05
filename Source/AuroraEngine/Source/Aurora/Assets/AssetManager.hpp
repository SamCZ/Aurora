#pragma once

#include <Aurora/Core/FileSystem.hpp>
#include <Aurora/Core/Container.hpp>
#include <Aurora/Core/FilePackager.hpp>
#include "../Render/ShaderCollection.hpp"

#include <TextureLoader.h>
#include <Image.h>

using namespace Diligent;
using namespace Aurora::Render;

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

    class FAssetManager
    {
    private:
        Map<Path, RefCntAutoPtr<IShader>> m_LoadedShaders;
        Map<Path, Pair<Path, FileHeader>> m_AssetPackageFiles;
        Map<Path, RefCntAutoPtr<ITexture>> m_LoadedTextures;
    public:
        FAssetManager();

        RefCntAutoPtr<IDataBlob> LoadFile(const Path& path);
        [[nodiscard]] bool FileExists(const Path& path) const;
        String LoadFileToString(const Path& path);

        void LoadPackageFile(const Path& path);
        FShaderCollectionPtr LoadShaders(const List<ShaderLoadDesc>& shaderLoadDescriptions);

        RefCntAutoPtr<ITexture> LoadTexture(const Path& path, const TextureLoadInfo& textureLoadInfo = {});
    private:
        static IMAGE_FILE_FORMAT CreateImageFromDataBlob(const String& filename, IDataBlob* dataBlob, Image** ppImage);
    };
}