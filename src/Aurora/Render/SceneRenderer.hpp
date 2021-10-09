#pragma once

#include <map>
#include <array>
#include "Aurora/Core/Vector.hpp"
#include "Aurora/Graphics/Material/Material.hpp"
#include "Aurora/Graphics/Base/IRenderDevice.hpp"
#include "Aurora/Graphics/Mesh.hpp"
#include "Aurora/Graphics/RenderManager.hpp"
#include "Aurora/Framework/Entity.hpp"

#include "Shaders/World/instancing.h"

namespace Aurora
{
	class Scene;
	class Material;
	class XMesh;
	class Frustum;

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
		std::vector<Matrix4> Instances;
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

		Shader_ptr m_PBRCompositeShader;
		Shader_ptr m_SkyShader;
		Shader_ptr m_SSAOShader;
		Shader_ptr m_RenderSkyCubeShader;

		Entity m_CurrentCameraEntity;

		Texture_ptr m_SkyCubeMap;
	public:
		SceneRenderer(Scene* scene, RenderManager* renderManager, IRenderDevice* renderDevice);
		~SceneRenderer();

		Texture_ptr RenderPreethamSky(const Vector2ui& resolution, float turbidity, float azimuth, float inclination);

		void AddVisibleEntity(Material* material, XMesh* mesh, uint meshSection, const Matrix4& transform);

		void PrepareRender(Frustum* frustum);
		void SortVisibleEntities();

		RenderSet BuildRenderSet();

		void Render(entt::entity cameraEntityID);
		void RenderPass(DrawCallState& drawCallState, const std::vector<ModelContext>& modelContexts, EPassType passType);
	};
}
