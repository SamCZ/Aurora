#pragma once

#include <Aurora/Core/Common.hpp>
#include <Aurora/Framework/Scene.hpp>

#include "RenderTargetManager.hpp"

namespace Aurora
{
	struct ModelContext
	{
		Mesh *m_Mesh;
		Material *m_Material;
		uint32_t m_iSectionIndex;
		Matrix4 m_Transform;
		LOD m_Lod;
		bool m_bMeshObjectChanged;
	};

	class MaterialRenderList
	{
	public:
		typedef std::vector<ModelContext*> ModelContextList;
		typedef std::map<UniqueIdentifier, ModelContextList> MaterialMap;
	private:
		MaterialMap m_Map;
	public:
		void Add(ModelContext *mc)
		{
			m_Map[mc->m_Material->GetUniqueID()].push_back(mc);
		}

		void Clear()
		{
			ZoneScopedN("Clear")
			m_Map.clear();
		}

		[[nodiscard]] const MaterialMap &Map() const
		{
			return m_Map;
		}
	};

	struct CameraQueueList
	{
		MaterialRenderList OpaqueQueue;
		MaterialRenderList TransparentQueue;
		MaterialRenderList TranslucentQueue;
		MaterialRenderList SkyQueue;
	};

	AU_CLASS(SceneRenderer)
	{
	private:
		Scene_ptr m_Scene;

		std::vector<ModelContext*> m_ModelContextCache;
		uint32_t m_iCurrentModelContextIndex = 0;

		std::unordered_map<uint, CameraQueueList> m_CameraQueueList;

		Buffer_ptr m_CameraConstantsUniformBuffer;
	public:
		explicit SceneRenderer(Scene_ptr scene);
		~SceneRenderer();

		void Update(double delta);
		void Render(RenderTargetPack* renderTargetPack, bool apply, bool clear);

	private:
		ModelContext *GetModelContext();
		void RenderQueue(DrawCallState& drawCallState, CameraComponent *camera, MaterialRenderList &renderQueue);
	};
}
