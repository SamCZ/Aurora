#include "SceneRenderer.hpp"

#include "Aurora/Engine.hpp"

#include "Aurora/Framework/Scene.hpp"
#include "Aurora/Framework/CameraComponent.hpp"
#include "Aurora/Framework/MeshComponent.hpp"
#include "Aurora/Framework/Lights.hpp"

#include "Aurora/Graphics/Base/IRenderDevice.hpp"
#include "Aurora/Graphics/ViewPortManager.hpp"
#include "Aurora/Graphics/RenderManager.hpp"
#include "Aurora/Graphics/OpenGL/GLBufferLock.hpp"

#include "Aurora/Resource/ResourceManager.hpp"

#include "Shaders/vs_common.h"
#include "Shaders/PBR/Composite.h"

namespace Aurora
{
	SceneRenderer::SceneRenderer()
	{
		m_BaseVsDataBuffer = GEngine->GetRenderDevice()->CreateBuffer(BufferDesc("BaseVSData", sizeof(BaseVSData), EBufferType::UniformBuffer));
		m_InstancesBuffer = GEngine->GetRenderDevice()->CreateBuffer(BufferDesc("Instances", sizeof(Matrix4) * MaxInstances, EBufferType::UniformBuffer, EBufferUsage::DynamicDraw, false));

		m_CompositeDefaultsBuffer = GEngine->GetRenderDevice()->CreateBuffer(BufferDesc("CompositeDefaults", sizeof(CompositeDefaults), EBufferType::UniformBuffer));
		m_DirLightsBuffer = GEngine->GetRenderDevice()->CreateBuffer(BufferDesc("DirLights", sizeof(DirectionalLightStorage), EBufferType::UniformBuffer));
		m_PointLightsBuffer = GEngine->GetRenderDevice()->CreateBuffer(BufferDesc("PointLights", sizeof(PointLightStorage), EBufferType::UniformBuffer, EBufferUsage::DynamicDraw, true));

		m_CompositeShader = GEngine->GetResourceManager()->LoadShader("Composite", {
			{EShaderType::Vertex, "Assets/Shaders/fs_quad.vss"},
			{EShaderType::Pixel, "Assets/Shaders/PBR/Composite.frag"}
		});
	}

	void SceneRenderer::PrepareVisibleEntities(Scene* scene, CameraComponent* camera)
	{
		for (MeshComponent* meshComponent : scene->GetComponents<MeshComponent>())
		{
			if(!meshComponent->HasMesh() || !meshComponent->IsActive() || !meshComponent->IsParentActive())
			{
				continue;
			}

			Matrix4 transform = meshComponent->GetTransformationMatrix();
			Mesh_ptr mesh = meshComponent->GetMesh();

			// TODO: Complete lod switching
			LOD lod = 0;
			const MeshLodResource& lodResource = mesh->LODResources[lod];

			for (int sectionID = 0; sectionID < lodResource.Sections.size(); ++sectionID)
			{
				const FMeshSection& meshSection = lodResource.Sections[sectionID];
				int32_t materialIndex = meshSection.MaterialIndex;

				Material_ptr material = meshComponent->GetMaterialSlot(materialIndex).Material;

				if(!material)
				{
					material = mesh->MaterialSlots[materialIndex].Material;
				}

				if(!material)
				{
					continue;
				}

				RenderSortType renderSortType = material->GetSortType();

				// TODO: if (in frustum)
				{
					VisibleEntity visibleEntity;
					visibleEntity.Material = material.get();
					visibleEntity.Mesh = mesh.get();
					visibleEntity.MeshSection = sectionID;
					visibleEntity.Lod = lod;
					visibleEntity.Transform = transform;

					m_VisibleEntities[(uint8)renderSortType].emplace_back(visibleEntity);
				}
			}
		}
	}

