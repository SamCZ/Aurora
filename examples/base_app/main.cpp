#include <Aurora/AuroraEngine.hpp>

#include <Aurora/Framework/Scene.hpp>
#include <Aurora/Assets/ModelImporter.hpp>
#include <Aurora/Assets/MaterialLoader.hpp>
#include <Aurora/Logger/std_sink.hpp>

#include <Aurora/Graphics/OpenGL/GLRenderDevice.hpp>
#include <Aurora/Graphics/Pipeline/DeferredRenderPipeline.hpp>

using namespace Aurora;

class BaseCameraActor : public Actor
{
private:
	CameraComponent* m_CameraComponent;

public:
	BaseCameraActor() : Actor(), m_CameraComponent(nullptr) {}

	void InitializeComponents() override
	{
		m_CameraComponent = AddComponent<CameraComponent>("Camera", 0, 0, 0);
		SetRootComponent(m_CameraComponent);
	}

	CameraComponent* GetCamera() { return m_CameraComponent; }
};

struct alignas(16) CameraConstants
{
	Matrix4 ProjectionViewMatrix;
	Matrix4 ModelMatrix;
};

class BaseAppContext : public WindowGameContext
{
private:
	Scene_ptr m_MainScene;
public:
	explicit BaseAppContext(const IWindow_ptr &window) : WindowGameContext(window)
	{
		m_MainScene = Scene::New();
	}

	~BaseAppContext() override
	{

	}

	void Init() override
	{
		{ // Create skybox
			auto cubeModel = ModelImporter::LoadMesh(AuroraEngine::AssetManager->LoadFile("Assets/Cube.FBX"));
			cubeModel->UpdateBuffers();


			auto cubemapMat = MaterialLoader::Load("Assets/Materials/skybox.json").Finish();

			cubemapMat->SetCullMode(ECullMode::None);
			cubemapMat->SetDepthEnable(false);

			cubeModel->MaterialSlots[0].Material = cubemapMat;

			for (int j = 0; j < 1000; ++j)
			{
				auto m_SkyBox = m_MainScene->SpawnActor<Actor, MeshComponent>("SkyBox", Vector3D(0.0), Vector3D(0.0), Vector3D(10.0));
				m_SkyBox->GetRootComponent()->SetCanReceiveCollisions(false);
				m_SkyBox->GetRootComponent()->SetSimulatePhysics(false);
				auto *meshComponent = m_SkyBox->GetRootComponent()->SafeCast<MeshComponent>();
				meshComponent->SetMesh(cubeModel);
				m_SkyBox->GetRootComponent()->GetBody().SetCollider(nullptr);
			}
		}

		auto* cameraActor = m_MainScene->SpawnActor<BaseCameraActor>("Camera", Vector3D(0.0));
		cameraActor->GetCamera()->Resize(GetWindow()->GetSize());

		ASM->LoadGLTF("Assets/box.gltf");
	}

	void Update(double delta, double currentTime) override
	{
		ZoneScopedN("Update");

		m_MainScene->Update(delta);
	}

	void Render() override
	{
		ZoneScopedN("Render");

	}
};

int main()
{
	std::filesystem::current_path(R"(C:\Sam\Projects\EmberSky)");

	Logger::AddSink<std_sink>();

	AuroraEngine::Init();

	AuroraEngine::AddWindow<BaseAppContext>(1280, 720, "Aurora - Base App", true);

	return AuroraEngine::Run();
}