#pragma once

#include <array>
#include "Aurora/Core/Delegate.hpp"
#include "Aurora/Tools/robin_hood.h"
#include "Aurora/Graphics/Material/Material.hpp"
#include "Aurora/Graphics/PassType.hpp"
#include "Aurora/Framework/Mesh/Mesh.hpp"

namespace Aurora
{
	class Scene;
	class FFrustum;

	constexpr uint32_t MaxInstances = 1024;

	struct VisibleEntity
	{
		Aurora::Material* Material;
		Aurora::Mesh* Mesh;
		uint MeshSection;
		LOD Lod;
		Matrix4 Transform;

		bool operator==(const VisibleEntity& other) const
		{
			return Material == other.Material && Mesh == other.Mesh && MeshSection == other.MeshSection;
		}

		bool operator!=(const VisibleEntity& other) const
		{
			return !operator==(other);
		}
	};

	struct ModelContext
	{
		Aurora::Material* Material;
		Aurora::Mesh* Mesh;
		MeshLodResource* LodResource;
		FMeshSection* MeshSection;
		std::vector<Matrix4> Instances;
	};

	using RenderSet = std::vector<ModelContext>;

	typedef EventEmitter<PassType_t, DrawCallState&, const FFrustum&, const glm::mat4&> PassRenderEventEmitter;

	class SceneRenderer
	{
	private:
		Buffer_ptr m_BaseVsDataBuffer;

		std::array<std::vector<VisibleEntity>, SortTypeCount> m_VisibleEntities;

		std::array<PassRenderEventEmitter, Pass::Count> m_InjectedPasses;
	public:
		SceneRenderer();

		void Render(Scene* scene);

		PassRenderEventEmitter& GetPassEmitter(PassType_t passType) { return m_InjectedPasses[passType]; }
	};
}