	void SceneRenderer::FillRenderSet(RenderSet& renderSet)
	{
		// Sort and prepare model contexts
		for (int i = 0; i < SortTypeCount; ++i)
		{
			std::vector<VisibleEntity>& visibleEntities = m_VisibleEntities[i];

			if(visibleEntities.empty())
				continue;

			std::sort(visibleEntities.begin(), visibleEntities.end(), [](const VisibleEntity& left, const VisibleEntity& right) -> bool
			{
				if(left.Material != right.Material)
					return left.Material < right.Material;

				if(left.Mesh != right.Mesh)
					return left.Mesh < right.Mesh;

				if(left.MeshSection != right.MeshSection)
					return left.MeshSection < right.MeshSection;

				return false;
			});

			VisibleEntity lastVisibleEntity = {nullptr, nullptr, 0, 0, {}};
			bool lastCanBeInstanced = false;
			ModelContext currentModelContext = {nullptr, nullptr, nullptr, nullptr, {}};

			for (const VisibleEntity& visibleEntity : visibleEntities)
			{
				bool canBeInstanced = visibleEntity.Material->HasFlag(MF_INSTANCED);

				if (lastVisibleEntity.Mesh == nullptr)
				{
					lastVisibleEntity = visibleEntity;
					lastCanBeInstanced = canBeInstanced;

					currentModelContext.Material = visibleEntity.Material;
					currentModelContext.Mesh = visibleEntity.Mesh;
					currentModelContext.LodResource = &visibleEntity.Mesh->LODResources[visibleEntity.Lod];
					currentModelContext.MeshSection = &currentModelContext.LodResource->Sections[visibleEntity.MeshSection];
					currentModelContext.Instances.push_back(visibleEntity.Transform);
					continue;
				}

				if (lastVisibleEntity == visibleEntity && lastCanBeInstanced == canBeInstanced && currentModelContext.Instances.size() < MaxInstances)
				{
					currentModelContext.Instances.push_back(visibleEntity.Transform);
				}
				else
				{
					lastVisibleEntity = visibleEntity;
					lastCanBeInstanced = canBeInstanced;

					if(!currentModelContext.Instances.empty())
					{
						renderSet.emplace_back(currentModelContext);
						currentModelContext = {nullptr, nullptr, nullptr, nullptr, {}};
					}

					currentModelContext.Material = visibleEntity.Material;
					currentModelContext.Mesh = visibleEntity.Mesh;
					currentModelContext.LodResource = &visibleEntity.Mesh->LODResources[visibleEntity.Lod];
					currentModelContext.MeshSection = &currentModelContext.LodResource->Sections[visibleEntity.MeshSection];
					currentModelContext.Instances.push_back(visibleEntity.Transform);
				}
			}

			if (!currentModelContext.Instances.empty())
			{
				renderSet.emplace_back(currentModelContext);
			}
		}
	}

