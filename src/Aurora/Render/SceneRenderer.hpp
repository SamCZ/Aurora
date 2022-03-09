#pragma once

#include <array>
#include "Aurora/Tools/robin_hood.h"
#include "Aurora/Graphics/Material/Material.hpp"
#include "Aurora/Framework/Mesh/Mesh.hpp"

namespace Aurora
{
	class Scene;

	constexpr uint32_t MaxInstances = 1024;

	struct VisibleEntity
	{
		Material* Material;
		Mesh* Mesh;
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
		Material* Material;
		Mesh* Mesh;
		MeshLodResource* LodResource;
		FMeshSection* MeshSection;
		std::vector<Matrix4> Instances;
	};

	using RenderSet = std::vector<ModelContext>;

	class SceneRenderer
	{
	private:
		Buffer_ptr m_BaseVsDataBuffer;

		std::array<std::vector<VisibleEntity>, SortTypeCount> m_VisibleEntities;
	public:
		SceneRenderer();

		void Render(Scene* scene);
	};
}
