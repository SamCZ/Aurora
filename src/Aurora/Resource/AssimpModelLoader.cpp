#include "AssimpModelLoader.hpp"

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Aurora/Logger/Logger.hpp"
#include "Aurora/Core/Vector.hpp"

#include <stb_image.h>
#include <stb_image_resize.h>

#include "Aurora/Engine.hpp"
#include "Aurora/Graphics/Base/IRenderDevice.hpp"

namespace Aurora
{
	std::vector<std::string> TextureTypeEnumToString = {
		"aiTextureType_NONE", "aiTextureType_DIFFUSE", "aiTextureType_SPECULAR",
		"aiTextureType_AMBIENT", "aiTextureType_EMISSIVE", "aiTextureType_HEIGHT",
		"aiTextureType_NORMALS", "aiTextureType_SHININESS", "aiTextureType_OPACITY",
		"aiTextureType_DISPLACEMENT", "aiTextureType_LIGHTMAP", "aiTextureType_REFLECTION",
		"aiTextureType_UNKNOWN", "_aiTextureType_Force32Bit" };

	inline Vector4 color_cast(const aiColor4D &v) { return {v.r, v.g, v.b, v.a}; }
	inline Vector3 vec3_cast(const aiVector3D &v) { return {v.x, v.y, v.z}; }
	inline Vector2 vec2_cast(const aiVector3D &v) { return {v.x, v.y}; }
	inline Quaternion quat_cast(const aiQuaternion &q) { return {q.w, q.x, q.y, q.z}; }
	inline Matrix4 mat4_cast(const aiMatrix4x4 &m) { return glm::transpose(glm::make_mat4(&m.a1)); }
	inline Matrix4 mat4_cast(const aiMatrix3x3 &m) { return glm::transpose(glm::make_mat3(&m.a1)); }

	void ProcessStaticMesh(const StaticMesh_ptr& mesh, const aiMesh* sourceMesh, const Matrix4& transform, LOD lod, const MeshImportOptions& importOptions, int32_t materialIndex)
	{
		auto& indices = mesh->LODResources[lod].Indices;
		auto* buffer = mesh->GetVertexBuffer<StaticMesh::Vertex>(lod);

		FMeshSection section;
		section.MaterialIndex = materialIndex;

		size_t lastVertex = buffer->GetCount();

		for (uint vertIdx = 0u; vertIdx < sourceMesh->mNumVertices; vertIdx++)
		{
			StaticMesh::Vertex vertex = {};

			aiVector3D vert = sourceMesh->mVertices[vertIdx];
			aiVector3D norm = sourceMesh->mNormals[vertIdx];

			vertex.Position = vec3_cast(vert);
			vertex.Normal = vec3_cast(norm);

			if(sourceMesh->HasTextureCoords(0))
			{
				aiVector3D tex = sourceMesh->mTextureCoords[0][vertIdx];
				vertex.TexCoord = vec2_cast(tex);
			}

			if(sourceMesh->HasTangentsAndBitangents())
			{
				aiVector3D tan = sourceMesh->mTangents[vertIdx];
				aiVector3D bit = sourceMesh->mBitangents[vertIdx];

				vertex.Tangent = vec3_cast(tan);
				vertex.BiTangent = vec3_cast(bit);
			}

			if(importOptions.PreTransform)
			{
				vertex.Position = transform * Vector4(vertex.Position, 1.0f);
				vertex.Normal = transform * Vector4(vertex.Normal, 0.0f);

				if(sourceMesh->HasTangentsAndBitangents())
				{
					vertex.Tangent = transform * Vector4(vertex.Tangent, 0.0f);
					vertex.BiTangent = transform * Vector4(vertex.BiTangent, 0.0f);
				}
			}

			buffer->Add(vertex);
		}

		section.FirstIndex = indices.size();

		for (uint faceIdx = 0u; faceIdx < sourceMesh->mNumFaces; faceIdx++)
		{
			for (uint8_t id = 0; id < 3; id++)
			{
				uint index = sourceMesh->mFaces[faceIdx].mIndices[id];
				uint32_t newIndex = lastVertex + index;
				indices.push_back(newIndex);
			}
		}

		section.NumTriangles = sourceMesh->mNumFaces * 3;
		mesh->LODResources[lod].Sections.emplace_back(section);
	}