	void SceneRenderer::Render(Scene* scene)
	{
		for (int i = 0; i < SortTypeCount; ++i)
		{
			m_VisibleEntities[i].clear();
		}

		DrawCallState drawCallState;

		for (CameraComponent* camera : scene->GetComponents<CameraComponent>())
		{
			if (!camera->GetOwner()->IsActive() || !camera->IsActive())
				continue;

			RenderViewPort* viewPort = camera->GetViewPort();

			if (!viewPort)
			{
				AU_LOG_ERROR("Cannot render camera ", camera->GetName(), " because it has no viewport !");
				continue;
			}

			if (camera->GetProjectionType() == CameraComponent::ProjectionType::None)
			{
				AU_LOG_ERROR("Cannot render camera ", camera->GetName(), " because it has no projection !");
				continue;
			}

			auto albedoBuffer = GEngine->GetRenderManager()->CreateTemporalRenderTarget("Albedo", (Vector2i)viewPort->ViewPort, GraphicsFormat::RGBA16_FLOAT);
			auto normalsBuffer = GEngine->GetRenderManager()->CreateTemporalRenderTarget("Normals", (Vector2i)viewPort->ViewPort, GraphicsFormat::RGB8_UNORM);
			auto depthBuffer = GEngine->GetRenderManager()->CreateTemporalRenderTarget("Depth", (Vector2i)viewPort->ViewPort, GraphicsFormat::D32);

			const FFrustum& frustum = camera->GetFrustum();
			Matrix4 viewMatrix = camera->GetViewMatrix();

			BaseVSData baseVsData;
			baseVsData.ProjectionMatrix = camera->GetProjectionMatrix();
			baseVsData.ProjectionViewMatrix = camera->GetProjectionViewMatrix();
			baseVsData.ViewMatrix = viewMatrix;
			GEngine->GetRenderDevice()->WriteBuffer(m_BaseVsDataBuffer, &baseVsData);

			drawCallState.BindUniformBuffer("BaseVSData", m_BaseVsDataBuffer);
			drawCallState.BindUniformBuffer("Instances", m_InstancesBuffer);

			drawCallState.ViewPort = viewPort->ViewPort;
			drawCallState.BindTarget(0, albedoBuffer);
			drawCallState.BindTarget(1, normalsBuffer);
			drawCallState.BindDepthTarget(depthBuffer, 0, 0);

			drawCallState.ClearColor = camera->GetClearColor();
			drawCallState.ClearColorTarget = true;

			GEngine->GetRenderDevice()->BindRenderTargets(drawCallState);
			GEngine->GetRenderDevice()->ClearRenderTargets(drawCallState);

			PrepareVisibleEntities(scene, camera);

			RenderSet modelContexts;
			FillRenderSet(modelContexts);

			RenderPass(Pass::Ambient, drawCallState, camera, modelContexts);

			/////////////////////////////////////////////////////////////////////////////////////////////////

			// Collect and update directional light buffers
			DirectionalLightStorage dirLights = {};
			dirLights.DirLightCount = 0;

			for (DirectionalLightComponent* lightComponent : scene->GetComponents<DirectionalLightComponent>())
			{
				if (!lightComponent->GetOwner()->IsActive() || !lightComponent->IsActive())
					continue;

				auto& light = dirLights.DirLights[dirLights.DirLightCount];
				light.DirectionIntensity = Vector4(lightComponent->GetForwardVector(), lightComponent->GetIntensity());
				light.Color = Vector4(lightComponent->GetColor(), 1.0f);
				dirLights.DirLightCount++;

				if(dirLights.DirLightCount == MAX_DIRECTIONAL_LIGHTS)
					break;
			}

			GEngine->GetRenderDevice()->WriteBuffer(m_DirLightsBuffer, &dirLights);

			// Collect and update point light buffers
			PointLightStorage pointLights = {};
			pointLights.PointLightCount = 0;

			for (PointLightComponent* lightComponent : scene->GetComponents<PointLightComponent>())
			{
				if (!lightComponent->GetOwner()->IsActive() || !lightComponent->IsActive())
					continue;

				auto& light = pointLights.PointLights[pointLights.PointLightCount];
				light.PositionIntensity = Vector4(lightComponent->GetWorldPosition(), lightComponent->GetIntensity());
				light.ColorRadius = Vector4(lightComponent->GetColor(), lightComponent->GetRadius());
				pointLights.PointLightCount++;

				if(pointLights.PointLightCount == MAX_POINT_LIGHTS)
					break;
			}

			GEngine->GetRenderDevice()->WriteBuffer(m_PointLightsBuffer, &pointLights);

			// Write defaults
			CompositeDefaults defaults = {};
			defaults.InvProjectionView = glm::inverse(camera->GetProjectionViewMatrix());
			defaults.ViewMatrix = camera->GetViewMatrix();
			defaults.CameraPos = Vector4(camera->GetWorldPosition(), 0);

			GEngine->GetRenderDevice()->WriteBuffer(m_CompositeDefaultsBuffer, &defaults);

			{
				DrawCallState state;
				state.Shader = m_CompositeShader;
				state.ViewPort = viewPort->ViewPort;
				state.BindTarget(0, viewPort->Target);
				state.BindTexture("AlbedoRT", albedoBuffer);
				state.BindTexture("NormalsRT", normalsBuffer);
				state.BindTexture("DepthRT", depthBuffer);

				state.BindUniformBuffer("DirectionalLightStorage", m_DirLightsBuffer);
				state.BindUniformBuffer("PointLightStorage", m_PointLightsBuffer);
				state.BindUniformBuffer("CompositeDefaults", m_CompositeDefaultsBuffer);

				state.PrimitiveType = EPrimitiveType::TriangleStrip;
				state.RasterState.CullMode = ECullMode::Front;
				state.DepthStencilState.DepthEnable = false;

				state.ClearColorTarget = false;
				state.ClearDepthTarget = false;
				GEngine->GetRenderDevice()->Draw(state, {DrawArguments(4)}, true);
			}

			depthBuffer.Free();
			normalsBuffer.Free();
			albedoBuffer.Free();
		}
	}

