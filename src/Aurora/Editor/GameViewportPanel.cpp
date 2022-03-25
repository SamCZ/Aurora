#include "GameViewportPanel.hpp"

#include "Utils.hpp"

#include "Aurora/Engine.hpp"
#include "Aurora/Aurora.hpp"
#include "Aurora/Graphics/ViewPortManager.hpp"
#include "Aurora/Framework/Actor.hpp"
#include "Aurora/Framework/CameraComponent.hpp"
#include "Aurora/Framework/MeshComponent.hpp"
#include "Aurora/Resource/ResourceManager.hpp"
#include "Aurora/Tools/IconsFontAwesome5.hpp"

#include "MainEditorPanel.hpp"

namespace Aurora
{

	void SwitchToPlayMode()
	{
		// TODO: Copy scene here
		GameModeBase* gameModeBase = AppContext::GetGameModeBase();

		if(gameModeBase)
		{
			//gameModeBase->BeginPlay();
		}
	}

	void SwitchToStopMode()
	{
		// TODO: Replace scene here

		GameModeBase* gameModeBase = AppContext::GetGameModeBase();

		if(gameModeBase)
		{
			//gameModeBase->BeginDestroy();
		}
	}

	GameViewportPanel::GameViewportPanel(MainEditorPanel* mainEditorPanel)
		: m_MainPanel(mainEditorPanel),
		m_MouseViewportGrabbed(false),
		m_CurrentManipulatorOperation(ImGuizmo::OPERATION::TRANSLATE),
		m_CurrentManipulatorMode(ImGuizmo::MODE::WORLD),
		m_FlySpeed(10.0f), m_CurrentScene(nullptr), m_SceneChangeEvent()
	{
		m_RenderViewPort = GEngine->GetViewPortManager()->Create(0, GraphicsFormat::SRGBA8_UNORM);
		m_RenderViewPort->Resize({1270, 720});

		m_SceneChangeEvent = AppContext::GetSceneChangeEmitter().BindUnique(this, &GameViewportPanel::OnSceneChanged);
	}

	GameViewportPanel::~GameViewportPanel()
	{

	}

	void GameViewportPanel::OnSceneChanged(Scene* scene)
	{
		if(m_CurrentScene == scene)
			return;

		m_CurrentScene = scene;

		m_EditorCameraActor = scene->SpawnActor<Actor, CameraComponent>("EditorCameraActor", Vector3(), Vector3());
		m_EditorCameraActor->SetActive(false);
		m_EditorCamera = CameraComponent::SafeCast(m_EditorCameraActor->GetRootComponent());
		m_EditorCamera->SetName("EditorCamera");
		m_EditorCamera->SetViewPort(m_RenderViewPort);
		m_EditorCamera->SetPerspective(70.0f, 0.1f, 2000.0f);
	}

	void GameViewportPanel::Update(double delta)
	{
		if(m_MouseViewportGrabbed && !ImGui::GetIO().MouseDown[1])
		{
			m_MouseViewportGrabbed = false;
			GEngine->GetWindow()->SetCursorMode(ECursorMode::Normal);
		}

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_MenuBar);
		{
			if(ImGui::IsWindowMouseDown(1))
			{
				m_MouseViewportGrabbed = true;
				GEngine->GetWindow()->SetCursorMode(ECursorMode::Disabled);
			}

			if (ImGui::BeginMenuBar())
			{
				ImGui::Spacing();
				if (ImGui::BeginMenu("Play", !m_MainPanel->IsPlayMode()))
				{
					m_MainPanel->SetPlayMode(true);
					SwitchToPlayMode();
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Stop", m_MainPanel->IsPlayMode()))
				{
					m_MainPanel->SetPlayMode(false);
					SwitchToStopMode();
					ImGui::EndMenu();
				}

				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 0.25f));
				ImGui::Text("Fly speed: %.1fx", m_FlySpeed);
				ImGui::PopStyleColor();

				ImGui::EndMenuBar();
			}

			ImVec2 avail_size = ImGui::GetContentRegionAvail();
			ImVec2 dragDelta = ImGui::GetMouseDragDelta();
			Vector2i viewPortSize = {avail_size.x, avail_size.y};

			if((dragDelta.x == 0 && dragDelta.y == 0) || !m_RenderViewPort->Initialized())
			{
				m_RenderViewPort->Resize(viewPortSize);
			}


