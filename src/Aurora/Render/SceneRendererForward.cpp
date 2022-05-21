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
			RenderSet modelContexts;
			FillRenderSet(modelContexts);

			// Setup base vs data
			BaseVSData baseVsData;
			baseVsData.ProjectionMatrix = camera->GetProjectionMatrix();
			baseVsData.ProjectionViewMatrix = camera->GetProjectionViewMatrix();
			baseVsData.ViewMatrix = viewMatrix;
			GEngine->GetRenderDevice()->WriteBuffer(m_BaseVsDataBuffer, &baseVsData);

			GEngine->GetRenderDevice()->InvalidateState();

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

				drawCallState.DepthStencilState.DepthFunc = EComparisonFunc::Less;
				drawCallState.DepthStencilState.DepthWriteMask = EDepthWriteMask::All;

				GEngine->GetRenderDevice()->BindRenderTargets(drawCallState);
				GEngine->GetRenderDevice()->ClearRenderTargets(drawCallState);

				GEngine->GetRenderDevice()->SetDepthStencilState(drawCallState.DepthStencilState);

				RenderPass(Pass::Depth, drawCallState, camera, modelContexts);
			}

			{ // Ambient
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
				drawCallState.DepthStencilState.DepthFunc = EComparisonFunc::LessEqual;
				drawCallState.DepthStencilState.DepthWriteMask = EDepthWriteMask::Zero;

				GEngine->GetRenderDevice()->BindRenderTargets(drawCallState);
				GEngine->GetRenderDevice()->ClearRenderTargets(drawCallState);

				GEngine->GetRenderDevice()->SetDepthStencilState(drawCallState.DepthStencilState);

				RenderPass(Pass::Ambient, drawCallState, camera, modelContexts);
			}

			// Reset State
			DrawCallState drawCallState;
			GEngine->GetRenderDevice()->SetDepthStencilState(drawCallState.DepthStencilState);
			// TODO

			depthBuffer.Free();
		}
	}
}