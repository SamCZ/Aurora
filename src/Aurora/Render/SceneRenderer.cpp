#include "SceneRenderer.hpp"

#include "Aurora/Engine.hpp"

#include "Aurora/Framework/Scene.hpp"
#include "Aurora/Framework/CameraComponent.hpp"
#include "Aurora/Framework/MeshComponent.hpp"

#include "Aurora/Graphics/Base/IRenderDevice.hpp"
#include "Aurora/Graphics/ViewPortManager.hpp"
#include "Aurora/Graphics/RenderManager.hpp"

#include "Shaders/vs_common.h"

namespace Aurora
{
	static InputLayout_ptr testInputLayout = nullptr;

	SceneRenderer::SceneRenderer()
	{
		testInputLayout = GEngine->GetRenderDevice()->CreateInputLayout({
			{"POSITION", GraphicsFormat::RGB32_FLOAT, 0, offsetof(StaticMesh::Vertex, Position), 0, sizeof(StaticMesh::Vertex), false, false},
			{"NORMAL", GraphicsFormat::RGB32_FLOAT, 0, offsetof(StaticMesh::Vertex, Normal), 1, sizeof(StaticMesh::Vertex), false, false}
		});

		m_BaseVsDataBuffer = GEngine->GetRenderDevice()->CreateBuffer(BufferDesc("BaseVSData", sizeof(BaseVSData), EBufferType::UniformBuffer));
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
			if (!camera->IsActive())
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

			auto depthBuffer = GEngine->GetRenderManager()->CreateTemporalRenderTarget("Depth", {viewPort->ViewPort.Width, viewPort->ViewPort.Height}, GraphicsFormat::D32);

			BaseVSData baseVsData;
			baseVsData.ProjectionMatrix = camera->GetProjectionMatrix();
			baseVsData.ProjectionViewMatrix = camera->GetProjectionViewMatrix();
			baseVsData.ViewMatrix = camera->GetViewMatrix();
			drawCallState.BindUniformBuffer("BaseVSData", m_BaseVsDataBuffer);

			drawCallState.ViewPort = viewPort->ViewPort;
			drawCallState.BindTarget(0, viewPort->Target);
			drawCallState.BindDepthTarget(depthBuffer, 0, 0);

			drawCallState.ClearColor = camera->GetClearColor();
			drawCallState.ClearColorTarget = true;

			GEngine->GetRenderDevice()->BindRenderTargets(drawCallState);
			GEngine->GetRenderDevice()->ClearRenderTargets(drawCallState);

			for (MeshComponent* meshComponent : scene->GetComponents<MeshComponent>())
			{
				if(!meshComponent->HasMesh() || !meshComponent->IsActive())
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
					const MeshSection& meshSection = lodResource.Sections[sectionID];
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

			RenderSet modelContexts;

			// Sort and prepare model contexts
			for (int i = 0; i < SortTypeCount; ++i)
			{
				std::vector<VisibleEntity>& visibleEntities = m_VisibleEntities[i];

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
							modelContexts.emplace_back(currentModelContext);
							currentModelContext = {nullptr, nullptr, nullptr, {}};
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
					modelContexts.emplace_back(currentModelContext);
				}
			}

			{ // TODO: Move this to some method
				// Render model contexts

				PassType_t pass = Pass::Ambient;

				Material* currentMaterial = nullptr;
				Mesh* currentMesh = nullptr;
				MeshLodResource* currentLodResource = nullptr;
				MeshSection* currentSection = nullptr;

				for (const ModelContext& modelContext : modelContexts)
				{
					if (currentMaterial != modelContext.Material)
					{
						if(currentMaterial)
						{
							currentMaterial->EndPass(pass, drawCallState);
						}
						currentMaterial = modelContext.Material;
						currentMaterial->BeginPass(pass, drawCallState);
					}

					if (!(currentMesh == modelContext.Mesh && currentLodResource == modelContext.LodResource && currentSection == modelContext.MeshSection))
					{
						currentMesh == modelContext.Mesh;
						currentLodResource = modelContext.LodResource;
						currentSection = modelContext.MeshSection;

						drawCallState.PrimitiveType = currentSection->PrimitiveType;
						// TODO: drawCallState.InputLayoutHandle =
						drawCallState.InputLayoutHandle = testInputLayout;
						if (currentLodResource->IndexBuffer)
						{
							drawCallState.SetIndexBuffer(currentLodResource->IndexBuffer, currentLodResource->IndexFormat);
						}
						drawCallState.SetVertexBuffer(0, currentLodResource->VertexBuffer);

						GEngine->GetRenderDevice()->BindShaderInputs(drawCallState, true);
					}

					//drawCallState.RasterState.CullMode = ECullMode::None;
					GEngine->GetRenderDevice()->SetRasterState(drawCallState.RasterState);

					// TODO: Change this to instance rendering, this is just for test
					for (const Matrix4& instance : modelContext.Instances)
					{
						baseVsData.ModelTransform = instance;
						GEngine->GetRenderDevice()->WriteBuffer(m_BaseVsDataBuffer, &baseVsData);

						DrawArguments drawArguments;
						drawArguments.VertexCount = modelContext.MeshSection->NumTriangles;
						drawArguments.StartIndexLocation = modelContext.MeshSection->FirstIndex * 4;
						drawArguments.InstanceCount = 1;
						GEngine->GetRenderDevice()->DrawIndexed(drawCallState, {drawArguments}, false);

						drawCallState.ClearColorTarget = false;
						drawCallState.ClearDepthTarget = false;
						drawCallState.ClearStencilTarget = false;
					}
				}

				if(currentMaterial)
				{
					currentMaterial->EndPass(pass, drawCallState);
				}
			}

			depthBuffer.Free();
		}
	}
}
