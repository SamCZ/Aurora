#include "SceneRendererForward.hpp"

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
#include "Shaders/PostProcess/ub_bloom.h"
#include "Shaders/PostProcess/ub_outline.h"

namespace Aurora
{
	void UpdateModelState(PassType_t pass, const RenderSet& set, void(*callback)(FRasterState& rasterState, FDepthStencilState& depthState, FBlendState& blendState))
	{
		Material* lastMaterial = nullptr;
		for (const ModelContext& mc : set)
		{
			if (mc.Material != lastMaterial)
			{
				lastMaterial = mc.Material;
				callback(mc.Material->RasterState(pass), mc.Material->DepthStencilState(pass), mc.Material->BlendState(pass));
			}
		}
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

				UpdateModelState(Pass::Depth, modelContextsOpaque, [](FRasterState& rasterState, FDepthStencilState& depthState, FBlendState& blendState)
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

				DrawCallState drawCallState;
				drawCallState.BindUniformBuffer("BaseVSData", m_BaseVsDataBuffer);
				drawCallState.BindUniformBuffer("Instances", m_InstancesBuffer);

				drawCallState.ViewPort = viewPort->ViewPort;
				drawCallState.BindTarget(0, viewPort->Target);
				drawCallState.BindDepthTarget(depthBuffer, 0, 0);

				drawCallState.ClearColor = camera->GetClearColor();
				drawCallState.ClearColorTarget = true;
				drawCallState.ClearDepthTarget = false;

				UpdateModelState(Pass::Ambient, modelContextsOpaque, [](FRasterState& rasterState, FDepthStencilState& depthState, FBlendState& blendState)
				{
					depthState.DepthFunc = EComparisonFunc::Equal;
					depthState.DepthWriteMask = EDepthWriteMask::Zero;
				});

				GEngine->GetRenderDevice()->BindRenderTargets(drawCallState);
				GEngine->GetRenderDevice()->ClearRenderTargets(drawCallState);

				RenderPass(Pass::Ambient, drawCallState, camera, modelContextsOpaque);
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

				UpdateModelState(Pass::Ambient, skyModelContexts, [](FRasterState& rasterState, FDepthStencilState& depthState, FBlendState& blendState)
				{
					depthState.DepthFunc = EComparisonFunc::LessEqual;
					depthState.DepthWriteMask = EDepthWriteMask::Zero;
				});

				GEngine->GetRenderDevice()->BindRenderTargets(drawCallState);

				RenderPass(Pass::Ambient, drawCallState, camera, skyModelContexts, false);
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

				UpdateModelState(Pass::Ambient, translucentModelContexts, [](FRasterState& rasterState, FDepthStencilState& depthState, FBlendState& blendState)
				{
					depthState.DepthFunc = EComparisonFunc::Less;
					depthState.DepthWriteMask = EDepthWriteMask::All;
				});

				GEngine->GetRenderDevice()->BindRenderTargets(drawCallState);

				RenderPass(Pass::Ambient, drawCallState, camera, translucentModelContexts, false);
			}

			// Reset State
			DrawCallState drawCallState;
			GEngine->GetRenderDevice()->SetDepthStencilState(drawCallState.DepthStencilState);
			// TODO

			depthBuffer.Free();
		}
	}
}