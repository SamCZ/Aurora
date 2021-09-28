#include "SceneRenderer.hpp"

#include "Aurora/Framework/Scene.hpp"
#include "Aurora/Framework/Entity.hpp"
#include "Aurora/Physics/Frustum.hpp"
#include "Aurora/Graphics/Material/Material.hpp"

#include "Shaders/vs_common.h"
#include "Shaders/ps_common.h"

namespace Aurora
{
	template< typename T >
	class pointer_comparator : public std::binary_function< T, T, bool >
	{
	public :
		bool operator()( T x, T y ) const { return x < y; }
	};

	SceneRenderer::SceneRenderer(Scene *scene, RenderManager* renderManager, IRenderDevice* renderDevice)
	: m_Scene(scene), m_RenderDevice(renderDevice), m_RenderManager(renderManager)
	{
		m_InstancingBuffer = m_RenderDevice->CreateBuffer(BufferDesc("InstanceBuffer", sizeof(Matrix4) * MAX_INSTANCES, EBufferType::UniformBuffer));
		m_BaseVSDataBuffer = m_RenderDevice->CreateBuffer(BufferDesc("BaseVSData", sizeof(BaseVSData), EBufferType::UniformBuffer));
	}

	SceneRenderer::~SceneRenderer()
	{

	}

	void SceneRenderer::AddVisibleEntity(Material* material, XMesh* mesh, uint meshSection, const Matrix4& transform)
	{
		CPU_DEBUG_SCOPE("SceneRenderer::AddVisibleEntity")
		entt::entity visibleEntityID = m_VisibleEntitiesRegistry.create();
		VisibleEntity& visibleEntity = m_VisibleEntitiesRegistry.emplace<VisibleEntity>(visibleEntityID);
		visibleEntity.Material = material;
		visibleEntity.Mesh = mesh;
		visibleEntity.MeshSection = meshSection;
		visibleEntity.Transform = transform;
		m_VisibleEntities.emplace_back(visibleEntityID);
		m_VisibleTypeCounters[material->GetTypeID()]++;
	}

	void SceneRenderer::PrepareRender()
	{
		CPU_DEBUG_SCOPE("SceneRenderer::PrepareRender")
		m_VisibleEntities.clear();
		m_VisibleEntitiesRegistry.clear();
		m_VisibleTypeCounters.clear();

		for (int i = 0; i < SortTypeCount; ++i)
		{
			m_FinalSortedEntities[i].clear();
		}

		auto meshesView = m_Scene->GetRegistry().view<TransformComponent, MeshComponent>();

		for(entt::entity meshesEntt : meshesView)
		{
			auto [transform, meshComponent] = meshesView.get<TransformComponent, MeshComponent>(meshesEntt);
			const std::shared_ptr<XMesh>& mesh = meshComponent.Mesh;

			if(mesh == nullptr) continue;

			for (size_t i = 0; i < mesh->m_Sections.size(); ++i)
			{
				const XMesh::PrimitiveSection& section = mesh->m_Sections[i];

				matref material = meshComponent.GetMaterial(section.MaterialIndex);

				if(material == nullptr) continue;

				// Check frustum culling

				Matrix4 modelTransform = transform.GetTransform();

				/*
				 * material, mesh, transform
				 */


				AddVisibleEntity(material.get(), mesh.get(), i, modelTransform);
			}
		}
	}

	void SceneRenderer::SortVisibleEntities()
	{
		CPU_DEBUG_SCOPE("SceneRenderer::SortVisibleEntities")
		// Sort by material base
		std::sort(m_VisibleEntities.begin(), m_VisibleEntities.end(), [this](const entt::entity current, const entt::entity other) -> bool {
			const auto& currentEntt = m_VisibleEntitiesRegistry.get<VisibleEntity>(current);
			const auto& otherEntt = m_VisibleEntitiesRegistry.get<VisibleEntity>(other);

			if(currentEntt.Mesh < otherEntt.Mesh) return true;
			if(currentEntt.Mesh > otherEntt.Mesh) return false;

			if(currentEntt.MeshSection < otherEntt.MeshSection) return true;
			if(currentEntt.MeshSection > otherEntt.MeshSection) return false;

			if(currentEntt.Material < otherEntt.Material) return true;
			if(currentEntt.Material > otherEntt.Material) return false;

			return false;
		});

		for (auto e : m_VisibleEntities)
		{
			const VisibleEntity& ve = m_VisibleEntitiesRegistry.get<VisibleEntity>(e);
			m_FinalSortedEntities[(uint8_t)ve.Material->GetSortType()].push_back(e);
		}
	}

