#include "FbxImporter.hpp"

#include <Aurora/Logger/Logger.hpp>
#include <Aurora/Framework/AnimationCore/Skeleton.hpp>
#include "FbxUtil.hpp"
#include "FbxMemoryStream.hpp"

#include <Aurora/Framework/Mesh/Mesh.hpp>

using namespace Aurora;

namespace FbxImport
{
	struct Vertex
	{
		vec3 Position;
		vec3 Normal;
		vec4 Tangent;
		vec2 TexCoord;
		uvec4 BoneIdx;
		vec4 BoneWeights;
	};

	void LoadMesh(FbxMesh* sourceMesh, const StaticMesh_ptr& mesh)
	{
		auto& indices_buff = mesh->LODResources[0].Indices;
		auto* buffer = mesh->GetVertexBuffer<StaticMesh::Vertex>(0);

		mat4 transform = FbxUtil::fbx_to_glm(sourceMesh->GetNode()->EvaluateGlobalTransform());
		int polygonCount = sourceMesh->GetPolygonCount();

		FbxLayerElementArrayTemplate<int>* materialIndices = nullptr;
		if (sourceMesh->GetMaterialIndices(&materialIndices))
		{
			for (int i = 0; i < polygonCount; ++i)
			{
				int materialIndex = materialIndices->GetAt(i);

				//AU_LOG_INFO(materialIndex);
			}
		}


		sourceMesh->GenerateNormals();
		sourceMesh->GenerateTangentsData();
		sourceMesh->GenerateTangentsDataForAllUVSets();

		FbxVector4* vertices_source = sourceMesh->GetControlPoints();
		auto const* const normal_layer = sourceMesh->GetElementNormal();
		auto const* const tangent_layer = sourceMesh->GetElementTangent();
		auto const* const uv_layer = sourceMesh->GetElementUV();

		std::vector<Vertex> vertices(sourceMesh->GetControlPointsCount());

		for (int i = 0; i < vertices.size(); i++)
		{
			vertices[i].Position = transform * glm::vec4{FbxUtil::to_glm_vec3(vertices_source[i]), 1.f};

			if (normal_layer->GetMappingMode() == FbxLayerElement::EMappingMode::eByControlPoint)
			{
				vertices[i].Normal = FbxUtil::layer_element_at(normal_layer, static_cast<int>(i));
			}

			if (tangent_layer->GetMappingMode() == FbxLayerElement::EMappingMode::eByControlPoint)
			{
				vertices[i].Tangent = FbxUtil::layer_element_at(tangent_layer, static_cast<int>(i));
			}

			if (uv_layer->GetMappingMode() == FbxLayerElement::EMappingMode::eByControlPoint)
			{
				vertices[i].TexCoord = FbxUtil::layer_element_at(uv_layer, static_cast<int>(i));
			}
		}

		auto const* const indices_source = sourceMesh->GetPolygonVertices();
		std::vector<uint32_t> indices(sourceMesh->GetPolygonVertexCount());

		for (int i = 0; i < indices.size(); ++i)
		{
			indices[i] = static_cast<uint32_t>(indices_source[i]);

			if (materialIndices != nullptr)
			{
				int polygonIdx = i / 3;
				int polygonMaterialIndex = materialIndices->GetAt(polygonIdx);

				//section.MaterialIndex = polygonMaterialIndex;
			}

			if (normal_layer->GetMappingMode() == FbxLayerElement::EMappingMode::eByPolygonVertex)
			{
				vertices[indices[i]].Normal = FbxUtil::layer_element_at(normal_layer, static_cast<int>(i));
			}

			if (tangent_layer->GetMappingMode() == FbxLayerElement::EMappingMode::eByPolygonVertex)
			{
				vertices[indices[i]].Tangent = FbxUtil::layer_element_at(tangent_layer, static_cast<int>(i));
			}

			if (uv_layer->GetMappingMode() == FbxLayerElement::EMappingMode::eByPolygonVertex)
			{
				vertices[indices[i]].TexCoord = FbxUtil::layer_element_at(uv_layer, static_cast<int>(i));
			}
		}

		FMeshSection section;
		section.MaterialIndex = 0;
		section.FirstIndex = indices.size();

		size_t lastVertex = buffer->GetCount();



		int a = 0;

		// Load skin bones

		//auto* skin = static_cast<FbxSkin*>(sourceMesh->GetDeformer(0, FbxDeformer::eSkin));
		//int cluster_count = skin->GetClusterCount();

		/*for (int cluster_index = 0; cluster_index < cluster_count; ++cluster_index)
		{
			FbxCluster* cluster = skin->GetCluster(cluster_index);

			auto const name = FbxUtil::trimmed_bone_name(cluster->GetLink());

			if (FbxUtil::is_end_bone(name)) {
				continue;
			}

			auto const bone_id = skeleton_.bone_id_by_name(name.c_str());
		}*/

		int b = 0;
	}

