#include "SceneRendererForward.hpp"

#include "Aurora/Engine.hpp"
#include "Aurora/App/AppContext.hpp"

#include "Aurora/Core/Profiler.hpp"

#include "Aurora/Framework/Scene.hpp"
#include "Aurora/Framework/CameraComponent.hpp"
#include "Aurora/Framework/MeshComponent.hpp"
#include "Aurora/Framework/Lights.hpp"
#include "Aurora/Framework/SkyLight.hpp"
#include "Aurora/Framework/Decal.hpp"
#include "Aurora/Framework/ParticleSystemComponent.hpp"

#include "Aurora/Graphics/Base/IRenderDevice.hpp"
#include "Aurora/Graphics/ViewPortManager.hpp"
#include "Aurora/Graphics/OpenGL/GLBufferLock.hpp"
#include "Aurora/Graphics/DShape.hpp"

#include "Aurora/Resource/ResourceManager.hpp"

#include "Shaders/vs_common.h"
#include "Shaders/PBR/Composite.h"
#include "Shaders/PostProcess/ub_bloom.h"
#include "Shaders/PostProcess/ub_outline.h"
#include "Shaders/Decals.h"

namespace Aurora
{
	void UpdateModelState(PassType_t pass, const RenderSet& set, void(*callback)(Material* mat, FRasterState& rasterState, FDepthStencilState& depthState, FBlendState& blendState))
	{
		Material* lastMaterial = nullptr;
		for (const ModelContext& mc : set)
		{
			if (mc.Material != lastMaterial)
			{
				lastMaterial = mc.Material;
				callback(mc.Material, mc.Material->RasterState(pass), mc.Material->DepthStencilState(pass), mc.Material->BlendState(pass));
			}
		}
	}

	SceneRendererForward::SceneRendererForward() : SceneRenderer()
	{
		m_VSDecalBuffer = GEngine->GetRenderDevice()->CreateBuffer(BufferDesc("GLOB_DecalMatricesVS", sizeof(GLOB_DecalMatricesVS), EBufferType::UniformBuffer, EBufferUsage::DynamicDraw));
		m_PSDecalBuffer = GEngine->GetRenderDevice()->CreateBuffer(BufferDesc("GLOB_DecalMatricesPS", sizeof(GLOB_DecalMatricesPS), EBufferType::UniformBuffer, EBufferUsage::DynamicDraw));

		m_ParticleInputLayout = GEngine->GetRenderDevice()->CreateInputLayout({
			VertexAttributeDesc{"in_Pos", GraphicsFormat::RGBA32_FLOAT, 0, 0, 0, sizeof(Vector4), false, false}
		});
		m_ParticleComputeShader = GEngine->GetResourceManager()->LoadComputeShader("Assets/Shaders/Forward/Particle/Particles.comp");
		m_ParticleRenderShader = GEngine->GetResourceManager()->LoadShader("Particles", {
			{EShaderType::Vertex, "Assets/Shaders/Forward/Particle/Particles.vert"},
			{EShaderType::Geometry, "Assets/Shaders/Forward/Particle/Particles.geom"},
			{EShaderType::Pixel, "Assets/Shaders/Forward/Particle/Particles.frag"}
		});

		m_TonemappingShader = GEngine->GetResourceManager()->LoadShader("Tonemaping", {
			{EShaderType::Vertex, "Assets/Shaders/FSQuad.vert"},
			{EShaderType::Pixel, "Assets/Shaders/PostProcess/Tonemapping.frag"}
		});

		m_ScreenTextureShader = GEngine->GetResourceManager()->LoadShader("ScreenSpaceQuadTexture", {
			{EShaderType::Vertex, "Assets/Shaders/FSQuad.vert"},
			{EShaderType::Pixel, "Assets/Shaders/ScreenTexture.frag"}
		});

	}

