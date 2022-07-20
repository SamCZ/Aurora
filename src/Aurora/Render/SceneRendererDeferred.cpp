#include "SceneRendererDeferred.hpp"

#include "Aurora/Engine.hpp"

#include "Aurora/Core/Profiler.hpp"

#include "Aurora/Framework/Scene.hpp"
#include "Aurora/Framework/CameraComponent.hpp"
#include "Aurora/Framework/MeshComponent.hpp"
#include "Aurora/Framework/Lights.hpp"
#include "Aurora/Framework/SkyLight.hpp"

#include "Aurora/Graphics/Base/IRenderDevice.hpp"
#include "Aurora/Graphics/ViewPortManager.hpp"
#include "Aurora/Graphics/OpenGL/GLBufferLock.hpp"
#include "Aurora/Graphics/DShape.hpp"

#include "Aurora/Resource/ResourceManager.hpp"

#include "Shaders/vs_common.h"
#include "Shaders/PBR/Composite.h"
#include "Shaders/PostProcess/ub_outline.h"

namespace Aurora
{
	SceneRendererDeferred::SceneRendererDeferred() : SceneRenderer()
	{
		m_CompositeDefaultsBuffer = GEngine->GetRenderDevice()->CreateBuffer(BufferDesc("CompositeDefaults", sizeof(CompositeDefaults), EBufferType::UniformBuffer));
		m_SkyLightBuffer = GEngine->GetRenderDevice()->CreateBuffer(BufferDesc("SkyLight", sizeof(SkyLightStorage), EBufferType::UniformBuffer));
		m_DirLightsBuffer = GEngine->GetRenderDevice()->CreateBuffer(BufferDesc("DirLights", sizeof(DirectionalLightStorage), EBufferType::UniformBuffer));
		m_PointLightsBuffer = GEngine->GetRenderDevice()->CreateBuffer(BufferDesc("PointLights", sizeof(PointLightStorage), EBufferType::UniformBuffer, EBufferUsage::DynamicDraw, true));

		m_OutlineDescBuffer = GEngine->GetRenderDevice()->CreateBuffer(BufferDesc("OutlineDesc", sizeof(OutlineGPUDesc), EBufferType::UniformBuffer, EBufferUsage::DynamicDraw));
		m_OutlineStripeTexture = GEngine->GetResourceManager()->LoadTexture("Assets/Textures/stripe.png");

		LoadShaders();
	}

	void SceneRendererDeferred::LoadShaders()
	{
		m_CompositeShader = GEngine->GetResourceManager()->LoadShader("Composite", {
			{EShaderType::Vertex, "Assets/Shaders/FSQuad.vert"},
			{EShaderType::Pixel, "Assets/Shaders/PBR/Composite.frag"}
		});

		m_HDRCompositeShader = GEngine->GetResourceManager()->LoadShader("HDRComposite", {
			{EShaderType::Vertex, "Assets/Shaders/FSQuad.vert"},
			{EShaderType::Pixel, "Assets/Shaders/PostProcess/HDR.frag"}
		}, ShaderMacros{
			{"USE_OUTLINE", "1"}
		});

		m_HDRCompositeShaderNoOutline = GEngine->GetResourceManager()->LoadShader("HDRComposite", {
			{EShaderType::Vertex, "Assets/Shaders/FSQuad.vert"},
			{EShaderType::Pixel, "Assets/Shaders/PostProcess/HDR.frag"}
		});

		m_OutlineShader = GEngine->GetResourceManager()->LoadShader("HDRComposite", {
			{EShaderType::Vertex, "Assets/Shaders/FSQuad.vert"},
			{EShaderType::Pixel, "Assets/Shaders/PostProcess/Outline.frag"}
		});
	}

	void SceneRendererDeferred::Render(Scene* scene)
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

