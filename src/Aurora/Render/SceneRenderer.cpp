#include "SceneRenderer.hpp"

#include "Aurora/Core/Profiler.hpp"
#include "Aurora/Framework/Scene.hpp"
#include "Aurora/Framework/CameraComponent.hpp"
#include "Aurora/Framework/MeshComponent.hpp"

#include "Aurora/Resource/ResourceManager.hpp"

#include "Shaders/vs_common.h"
#include "Shaders/PostProcess/ub_bloom.h"

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

		m_BloomDescBuffer = GEngine->GetRenderDevice()->CreateBuffer(BufferDesc("BloomDesc", sizeof(BloomDesc), EBufferType::UniformBuffer, EBufferUsage::DynamicDraw));
	}

	void SceneRenderer::LoadShaders()
	{
		m_BloomShader = GEngine->GetResourceManager()->LoadComputeShader("Assets/Shaders/PostProcess/bloom.glsl");

		m_BloomShaderSS = GEngine->GetResourceManager()->LoadShader("BloomScreenSpace", {
			{EShaderType::Vertex, "Assets/Shaders/FSQuad.vert"},
			{EShaderType::Pixel, "Assets/Shaders/PostProcess/bloom.frag"}
		});
	}

	void SceneRenderer::PrepareMeshComponent(MeshComponent* meshComponent, CameraComponent* camera, const FFrustum& frustum)
	{
		if(!meshComponent->HasMesh() || !meshComponent->IsActive() || !meshComponent->IsParentActive())
		{
			return;
		}

		Matrix4 transform = meshComponent->GetTransformationMatrix();
		Mesh_ptr mesh = meshComponent->GetMesh();

		if (not frustum.IsBoxVisible(mesh->m_Bounds.Transform(transform)) &&  not meshComponent->IsIgnoringFrustumChecks())
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

	void SceneRenderer::PrepareVisibleEntities(Scene* scene, CameraComponent* camera, const FFrustum& frustum)
	{
		for (MeshComponent* meshComponent : scene->GetComponents<MeshComponent>())
		{
			PrepareMeshComponent(meshComponent, camera, frustum);
		}
	}

	void SceneRenderer::PrepareVisibleEntities(Actor* actor, CameraComponent* camera, const FFrustum& frustum)
	{
		for (MeshComponent* meshComponent : actor->FindComponentsOfType<MeshComponent>())
		{
			PrepareMeshComponent(meshComponent, camera, frustum);
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

	TemporalRenderTarget SceneRenderer::RenderBloom(const FViewPort& wp, const Texture_ptr& inputHDRRT)
	{
		TemporalRenderTarget bloomRTs[3];
		if(m_BloomSettings.Enabled)
		{ // Bloom
			CPU_DEBUG_SCOPE("Bloom");
			GPU_DEBUG_SCOPE("Bloom");

			Vector2ui bloomTexSize = (Vector2i)wp / 2;
			bloomTexSize += Vector2ui(m_BloomComputeWorkgroupSize - (bloomTexSize.x % m_BloomComputeWorkgroupSize), m_BloomComputeWorkgroupSize - (bloomTexSize.y % m_BloomComputeWorkgroupSize));

			uint32_t mips = TextureDesc::GetMipLevelCount(bloomTexSize.x, bloomTexSize.y) - 2;
			mips = std::min<uint32_t>(mips, 4);
			//mips -= mips / 3;

			for (int i = 0; i < 3; ++i)
			{
				std::string texName = "BloomCompute #" + std::to_string(i);

				bloomRTs[i] = GEngine->GetRenderManager()->CreateTemporalRenderTarget(texName, bloomTexSize, GraphicsFormat::RGBA16_FLOAT, EDimensionType::TYPE_2D, mips, 0, TextureDesc::EUsage::Default, true);
			}

			BloomDesc bloomDesc;
			bloomDesc.Params = { m_BloomSettings.Threshold, m_BloomSettings.Threshold - m_BloomSettings.Knee, m_BloomSettings.Knee * 2.0f, 0.25f / m_BloomSettings.Knee };
			bloomDesc.LodAndMode.x = 0;
			bloomDesc.LodAndMode.y = BLOOM_MODE_PREFILTER;

			if (not m_BloomSettings.UseComputeShader)
			{
				DrawCallState state;
				state.Shader = m_BloomShaderSS;
				state.ViewPort = bloomRTs[0]->GetDesc().GetSize();

				state.PrimitiveType = EPrimitiveType::TriangleStrip;
				state.RasterState.CullMode = ECullMode::Front;
				state.DepthStencilState.DepthEnable = false;
				state.ClearColorTarget = false;
				state.ClearDepthTarget = false;

				state.BindUniformBuffer("BloomDesc", m_BloomDescBuffer);
				state.BindSampler("u_Texture", Samplers::ClampClampLinearLinear);
				state.BindSampler("u_BloomTexture", Samplers::ClampClampLinearLinear);

				state.BindTexture("u_Texture", inputHDRRT);
				state.BindTexture("u_BloomTexture", inputHDRRT);
				state.BindTexture("o_Image", bloomRTs[0], true);

				state.BindTarget(0, bloomRTs[0], 0, 0);
				bloomDesc.HalfTexel = (1.0f / (Vector2)bloomRTs[0]->GetDesc().GetSize()) * 0.5f;
				GEngine->GetRenderDevice()->WriteBuffer(m_BloomDescBuffer, &bloomDesc);

				// Prefilter
				GEngine->GetRenderDevice()->Draw(state, {DrawArguments(4)}, true);

				// // Downsample
				bloomDesc.LodAndMode.y = BLOOM_MODE_DOWNSAMPLE;

				for (uint32_t i = 1; i < mips; i++)
				{
					auto mipSize = bloomRTs[0]->GetDesc().GetMipSize(i);

					{ // Ping
						// Output
						state.BindTarget(0, bloomRTs[1], 0, i);
						state.ViewPort = mipSize;
						bloomDesc.HalfTexel = (1.0f / (Vector2)mipSize) * 0.5f;
						// Input
						bloomDesc.LodAndMode.x = (float)i - 1.0f;
						GEngine->GetRenderDevice()->WriteBuffer(m_BloomDescBuffer, &bloomDesc);

						state.BindTexture("u_Texture", bloomRTs[0]);
						GEngine->GetRenderDevice()->Draw(state, {DrawArguments(4)}, true);
					}

					{ // Pong
						//Output
						state.BindTarget(0, bloomRTs[0], 0, i);
						state.ViewPort = mipSize;
						bloomDesc.HalfTexel = (1.0f / (Vector2)mipSize) * 0.5f;
						// Input
						bloomDesc.LodAndMode.x = (float)i;
						GEngine->GetRenderDevice()->WriteBuffer(m_BloomDescBuffer, &bloomDesc);
						state.BindTexture("u_Texture", bloomRTs[1]);
						GEngine->GetRenderDevice()->Draw(state, {DrawArguments(4)}, true);
					}
				}

				// Upsample first
				bloomDesc.LodAndMode.y = BLOOM_MODE_UPSAMPLE_FIRST;

				{ // Upsample First
					//Output
					auto mipSize = bloomRTs[2]->GetDesc().GetMipSize(mips - 1);

					state.BindTarget(0, bloomRTs[2], 0, mips - 1);
					state.ViewPort = mipSize;
					bloomDesc.HalfTexel = (1.0f / (Vector2)mipSize) * 0.5f;

					// Input
					bloomDesc.LodAndMode.x = (float)mips - 2.0f;
					GEngine->GetRenderDevice()->WriteBuffer(m_BloomDescBuffer, &bloomDesc);
					state.BindTexture("u_Texture", bloomRTs[0]);

					GEngine->GetRenderDevice()->Draw(state, {DrawArguments(4)}, true);
				}

				// Upsample
				bloomDesc.LodAndMode.y = BLOOM_MODE_UPSAMPLE;

				for (int32_t mip = mips - 2; mip >= 0; mip--)
				{
					auto mipSize = bloomRTs[2]->GetDesc().GetMipSize(mip);

					//Output
					state.BindTarget(0, bloomRTs[2], 0, mip);
					state.ViewPort = mipSize;
					bloomDesc.HalfTexel = (1.0f / (Vector2)mipSize) * 0.5f;

					// Input
					bloomDesc.LodAndMode.x = mip;
					GEngine->GetRenderDevice()->WriteBuffer(m_BloomDescBuffer, &bloomDesc);
					state.BindTexture("u_Texture", bloomRTs[0]);
					state.BindTexture("u_BloomTexture", bloomRTs[2]);

					GEngine->GetRenderDevice()->Draw(state, {DrawArguments(4)}, true);
				}
			}
			else
			{
				uint32_t workGroupsX = bloomTexSize.x / m_BloomComputeWorkgroupSize;
				uint32_t workGroupsY = bloomTexSize.y / m_BloomComputeWorkgroupSize;

				DispatchState dispatchState;
				dispatchState.Shader = m_BloomShader;
				dispatchState.BindUniformBuffer("BloomDesc", m_BloomDescBuffer);
				dispatchState.BindSampler("u_Texture", Samplers::ClampClampLinearLinear);
				dispatchState.BindSampler("u_BloomTexture", Samplers::ClampClampLinearLinear);

				GEngine->GetRenderDevice()->WriteBuffer(m_BloomDescBuffer, &bloomDesc);

				dispatchState.BindTexture("u_Texture", inputHDRRT);
				dispatchState.BindTexture("u_BloomTexture", inputHDRRT);
				dispatchState.BindTexture("o_Image", bloomRTs[0], true);

				// Prefilter
				GEngine->GetRenderDevice()->Dispatch(dispatchState, workGroupsX, workGroupsY, 1);

				// Downsample
				bloomDesc.LodAndMode.y = BLOOM_MODE_DOWNSAMPLE;

				for (uint32_t i = 1; i < mips; i++)
				{
					auto mipSize = bloomRTs[0]->GetDesc().GetMipSize(i);

					workGroupsX = (uint32_t)glm::ceil((float)mipSize.x / (float)m_BloomComputeWorkgroupSize);
					workGroupsY = (uint32_t)glm::ceil((float)mipSize.y / (float)m_BloomComputeWorkgroupSize);

					{ // Ping
						// Output
						dispatchState.BindTexture("o_Image", bloomRTs[1], true, Aurora::TextureBinding::EAccess::Write, i);
						// Input
						bloomDesc.LodAndMode.x = (float)i - 1.0f;
						GEngine->GetRenderDevice()->WriteBuffer(m_BloomDescBuffer, &bloomDesc);

						dispatchState.BindTexture("u_Texture", bloomRTs[0]);
						GEngine->GetRenderDevice()->Dispatch(dispatchState, workGroupsX, workGroupsY, 1);
					}

					{ // Pong
						//Output
						dispatchState.BindTexture("o_Image", bloomRTs[0], true, Aurora::TextureBinding::EAccess::Write, i);

						// Input
						bloomDesc.LodAndMode.x = (float)i;
						GEngine->GetRenderDevice()->WriteBuffer(m_BloomDescBuffer, &bloomDesc);
						dispatchState.BindTexture("u_Texture", bloomRTs[1]);
						GEngine->GetRenderDevice()->Dispatch(dispatchState, workGroupsX, workGroupsY, 1);
					}
				}

				// Upsample first
				bloomDesc.LodAndMode.y = BLOOM_MODE_UPSAMPLE_FIRST;

				{ // Upsample First
					//Output
					dispatchState.BindTexture("o_Image", bloomRTs[2], true, Aurora::TextureBinding::EAccess::Write, mips - 1);

					// Input
					bloomDesc.LodAndMode.x = (float)mips - 2.0f;
					GEngine->GetRenderDevice()->WriteBuffer(m_BloomDescBuffer, &bloomDesc);
					dispatchState.BindTexture("u_Texture", bloomRTs[0]);

					auto mipSize = bloomRTs[2]->GetDesc().GetMipSize(mips - 1);
					workGroupsX = (uint32_t)glm::ceil((float)mipSize.x / (float)m_BloomComputeWorkgroupSize);
					workGroupsY = (uint32_t)glm::ceil((float)mipSize.y / (float)m_BloomComputeWorkgroupSize);

					GEngine->GetRenderDevice()->Dispatch(dispatchState, workGroupsX, workGroupsY, 1);
				}

				// Upsample
				bloomDesc.LodAndMode.y = BLOOM_MODE_UPSAMPLE;

				for (int32_t mip = mips - 2; mip >= 0; mip--)
				{
					auto mipSize = bloomRTs[2]->GetDesc().GetMipSize(mip);
					workGroupsX = (uint32_t)glm::ceil((float)mipSize.x / (float)m_BloomComputeWorkgroupSize);
					workGroupsY = (uint32_t)glm::ceil((float)mipSize.y / (float)m_BloomComputeWorkgroupSize);

					//Output
					dispatchState.BindTexture("o_Image", bloomRTs[2], true, Aurora::TextureBinding::EAccess::Write, mip);

					// Input
					bloomDesc.LodAndMode.x = mip;
					GEngine->GetRenderDevice()->WriteBuffer(m_BloomDescBuffer, &bloomDesc);
					dispatchState.BindTexture("u_Texture", bloomRTs[0]);
					dispatchState.BindTexture("u_BloomTexture", bloomRTs[2]);

					GEngine->GetRenderDevice()->Dispatch(dispatchState, workGroupsX, workGroupsY, 1);
				}
			}
		}

		bloomRTs[0].Free();
		bloomRTs[1].Free();

		return bloomRTs[2];
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