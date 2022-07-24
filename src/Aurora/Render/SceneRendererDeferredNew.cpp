#include "SceneRendererDeferredNew.hpp"

#include "Aurora/Engine.hpp"
#include "Aurora/App/AppContext.hpp"

#include "Aurora/Core/Profiler.hpp"

#include "Aurora/Framework/Scene.hpp"
#include "Aurora/Framework/CameraComponent.hpp"
#include "Aurora/Framework/MeshComponent.hpp"
#include "Aurora/Framework/Lights.hpp"
#include "Aurora/Framework/SkyLight.hpp"
#include "Aurora/Framework/Decal.hpp"

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
	SceneRendererDeferredNew::SceneRendererDeferredNew()
	{

	}

	void SceneRendererDeferredNew::LoadShaders()
	{

	}

	void SceneRendererDeferredNew::Render(Scene* scene, CameraComponent* debugCamera)
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
			auto colorBuffer = GEngine->GetRenderManager()->CreateTemporalRenderTarget("Color", (Vector2i)viewPort->ViewPort, GraphicsFormat::RGB16_FLOAT);
			auto normalBuffer = GEngine->GetRenderManager()->CreateTemporalRenderTarget("Normal", (Vector2i)viewPort->ViewPort, GraphicsFormat::RGB16_FLOAT);
			auto rmaBuffer = GEngine->GetRenderManager()->CreateTemporalRenderTarget("RMA", (Vector2i)viewPort->ViewPort, GraphicsFormat::RGB8_UNORM);

			const FFrustum& frustum = camera->GetFrustum();
			Matrix4 viewMatrix = camera->GetViewMatrix();

			// Prepate sets
			PrepareVisibleEntities(scene, camera, frustum);

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

			{ // Ambient opaque + transparent
				GPU_DEBUG_SCOPE("AmbientPass");
				CPU_DEBUG_SCOPE("AmbientPass");

				DrawCallState drawState;
				drawState.BindUniformBuffer("BaseVSData", m_BaseVsDataBuffer);
				drawState.BindUniformBuffer("Instances", m_InstancesBuffer);

				drawState.ViewPort = viewPort->ViewPort;
				drawState.BindTarget(0, colorBuffer);
				drawState.BindTarget(1, normalBuffer);
				drawState.BindTarget(2, rmaBuffer);
				drawState.BindDepthTarget(depthBuffer, 0, 0);

				drawState.ClearColor = camera->GetClearColor();
				drawState.ClearColorTarget = true;
				drawState.ClearDepthTarget = true;

				GEngine->GetRenderDevice()->BindRenderTargets(drawState);
				GEngine->GetRenderDevice()->ClearRenderTargets(drawState);

				RenderPass(Pass::Ambient, drawState, camera, modelContextsOpaque);
			}

			GEngine->GetRenderDevice()->Blit(colorBuffer, viewPort->Target);

			rmaBuffer.Free();
			normalBuffer.Free();
			colorBuffer.Free();
			depthBuffer.Free();
		}
	}
}