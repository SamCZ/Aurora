#pragma once

#include <map>
#include <array>
#include "Aurora/Core/Vector.hpp"
#include "Aurora/Graphics/Material/Material.hpp"
#include "Aurora/Graphics/Base/IRenderDevice.hpp"
#include "Aurora/Graphics/Mesh.hpp"
#include "Aurora/Graphics/RenderManager.hpp"
#include <entt/entt.hpp>

#include "World/instancing.h"

namespace Aurora
{
	class Scene;
	class Material;
	class XMesh;

	struct VisibleEntity
	{
		Aurora::Material* Material;
		XMesh* Mesh;
		uint MeshSection;
		Matrix4 Transform;
	};

	struct ModelContext
	{
		Aurora::Material* Material;
		XMesh* Mesh;
		XMesh::PrimitiveSection* MeshSection;
		std::vector<ObjectInstanceData> Instances;
	};

	using RenderSet = std::vector<ModelContext>;

	class SceneRenderer
	{
	private:
		Scene* m_Scene;
		RenderManager* m_RenderManager;
		IRenderDevice* m_RenderDevice;

		std::vector<entt::entity> m_VisibleEntities;
		std::map<TTypeID, uint> m_VisibleTypeCounters;
		entt::registry m_VisibleEntitiesRegistry;

		std::array<std::vector<entt::entity>, SortTypeCount> m_FinalSortedEntities;

		Buffer_ptr m_InstancingBuffer;
		Buffer_ptr m_BaseVSDataBuffer;
	public:
		SceneRenderer(Scene* scene, RenderManager* renderManager, IRenderDevice* renderDevice);
		~SceneRenderer();

		void AddVisibleEntity(Material* material, XMesh* mesh, uint meshSection, const Matrix4& transform);

		void PrepareRender();
		void SortVisibleEntities();

		RenderSet BuildRenderSet();

		void Render(entt::entity cameraEntityID);
		void RenderPass(DrawCallState& drawCallState, const std::vector<ModelContext>& modelContexts, EPassType passType);
	};
}