			auto albedoBuffer = GEngine->GetRenderManager()->CreateTemporalRenderTarget("Albedo", (Vector2i)viewPort->ViewPort, GraphicsFormat::RGBA16_FLOAT);
			auto normalsBuffer = GEngine->GetRenderManager()->CreateTemporalRenderTarget("Normals", (Vector2i)viewPort->ViewPort, GraphicsFormat::RGBA8_UNORM);
			auto depthBuffer = GEngine->GetRenderManager()->CreateTemporalRenderTarget("Depth", (Vector2i)viewPort->ViewPort, GraphicsFormat::D32);

			const FFrustum& frustum = camera->GetFrustum();
			Matrix4 viewMatrix = camera->GetViewMatrix();

			BaseVSData baseVsData;
			baseVsData.ProjectionMatrix = camera->GetProjectionMatrix();
			baseVsData.ProjectionViewMatrix = camera->GetProjectionViewMatrix();
			baseVsData.ViewMatrix = viewMatrix;
			GEngine->GetRenderDevice()->WriteBuffer(m_BaseVsDataBuffer, &baseVsData);

			{ // Base Ambient pass
				GPU_DEBUG_SCOPE("AmbientPass");
				CPU_DEBUG_SCOPE("AmbientPass");
				DrawCallState drawCallState;
				drawCallState.BindUniformBuffer("BaseVSData", m_BaseVsDataBuffer);
				drawCallState.BindUniformBuffer("Instances", m_InstancesBuffer);

				drawCallState.ViewPort = viewPort->ViewPort;
				drawCallState.BindTarget(0, albedoBuffer);
				drawCallState.BindTarget(1, normalsBuffer);
				drawCallState.BindDepthTarget(depthBuffer, 0, 0);

				drawCallState.ClearColor = camera->GetClearColor();
				drawCallState.ClearColorTarget = true;
				drawCallState.ClearDepthTarget = true;

				GEngine->GetRenderDevice()->BindRenderTargets(drawCallState);
				GEngine->GetRenderDevice()->ClearRenderTargets(drawCallState);

				PrepareVisibleEntities(scene, camera);

				RenderSet modelContexts;
				FillRenderSet(modelContexts, 4, RenderSortType::Opaque, RenderSortType::Transparent, RenderSortType::Sky, RenderSortType::Translucent);

				RenderPass(Pass::Ambient, drawCallState, camera, modelContexts);
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////



			/////////////////////////////////////////////////////////////////////////////////////////////////

			{
				CPU_DEBUG_SCOPE("LightBufferUpdate");
				// Sky light update
				SkyLightStorage skyLightStorage = {};

				if (SkyLightComponent* skylightComponent = scene->FindFirstComponent<SkyLightComponent>())
				{
					skyLightStorage.AmbientColorAndIntensity = Vector4(skylightComponent->GetAmbientColor(), skylightComponent->GetAmbientIntensity());
				}

				GEngine->GetRenderDevice()->WriteBuffer(m_SkyLightBuffer, &skyLightStorage);

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
			}

			// Write defaults
			CompositeDefaults defaults = {};
			defaults.InvProjectionView = glm::inverse(camera->GetProjectionViewMatrix());
			defaults.ViewMatrix = camera->GetViewMatrix();
			defaults.CameraPos = Vector4(camera->GetWorldPosition(), 0);

			GEngine->GetRenderDevice()->WriteBuffer(m_CompositeDefaultsBuffer, &defaults);

			auto compositeRT = GEngine->GetRenderManager()->CreateTemporalRenderTarget("CompositeRT", (Vector2i)viewPort->ViewPort, GraphicsFormat::RGBA16_FLOAT);

			{
				CPU_DEBUG_SCOPE("CompositeLighting");
				GPU_DEBUG_SCOPE("CompositeLighting");
				DrawCallState state;
				state.Shader = m_CompositeShader;
				state.ViewPort = viewPort->ViewPort;
				state.BindTarget(0, compositeRT);
				state.BindTexture("AlbedoRT", albedoBuffer);
				state.BindTexture("NormalsRT", normalsBuffer);
				state.BindTexture("DepthRT", depthBuffer);

				state.BindUniformBuffer("SkyLightStorage", m_SkyLightBuffer);
				state.BindUniformBuffer("DirectionalLightStorage", m_DirLightsBuffer);
				state.BindUniformBuffer("PointLightStorage", m_PointLightsBuffer);
				state.BindUniformBuffer("CompositeDefaults", m_CompositeDefaultsBuffer);

				state.PrimitiveType = EPrimitiveType::TriangleStrip;
				state.RasterState.CullMode = ECullMode::Back;
				state.DepthStencilState.DepthEnable = false;

				state.ClearColorTarget = false;
				state.ClearDepthTarget = false;
				GEngine->GetRenderDevice()->Draw(state, {DrawArguments(4)}, true);
			}


			/////////////////////////////////////////////////////////////////////////////////////////////////

			TemporalRenderTarget bloomRT = RenderBloom(viewPort->ViewPort, compositeRT);

			/////////////////////////////////////////////////////////////////////////////////////////////////

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

				drawState.BindTarget(0, compositeRT);
				drawState.BindDepthTarget(depthBuffer, 0, 0);

				GEngine->GetRenderDevice()->BindRenderTargets(drawState);
				// Render debug shapes
				DShapes::Render(drawState);
			}