	RenderSet SceneRenderer::BuildRenderSet()
	{
		std::vector<ModelContext> modelContexts;

		for (int i = 0; i < SortTypeCount; ++i)
		{
			XMesh* lastMesh = nullptr;
			Material* lastMaterial = nullptr;
			uint lastSection = 0;
			bool lastCanBeInstanced = true;

			ModelContext currentModelContext;

			for(auto e : m_FinalSortedEntities[i])
			{
				const VisibleEntity& visibleEntity = m_VisibleEntitiesRegistry.get<VisibleEntity>(e);
				XMesh::PrimitiveSection& section = visibleEntity.Mesh->m_Sections[visibleEntity.MeshSection];
				bool canBeInstanced = !!(visibleEntity.Material->GetFlags() & MaterialFlags::Instanced);

				//std::cout << visibleEntity.Material->GetTypeName() << " - " << visibleEntity.Mesh << " - " << visibleEntity.MeshSection << std::endl;

				// Initialize last variables
				if(lastMesh == nullptr)
				{
					lastMesh = visibleEntity.Mesh;
					lastMaterial = visibleEntity.Material;
					lastCanBeInstanced = canBeInstanced;
					lastSection = visibleEntity.MeshSection;

					currentModelContext.Material = visibleEntity.Material;
					currentModelContext.Mesh = visibleEntity.Mesh;
					currentModelContext.MeshSection = &section;
					currentModelContext.Instances.push_back(visibleEntity.Transform);
					continue;
				}

				if(lastMesh == visibleEntity.Mesh && lastMaterial == visibleEntity.Material && lastCanBeInstanced == canBeInstanced && lastSection == visibleEntity.MeshSection)
				{
					currentModelContext.Instances.push_back(visibleEntity.Transform);
				}
				else
				{
					lastMesh = visibleEntity.Mesh;
					lastMaterial = visibleEntity.Material;
					lastCanBeInstanced = canBeInstanced;
					lastSection = visibleEntity.MeshSection;

					if(!currentModelContext.Instances.empty())
					{
						modelContexts.emplace_back(currentModelContext);
						currentModelContext = {};

						currentModelContext.Material = visibleEntity.Material;
						currentModelContext.Mesh = visibleEntity.Mesh;
						currentModelContext.MeshSection = &section;
						currentModelContext.Instances.push_back(visibleEntity.Transform);
					}
				}
			}

			if(!currentModelContext.Instances.empty())
			{
				modelContexts.emplace_back(currentModelContext);
			}

		}

		return modelContexts;
	}

	void SceneRenderer::Render(entt::entity cameraEntityID)
	{
		CPU_DEBUG_SCOPE("SceneRenderer::Render")
		assert(m_Scene);

		Entity cameraEntity(cameraEntityID, m_Scene);
		auto& cameraTransform = cameraEntity.GetComponent<TransformComponent>();
		auto& camera = cameraEntity.GetComponent<CameraComponent>();
		Matrix4 projectionViewMatrix = camera.Projection * glm::inverse(cameraTransform.GetTransform());
		Frustum frustum(projectionViewMatrix);

		/*{
			auto* baseVsData = m_RenderDevice->MapBuffer<BaseVSData>(m_BaseVSDataBuffer, EBufferAccess::WriteOnly);
			baseVsData->ProjectionViewMatrix = projectionViewMatrix;
			m_RenderDevice->UnmapBuffer(m_BaseVSDataBuffer);
		}*/
		/*{
			BaseVSData baseVsData {projectionViewMatrix};
			m_RenderDevice->WriteBuffer(m_BaseVSDataBuffer, &baseVsData, sizeof baseVsData);
		}*/

		PrepareRender();
		SortVisibleEntities();

		// Actual render

		DrawCallState drawState;
		//drawCallState.BindUniformBuffer("BaseVSData", m_BaseVSDataBuffer);
		drawState.BindUniformBuffer("Instances", m_InstancingBuffer);

		{
			BEGIN_UB(BaseVSData, baseVsData)
				baseVsData->ProjectionViewMatrix = projectionViewMatrix;
			END_UB(BaseVSData)
		}

		drawState.ClearDepthTarget = true;
		drawState.ClearColorTarget = true;
		drawState.DepthStencilState.DepthEnable = true;
		drawState.RasterState.CullMode = ECullMode::Back;

		drawState.ViewPort = camera.Size;

		RenderSet globalRenderSet = BuildRenderSet();

		auto albedoAndFlagsRT = m_RenderManager->CreateTemporalRenderTarget("Albedo", camera.Size, GraphicsFormat::RGBA8_UNORM);
		auto normalsRT = m_RenderManager->CreateTemporalRenderTarget("Normals", camera.Size, GraphicsFormat::RGBA8_UNORM);
		auto roughnessMetallicAORT = m_RenderManager->CreateTemporalRenderTarget("RoughnessMetallicAO", camera.Size, GraphicsFormat::RGBA8_UNORM);

		auto depthRT = m_RenderManager->CreateTemporalRenderTarget("Depth", camera.Size, GraphicsFormat::D32);

		drawState.BindDepthTarget(depthRT, 0, 0);
		drawState.BindTarget(0, albedoAndFlagsRT);
		drawState.BindTarget(1, normalsRT);
		drawState.BindTarget(2, roughnessMetallicAORT);

		m_RenderDevice->BindRenderTargets(drawState);
		m_RenderDevice->ClearRenderTargets(drawState);
		RenderPass(drawState, globalRenderSet, EPassType::Ambient);

		{ // Composite Deferred renderer

			{
				/*VBufferCacheIndex cacheIndex;
				Vector4* test = m_RenderManager->GetUniformBufferCache().GetOrMap<Vector4>(sizeof(Vector4) * 1, cacheIndex);
				if(test)
				{
					*test = Vector4(0, 1, 2, 3);

					std::cout << cacheIndex.Offset << " - " << cacheIndex.BufferIndex << std::endl;

					m_RenderManager->GetUniformBufferCache().Unmap(cacheIndex);
				}*/

			}

			m_RenderManager->Blit(albedoAndFlagsRT);
		}

		albedoAndFlagsRT.Free();
		normalsRT.Free();
		roughnessMetallicAORT.Free();
		depthRT.Free();
	}

