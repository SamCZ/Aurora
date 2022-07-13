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
	AssimpModelLoader::AssimpModelLoader()
	{
		//m_Importer.SetIOHandler();
	}

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

	void ProcessStaticMesh(const Mesh_ptr& meshBase, const aiMesh* sourceMesh, const Matrix4& transform, LOD lod, const MeshImportOptions& importOptions, int32_t materialIndex)
	{
		StaticMesh_ptr mesh = StaticMesh::Cast(meshBase);

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

			vertex.Position = vec3_cast(vert) * importOptions.DefaultScale;
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

	void ProcessSkeletalMesh(const Mesh_ptr& meshBase, const aiMesh* sourceMesh, const Matrix4& transform, LOD lod, const MeshImportOptions& importOptions, int32_t materialIndex)
	{
		SkeletalMesh_ptr mesh = SkeletalMesh::Cast(meshBase);

		auto& indices = mesh->LODResources[lod].Indices;
		auto* buffer = mesh->GetVertexBuffer<SkeletalMesh::Vertex>(lod);

		FMeshSection section;
		section.MaterialIndex = materialIndex;

		size_t lastVertex = buffer->GetCount();

		std::vector<VertexBoneData> bones_id_weights_for_each_vertex;
		bones_id_weights_for_each_vertex.resize(sourceMesh->mNumVertices);

		// Load bones
		auto& BoneMapping = mesh->Armature.BoneMapping;

		for (uint32_t i = 0; i < sourceMesh->mNumBones; i++)
		{
			int32_t bone_index;
			String bone_name = sourceMesh->mBones[i]->mName.C_Str();

			if (BoneMapping.find(bone_name) == BoneMapping.end())
			{
				bone_index = int32_t(mesh->Armature.Bones.size());
				mesh->Armature.Bones.emplace_back(bone_index, -1, bone_name, mat4_cast(sourceMesh->mBones[i]->mOffsetMatrix));
				BoneMapping[bone_name] = bone_index;
			} else
			{
				bone_index = BoneMapping[bone_name];
			}

			for (uint32_t j = 0; j < sourceMesh->mBones[i]->mNumWeights; j++)
			{
				unsigned int vertex_id = sourceMesh->mBones[i]->mWeights[j].mVertexId;
				float weight = sourceMesh->mBones[i]->mWeights[j].mWeight;
				bones_id_weights_for_each_vertex[vertex_id].addBoneData(bone_index, weight);
			}
		}

		// Load vertex data
		for (uint vertIdx = 0u; vertIdx < sourceMesh->mNumVertices; vertIdx++)
		{
			SkeletalMesh::Vertex vertex = {};

			aiVector3D vert = sourceMesh->mVertices[vertIdx];
			aiVector3D norm = sourceMesh->mNormals[vertIdx];

			vertex.Position = vec3_cast(vert) * importOptions.DefaultScale;
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

			VertexBoneData& boneData = bones_id_weights_for_each_vertex[vertIdx];
			vertex.BoneIndices = {boneData.Ids[0], boneData.Ids[1], boneData.Ids[2], boneData.Ids[3]};
			vertex.BoneWeights = {boneData.Weights[0], boneData.Weights[1], boneData.Weights[2], boneData.Weights[3]};

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

	void AssimpModelLoader::LoadMaterial(const aiScene* scene, const Mesh_ptr& mesh, int32_t materialIndex, const aiMaterial* sourceMaterial)
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

				AU_LOG_INFO("Texture count: ", textureCount);

				aiString texturePathAiStr;
				if (sourceMaterial->GetTexture(textureType, 0, &texturePathAiStr) != aiReturn_SUCCESS)
				{
					continue;
				}

				AU_LOG_INFO("Texture path: ", texturePathAiStr.data);

				if (const aiTexture* asTexture = scene->GetEmbeddedTexture(texturePathAiStr.C_Str()))
				{
					Texture_ptr texture;

					if (m_TextureCache.contains(asTexture))
					{
						texture = m_TextureCache.at(asTexture);
					}
					else
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
								format = GraphicsFormat::SRGBA8_UNORM;
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
						texture = GEngine->GetRenderDevice()->CreateTexture(textureDesc, nullptr);

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
					}

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

	template<typename MeshProcessor>
	void ProcessNode(AssimpModelLoader* loader, std::vector<Mesh_ptr>& meshes, int32_t& currentMeshIndex, int32_t& currentMaterialIndex, MeshProcessor& meshProcessor, const aiScene* scene, aiNode* node, const aiMatrix4x4& parentTransform, const MeshImportOptions& importOptions)
	{
		aiMatrix4x4 transform;

		if (scene->HasAnimations())
		{
			// No parent transform for animated meshes
			transform = node->mTransformation;
		}
		else
		{
			transform = parentTransform * node->mTransformation;
		}

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

			if (importOptions.SplitMeshes)
			{
				if (meshes.size() <= currentMeshIndex)
				{
					if (scene->HasAnimations())
					{
						AU_LOG_ERROR("Could not split meshes for skeletal mesh !");
					}
					else
					{
						StaticMesh_ptr staticMesh = std::make_shared<StaticMesh>();
						staticMesh->Name = node->mName.C_Str();
						staticMesh->CreateVertexBuffer<StaticMesh::Vertex>(0);
						meshes.push_back(staticMesh);
					}
				}
			}
			else if (meshes.empty())
			{
				if (scene->HasAnimations())
				{
					SkeletalMesh_ptr skeletalMesh = std::make_shared<SkeletalMesh>();
					skeletalMesh->Name = node->mName.C_Str();
					skeletalMesh->CreateVertexBuffer<SkeletalMesh::Vertex>(0);
					meshes.push_back(skeletalMesh);
				}
				else
				{
					StaticMesh_ptr staticMesh = std::make_shared<StaticMesh>();
					staticMesh->Name = node->mName.C_Str();
					staticMesh->CreateVertexBuffer<StaticMesh::Vertex>(0);
					meshes.push_back(staticMesh);
				}
			}

			Mesh_ptr mesh = meshes[currentMeshIndex];

			aiMaterial* sourceMaterial = scene->mMaterials[sourceMesh->mMaterialIndex];
			loader->LoadMaterial(scene, mesh, currentMaterialIndex, sourceMaterial);
			meshProcessor(mesh, sourceMesh, mat4_cast(transform), (LOD)0, importOptions, currentMaterialIndex);

			currentMaterialIndex++;

			if (importOptions.SplitMeshes)
			{
				currentMeshIndex++;
				currentMaterialIndex = 0;
			}
		}

		// Iterate over children
		for (uint i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(loader, meshes, currentMeshIndex, currentMaterialIndex, meshProcessor, scene, node->mChildren[i], node->mTransformation, importOptions);
		}
	}

	void ProcessArmature(const aiNode* node, Animation::Armature& armature, int32_t parentIndex)
	{
		int32_t index = -1;
		String nodeName = node->mName.C_Str();

		for (int i = 0; i < armature.Bones.size(); ++i)
		{
			auto& bone = armature.Bones[i];

			if (bone.Name == nodeName)
			{
				bone.Parent = parentIndex;
				index = bone.Index;
				if (parentIndex == -1) {
					armature.RootBones.push_back(&bone);
				} else {
					armature.Bones[bone.Parent].Children.push_back(&bone);
				}

				break;
			}
		}

		for (uint32_t i = 0; i < node->mNumChildren; ++i)
		{
			ProcessArmature(node->mChildren[i], armature, index);
		}
	}

	bool LoadAnimation(Animation::FAnimation& out_animation, const aiAnimation* sourceAnimation, const Animation::Armature& armature)
	{
		if (sourceAnimation->mName.length == 0)
		{
			return false;
		}

		out_animation.Name = sourceAnimation->mName.C_Str();
		out_animation.Duration = sourceAnimation->mDuration;
		out_animation.TicksPerSecond = sourceAnimation->mTicksPerSecond;
		out_animation.Channels.resize(armature.Bones.size());

		for (uint32_t i = 0; i < sourceAnimation->mNumChannels; i++)
		{
			const aiNodeAnim* aiChannel = sourceAnimation->mChannels[i];
			String channelName = aiChannel->mNodeName.C_Str();

			int32_t bone_index = -1;

			for (auto& bone : armature.Bones)
			{
				if (channelName == bone.Name)
				{
					bone_index = bone.Index;
				}
			}

			if(bone_index < 0)
			{
				AU_LOG_WARNING("Bone not found for channel: ", channelName);
				continue;
			}

			Animation::AnimationChannel channel(bone_index, channelName);

			channel.PositionKeys.reserve(aiChannel->mNumPositionKeys);
			for (uint32_t k = 0; k < aiChannel->mNumPositionKeys; k++)
			{
				aiVectorKey* key = &aiChannel->mPositionKeys[k];
				channel.PositionKeys.emplace_back(Animation::AnimationKey<Vector3>{key->mTime, vec3_cast(key->mValue)});
			}

			channel.RotationKeys.reserve(aiChannel->mNumRotationKeys);
			for (uint32_t k = 0; k < aiChannel->mNumRotationKeys; k++)
			{
				aiQuatKey* key = &aiChannel->mRotationKeys[k];
				channel.RotationKeys.emplace_back(Animation::AnimationKey<Quaternion>{key->mTime, quat_cast(key->mValue)});
			}

			channel.ScaleKeys.reserve(aiChannel->mNumScalingKeys);
			for (uint32_t k = 0; k < aiChannel->mNumScalingKeys; k++)
			{
				aiVectorKey* key = &aiChannel->mScalingKeys[k];
				channel.ScaleKeys.emplace_back(Animation::AnimationKey<Vector3>{key->mTime, vec3_cast(key->mValue)});
			}

			out_animation.Channels[channel.Index] = channel;
		}

		AU_LOG_INFO("Animation loaded: ", out_animation.Name, " - channels: ", std::to_string(out_animation.Channels.size()));

		return true;
	}

	MeshImportedData AssimpModelLoader::ImportModel(const String &name, const DataBlob &data, MeshImportOptions importOptions)
	{
		MeshImportedData importedData;

		int baseFlags = aiProcess_CalcTangentSpace | aiProcess_GenNormals | aiProcess_GenUVCoords | aiProcess_Triangulate;
		int additionalFlags = 0;
		const aiScene* scene = m_Importer.ReadFileFromMemory(data.data(), data.size(), baseFlags | aiProcess_PopulateArmatureData | aiProcess_FlipUVs | aiProcess_EmbedTextures | additionalFlags);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			AU_LOG_ERROR("Import error", m_Importer.GetErrorString());
			return importedData;
		}

		AU_LOG_INFO("Has animations = ", scene->HasAnimations() ? "Yes" : "no");

		int32_t currentMesh = 0;
		int32_t currentMaterial = 0;
		if (scene->HasAnimations())
		{
			std::function<void(aiNode*, int)> printScene;
			printScene = [&printScene](aiNode* node, int depth) -> void
			{
				for (int i = 0; i < depth; ++i)
				{
					std::cout << " ";
				}

				std::cout << node->mName.C_Str() << std::endl;

				for (int i = 0; i < node->mNumChildren; ++i)
				{
					printScene(node->mChildren[i], depth + 1);
				}
			};

			printScene(scene->mRootNode, 0);

			importOptions.SplitMeshes = false;
			importOptions.PreTransform = false;
			ProcessNode(this, importedData.Meshes, currentMesh, currentMaterial, ProcessSkeletalMesh, scene, scene->mRootNode, aiMatrix4x4(), importOptions);

			if (not importedData.Meshes.empty())
			{
				SkeletalMesh_ptr skeletalMesh = importedData.Get<SkeletalMesh>();

				aiMatrix4x4 invTrans = scene->mRootNode->mTransformation;
				invTrans.Inverse();
				skeletalMesh->Armature.GlobalInverseTransform = mat4_cast(invTrans);

				/*for (int i = 0; i < scene->mNumAnimations; ++i)
				{
					aiAnimation* anim = scene->mAnimations[i];

					for (int j = 0; j < anim->mNumChannels; ++j)
					{
						auto channel = anim->mChannels[i];
						std::string boneName = channel->mNodeName.data;


						bool foundBone = false;
						for (const Animation::Bone& bone: skeletalMesh->Armature.Bones)
						{
							if (bone.Name == boneName)
							{
								foundBone = true;
								break;
							}
						}

						if (not foundBone)
						{
							skeletalMesh->Armature.Bones.emplace_back(skeletalMesh->Armature.Bones.size(), -1, boneName, glm::identity<Matrix4>());
						}

					}
				}*/

				ProcessArmature(scene->mRootNode, skeletalMesh->Armature, -1);

				for (int i = 0; i < scene->mNumAnimations; ++i)
				{
					Animation::FAnimation animation;
					if (LoadAnimation(animation, scene->mAnimations[i], skeletalMesh->Armature) == true)
					{
						skeletalMesh->Animations.emplace_back(animation);
					}
				}
			}
		}
		else
		{
			ProcessNode(this, importedData.Meshes, currentMesh, currentMaterial, ProcessStaticMesh, scene, scene->mRootNode, aiMatrix4x4(), importOptions);
		}

		m_Importer.FreeScene();

		for (auto& mesh : importedData.Meshes)
		{
			mesh->ComputeAABB();

			if(importOptions.UploadToGPU)
				mesh->UploadToGPU(importOptions.KeepCPUData);
		}

		importedData.Imported = true;
		importedData.Mesh = importedData.Meshes[0];

		return importedData;
	}

	bool AssimpModelLoader::ImportAnimation(const DataBlob& data, SkeletalMesh_ptr& skeletalMesh)
	{
		uint8 additionalFlags = 0;
		const aiScene* scene = m_Importer.ReadFileFromMemory(data.data(), data.size(), aiProcessPreset_TargetRealtime_Fast | aiProcess_PopulateArmatureData | aiProcess_FlipUVs | additionalFlags);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			AU_LOG_ERROR("Import error", m_Importer.GetErrorString());
			return false;
		}

		for (int i = 0; i < scene->mNumAnimations; ++i)
		{
			Animation::FAnimation animation;
			if (LoadAnimation(animation, scene->mAnimations[i], skeletalMesh->Armature) == true)
			{
				skeletalMesh->Animations.emplace_back(animation);
			}
		}

		m_Importer.FreeScene();

		return true;
	}
}