	void LoadMaterial(const aiScene* scene, const Mesh_ptr& mesh, uint32_t materialIndex, const aiMaterial* sourceMaterial)
	{
		if(!mesh->MaterialSlots.contains(materialIndex))
		{
			MaterialSlot slot(nullptr, sourceMaterial->GetName().data);

			for (uint8_t texTypeID = 0; texTypeID < (uint8_t)aiTextureType::aiTextureType_UNKNOWN; texTypeID++)
			{
				auto textureType = (aiTextureType)texTypeID;

				uint32_t textureCount = sourceMaterial->GetTextureCount(textureType);

				if(!textureCount)
					continue;

				aiString texturePathAiStr;
				if (sourceMaterial->GetTexture(textureType, 0, &texturePathAiStr) != aiReturn_SUCCESS)
				{
					continue;
				}

				if (const aiTexture* asTexture = scene->GetEmbeddedTexture(texturePathAiStr.C_Str()))
				{
					AU_LOG_INFO("Image format: ", asTexture->achFormatHint);

					String imageFormatStr = asTexture->achFormatHint;

					int desiredChannels = 4;

					if(imageFormatStr == "tga")
					{
						desiredChannels = 3;
					}

					int width;
					int height;
					int channels;
					uint8_t* imageData = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(asTexture->pcData), asTexture->mWidth, &width, &height, &channels, desiredChannels);

					if(desiredChannels != channels)
					{
						stbi_image_free(imageData);
						imageData = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(asTexture->pcData), asTexture->mWidth, &width, &height, &channels, channels);
					}

					AU_LOG_INFO("Image info: ", width, ",", height, ",", channels);

					GraphicsFormat format = GraphicsFormat::Unknown;

					switch (channels)
					{
						case 1:
							format = GraphicsFormat::R8_UNORM;
							break;
						case 2:
							format = GraphicsFormat::RG8_UNORM;
							break;
						case 3:
							format = GraphicsFormat::RGB8_UNORM;
							break;
						case 4:
							format = GraphicsFormat::RGBA8_UNORM;
							break;
						default:
							AU_LOG_WARNING("Unknown number of channels: ", channels);
							break;
					}

					TextureDesc textureDesc;
					textureDesc.Width = width;
					textureDesc.Height = height;
					textureDesc.MipLevels = textureDesc.GetMipLevelCount();
					textureDesc.ImageFormat = format;
					textureDesc.Name = "[Embedded]";
					auto texture = GEngine->GetRenderDevice()->CreateTexture(textureDesc, nullptr);

					for (unsigned int mipLevel = 0; mipLevel < textureDesc.MipLevels; mipLevel++)
					{
						GEngine->GetRenderDevice()->WriteTexture(texture, mipLevel, 0, imageData);

						if (mipLevel < textureDesc.MipLevels - 1u)
						{
							int newWidth = std::max<int>(1, width >> 1);
							int newHeight = std::max<int>(1, height >> 1);

							auto* resizedData = new uint8_t[newWidth * newHeight * channels];
							stbir_resize_uint8(imageData, width, height, 0, resizedData, newWidth, newHeight, 0, channels);

							stbi_image_free(imageData);
							imageData = resizedData;
							width = newWidth;
							height = newHeight;
						}
					}

					stbi_image_free(imageData);

					// Convert texture type

					switch (textureType)
					{
						case aiTextureType_DIFFUSE:
							slot.Textures["Diffuse"] = texture;
							break;
						/*case aiTextureType_BASE_COLOR:
							slot.Textures["BaseColor"] = texture;
							break;*/
						default:
							AU_LOG_WARNING("Unsupported texture type: ", TextureTypeEnumToString[texTypeID], " !");
					}

					break;
				}
			}

