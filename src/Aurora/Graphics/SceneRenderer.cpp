#include "SceneRenderer.hpp"
#include "Aurora/AuroraEngine.hpp"

namespace Aurora
{

	SceneRenderer::SceneRenderer(Scene_ptr scene) : m_Scene(std::move(scene)), m_SortedRenderer()
	{

	}

	void SceneRenderer::Update(double delta, Frustum* frustum)
	{
		(void) delta;
		m_SortedRenderer.clear();

        Profiler::Begin("Prepare renderers");
		for(auto* cameraComponent : m_Scene->GetCameraComponents()) {
			for(auto* meshComponent : m_Scene->GetMeshComponents())
			{
				if(!meshComponent->GetOwner()->IsActive() || !meshComponent->IsActive()) continue;

				// Perform frustum culling
				//Profiler::Begin("Frustum culling");
				if(meshComponent->GetBody().HasCollider() && frustum != nullptr) {
					if(!frustum->IsBoxVisible(meshComponent->GetBody().GetTransformedBounds())) {
						//Profiler::End("Frustum culling");
						continue;
					}
				}
				//Profiler::End("Frustum culling");

				auto& mesh = meshComponent->GetMesh();

				if(mesh == nullptr) {
					continue;
				}

				auto& sections = mesh->LODResources[0].Sections;

				for (size_t i = 0; i < sections.size(); ++i) {
					auto& section = sections[i];
					int materialIndex = section.MaterialIndex;

					if(!mesh->MaterialSlots.contains(materialIndex)) {
						continue;
					}

					auto& materialSlot = mesh->MaterialSlots[materialIndex];

					if(materialSlot.Material == nullptr) {
						continue;
					}

					m_SortedRenderer[cameraComponent][materialSlot.Material.get()].push_back(std::tuple<Mesh*, uint32_t, Matrix4, Actor*>(meshComponent->GetMesh().get(), i, meshComponent->GetTransformMatrix(), meshComponent->GetOwner()));
				}
			}
		}
        Profiler::End("Prepare renderers");
	}

	void SceneRenderer::Render(RenderTargetPack* renderTargetPack, bool apply, bool clear)
	{
		DrawCallState drawCallState;

		if(apply) {
			renderTargetPack->Apply(drawCallState);
		}

		for(auto& it2 : m_SortedRenderer) {
			CameraComponent *cameraComponent = it2.first;
			for (const auto &it : it2.second) {
				Material *material = it.first;

				material->SetVariable("ProjectionViewMatrix", cameraComponent->GetProjectionViewMatrix());
				material->SetVariable("ViewMatrix", cameraComponent->GetViewMatrix());
				material->SetVariable<float>("g_Time", static_cast<float>(glfwGetTime()));

				for (const auto& renderData : it.second) {
					Mesh *mesh = std::get<0>(renderData);
					uint32_t sectionIndex = std::get<1>(renderData);
					const Matrix4 &modelMatrix = std::get<2>(renderData);

					drawCallState.InputLayoutHandle = mesh->GetInputLayout();

					drawCallState.SetVertexBuffer(0, mesh->LODResources[0].VertexBuffer);
					drawCallState.SetIndexBuffer(mesh->LODResources[0].IndexBuffer);

					material->SetVariable("ModelMatrix", modelMatrix);

					std::get<3>(renderData)->OnPreRender(material);

					material->Apply(drawCallState);

					auto &section = mesh->LODResources[0].Sections[sectionIndex];

					DrawArguments drawArguments;
					drawArguments.VertexCount = section.NumTriangles;
					drawArguments.StartIndexLocation = section.FirstIndex;
					drawArguments.InstanceCount = 1;
					RD->DrawIndexed(drawCallState, {drawArguments});

					drawCallState.ClearColorTarget = false;
					drawCallState.ClearDepthTarget = false;
				}
			}
		}

		RD->InvalidateState();
	}
}