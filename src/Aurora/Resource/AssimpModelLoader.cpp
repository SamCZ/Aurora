#include "AssimpModelLoader.hpp"

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Aurora/Logger/Logger.hpp"

namespace Aurora
{
	void AssimpModelLoader::ImportModel(const String &name, const DataBlob &data)
	{
		const aiScene* scene = m_Importer.ReadFileFromMemory(data.data(), data.size(), aiProcessPreset_TargetRealtime_Fast | aiProcess_EmbedTextures);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			AU_LOG_ERROR("ModelImporter::Import", m_Importer.GetErrorString());
			return;
		}

		AU_LOG_INFO("Has animations = ", scene->HasAnimations() ? "Yes" : "no");

		m_Importer.FreeScene();
	}
}