			ImGuiViewport* imwp = ImGui::GetWindowViewport();
			// This outputs coords without the title bar... so we need to subtract that from CursorPos
			// because all positions in imgui are without the windows title bar
			// TODO: Create some helper function for this
			ImVec2 pos = ImGui::GetCurrentWindow()->DC.CursorPos;
			ImVec2 offset = imwp->Pos;
			// Sets proxy for rmlui cursor pos
			m_RenderViewPort->ProxyLocation = {pos.x - offset.x, pos.y - offset.y };

			EUI::Image(m_RenderViewPort->Target, (Vector2)m_RenderViewPort->ViewPort);

			//Manipulator
			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(
				ImGui::GetWindowPos().x,
				ImGui::GetWindowPos().y,
				ImGui::GetWindowWidth(),
				ImGui::GetWindowHeight()
			);

			if (!m_MainPanel->IsPlayMode())
			{
				if(!ImGui::GetIO().WantTextInput && !m_MouseViewportGrabbed)
				{
					if(ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_S)])
					{
						m_CurrentManipulatorOperation = ImGuizmo::OPERATION::SCALE;
					}

					if(ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_R)])
					{
						m_CurrentManipulatorOperation = ImGuizmo::OPERATION::ROTATE;
					}

					if(ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_T)])
					{
						m_CurrentManipulatorOperation = ImGuizmo::OPERATION::TRANSLATE;
					}
				}



				Matrix4 proj = m_EditorCamera->GetProjectionMatrix();
				Matrix4 view = m_EditorCamera->GetViewMatrix();

				Matrix4 transform;
				if(m_MainPanel->GetSelectedObjectTransform(transform))
				{
					bool manipulated = ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj), m_CurrentManipulatorOperation, m_CurrentManipulatorMode, glm::value_ptr(transform));
					if(manipulated)
					{
						(void)m_MainPanel->SetSelectedObjectTransform(transform);
					}
				}

				//ImGuizmo::ViewManipulate(glm::value_ptr(view), 1, ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowWidth() - 128, 30), ImVec2(128, 128), 0);
			}
		}
		ImGui::End();
		ImGui::PopStyleVar();

		HandleEditorCamera(delta);
	}

	void GameViewportPanel::HandleEditorCamera(double delta)
	{
		bool playMode = m_MainPanel->IsPlayMode();

		m_EditorCameraActor->SetActive(!playMode);

		for (CameraComponent* camera : m_CurrentScene->GetComponents<CameraComponent>())
		{
			if(camera->GetOwner() == m_EditorCameraActor)
				continue;

			camera->SetActive(playMode);
		}

		if (m_MouseViewportGrabbed && !m_MainPanel->IsPlayMode())
		{
			m_FlySpeed += ImGui::GetIO().MouseWheel;

			if(m_FlySpeed < 0) m_FlySpeed = 0;

			if(auto* camera= m_EditorCameraActor->GetRootComponent())
			{
				camera->GetTransform().Rotation.x -= ImGui::GetIO().MouseDelta.y * 0.1f;
				camera->GetTransform().Rotation.y -= ImGui::GetIO().MouseDelta.x * 0.1f;

				camera->GetTransform().Rotation.x = glm::clamp(camera->GetTransform().Rotation.x, -90.0f, 90.0f);
				camera->GetTransform().Rotation.y = fmod(camera->GetTransform().Rotation.y, 360.0f);

				Matrix4 transform = camera->GetTransformationMatrix();

				if(ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_W)])
				{
					camera->GetTransform().Location -= Vector3(transform[2]) * (float)delta * m_FlySpeed;
				}

				if(ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_S)])
				{
					camera->GetTransform().Location += Vector3(transform[2]) * (float)delta * m_FlySpeed;
				}

				if(ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_A)])
				{
					camera->GetTransform().Location -= Vector3(transform[0]) * (float)delta * m_FlySpeed;
				}

				if(ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_D)])
				{
					camera->GetTransform().Location += Vector3(transform[0]) * (float)delta * m_FlySpeed;
				}
			}
		}
	}

	CameraComponent *GameViewportPanel::GetCurrentCamera()
	{
		if (!m_MainPanel->IsPlayMode())
		{
			return CameraComponent::Cast(m_EditorCameraActor->GetRootComponent());
		}

		for (CameraComponent* camera : m_CurrentScene->GetComponents<CameraComponent>())
		{
			if(camera->GetOwner() == m_EditorCameraActor)
				continue;

			return camera;
		}

		return nullptr;
	}
}