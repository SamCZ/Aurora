#pragma once

#include "Aurora/Framework/Scene.hpp"
#include "Aurora/Graphics/Material/Material.hpp"
#include "Aurora/Render/SceneRenderer.hpp"

namespace Aurora
{
	class CameraComponent;
	class ViewPortManager;
	class StaticMeshComponent;

	class AssetPreviewRenderer
	{
	private:
		Scene* m_Scene;
		SceneRenderer m_SceneRenderer;

		ViewPortManager* m_ViewPortManager;
		CameraComponent* m_Camera;

		StaticMeshComponent* m_StaticMeshComponentBox;
		StaticMeshComponent* m_StaticMeshComponentSphere;
		Material_ptr m_SkyBoxMaterial;
	public:
		AssetPreviewRenderer();
		~AssetPreviewRenderer();

		Texture_ptr SetupRender(const Vector2i& size);

		Texture_ptr RenderCubeMap(const Vector2i& size, const Texture_ptr& cubeMap);
		Texture_ptr RenderMaterial(const Vector2i& size, const Material_ptr& material);
	};
}