	void SceneRenderer::RenderPass(DrawCallState& drawCallState, const std::vector<ModelContext> &modelContexts, EPassType passType)
	{
		CPU_DEBUG_SCOPE("SceneRenderer::RenderPass")
		GPU_DEBUG_SCOPE(String("RenderPass [") + PassTypesToString[(int)passType] + "]");

		Material* lastMaterial = nullptr;
		XMesh* lastMesh = nullptr;
		XMesh::PrimitiveSection* lastSection = nullptr;
		for (int i = 0; i < modelContexts.size(); ++i)
		{
			const ModelContext& mc = modelContexts[i];
			Material* mat = mc.Material;
			XMesh* mesh = mc.Mesh;
			const auto& section = *mc.MeshSection;

			if(lastMaterial != mat)
			{
				mat->BeginPass(drawCallState, passType);

				m_RenderDevice->SetShader(drawCallState.Shader);
				m_RenderDevice->BindShaderResources(drawCallState);

				m_RenderDevice->SetRasterState(drawCallState.RasterState);
				m_RenderDevice->SetDepthStencilState(drawCallState.DepthStencilState);

				for (int a = 0; a < mesh->m_Buffers.size(); ++a)
				{
					drawCallState.SetVertexBuffer(a, mesh->m_Buffers[a]);
				}

				lastMaterial = mat;
			}

			if(lastMesh != mesh || lastSection != mc.MeshSection)
			{
				drawCallState.PrimitiveType = section.PrimitiveType;
				drawCallState.InputLayoutHandle = section.Layout;
				drawCallState.SetIndexBuffer(mesh->m_Buffers[section.BufferIndex], section.IndexFormat);

				m_RenderDevice->BindShaderInputs(drawCallState);

				lastMesh = mesh;
				lastSection = mc.MeshSection;
			}

			{
				//CPU_DEBUG_SCOPE("WriteInstances")
				/*auto* instancesPtr = m_RenderDevice->MapBuffer<ObjectInstanceData>(m_InstancingBuffer, EBufferAccess::WriteOnly);
				std::memcpy(instancesPtr, mc.Instances.data(), sizeof(ObjectInstanceData) * mc.Instances.size());
				m_RenderDevice->UnmapBuffer(m_InstancingBuffer);*/
				m_RenderDevice->WriteBuffer(m_InstancingBuffer, mc.Instances.data(), sizeof(Matrix4) * mc.Instances.size());
			}

			DrawArguments drawArguments;
			drawArguments.VertexCount = section.IndexCount;
			drawArguments.StartIndexLocation = section.IndexByteOffset;
			drawArguments.InstanceCount = mc.Instances.size();

			m_RenderDevice->DrawIndexed(drawCallState, {drawArguments});

			if(i != modelContexts.size() - 1)
			{
				if(modelContexts[i + 1].Material != mat)
				{
					mat->EndPass(drawCallState, passType);
					lastMaterial = nullptr;
				}
			}

		}

		if(lastMaterial)
		{
			lastMaterial->EndPass(drawCallState, passType);
		}
	}
}