			GEngine->GetRenderDevice()->InvalidateState();

			TemporalRenderTarget outlineRT;
			TemporalRenderTarget outlineDepthRT;
			{ // Outline
				CPU_DEBUG_SCOPE("Outline");
				GPU_DEBUG_SCOPE("Outline");

				bool firstOutlineIteration = true;

				for (const OutlineActorSet& outlineSet: m_OutlineContext.Sets)
				{
					ClearVisibleEntities();
					for (Actor* actor : outlineSet.Actors)
					{
						PrepareVisibleEntities(actor, camera);
					}

					for (SceneComponent* sceneComponent : outlineSet.Components)
					{
						std::vector<MeshComponent*> meshComponents;
						sceneComponent->GetComponentsOfType<MeshComponent>(meshComponents);
						for (MeshComponent* meshComponent : meshComponents)
						{
							PrepareMeshComponent(meshComponent, camera);
						}
					}

					RenderSet outlineModelContexts;
					FillRenderSet(outlineModelContexts, 4, RenderSortType::Opaque, RenderSortType::Transparent, RenderSortType::Sky, RenderSortType::Translucent);

					if (outlineModelContexts.empty())
						continue;

					if (outlineRT.Empty())
					{
						outlineRT = GEngine->GetRenderManager()->CreateTemporalRenderTarget("OutlineRT", (Vector2i)viewPort->ViewPort, GraphicsFormat::RGBA8_UNORM);
						outlineDepthRT = GEngine->GetRenderManager()->CreateTemporalRenderTarget("OutlineDepthRT", (Vector2i)viewPort->ViewPort, GraphicsFormat::D32);
					}

					// Render outline set actors to depth target
					{
						GPU_DEBUG_SCOPE("DepthPass");
						DrawCallState drawCallState;
						drawCallState.BindUniformBuffer("BaseVSData", m_BaseVsDataBuffer);
						drawCallState.BindUniformBuffer("Instances", m_InstancesBuffer);

						drawCallState.ViewPort = viewPort->ViewPort;
						drawCallState.ClearColorTarget = false;
						drawCallState.ClearDepthTarget = true;
						drawCallState.BindDepthTarget(outlineDepthRT, 0, 0);

						GEngine->GetRenderDevice()->BindRenderTargets(drawCallState);
						GEngine->GetRenderDevice()->ClearRenderTargets(drawCallState);

						RenderPass(Pass::Depth, drawCallState, camera, outlineModelContexts, false);
					}

					// Render outline post process
					{
						GPU_DEBUG_SCOPE("PostPass");
						CPU_DEBUG_SCOPE("Outline");

						glEnable(GL_BLEND);
						glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

						OutlineGPUDesc outlineGpuDesc = {};
						outlineGpuDesc.MainRTSize = (Vector2)viewPort->ViewPort;
						outlineGpuDesc.InvMainRTSize = 1.0f / (Vector2)viewPort->ViewPort;
						outlineGpuDesc.CrossTextureTexelSize = Vector2((float)viewPort->ViewPort.Width / (float)m_OutlineStripeTexture->GetDesc().Width, (float)viewPort->ViewPort.Height / (float)m_OutlineStripeTexture->GetDesc().Height);
						outlineGpuDesc.CrossTextureMaskOpacityAndOutlineThickness = Vector2(outlineSet.CrossMaskOpacity * (outlineSet.CrossMaskEnabled ? 1.0f : 0.0f), outlineSet.Thickness + 1.0f);
						outlineGpuDesc.OutlineColorAndCrossEnabled = Vector4(outlineSet.BaseColor, outlineSet.CrossEnabled);
						outlineGpuDesc.CrossColorAndAlpha = Vector4(outlineSet.CrossColor, outlineSet.IntersectionMaskOpacity);
						GEngine->GetRenderDevice()->WriteBuffer(m_OutlineDescBuffer, &outlineGpuDesc);

						DrawCallState state;
						state.Shader = m_OutlineShader;
						state.ViewPort = viewPort->ViewPort;
						state.BindTarget(0, outlineRT);

						state.BindTexture("SceneDepthRT", depthBuffer);
						state.BindTexture("OutlineMaskDepthRT", outlineDepthRT);
						state.BindTexture("StripeTexture", m_OutlineStripeTexture);

						state.BindSampler("SceneDepthRT", Samplers::ClampClampNearestNearest);
						state.BindSampler("OutlineMaskDepthRT", Samplers::ClampClampNearestNearest);
						state.BindSampler("StripeTexture", Samplers::WrapWrapNearestNearest);

						state.BindUniformBuffer("OutlineGPUDesc", m_OutlineDescBuffer);

						state.PrimitiveType = EPrimitiveType::TriangleStrip;
						state.RasterState.CullMode = ECullMode::Back;
						state.DepthStencilState.DepthEnable = false;

						state.ClearColorTarget = firstOutlineIteration;
						state.ClearDepthTarget = false;

						GEngine->GetRenderDevice()->Draw(state, {DrawArguments(4)}, true);

						glDisable(GL_BLEND);

						firstOutlineIteration = false;
					}
				}

				if (m_OutlineContext.ClearAfterFrame)
					m_OutlineContext.Clear();
			}

