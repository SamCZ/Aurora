#include "SceneRenderer.hpp"
#include "Aurora/AuroraEngine.hpp"

namespace Aurora
{
	struct alignas(16) CameraConstants
	{
		Matrix4 ProjectionViewMatrix;
		Matrix4 ModelMatrix;
	};

	SceneRenderer::SceneRenderer(Scene_ptr scene) : m_Scene(std::move(scene)), m_NumDrawCalls(0), m_NumTriangles(0)
	{
		m_CameraConstantsUniformBuffer = RD->CreateBuffer(BufferDesc("CameraConstants", sizeof(CameraConstants), 0, EBufferType::UniformBuffer));
	}

	SceneRenderer::~SceneRenderer()
	{
		for (ModelContext* mc : m_ModelContextCache)
		{
			delete mc;
		}
	}

	void SceneRenderer::Update(double delta)
	{
		ZoneNamedN(sceneRendererZone, "SceneRendererUpdate", true);

		m_iCurrentModelContextIndex = 0;

		m_CameraQueueList.clear();

		Mesh* lastMesh = nullptr;
		Material* lastMaterial = nullptr;

		for (MeshComponent *meshComponent : m_Scene->GetMeshComponents())
		{
			if(!meshComponent->GetOwner()->IsActive() || !meshComponent->IsActive()) continue;

			auto &mesh = meshComponent->GetMesh();

			if (mesh == nullptr)
			{
				continue;
			}

			const Matrix4& transform = meshComponent->GetTransformMatrix();

			// TODO: Complete lods
			LOD lod = 0;

			auto &sections = mesh->LODResources[lod].Sections;

			for (uint32_t i = 0; i < sections.size(); ++i)
			{
				auto &section = sections[i];
				int materialIndex = section.MaterialIndex;

				auto &materialSlot = mesh->MaterialSlots[materialIndex];

				if (materialSlot.Material == nullptr)
				{
					continue;
				}

				auto &material = materialSlot.Material;

				if(!material->IsEnabled()) continue;

				QueueBucket bucket = material->GetQueueBucket();

				ModelContext *mc = GetModelContext();
				mc->m_Material = material.get();
				mc->m_Mesh = mesh.get();
				mc->m_iSectionIndex = i;
				mc->m_Transform = transform;
				mc->m_Lod = lod;
				mc->m_bMeshObjectChanged = lastMesh != mesh.get() || lastMaterial != material.get();

				for (CameraComponent *cameraComponent : m_Scene->GetCameraComponents())
				{
					if(!meshComponent->GetBody().HasCollider() || cameraComponent->GetFrustum().IsBoxVisible(meshComponent->GetBody().GetTransformedBounds())) {
						CameraQueueList& queueList = m_CameraQueueList[cameraComponent->ID()];

						switch (bucket)
						{
							case QueueBucket::Opaque:
								queueList.OpaqueQueue.Add(mc);
								break;
							case QueueBucket::Transparent:
								queueList.TransparentQueue.Add(mc);
								break;
							case QueueBucket::Translucent:
								queueList.TranslucentQueue.Add(mc);
								break;
							case QueueBucket::Sky:
								queueList.SkyQueue.Add(mc);
								break;
						}
					}
				}

				lastMesh = mesh.get();
				lastMaterial = material.get();
			}
		}
	}

