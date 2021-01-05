#include "AssetManager.hpp"
#include <BasicFileSystem.hpp>
#include <BasicFileStream.hpp>
#include <StringTools.hpp>
#include "../AuroraEngine.hpp"

namespace Aurora
{
    FAssetManager::FAssetManager()
    {

    }

    void FAssetManager::LoadPackageFile(const Path& path)
    {
        auto map = FFilePackager::ReadHeadersFromPackage(path);

        for(auto& it : map) {
            if(m_AssetPackageFiles.find(it.first) != m_AssetPackageFiles.end()) {
                std::cerr << "Found duplicated file: " << it.first << " in package:" << path << std::endl;
                continue;
            }

            m_AssetPackageFiles[it.first] = Pair<Path, FileHeader>(path, it.second);
        }
    }

    FShaderCollectionPtr FAssetManager::LoadShaders(const List<ShaderLoadDesc>& shaderLoadDescriptions)
    {
        FShaderCollectionPtr collection = New(FShaderCollection);

        for(auto& desc : shaderLoadDescriptions) {
            RefCntAutoPtr<IShader> shader;

            if(m_LoadedShaders.find(desc.FilePath) != m_LoadedShaders.end() && desc.Source.length() == 0) {
                shader = m_LoadedShaders[desc.FilePath];
            } else {
                if(!FileExists(desc.FilePath) && desc.Source.length() == 0) {
                    std::cerr << "Shader not found: " << desc.FilePath << std::endl;
                    continue;
                }

                ShaderCreateInfo ShaderCI;
                ShaderCI.SourceLanguage = desc.SourceLanguage;
                ShaderCI.UseCombinedTextureSamplers = true;
                ShaderCI.Desc.ShaderType = desc.ShaderType;
                ShaderCI.EntryPoint      = "main";
                ShaderCI.Desc.Name       = desc.FilePath.string().c_str();

                if(desc.Source.length() > 0) {
                    ShaderCI.Source          = desc.Source.c_str();
                } else {
                    ShaderCI.Source          = LoadFileToString(desc.FilePath).c_str();
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

    String FAssetManager::LoadFileToString(const Path& path)
    {
        auto blob = LoadFile(path);
        return String(reinterpret_cast<char*>(blob->GetDataPtr()), blob->GetSize());
    }

    RefCntAutoPtr<IDataBlob> FAssetManager::LoadFile(const Path& path)
    {
        if(m_AssetPackageFiles.find(path) != m_AssetPackageFiles.end()) {
            auto& packageDesc = m_AssetPackageFiles[path];
            return FFilePackager::ReadFileFromPackage(packageDesc.first, packageDesc.second);
        }

        RefCntAutoPtr<BasicFileStream> pFileStream(MakeNewRCObj<BasicFileStream>()((const Char*)path.string().c_str(), EFileAccessMode::Read));
        if (!pFileStream->IsValid())
            LOG_ERROR_AND_THROW("Failed to open image file \"", path, '\"');

        RefCntAutoPtr<IDataBlob> pFileData(MakeNewRCObj<DataBlobImpl>()(0));
        pFileStream->ReadBlob(pFileData);

        return pFileData;
    }

    bool FAssetManager::FileExists(const Path& path) const
    {
        if(m_AssetPackageFiles.find(path) != m_AssetPackageFiles.end()) {
            return true;
        }

        return FS::FileExists(path);
    }

    RefCntAutoPtr<ITexture> FAssetManager::LoadTexture(const Path& path, const TextureLoadInfo& textureLoadInfo)
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

        return ppTexture;
    }

    IMAGE_FILE_FORMAT FAssetManager::CreateImageFromDataBlob(const String& filename, IDataBlob* pFileData, Image** ppImage)
    {
        auto ImgFileFormat = IMAGE_FILE_FORMAT_UNKNOWN;

        try
        {
            ImgFileFormat = Image::GetFileFormat(reinterpret_cast<Uint8*>(pFileData->GetDataPtr()), pFileData->GetSize());
            if (ImgFileFormat == IMAGE_FILE_FORMAT_UNKNOWN)
            {
                LOG_WARNING_MESSAGE("Unable to derive image format from the header for file \"", filename, "\". Trying to analyze extension.");

                // Try to use extension to derive format
                auto* pDotPos = strrchr(filename.c_str(), '.');
                if (pDotPos == nullptr)
                    LOG_ERROR_AND_THROW("Unable to recognize file format: file name \"", filename, "\" does not contain extension");

                auto* pExtension = pDotPos + 1;
                if (*pExtension == 0)
                    LOG_ERROR_AND_THROW("Unable to recognize file format: file name \"", filename, "\" contain empty extension");

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
                else
                    LOG_ERROR_AND_THROW("Unsupported file format ", Extension);
            }

            if (ImgFileFormat == IMAGE_FILE_FORMAT_PNG ||
                ImgFileFormat == IMAGE_FILE_FORMAT_JPEG ||
                ImgFileFormat == IMAGE_FILE_FORMAT_TIFF)
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
}