			{
				CPU_DEBUG_SCOPE("HDR");
				GPU_DEBUG_SCOPE("HDR");

				DrawCallState state;
				state.Shader = outlineRT.Empty() ? m_HDRCompositeShaderNoOutline : m_HDRCompositeShader;
				state.ViewPort = viewPort->ViewPort;
				state.BindTarget(0, viewPort->Target);
				state.BindTexture("SceneHRDTexture", compositeRT);
				state.BindTexture("BloomTexture", bloomRT);
				state.Uniforms.SetFloat("BloomIntensity"_HASH, m_BloomSettings.Intensity);

				if (!outlineRT.Empty())
				{
					state.BindTexture("OutlineTexture", outlineRT);
				}

				state.BindSampler("SceneHRDTexture", Samplers::ClampClampNearestNearest);
				state.BindSampler("BloomTexture", Samplers::ClampClampLinearLinear);

				state.PrimitiveType = EPrimitiveType::TriangleStrip;
				state.RasterState.CullMode = ECullMode::Back;
				state.DepthStencilState.DepthEnable = false;

				state.ClearColorTarget = false;
				state.ClearDepthTarget = false;

				GEngine->GetRenderDevice()->Draw(state, {DrawArguments(4)}, true);
			}

			outlineRT.Free();
			outlineDepthRT.Free();

			bloomRT.Free();

			compositeRT.Free();
			depthBuffer.Free();
			normalsBuffer.Free();
			albedoBuffer.Free();
		}
	}
}