	void SceneRenderer::RenderQueue(DrawCallState& drawCallState, CameraComponent *camera, MaterialRenderList &renderQueue)
	{
		drawCallState.ViewPort = camera->GetSize();

		CameraConstants cameraData = {};
		cameraData.ProjectionViewMatrix = camera->GetProjectionViewMatrix();

		for (const auto &item : renderQueue.Map())
		{
			const MaterialRenderList::ModelContextList &mcList = item.second;

			if (mcList.empty()) continue;

			Material *material = mcList[0]->m_Material;

			//renderDevice->SetShader(material->GetShader());
			material->Apply(drawCallState);

			for (ModelContext* mc : mcList)
			{
				Mesh* mesh = mc->m_Mesh;
				drawCallState.InputLayoutHandle = mesh->GetInputLayout();

				drawCallState.SetVertexBuffer(0, mesh->LODResources[mc->m_Lod].VertexBuffer);
				drawCallState.SetIndexBuffer(mesh->LODResources[mc->m_Lod].IndexBuffer);

				if(mc->m_bMeshObjectChanged)
				{
					cameraData.ModelMatrix = mc->m_Transform;

					RD->WriteBuffer(m_CameraConstantsUniformBuffer, &cameraData, sizeof(cameraData));
					drawCallState.BindUniformBuffer("CameraConstants", m_CameraConstantsUniformBuffer);
				}

				auto &section = mesh->LODResources[mc->m_Lod].Sections[mc->m_iSectionIndex];

				DrawArguments drawArguments;
				drawArguments.VertexCount = section.NumTriangles;
				drawArguments.StartIndexLocation = section.FirstIndex;
				drawArguments.InstanceCount = 1;
				RD->DrawIndexed(drawCallState, {drawArguments});
				m_NumDrawCalls++;
				m_NumTriangles += section.NumTriangles / 3;

				drawCallState.ClearColorTarget = false;
				drawCallState.ClearDepthTarget = false;
			}
		}
	}

	void SceneRenderer::Render(RenderTargetPack* renderTargetPack, bool apply, bool clear)
	{
		m_NumDrawCalls = 0;
		m_NumTriangles = 0;
		ZoneScopedN("SceneRender");
		GPU_DEBUG_SCOPE("Scene Render")
		DrawCallState drawCallState;

		if(apply) {
			renderTargetPack->Apply(drawCallState);
		}

		for (CameraComponent *cameraComponent : m_Scene->GetCameraComponents())
		{
			CameraQueueList& queueList = m_CameraQueueList[cameraComponent->ID()];

			RenderQueue(drawCallState, cameraComponent, queueList.SkyQueue);
			RenderQueue(drawCallState, cameraComponent, queueList.OpaqueQueue);
			RenderQueue(drawCallState, cameraComponent, queueList.TransparentQueue);
			RenderQueue(drawCallState, cameraComponent, queueList.TranslucentQueue);
		}
	}

	ModelContext *SceneRenderer::GetModelContext()
	{
		if (m_iCurrentModelContextIndex < m_ModelContextCache.size())
		{
			return m_ModelContextCache[m_iCurrentModelContextIndex++];
		}
		else
		{
			m_ModelContextCache.push_back(new ModelContext);
			return m_ModelContextCache[m_iCurrentModelContextIndex++];
		}
	}

	void SceneRenderer::CaptureDepthToCubeMap(const Texture_ptr& cubeMap, const Vector3D& pos)
	{
		ZoneScopedN("CaptureDepthToCubeMap");
		GPU_DEBUG_SCOPE("CaptureDepthToCubeMap")

		const TextureDesc& desc = cubeMap->GetDesc();

		CameraComponent camera = *m_Scene->GetCameraComponents()[0];
		camera.SetFov(90);
		camera.SetZNear(0.5f);
		camera.SetZFar(250.0f);
		camera.Resize(desc.GetSize());
		camera.SetRotation(0, 0, 0);

		CameraQueueList& queueList = m_CameraQueueList[0];
		DrawCallState drawCallState;
		drawCallState.ResetTargets();

		drawCallState.BindDepthTarget(cubeMap, 0, 0);
		drawCallState.ViewPort = desc.GetSize();

		Vector3 captureViews[] = {
				Vector3(0, -90, 0), // X+
				Vector3(0, 90, 0), // X-
				Vector3(90, 0, 0), // Y+
				Vector3(-90, 0, 0), // Y-
				Vector3(0, 0, 0), // Z+
				Vector3(0, -180, 0), // Z-
		};

		for (int i = 0; i < 6; ++i)
		{
			camera.SetLocation(pos);
			camera.SetRotation(captureViews[i]);
			camera.Tick(0);

			GPU_DEBUG_SCOPE("Face " + std::to_string(i))
			drawCallState.DepthIndex = i;
			drawCallState.ClearDepthTarget = true;

			//RenderQueue(drawCallState, &camera, queueList.SkyQueue);
			RenderQueue(drawCallState, &camera, queueList.OpaqueQueue);
			RenderQueue(drawCallState, &camera, queueList.TransparentQueue);
			RenderQueue(drawCallState, &camera, queueList.TranslucentQueue);
		}
	}
}