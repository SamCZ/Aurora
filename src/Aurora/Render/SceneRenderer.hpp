#pragma once

#include <array>
#include "Aurora/Core/Delegate.hpp"
#include "Aurora/Core/Library.hpp"
#include "Aurora/Tools/robin_hood.h"
#include "Aurora/Graphics/Material/Material.hpp"
#include "Aurora/Graphics/PassType.hpp"
#include "Aurora/Framework/Mesh/Mesh.hpp"

namespace Aurora
{
	class Scene;
	class FFrustum;
	class CameraComponent;

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

	typedef EventEmitter<PassType_t, DrawCallState&, CameraComponent*> PassRenderEventEmitter;

	class AU_API SceneRenderer
	{
	private:
		Buffer_ptr m_BaseVsDataBuffer;
		Buffer_ptr m_InstancesBuffer;

		robin_hood::unordered_map<TTypeID, InputLayout_ptr> m_MeshInputLayouts;

		std::array<std::vector<VisibleEntity>, SortTypeCount> m_VisibleEntities;
		std::array<PassRenderEventEmitter, Pass::Count> m_InjectedPasses;
	public:
		SceneRenderer();

		void PrepareVisibleEntities(Scene* scene, CameraComponent* camera);
		void FillRenderSet(RenderSet& renderSet);

		void Render(Scene* scene);
		void RenderPass(PassType_t pass, DrawCallState& drawCallState, CameraComponent* camera, const RenderSet& renderSet);

		const InputLayout_ptr& GetInputLayoutForMesh(Mesh* mesh);

		PassRenderEventEmitter& GetPassEmitter(PassType_t passType) { return m_InjectedPasses[passType]; }
	};
}