			mesh->MaterialSlots.emplace(materialIndex, slot);
		}
	}

	template<typename Mesh, typename MeshProcessor>
	void ProcessNode(const Mesh& mesh, MeshProcessor& meshProcessor, const aiScene* scene, aiNode* node, const aiMatrix4x4& parentTransform, const MeshImportOptions& importOptions)
	{
		//std::cout << node->mName.C_Str() << std::endl;

		aiMatrix4x4 transform = parentTransform * node->mTransformation;

		aiVector3D scale;
		aiVector3D rotation;
		aiVector3D position;
		aiMatrix4DecomposeIntoScalingEulerAnglesPosition(&transform, &scale, &rotation, &position);

		for (uint i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* sourceMesh = scene->mMeshes[node->mMeshes[i]];

			/*AU_LOG_INFO("aiPrimitiveType_POINT = ", (sourceMesh->mPrimitiveTypes & aiPrimitiveType_POINT) ? "Y" : "N");
			AU_LOG_INFO("aiPrimitiveType_LINE = ", (sourceMesh->mPrimitiveTypes & aiPrimitiveType_LINE) ? "Y" : "N");
			AU_LOG_INFO("aiPrimitiveType_TRIANGLE = ", (sourceMesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) ? "Y" : "N");
			AU_LOG_INFO("aiPrimitiveType_POLYGON = ", (sourceMesh->mPrimitiveTypes & aiPrimitiveType_POLYGON) ? "Y" : "N");
			AU_LOG_INFO("aiPrimitiveType_NGONEncodingFlag = ", (sourceMesh->mPrimitiveTypes & aiPrimitiveType_NGONEncodingFlag) ? "Y" : "N");*/

			if ((sourceMesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) == 0)
				continue;

			aiMaterial* sourceMaterial = scene->mMaterials[sourceMesh->mMaterialIndex];

			LoadMaterial(scene, mesh, sourceMesh->mMaterialIndex, sourceMaterial);

			meshProcessor(mesh, sourceMesh, mat4_cast(transform), (LOD)0, importOptions, sourceMesh->mMaterialIndex);
		}

		// Iterate over children
		for (uint i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(mesh, meshProcessor, scene, node->mChildren[i], node->mTransformation, importOptions);
		}
	}

	MeshImportedData AssimpModelLoader::ImportModel(const String &name, const DataBlob &data, const MeshImportOptions& importOptions)
	{
		MeshImportedData importedData;

		uint8 additionalFlags = 0;
		const aiScene* scene = m_Importer.ReadFileFromMemory(data.data(), data.size(), aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs | aiProcess_EmbedTextures | additionalFlags);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			AU_LOG_ERROR("Import error", m_Importer.GetErrorString());
			return importedData;
		}

		AU_LOG_INFO("Has animations = ", scene->HasAnimations() ? "Yes" : "no");

		StaticMesh_ptr mesh = std::make_shared<StaticMesh>();
		mesh->CreateVertexBuffer<StaticMesh::Vertex>(0);

		ProcessNode(mesh, ProcessStaticMesh, scene, scene->mRootNode, aiMatrix4x4(), importOptions);

		AU_LOG_INFO("Mesh section count: ", mesh->LODResources[0].Sections.size());

		/*{ // TODO: just for testing purposes
			mesh->MaterialSlots.emplace_back(nullptr, "Material");
		}*/

		m_Importer.FreeScene();

		if(importOptions.UploadToGPU)
			mesh->UploadToGPU(importOptions.KeepCPUData);

		importedData.Imported = true;
		importedData.Mesh = mesh;

		return importedData;
	}
}