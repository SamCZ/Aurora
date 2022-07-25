#include <Aurora/Aurora.hpp>

#include <Aurora/Core/AUID.hpp>
#include <Aurora/Resource/ResourceManager.hpp>

#include <Aurora/Graphics/Material/MaterialDefinition.hpp>
#include <Aurora/Graphics/Material/Material.hpp>
#include <Aurora/Graphics/ViewPortManager.hpp>
#include <Aurora/Resource/MaterialLoader.hpp>

#include <Shaders/World/PBRBasic/cb_pbr.h>

#include <Aurora/Memory/Aum.hpp>
#include <Aurora/Framework/Scene.hpp>
#include <Aurora/Framework/Actor.hpp>
#include <Aurora/Framework/SceneComponent.hpp>
#include <Aurora/Framework/CameraComponent.hpp>
#include <Aurora/Framework/StaticMeshComponent.hpp>
#include <Aurora/Framework/SkeletalMeshComponent.hpp>
#include <Aurora/Framework/Lights.hpp>
#include <Aurora/Framework/ParticleSystemComponent.hpp>

#include <Aurora/Resource/AssimpModelLoader.hpp>
#include <Aurora/Resource/ResourceName.hpp>

#include "Aurora/Editor/MainEditorPanel.hpp"

#include <Aurora/Graphics/VgRender.hpp>

#include <Aurora/Render/SceneRendererDeferred.hpp>
#include <Aurora/Render/SceneRendererForward.hpp>
#include <Aurora/Render/SceneRendererDeferredNew.hpp>

#include <Aurora/Framework/AnimationProcessing/FbxImporter.hpp>

using namespace Aurora;

class BaseAppContext : public AppContext
{
	SceneRenderer* sceneRenderer;

	~BaseAppContext() override
	{
		delete sceneRenderer;

	}

	// FIXME: this is just an hack!
	SceneRenderer* GetSceneRenderer() override { return sceneRenderer; }

	void Init() override
	{
		SetGameContext<GameContext>();
		SwitchGameModeImmediately<GameModeBase>()->BeginPlay();

		sceneRenderer = new SceneRendererForward();

		FbxImport::LoadScene(GEngine->GetResourceManager()->LoadFile("Assets/socuwan.fbx"));
	}

	void Update(double delta) override
	{

	}

	void Render() override
	{
		sceneRenderer->Render(GetScene());
	}

	void RenderVg() override
	{

	}
};

int main()
{
	WindowDefinition windowDefinition = {};
	windowDefinition.Width = 1270;
	windowDefinition.Height = 720;
	windowDefinition.HasOSWindowBorder = true;
	windowDefinition.Maximized = true;
	windowDefinition.Title = "Aurora - BaseApp";

	Aurora::AuroraEngine engine;
	engine.Init(new BaseAppContext(), windowDefinition, true);
	engine.Run();
	return 0;
}