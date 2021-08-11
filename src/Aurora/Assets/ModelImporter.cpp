#include "ModelImporter.hpp"

#include <Aurora/Physics/BoundingBox.hpp>
#include <Aurora/AuroraEngine.hpp>

namespace Aurora
{

	static std::vector<std::string> TextureTypeEnumToString = {
			"aiTextureType_NONE", "aiTextureType_DIFFUSE", "aiTextureType_SPECULAR",
			"aiTextureType_AMBIENT", "aiTextureType_EMISSIVE", "aiTextureType_HEIGHT",
			"aiTextureType_NORMALS", "aiTextureType_SHININESS", "aiTextureType_OPACITY",
			"aiTextureType_DISPLACEMENT", "aiTextureType_LIGHTMAP", "aiTextureType_REFLECTION",
			"aiTextureType_UNKNOWN", "_aiTextureType_Force32Bit" };

	static std::vector<std::string> TextureTypeEnumToShaderNameString = {
			"aiTextureType_NONE", "AlbedoMap", "SpecularMap",
			"aiTextureType_AMBIENT", "EmissionMap", "HeightMap",
			"NormalMap", "aiTextureType_SHININESS", "OpacityMap",
			"DisplacementMap", "LightMap", "ReflectionMap",
			"aiTextureType_UNKNOWN", "_aiTextureType_Force32Bit" };

	static std::map<uint8_t, String> PrimitiveTypeToString {
			{(uint8_t)aiPrimitiveType::aiPrimitiveType_POINT, "aiPrimitiveType_POINT"},
			{(uint8_t)aiPrimitiveType::aiPrimitiveType_LINE, "aiPrimitiveType_LINE"},
			{(uint8_t)aiPrimitiveType::aiPrimitiveType_TRIANGLE, "aiPrimitiveType_TRIANGLE"},
			{(uint8_t)aiPrimitiveType::aiPrimitiveType_POLYGON, "aiPrimitiveType_POLYGON"},
			{(uint8_t)aiPrimitiveType::_aiPrimitiveType_Force32Bit, "_aiPrimitiveType_Force32Bit"}
	};

	static inline Vector4 color_cast(const aiColor4D &v) { return glm::vec4(v.r, v.g, v.b, v.a); }
	static inline Vector3 vec3_cast(const aiVector3D &v) { return glm::vec3(v.x, v.y, v.z); }
	static inline Vector2 vec2_cast(const aiVector3D &v) { return glm::vec2(v.x, v.y); }
	static inline Quaternion quat_cast(const aiQuaternion &q) { return glm::quat(q.w, q.x, q.y, q.z); }
	static inline Matrix4 mat4_cast(const aiMatrix4x4 &m) { return glm::transpose(glm::make_mat4(&m.a1)); }
	static inline Matrix4 mat4_cast(const aiMatrix3x3 &m) { return glm::transpose(glm::make_mat3(&m.a1)); }

	static constexpr uint32_t MESH_PROCESS_FLAGS = aiProcessPreset_TargetRealtime_Quality |
												   aiProcess_ValidateDataStructure |
												   aiProcess_OptimizeMeshes |
												   aiProcess_FlipUVs;

	template<typename MeshProcessor, typename MeshType>
	static void ProcessNodes(MeshProcessor meshProcessor, MeshType* engineMesh, const aiScene* scene, aiNode* node, int& materialSlotIndex, BoundingBox& boundingBox, std::map<String, aiTexture*>& embeddedTextures);

	static void ProcessStaticMesh(aiMesh* mesh, aiNode* node, StaticMesh* staticMesh, int materialSlotIndex, BoundingBox& boundingBox);

