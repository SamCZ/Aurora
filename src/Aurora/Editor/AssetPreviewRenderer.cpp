#include "AssetPreviewRenderer.hpp"
#include "Aurora/Engine.hpp"
#include "Aurora/Resource/ResourceManager.hpp"
#include "Aurora/Framework/CameraComponent.hpp"
#include "Aurora/Framework/StaticMeshComponent.hpp"
#include "Aurora/Framework/Lights.hpp"
#include "Aurora/Graphics/ViewPortManager.hpp"

namespace Aurora
{

	AssetPreviewRenderer::AssetPreviewRenderer() : m_ViewPortManager(new ViewPortManager())
	{
		m_Camera = m_Scene.SpawnActor<Actor, CameraComponent>("Camera", {})->GetRootComponent<CameraComponent>();
		m_Camera->SetClearColor(FColor(0, 0, 0, 0));

		auto* dirLight = m_Scene.SpawnActor<DirectionalLight>("DirLight", {})->GetRootComponent<DirectionalLightComponent>();
		dirLight->GetTransform().Rotation = Vector3(-45, -45, 0);

		m_StaticMeshComponentBox = m_Scene.SpawnActor<Actor, StaticMeshComponent>("Mesh", {})->GetRootComponent<StaticMeshComponent>();
		m_StaticMeshComponentBox->GetTransform().Scale = Vector3(0.01f);
		m_StaticMeshComponentBox->GetTransform().Rotation = Vector3(-45, 45, 0);
		m_StaticMeshComponentBox->SetMesh(GEngine->GetResourceManager()->LoadMesh("Assets/Shapes/Box.amesh"));
		m_SkyBoxMaterial = GEngine->GetResourceManager()->GetOrLoadMaterialDefinition("Assets/Materials/Base/SkyBox.matd")->CreateInstance();
		m_StaticMeshComponentBox->SetMaterial(0, m_SkyBoxMaterial);

		m_StaticMeshComponentSphere = m_Scene.SpawnActor<Actor, StaticMeshComponent>("Sphere", {})->GetRootComponent<StaticMeshComponent>();
		m_StaticMeshComponentSphere->GetTransform().Scale = Vector3(0.01f);
		m_StaticMeshComponentSphere->GetTransform().Rotation = Vector3(-45, 45, 0);
		m_StaticMeshComponentSphere->SetMesh(GEngine->GetResourceManager()->LoadMesh("Assets/Shapes/Sphere.amesh"));
		m_StaticMeshComponentSphere->SetMaterial(0, m_SkyBoxMaterial);
	}

	AssetPreviewRenderer::~AssetPreviewRenderer()
	{
		delete m_ViewPortManager;
	}

	Texture_ptr AssetPreviewRenderer::SetupRender(const Vector2i& size)
	{
		RenderViewPort* rwp = m_ViewPortManager->Get(0);

		if (!rwp)
		{
			rwp = m_ViewPortManager->Create(0, GraphicsFormat::SRGBA8_UNORM);
		}

		rwp->Resize(size, true);

		m_Camera->SetViewPort(rwp);

		//float half = 1;
		//m_Camera->SetOrthographic(-half, half, half, -half, -half, half);
		m_Camera->SetPerspective(1.0f, 0.1f, 1000.0f);

		m_Scene.Update(0);

		return rwp->Target;
	}

	Texture_ptr AssetPreviewRenderer::RenderCubeMap(const Vector2i &size, const Texture_ptr& cubeMap)
	{
		Texture_ptr target = SetupRender(size);
		m_Camera->GetTransform().Location = Vector3(0, 0, 200);

		m_StaticMeshComponentBox->SetActive(true);
		m_StaticMeshComponentSphere->SetActive(false);

		m_SkyBoxMaterial->SetTexture("CubeMap"_HASH, cubeMap);

		m_SceneRenderer.Render(&m_Scene);
		return target;
	}

	Texture_ptr AssetPreviewRenderer::RenderMaterial(const Vector2i &size, const Material_ptr& material)
	{
		Texture_ptr target = SetupRender(size);
		m_Camera->SetOrthographic(-1, 1, 1, -1, -1, 1);
		m_Camera->GetTransform().Location = Vector3(0, 0, 0);

		m_StaticMeshComponentBox->SetActive(false);
		m_StaticMeshComponentSphere->SetActive(true);
		m_StaticMeshComponentSphere->SetMaterial(0, material);

		m_SceneRenderer.Render(&m_Scene);
		return target;
	}
}