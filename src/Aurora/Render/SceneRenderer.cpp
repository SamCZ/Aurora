#include "SceneRenderer.hpp"

#include "Aurora/Framework/Scene.hpp"
#include "Aurora/Framework/CameraComponent.hpp"
#include "Aurora/Framework/MeshComponent.hpp"

#include "Shaders/vs_common.h"

namespace Aurora
{
	SceneRenderer::SceneRenderer()
	{
		m_InstancesBuffer = GEngine->GetRenderDevice()->CreateBuffer(BufferDesc("Instances", sizeof(Matrix4) * MaxInstances, EBufferType::UniformBuffer, EBufferUsage::DynamicDraw, false));
		m_BaseVsDataBuffer = GEngine->GetRenderDevice()->CreateBuffer(BufferDesc("BaseVSData", sizeof(BaseVSData), EBufferType::UniformBuffer));
		m_GlobDataBuffer = GEngine->GetRenderDevice()->CreateBuffer(BufferDesc("GlobData", sizeof(GLOB_Data), EBufferType::UniformBuffer));
		m_BonesBuffer = GEngine->GetRenderDevice()->CreateBuffer(BufferDesc("Bones", sizeof(Matrix4) * MAX_BONES, EBufferType::UniformBuffer));

		Matrix4* bones = GEngine->GetRenderDevice()->MapBuffer<Matrix4>(m_BonesBuffer, EBufferAccess::WriteOnly);
		for (int i = 0; i < MAX_BONES; ++i)
		{
			bones[i] = glm::identity<Matrix4>();
		}
		GEngine->GetRenderDevice()->UnmapBuffer(m_BonesBuffer);
	}

	void SceneRenderer::LoadShaders()
	{

	}

	/*TemporalRenderTarget SceneRenderer::Blur(const Texture_ptr& input, const Vector2i& resultSize)
	{

	}*/

	void SceneRenderer::PrepareMeshComponent(MeshComponent* meshComponent, CameraComponent* camera)
	{
		if(!meshComponent->HasMesh() || !meshComponent->IsActive() || !meshComponent->IsParentActive())
		{
			return;
		}

		Matrix4 transform = meshComponent->GetTransformationMatrix();
		Mesh_ptr mesh = meshComponent->GetMesh();

		if (!camera->GetFrustum().IsBoxVisible(mesh->m_Bounds.Transform(transform)) && !meshComponent->IsIgnoringFrustumChecks())
		{
			return;
		}

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

			{
				VisibleEntity visibleEntity;
				visibleEntity.Material = material.get();
				visibleEntity.MeshComponent = meshComponent;
				visibleEntity.Mesh = mesh.get();
				visibleEntity.MeshSection = sectionID;
				visibleEntity.Lod = lod;
				visibleEntity.Transform = transform;

				m_VisibleEntities[(uint8)renderSortType].emplace_back(visibleEntity);
			}
		}
	}

	void SceneRenderer::PrepareVisibleEntities(Scene* scene, CameraComponent* camera)
	{
		for (MeshComponent* meshComponent : scene->GetComponents<MeshComponent>())
		{
			PrepareMeshComponent(meshComponent, camera);
		}
	}

	void SceneRenderer::PrepareVisibleEntities(Actor* actor, CameraComponent* camera)
	{
		for (MeshComponent* meshComponent : actor->FindComponentsOfType<MeshComponent>())
		{
			PrepareMeshComponent(meshComponent, camera);
		}
	}

	void SceneRenderer::FillRenderSet(RenderSet& renderSet, int numberOfPasses, ...)
	{
		std::va_list args;
		va_start(args, numberOfPasses);

		// Sort and prepare model contexts
		for (uint8_t j = 0; j < numberOfPasses; ++j)
		{
			uint8_t i = (uint8_t) va_arg(args, RenderSortType);

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

			VisibleEntity lastVisibleEntity = {nullptr, nullptr, nullptr, 0, 0, {}};
			bool lastCanBeInstanced = false;
			ModelContext currentModelContext = {nullptr, nullptr, nullptr, nullptr, nullptr, {}};

			for (const VisibleEntity& visibleEntity : visibleEntities)
			{
				bool canBeInstanced = visibleEntity.Material->HasFlag(MF_INSTANCED);

				if (lastVisibleEntity.Mesh == nullptr)
				{
					lastVisibleEntity = visibleEntity;
					lastCanBeInstanced = canBeInstanced;

					currentModelContext.Material = visibleEntity.Material;
					currentModelContext.MeshComponent = visibleEntity.MeshComponent;
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
						currentModelContext = {nullptr, nullptr, nullptr, nullptr, nullptr, {}};
					}

					currentModelContext.Material = visibleEntity.Material;
					currentModelContext.MeshComponent = visibleEntity.MeshComponent;
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
		va_end(args);
	}

	void SceneRenderer::RenderPass(PassType_t pass, DrawCallState& drawCallState, CameraComponent* camera, const RenderSet& renderSet, bool drawInjected)
	{
		// Render model contexts

		Material* currentMaterial = nullptr;
		Mesh* currentMesh = nullptr;
		MeshLodResource* currentLodResource = nullptr;
		FMeshSection* currentSection = nullptr;
		MeshComponent* currentComponent = nullptr;
		bool updateInputLayout = false;

		for (const ModelContext& modelContext : renderSet)
		{
			if (currentMaterial != modelContext.Material)
			{
				if (currentMaterial)
				{
					currentMaterial->EndPass(pass, drawCallState);
				}
				currentMaterial = modelContext.Material;
				currentMaterial->BeforeMaterialBegin.Invoke(std::forward<PassType_t>(pass), std::forward<DrawCallState&>(drawCallState), std::forward<CameraComponent*>(camera), std::forward<Material*>(currentMaterial));
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

			if (currentComponent != modelContext.MeshComponent)
			{
				currentComponent = modelContext.MeshComponent;

				currentComponent->UploadAnimation(m_BonesBuffer);
				drawCallState.BindUniformBuffer("GLOB_BoneData", m_BonesBuffer);
				GEngine->GetRenderDevice()->BindShaderResources(drawCallState);
			}

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

		if (currentMaterial)
		{
			currentMaterial->EndPass(pass, drawCallState);
		}

		if (drawInjected)
			m_InjectedPasses[pass].Invoke(std::forward<PassType_t>(pass), drawCallState, std::forward<CameraComponent*>(camera));
	}

	const InputLayout_ptr& SceneRenderer::GetInputLayoutForMesh(Mesh* mesh)
	{
		auto it = m_MeshInputLayouts.find(mesh->GetTypeID());

		if(it != m_MeshInputLayouts.end())
		{
			return it->second;
		}

		return (m_MeshInputLayouts[mesh->GetTypeID()] = GEngine->GetRenderDevice()->CreateInputLayout(mesh->GetVertexLayoutDesc()));
	}
}