	template<typename U>
	static auto create_value_(FbxDouble3 const& vector) -> std::enable_if_t<std::is_same<U, glm::vec3>::value, U>
	{
		return FbxUtil::fbx_to_glm(vector);
	}
	template<typename U>
	static auto create_value_(FbxDouble3 const& vector) -> std::enable_if_t<std::is_same<U, glm::quat>::value, U>
	{
		return glm::quat{glm::radians(FbxUtil::fbx_to_glm(vector))};
	}

	template<typename T>
	AnimationTrack<T> LoadTrack(FbxAnimLayer* const layer, FbxPropertyT<FbxDouble3>& property)
	{
		AnimationTrack<T> track;

		auto const curves = std::array<FbxAnimCurve const*, 3>{
			property.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_X),
			property.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_Y),
			property.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_Z),
		};

		if (curves[0] || curves[1] || curves[2])
		{
			auto const get_key_count = [](auto const* const curve) { return curve ? curve->KeyGetCount() : 0; };
			auto const key_counts = glm::ivec3{get_key_count(curves[0]), get_key_count(curves[1]), get_key_count(curves[2])};

			track.keyframes_.reserve(glm::compMax(key_counts));

			// The current keyframe index for each curve/channel.
			auto cursors = glm::ivec3{};

			while (cursors != key_counts)
			{
				// Pick the curve/channel with the next keyframe time.
				auto const channel = std::min({0u, 1u, 2u}, [&](auto const a, auto const b) {
					return cursors[a] != key_counts[a] ? cursors[b] != key_counts[b] ?
						curves[a]->KeyGetTime(cursors[a]) < curves[b]->KeyGetTime(cursors[b]) : a : b;
				});

				auto const time = curves[channel]->KeyGetTime(cursors[channel]);

				// Increment the keyframe cursor(s).
				++cursors[channel];
				for (auto const i : {0u, 1u, 2u}) {
					if (i != channel && cursors[i] != key_counts[i] && curves[i]->KeyGetTime(cursors[i]).GetMilliSeconds() <= time.GetMilliSeconds() + 1) {
						++cursors[i];
					}
				}

				track.keyframes_.push_back(Keyframe<T>{
					Seconds{time.GetSecondDouble()},
					create_value_(property.EvaluateValue(time))
				});
			}
		}

		return track;
	}

	std::shared_ptr<Mesh> LoadScene(const std::vector<uint8_t>& data)
	{
		AU_LOG_INFO(FbxManager::GetVersion(true));

		FbxManager* fbxManager = FbxManager::Create();
		auto* settings = FbxIOSettings::Create(fbxManager, IOSROOT);

		//Load plugins from the executable directory (optional)
		FbxString lPath = FbxGetApplicationDirectory();
		fbxManager->LoadPluginsDirectory(lPath.Buffer());

		FbxImporter* importer = FbxImporter::Create(fbxManager, "Importer");

		MemoryFbxStream stream(data);

		int a = 5;

		const bool hasInitialized = importer->Initialize(&stream, &a, -1, settings);

		int fileVersion[3];
		importer->GetFileVersion(fileVersion[0], fileVersion[1], fileVersion[2]);

		if (not hasInitialized)
		{
			FbxString error = importer->GetStatus().GetErrorString();
			AU_LOG_WARNING("Call to FbxImporter::Initialize() failed.");
			AU_LOG_WARNING("Error returned:", error.Buffer());

			if (importer->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
			{
				int SdkVersion[3];
				FbxManager::GetFileFormatVersion(SdkVersion[0], SdkVersion[1], SdkVersion[2]);

				AU_LOG_WARNING("FBX file format version for this FBX SDK is ", SdkVersion[0], ".", SdkVersion[1], ".", SdkVersion[2]);
				AU_LOG_WARNING("FBX file format version for the file is  ", fileVersion[0], ".", fileVersion[1], ".", fileVersion[2]);
			}

			return nullptr;
		}

		FbxScene* scene = FbxScene::Create(fbxManager, "Imported Scene");
		if (not importer->Import(scene))
		{
			AU_LOG_WARNING(importer->GetStatus().GetErrorString());
			return nullptr;
		}

		FbxAxisSystem::OpenGL.DeepConvertScene(scene);
		FbxGeometryConverter{fbxManager}.Triangulate(scene, true);

		// This will convert scene scale
		FbxSystemUnit::m.ConvertScene(scene);
		double sceneScale = scene->GetGlobalSettings().GetSystemUnit().GetConversionFactorTo(FbxSystemUnit::m);

		AU_LOG_INFO("Scene scale: ", sceneScale);

		FbxNode* const root_node = scene->GetRootNode();

		if (root_node == nullptr)
		{
			AU_LOG_WARNING("An FBX scene did not contain a root node.");
			return nullptr;
		}

		StaticMesh_ptr mesh = MakeShared<StaticMesh>();
		mesh->CreateVertexBuffer<StaticMesh::Vertex>(0);

		for (int i = 0; i < root_node->GetChildCount(); ++i)
		{
			FbxNode* childNode = root_node->GetChild(i);
			FbxNodeAttribute* nodeAttribute = childNode->GetNodeAttribute();

			if (nodeAttribute)
			{
				// All lights has been processed before the whole scene because they influence every geometry.
				if (nodeAttribute->GetAttributeType() == FbxNodeAttribute::eMarker)
				{
					//DrawMarker(pGlobalPosition);
				}
				else if (nodeAttribute->GetAttributeType() == FbxNodeAttribute::eSkeleton)
				{
					//DrawSkeleton(pNode, pParentGlobalPosition, pGlobalPosition);
				}
					// NURBS and patch have been converted into triangluation meshes.
				else if (nodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
				{
					FbxMesh* lMesh = childNode->GetMesh();
					const int lVertexCount = lMesh->GetControlPointsCount();

					if (lVertexCount == 0)
					{
						continue;
					}

					// If it has some defomer connection, update the vertices position
					const bool lHasVertexCache = lMesh->GetDeformerCount(FbxDeformer::eVertexCache) &&
						(static_cast<FbxVertexCacheDeformer*>(lMesh->GetDeformer(0, FbxDeformer::eVertexCache)))->Active.Get();
					const bool lHasShape = lMesh->GetShapeCount() > 0;
					const bool lHasSkin = lMesh->GetDeformerCount(FbxDeformer::eSkin) > 0;
					const bool lHasDeformation = lHasVertexCache || lHasShape || lHasSkin;

					AU_LOG_INFO("MESH ", childNode->GetName());

					{ // Load mesh
						LoadMesh(lMesh, mesh);
					}

					if (lHasDeformation)
					{
						FbxBlendShape* blendShape = static_cast<FbxBlendShape*>(lMesh->GetDeformer(0, FbxDeformer::eBlendShape));

						if (blendShape)
						{
							for (int j = 0; j < blendShape->GetBlendShapeChannelCount(); ++j)
							{
								FbxBlendShapeChannel* blendShapeChannel = blendShape->GetBlendShapeChannel(j);

								AU_LOG_INFO(blendShapeChannel->GetName());
							}
						}

						/*AU_LOG_INFO("Deform count ", lMesh->GetDeformerCount());
						for (int j = 0; j < lMesh->GetDeformerCount(); ++j)
						{
							FbxVertexCacheDeformer* deformer = static_cast<FbxVertexCacheDeformer*>(lMesh->GetDeformer(j, FbxDeformer::eVertexCache));
							AU_LOG_INFO(deformer->Channel.Get().Buffer());
						}*/

						AU_LOG_INFO("HAS DEFORMATION");
					}
				}
				else if (nodeAttribute->GetAttributeType() == FbxNodeAttribute::eCamera)
				{
					//DrawCamera(pNode, pTime, pAnimLayer, pGlobalPosition);
				}
				else if (nodeAttribute->GetAttributeType() == FbxNodeAttribute::eNull)
				{
					//DrawNull(pGlobalPosition);
				}
			}

			AU_LOG_INFO(childNode->GetName());
		}

		scene->Destroy();
		importer->Destroy();
		fbxManager->Destroy();

		return mesh;
	}
}