	StaticMesh_ptr ModelImporter::LoadMesh(const DataBlob& dataBlob)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFileFromMemory(dataBlob.data(), dataBlob.size(), MESH_PROCESS_FLAGS | aiProcess_PreTransformVertices | aiProcess_EmbedTextures);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			//LogError("ModelImporter::Import", importer.GetErrorString());
			return nullptr;
		}

		StaticMesh_ptr mesh = std::make_shared<StaticMesh>();

		mesh->CreateVertexBuffer(0);

		BoundingBox boundingBox;

		std::map<String, aiTexture*> embeddedTextures;
		for (uint32_t i = 0; i < scene->mNumTextures; i++)
		{
			aiTexture* tex = scene->mTextures[i];
			Path file = tex->mFilename.C_Str();
			embeddedTextures[file.string()] = tex;
		}

		int materialSlotIndex = 0;
		ProcessNodes(&ProcessStaticMesh, mesh.get(), scene, scene->mRootNode, materialSlotIndex, boundingBox, embeddedTextures);

		mesh->SetBounds(boundingBox);
		mesh->UpdateBuffers();

		return mesh;
	}

	template<typename MeshProcessor, typename MeshType>
	static void ProcessNodes(MeshProcessor meshProcessor, MeshType* engineMesh, const aiScene* scene, aiNode* node, int& materialSlotIndex, BoundingBox& boundingBox, std::map<String, aiTexture*>& embeddedTextures)
	{
		std::vector<aiMesh*> triangleMeshes;

		// Setup triangle meshes
		for (uint32_t i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			if (mesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE)
			{
				triangleMeshes.push_back(mesh);
			}
		}

		for (auto mesh : triangleMeshes) {
			int meshMaterialSlot = 0;

			if (mesh->mMaterialIndex >= 0) {
				meshMaterialSlot = materialSlotIndex++;

				aiMaterial* sourceMaterial = scene->mMaterials[mesh->mMaterialIndex];

				MaterialSlot materialSlot = {};
				materialSlot.Material = nullptr;
				materialSlot.MaterialSlotName = String(sourceMaterial->GetName().C_Str()) + "_" + std::to_string(meshMaterialSlot);

				aiColor4D diffuse;
				if(AI_SUCCESS == aiGetMaterialColor(sourceMaterial, AI_MATKEY_COLOR_DIFFUSE, &diffuse)) {
					materialSlot.Colors["Color"] = color_cast(diffuse);
				}

				for (uint8_t texType = 0; texType < (uint8_t)aiTextureType::aiTextureType_UNKNOWN; texType++) {
					unsigned int textureCount = sourceMaterial->GetTextureCount((aiTextureType)texType);

					if (textureCount == 0)
					{
						continue;
					}

					aiString texturePathAiStr;

					if (sourceMaterial->GetTexture((aiTextureType)texType, 0, &texturePathAiStr) != aiReturn_SUCCESS)
					{
						continue;
					}

					Path filePath = texturePathAiStr.C_Str();

					//std::cout << TextureTypeEnumToString[texType] << ": " << filePath << std::endl;

					auto iter = embeddedTextures.find(filePath.string());

					if (iter != embeddedTextures.end())
					{
						aiTexture* tex = iter->second;
						auto* aiData = (unsigned char*)tex->pcData;

						auto dataSize = tex->mWidth;
						if(tex->mHeight != 0) {
							dataSize *= tex->mHeight;
						}

						//aiTexel* aiData = tex->pcData;
						//std::cout << tex->achFormatHint << std::endl;
						DataBlob fileData(dataSize);

						memcpy(fileData.data(), aiData, fileData.size());

						// TODO: Do not set GraphicsFormat::SRGBA8_UNORM for normal maps

						auto texture = AuroraEngine::AssetManager->LoadTexture(filePath.string(), GraphicsFormat::RGBA8_UNORM, fileData);
						materialSlot.Textures[TextureTypeEnumToShaderNameString[texType]] = texture;
						//std::cout << "TEXXX: " << TextureTypeEnumToShaderNameString[texType] << " - " << filePath.string() << std::endl;
					} else {
						/*auto texture = AuroraEngine::AssetManager->LoadTexture(filePath);

						if(texture != nullptr) {
							materialSlot.Textures[TextureTypeEnumToShaderNameString[texType]] = texture;
						}*/
					}

					//Log(TextureTypeEnumToString[texType], filePath.string());
				}

				engineMesh->MaterialSlots[meshMaterialSlot] = materialSlot;
			}

			meshProcessor(mesh, node, engineMesh, meshMaterialSlot, boundingBox);
		}

		// Iterate over children
		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			ProcessNodes(meshProcessor, engineMesh, scene, node->mChildren[i], materialSlotIndex, boundingBox, embeddedTextures);
		}
	}

	static void ProcessStaticMesh(aiMesh* mesh, aiNode* node, StaticMesh* staticMesh, int materialSlotIndex, BoundingBox& boundingBox)
	{
		auto& indices = staticMesh->LODResources[0].Indices;
		//auto* buffer = dynamic_cast<VertexBuffer<StaticMeshVertexBufferEntry>*>(staticMesh->LODResources[0].Vertices);
		auto* buffer = staticMesh->GetVertexBuffer(0);

		MeshSection section;
		section.MaterialIndex = materialSlotIndex;

		int lastIndex = buffer->GetCount();

		for (uint32_t vertIdx = 0u; vertIdx < mesh->mNumVertices; vertIdx++)
		{
			aiVector3D vert = mesh->mVertices[vertIdx];
			aiVector3D norm = mesh->mNormals[vertIdx];

			StaticMeshVertexBufferEntry vertexEntry = {};

			vertexEntry.Position = vec3_cast(vert);
			vertexEntry.Normal = vec3_cast(norm);

			if(mesh->HasTextureCoords(0)) {
				aiVector3D tex = mesh->mTextureCoords[0][vertIdx];
				vertexEntry.TexCoord = vec2_cast(tex);
			}

			if(mesh->HasTangentsAndBitangents()) {
				aiVector3D tan = mesh->mTangents[vertIdx];
				aiVector3D bit = mesh->mBitangents[vertIdx];
				vertexEntry.Tangent = vec3_cast(tan);
				vertexEntry.BiTangent = vec3_cast(bit);
			}

			boundingBox.Extend(vertexEntry.Position);

			buffer->Add(vertexEntry);
		}

		section.FirstIndex = indices.size();

		for (std::uint32_t faceIdx = 0u; faceIdx < mesh->mNumFaces; faceIdx++) {
			for (uint8_t id = 0; id < 3; id++)
			{
				uint32_t index = mesh->mFaces[faceIdx].mIndices[id];

				uint32_t newIndex = lastIndex + index;

				indices.push_back(newIndex);
			}
		}

		section.NumTriangles = mesh->mNumFaces * 3;

		staticMesh->LODResources[0].Sections.push_back(section);
	}
}