	void SceneRenderer::RenderPass(PassType_t pass, DrawCallState& drawCallState, CameraComponent* camera, const RenderSet& renderSet)
	{
		// Render model contexts

		Material* currentMaterial = nullptr;
		Mesh* currentMesh = nullptr;
		MeshLodResource* currentLodResource = nullptr;
		FMeshSection* currentSection = nullptr;
		bool updateInputLayout = false;

		for (const ModelContext& modelContext : renderSet)
		{
			if (currentMaterial != modelContext.Material)
			{
				if(currentMaterial)
				{
					currentMaterial->EndPass(pass, drawCallState);
				}
				currentMaterial = modelContext.Material;
				currentMaterial->BeginPass(pass, drawCallState);
				updateInputLayout = true;
			}

			if (!(currentMesh == modelContext.Mesh && currentLodResource == modelContext.LodResource && currentSection == modelContext.MeshSection))
			{
				currentMesh = modelContext.Mesh;
				currentLodResource = modelContext.LodResource;
				currentSection = modelContext.MeshSection;

				drawCallState.PrimitiveType = currentSection->PrimitiveType;
				drawCallState.InputLayoutHandle = GetInputLayoutForMesh(currentMesh);
				if (currentLodResource->IndexBuffer)
				{
					drawCallState.SetIndexBuffer(currentLodResource->IndexBuffer, currentLodResource->IndexFormat);
				}
				drawCallState.SetVertexBuffer(0, currentLodResource->VertexBuffer);
				updateInputLayout = true;
			}

			if (updateInputLayout)
			{
				GEngine->GetRenderDevice()->BindShaderInputs(drawCallState, true);
				updateInputLayout = false;
			}


			drawCallState.RasterState.CullMode = ECullMode::Front;
			GEngine->GetRenderDevice()->SetRasterState(drawCallState.RasterState);

			// Write instances
			GEngine->GetRenderDevice()->WriteBuffer(m_InstancesBuffer, modelContext.Instances.data(), modelContext.Instances.size() * sizeof(Matrix4), 0);

			DrawArguments drawArguments;
			drawArguments.VertexCount = modelContext.MeshSection->NumTriangles;
			drawArguments.StartIndexLocation = modelContext.MeshSection->FirstIndex * 4;
			drawArguments.InstanceCount = modelContext.Instances.size();
			GEngine->GetRenderDevice()->DrawIndexed(drawCallState, {drawArguments}, false);

			drawCallState.ClearColorTarget = false;
			drawCallState.ClearDepthTarget = false;
			drawCallState.ClearStencilTarget = false;
		}

		if(currentMaterial)
		{
			currentMaterial->EndPass(pass, drawCallState);
		}

		m_InjectedPasses[pass].Invoke(std::forward<PassType_t>(pass), drawCallState, std::forward<CameraComponent*>(camera));
	}

	const InputLayout_ptr &SceneRenderer::GetInputLayoutForMesh(Mesh* mesh)
	{
		auto it = m_MeshInputLayouts.find(mesh->GetTypeID());

		if(it != m_MeshInputLayouts.end())
		{
			return it->second;
		}

		return (m_MeshInputLayouts[mesh->GetTypeID()] = GEngine->GetRenderDevice()->CreateInputLayout(mesh->GetVertexLayoutDesc()));
	}
}