	void SceneRendererForward::Render(Scene* scene)
	{
		for (CameraComponent* camera : scene->GetComponents<CameraComponent>())
		{
			CPU_DEBUG_SCOPE("RenderCamera");

			ClearVisibleEntities();

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

			camera->UpdateFrustum();

			auto depthBuffer = GEngine->GetRenderManager()->CreateTemporalRenderTarget("Depth", (Vector2i)viewPort->ViewPort, GraphicsFormat::D32);

			const FFrustum& frustum = camera->GetFrustum();
			Matrix4 viewMatrix = camera->GetViewMatrix();

			// Prepate sets
			PrepareVisibleEntities(scene, camera);

			RenderSet modelContextsOpaque;
			FillRenderSet(modelContextsOpaque, 2, RenderSortType::Opaque, RenderSortType::Transparent);

			RenderSet translucentModelContexts;
			FillRenderSet(translucentModelContexts, 1, RenderSortType::Translucent);

			RenderSet skyModelContexts;
			FillRenderSet(skyModelContexts, 1, RenderSortType::Sky);

			RenderSet overlayModelContexts;
			FillRenderSet(overlayModelContexts, 1, RenderSortType::Overlay);

			// Setup base vs data
			BaseVSData baseVsData;
			baseVsData.ProjectionMatrix = camera->GetProjectionMatrix();
			baseVsData.ProjectionViewMatrix = camera->GetProjectionViewMatrix();
			baseVsData.ViewMatrix = viewMatrix;
			GEngine->GetRenderDevice()->WriteBuffer(m_BaseVsDataBuffer, &baseVsData);

			GEngine->GetRenderDevice()->InvalidateState();

			//if (!modelContextsOpaque.empty())
			{ // Depth pre pass
				GPU_DEBUG_SCOPE("DepthPrePass");
				CPU_DEBUG_SCOPE("DepthPrePass");

				DrawCallState drawCallState;
				drawCallState.BindUniformBuffer("BaseVSData", m_BaseVsDataBuffer);
				drawCallState.BindUniformBuffer("Instances", m_InstancesBuffer);

				drawCallState.ViewPort = viewPort->ViewPort;
				drawCallState.BindDepthTarget(depthBuffer, 0, 0);

				drawCallState.ClearColorTarget = false;
				drawCallState.ClearDepthTarget = true;

				UpdateModelState(Pass::Depth, modelContextsOpaque, [](Material* mat, FRasterState& rasterState, FDepthStencilState& depthState, FBlendState& blendState)
				{
					depthState.DepthFunc = EComparisonFunc::Less;
					depthState.DepthWriteMask = EDepthWriteMask::All;
				});

				GEngine->GetRenderDevice()->BindRenderTargets(drawCallState);
				GEngine->GetRenderDevice()->ClearRenderTargets(drawCallState);

				RenderPass(Pass::Depth, drawCallState, camera, modelContextsOpaque);
			}

			//if (!modelContextsOpaque.empty())
			{ // Ambient opaque + transparent
				GPU_DEBUG_SCOPE("AmbientPass");
				CPU_DEBUG_SCOPE("AmbientPass");

				DrawCallState drawState;
				drawState.BindUniformBuffer("BaseVSData", m_BaseVsDataBuffer);
				drawState.BindUniformBuffer("Instances", m_InstancesBuffer);

				{ // Decals
					drawState.BindTexture("g_DecalTexture", GEngine->GetResourceManager()->LoadTexture("Assets/Textures/decals_0003_1k_F6fzPK.png"));
					drawState.BindSampler("g_DecalTexture", Samplers::ClampClampLinearLinear);

					drawState.BindUniformBuffer("GLOB_DecalMatricesVS", m_VSDecalBuffer);
					drawState.BindUniformBuffer("GLOB_DecalMatricesPS", m_PSDecalBuffer);

					Matrix4 scale = glm::translate(Vector3(0.5f)) * glm::scale(Vector3(0.5f / 1.0f));
					uint i = 0;

					GLOB_DecalMatricesVS vsDecalData = {};
					GLOB_DecalMatricesPS psDecalData = {};

					for (DecalComponent* decalComponent : AppContext::GetScene()->GetComponents<DecalComponent>())
					{
						const Transform& decalTransform = decalComponent->GetTransform();
						Matrix4 rawDecalMatrix = decalTransform.GetTransform();
						rawDecalMatrix[3] = Vector4((Vector3)rawDecalMatrix[3] - glm::normalize((Vector3)rawDecalMatrix[2]) * 0.5f, rawDecalMatrix[3].w);
						rawDecalMatrix[0] *= 0.5f;
						rawDecalMatrix[1] *= 0.5f;
						//rawDecalMatrix[2] *= 0.5f;
						vsDecalData.DecalMatrices[i] = glm::inverse(rawDecalMatrix);
						i++;
					}

					vsDecalData.DecalCountVS = i;
					psDecalData.DecalCountPS = i;

					GEngine->GetRenderDevice()->WriteBuffer(m_VSDecalBuffer, &vsDecalData);
					GEngine->GetRenderDevice()->WriteBuffer(m_PSDecalBuffer, &psDecalData);
				}

				for (DirectionalLightComponent* dirLight : scene->GetComponents<DirectionalLightComponent>())
				{
					drawState.Uniforms.SetVec3("LightDir"_HASH, glm::normalize(dirLight->GetForwardVector()));
				}

				drawState.ViewPort = viewPort->ViewPort;
				drawState.BindTarget(0, viewPort->Target);
				drawState.BindDepthTarget(depthBuffer, 0, 0);

				drawState.ClearColor = camera->GetClearColor();
				drawState.ClearColorTarget = true;
				drawState.ClearDepthTarget = false;

				UpdateModelState(Pass::Ambient, modelContextsOpaque, [](Material* mat, FRasterState& rasterState, FDepthStencilState& depthState, FBlendState& blendState)
				{
					depthState.DepthFunc = EComparisonFunc::Equal;
					depthState.DepthWriteMask = EDepthWriteMask::Zero;
					//mat->SetMacro("HAS_DECALS", "1");
				});

				GEngine->GetRenderDevice()->BindRenderTargets(drawState);
				GEngine->GetRenderDevice()->ClearRenderTargets(drawState);

				RenderPass(Pass::Ambient, drawState, camera, modelContextsOpaque);
			}

			if (!skyModelContexts.empty())
			{ // Sky
				GPU_DEBUG_SCOPE("SkyPass");
				CPU_DEBUG_SCOPE("SkyPass");

				DrawCallState drawCallState;
				drawCallState.BindUniformBuffer("BaseVSData", m_BaseVsDataBuffer);
				drawCallState.BindUniformBuffer("Instances", m_InstancesBuffer);

				drawCallState.ViewPort = viewPort->ViewPort;
				drawCallState.BindTarget(0, viewPort->Target);
				drawCallState.BindDepthTarget(depthBuffer, 0, 0);

				UpdateModelState(Pass::Ambient, skyModelContexts, [](Material* mat, FRasterState& rasterState, FDepthStencilState& depthState, FBlendState& blendState)
				{
					depthState.DepthFunc = EComparisonFunc::LessEqual;
					depthState.DepthWriteMask = EDepthWriteMask::Zero;
				});

				GEngine->GetRenderDevice()->BindRenderTargets(drawCallState);

				RenderPass(Pass::Ambient, drawCallState, camera, skyModelContexts, false);
			}

			{ // Particles
				DrawCallState drawCallState;
				drawCallState.Shader = m_ParticleRenderShader;
				drawCallState.BindUniformBuffer("BaseVSData", m_BaseVsDataBuffer);
				drawCallState.ViewPort = viewPort->ViewPort;
				drawCallState.BindTarget(0, viewPort->Target);
				drawCallState.BindDepthTarget(depthBuffer, 0, 0);
				drawCallState.PrimitiveType = EPrimitiveType::PointList;

				drawCallState.ClearColorTarget = false;
				drawCallState.ClearDepthTarget = false;
				drawCallState.DepthStencilState.DepthEnable = true;
				drawCallState.InputLayoutHandle = m_ParticleInputLayout;

				std::vector<DrawArguments> args;
				args.resize(1);
				DrawArguments& arg = args[0];

				DispatchState dispatchState;
				dispatchState.Shader = m_ParticleComputeShader;
				for (ParticleSystemComponent* particleSystemComponent : scene->GetComponents<ParticleSystemComponent>())
				{
					dispatchState.BindSSBOBuffer("Pos", particleSystemComponent->m_GPUPosBuffer);
					dispatchState.Uniforms.SetUInt("ParticleCount"_HASH, particleSystemComponent->m_CurrentParticles);
					GEngine->GetRenderDevice()->Dispatch(dispatchState, (particleSystemComponent->m_CurrentParticles / 128) + 1, 1, 1);

					drawCallState.SetVertexBuffer(0, particleSystemComponent->m_GPUPosBuffer);

					arg.VertexCount = particleSystemComponent->m_CurrentParticles;
					GEngine->GetRenderDevice()->Draw(drawCallState, args);
				}
			}

			if (!translucentModelContexts.empty())
			{ // Translucent
				GPU_DEBUG_SCOPE("TranslucentPass");
				CPU_DEBUG_SCOPE("TranslucentPass");

				DrawCallState drawCallState;
				drawCallState.BindUniformBuffer("BaseVSData", m_BaseVsDataBuffer);
				drawCallState.BindUniformBuffer("Instances", m_InstancesBuffer);

				drawCallState.ViewPort = viewPort->ViewPort;
				drawCallState.BindTarget(0, viewPort->Target);
				drawCallState.BindDepthTarget(depthBuffer, 0, 0);

				UpdateModelState(Pass::Ambient, translucentModelContexts, [](Material* mat, FRasterState& rasterState, FDepthStencilState& depthState, FBlendState& blendState)
				{
					depthState.DepthFunc = EComparisonFunc::Less;
					depthState.DepthWriteMask = EDepthWriteMask::All;
				});

				GEngine->GetRenderDevice()->BindRenderTargets(drawCallState);

				RenderPass(Pass::Ambient, drawCallState, camera, translucentModelContexts, false);
			}

			{ // Debug shapes
				CPU_DEBUG_SCOPE("DebugShapes");
				GPU_DEBUG_SCOPE("Debug Shapes");
				DrawCallState drawState;
				drawState.BindUniformBuffer("BaseVSData", m_BaseVsDataBuffer);

				drawState.ClearDepthTarget = false;
				drawState.ClearColorTarget = false;
				drawState.DepthStencilState.DepthEnable = true;
				drawState.RasterState.CullMode = ECullMode::Back;

				drawState.ViewPort = viewPort->ViewPort;

				drawState.BindTarget(0, viewPort->Target);
				drawState.BindDepthTarget(depthBuffer, 0, 0);

				drawState.DepthStencilState.DepthWriteMask = EDepthWriteMask::Zero;

				GEngine->GetRenderDevice()->BindRenderTargets(drawState);
				// Render debug shapes
				DShapes::Render(drawState);
			}


			if (overlayModelContexts.empty() == false)
			{
				GPU_DEBUG_SCOPE("OverlayPass");
				CPU_DEBUG_SCOPE("OverlayPass");

				DrawCallState drawCallState;
				drawCallState.BindUniformBuffer("BaseVSData", m_BaseVsDataBuffer);
				drawCallState.BindUniformBuffer("Instances", m_InstancesBuffer);

				drawCallState.ViewPort = viewPort->ViewPort;
				drawCallState.BindTarget(0, viewPort->Target);
				drawCallState.BindDepthTarget(depthBuffer, 0, 0);
				drawCallState.ClearColor = camera->GetClearColor();
				drawCallState.ClearColorTarget = false;
				drawCallState.ClearDepthTarget = true;
				drawCallState.DepthStencilState.DepthEnable = true;

				for (DirectionalLightComponent* dirLight : scene->GetComponents<DirectionalLightComponent>())
				{
					drawCallState.Uniforms.SetVec3("LightDir"_HASH, glm::normalize(dirLight->GetForwardVector()));
				}

				GEngine->GetRenderDevice()->BindRenderTargets(drawCallState);

				// TODO: Find out why clearing depth does not work
				//GEngine->GetRenderDevice()->ClearRenderTargets(drawCallState);
				GEngine->GetRenderDevice()->ClearTextureFloat(depthBuffer, 1);

				UpdateModelState(Pass::Ambient, overlayModelContexts, [](Material* mat, FRasterState& rasterState, FDepthStencilState& depthState, FBlendState& blendState)
				{
					depthState.DepthFunc = EComparisonFunc::Less;
					depthState.DepthWriteMask = EDepthWriteMask::All;
				});

				RenderPass(Pass::Ambient, drawCallState, camera, overlayModelContexts, false);
			}

			{
				CPU_DEBUG_SCOPE("Tonemapping");
				GPU_DEBUG_SCOPE("Tonemapping");

				TemporalRenderTarget tempRenderTarget =
					GEngine->GetRenderManager()->CreateTemporalRenderTarget(
						"TempRenderTarget",
						(Vector2i)viewPort->ViewPort,
						viewPort->Target->GetDesc().ImageFormat
					);
				// prepare post-processing
				DrawCallState drawCallState;
				drawCallState.Shader = m_TonemappingShader;

				drawCallState.ViewPort = viewPort->ViewPort;
				drawCallState.BindTarget(0, tempRenderTarget);
				drawCallState.BindTexture("u_Texture", viewPort->Target);
				drawCallState.BindSampler("u_Texture", Samplers::ClampClampNearestNearest);
				drawCallState.PrimitiveType = EPrimitiveType::TriangleStrip;
				drawCallState.RasterState.CullMode = ECullMode::Back;
				drawCallState.DepthStencilState.DepthEnable = false;

				drawCallState.ClearColorTarget = false;
				drawCallState.ClearDepthTarget = false;

				GEngine->GetRenderDevice()->BindRenderTargets(drawCallState);
				GEngine->GetRenderDevice()->Draw(drawCallState, {DrawArguments(4) }, true);

				// draw post-processing to the screen
				DrawCallState screenCallState;
				screenCallState.Shader = m_ScreenTextureShader;

				screenCallState.ViewPort = viewPort->ViewPort;
				screenCallState.BindTarget(0, viewPort->Target);
				screenCallState.BindTexture("u_Texture", tempRenderTarget);
				screenCallState.BindSampler("u_Texture", Samplers::ClampClampNearestNearest);
				screenCallState.PrimitiveType = EPrimitiveType::TriangleStrip;
				screenCallState.RasterState.CullMode = ECullMode::Back;
				screenCallState.DepthStencilState.DepthEnable = false;

				screenCallState.ClearColorTarget = false;
				screenCallState.ClearDepthTarget = false;

				GEngine->GetRenderDevice()->BindRenderTargets(screenCallState);
				GEngine->GetRenderDevice()->Draw(screenCallState, {DrawArguments(4) }, true);

				tempRenderTarget.Free();
			}

			// Reset State
			DrawCallState drawCallState;
			GEngine->GetRenderDevice()->SetDepthStencilState(drawCallState.DepthStencilState);

			depthBuffer.Free();
		}
	}
}