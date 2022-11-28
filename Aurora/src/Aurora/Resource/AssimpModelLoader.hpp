#pragma once

#include <assimp/Importer.hpp>

#include "Aurora/Core/Types.hpp"
#include "Aurora/Core/String.hpp"
#include "Aurora/Core/Library.hpp"

#include "Aurora/Framework/Mesh/Mesh.hpp"

struct aiTexture;
struct aiMaterial;
struct aiScene;

namespace Aurora
{
	struct MeshImportOptions
	{
		bool SplitMeshes = false;
		bool MergeSameMaterialMeshes = false;
		bool PreTransform = true;
		bool KeepCPUData = false;
		bool UploadToGPU = true;
		float DefaultScale = 1.0f;
	};

	struct MeshImportedData
	{
		bool Imported = false;
		Mesh_ptr Mesh = nullptr;
		std::vector<Mesh_ptr> Meshes = {};

		explicit operator bool() const
		{
			return Imported;
		}

		bool operator==(bool other) const
		{
			return Imported == other;
		}

		template<class T>
		std::shared_ptr<T> Get(uint32_t index = 0)
		{
			return T::SafeCast(Meshes[index]);
		}
	};

	class AU_API AssimpModelLoader
	{
	private:
		Assimp::Importer m_Importer;
		std::unordered_map<const aiTexture*, Texture_ptr> m_TextureCache;
	public:
		AssimpModelLoader();

		MeshImportedData ImportModel(const String& name, const DataBlob& data, MeshImportOptions importOptions = {});
		bool ImportAnimation(const DataBlob& data, SkeletalMesh_ptr& skeletalMesh);

	public:
		void LoadMaterial(const aiScene* scene, const Mesh_ptr& mesh, int32_t materialIndex, const aiMaterial* sourceMaterial);
	};
}
