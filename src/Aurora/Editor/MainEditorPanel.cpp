#include "MainEditorPanel.hpp"

#include "Style.hpp"
#include "Utils.hpp"

#include "Aurora/Engine.hpp"
#include "Aurora/Aurora.hpp"
#include "Aurora/Graphics/ViewPortManager.hpp"
#include "Aurora/Framework/Actor.hpp"
#include "Aurora/Framework/CameraComponent.hpp"
#include "Aurora/Framework/MeshComponent.hpp"
#include "Aurora/Resource/ResourceManager.hpp"
#include "Aurora/Tools/IconsFontAwesome5.hpp"

ImVec2 operator+(const ImVec2& left, const ImVec2& right)
{
	return {left.x + right.x, left.y + right.y};
}

namespace Aurora
{
	MainEditorPanel::MainEditorPanel()
	: m_SelectedActor(nullptr),
		m_MouseViewportGrabbed(false),
		m_SelectedComponent(nullptr),
		m_CurrentManipulatorOperation(ImGuizmo::OPERATION::TRANSLATE),
		m_CurrentManipulatorMode(ImGuizmo::MODE::WORLD),
		m_IsPlayMode(true),
		m_FlySpeed(10.0f)
	{
		m_ConsoleWindow = std::make_shared<ConsoleWindow>();
		Logger::AddSinkPtr(m_ConsoleWindow);

		m_RenderViewPort = GEngine->GetViewPortManager()->Create(0, GraphicsFormat::SRGBA8_UNORM);
		m_RenderViewPort->Resize({1270, 720});

		SetEditorStyle();
	}

	MainEditorPanel::~MainEditorPanel() = default;

	const char* GetIconForComponent(ActorComponent* component)
	{
		if(component->HasType(CameraComponent::TypeID()))
			return ICON_FA_CAMERA;

		if(component->HasType(MeshComponent::TypeID()))
			return ICON_FA_CUBE;

		if(component->HasType(SceneComponent::TypeID()))
			return ICON_FA_LAYER_GROUP;

		return ICON_FA_QUESTION;
	}

	void SwitchToPlayMode()
	{
		// TODO: Copy scene here
		GameModeBase* gameModeBase = AppContext::GetGameModeBase();

		if(gameModeBase)
		{
			gameModeBase->BeginPlay();
		}
	}

	void SwitchToStopMode()
	{
		// TODO: Replace scene here

		GameModeBase* gameModeBase = AppContext::GetGameModeBase();

		if(gameModeBase)
		{
			gameModeBase->BeginDestroy();
		}
	}

	void MainEditorPanel::Update(double delta)
	{
		DrawMainMenu();
		BeginDockSpace();

		static int ID = 0;
		ID = 0;

		std::function<void(SceneComponent* component, bool)> drawComponent;
		drawComponent = [this, &drawComponent](SceneComponent* component, bool opened) -> void
		{
			ImGui::PushID(ID++);

			const std::vector<SceneComponent*>& childComponents = component->GetComponents();

			if(!opened)
			{
				for(SceneComponent* child : childComponents)
				{
					drawComponent(child, false);
				}
				ImGui::PopID();
				return;
			}

			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			int flags = m_SelectedComponent == component ? ImGuiTreeNodeFlags_Selected : 0;

			if(childComponents.empty())
			{
				ImGui::TreeNodeEx(component->GetName().c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth | flags);

				if(ImGui::IsItemClicked())
				{
					m_SelectedActor = nullptr;
					m_SelectedComponent = component;
				}

				/*ImGui::TableNextColumn();
				ImGui::TextDisabled("%s", GetIconForComponent(component));*/
				ImGui::TableNextColumn();
				ImGui::Text("%s %s", GetIconForComponent(component), component->GetTypeName());
				ImGui::TableNextColumn();

				bool active = component->IsActive() && component->IsParentActive();
				if(ImGui::IconCheckbox(ICON_FA_EYE, &active))
				{
					component->SetActive(active);
				}
			}
			else
			{
				bool open = ImGui::TreeNodeEx(component->GetName().c_str(), ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnArrow | flags);

				if(ImGui::IsItemClicked())
				{
					m_SelectedActor = nullptr;
					m_SelectedComponent = component;
				}

				/*ImGui::TableNextColumn();
				ImGui::TextDisabled("%s", GetIconForComponent(component));*/
				ImGui::TableNextColumn();
				ImGui::Text("%s %s", GetIconForComponent(component), component->GetTypeName());
				ImGui::TableNextColumn();
				bool active = component->IsActive() && component->IsParentActive();
				if(ImGui::IconCheckbox(ICON_FA_EYE, &active))
				{
					component->SetActive(active);
				}

				for(SceneComponent* child : childComponents)
				{
					drawComponent(child, open && opened);
				}

				if (open)
				{
					ImGui::TreePop();
				}
			}

			ImGui::PopID();
		};

		const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
		const float ICON_BASE_WIDTH = ImGui::CalcTextSize(ICON_FA_EYE).x;
		const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin(ICON_FA_LIST " Hierarchy");
		{
			if(ImGui::IsWindowClicked())
			{
				m_SelectedActor = nullptr;
				m_SelectedComponent = nullptr;
			}

			ImGui::BeginGroup();
			{
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 1));
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.16f, 0.16f, 0.16f, 0.16f));

				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
				ImGui::Button(ICON_FA_PLUS "##AddActorToScene");
				ImGui::PopStyleVar(2);

				ImGui::PopStyleColor(2);

				ImGui::SameLine();
				static String searchText;
				ImGui::InputTextLabel(ICON_FA_SEARCH, searchText, true);
			}
			ImGui::EndGroup();

			// ImGuiTableFlags_RowBg
			static ImGuiTableFlags tableFlags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoBordersInBody;
			if(ImGui::BeginTable("SceneView", 3, tableFlags))
			{
				ImGui::PushID(ID++);
				ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
				//ImGui::TableSetupColumn("---", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 12.0f);
				ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 18.0f);
				ImGui::TableSetupColumn(ICON_FA_EYE, ImGuiTableColumnFlags_WidthFixed, ICON_BASE_WIDTH * 1.0f);
				ImGui::TableHeadersRow();
				ImGui::PopID();

				for (Actor* actor : *AppContext::GetGameContext<GameContext>()->GetScene())
				{
					//String name = String("[") + actor->GetTypeName() + "] " + actor->GetName();

					ImGui::PushID(ID++);

					ImGui::TableNextRow();
					ImGui::TableNextColumn();

					int flags = m_SelectedActor == actor ? ImGuiTreeNodeFlags_Selected : 0;

					bool open = ImGui::TreeNodeEx(actor->GetName().c_str(), ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnArrow | flags);

					if(ImGui::IsItemClicked())
					{
						m_SelectedActor = actor;
						m_SelectedComponent = nullptr;
					}

					/*ImGui::TableNextColumn();
					ImGui::TextDisabled(ICON_FA_USER);*/
					ImGui::TableNextColumn();
					ImGui::Text("%s %s", ICON_FA_USER, actor->GetTypeName());
					ImGui::TableNextColumn();
					//ImGui::TextUnformatted(ICON_FA_EYE);

					bool active = actor->IsActive();
					if(ImGui::IconCheckbox(ICON_FA_EYE, &active))
					{
						actor->SetActive(active);
					}

					drawComponent(actor->GetRootComponent(), open);

					if (open)
					{
						ImGui::TreePop();
					}

					ImGui::PopID();
				}

				ImGui::EndTable();
			}
		}
		ImGui::End();
		ImGui::PopStyleVar();

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
				if (ImGui::BeginMenu("Play", !m_IsPlayMode))
				{
					m_IsPlayMode = true;
					SwitchToPlayMode();
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Stop", m_IsPlayMode))
				{
					m_IsPlayMode = false;
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

			if(CameraComponent* camera = *AppContext::GetScene()->GetComponents<CameraComponent>().begin())
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



				Matrix4 proj = camera->GetProjectionMatrix();
				Matrix4 view = camera->GetViewMatrix();

				if(m_SelectedActor || m_SelectedComponent)
				{
					Matrix4 transform = m_SelectedActor ? m_SelectedActor->GetRootComponent()->GetTransformationMatrix() : m_SelectedComponent->GetTransformationMatrix();

					bool manipulated = ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj), m_CurrentManipulatorOperation, m_CurrentManipulatorMode, glm::value_ptr(transform));
					if(manipulated)
					{
						if(m_SelectedActor)
							m_SelectedActor->GetRootComponent()->GetTransform().SetFromMatrix(transform);
						else
						if(m_SelectedComponent)
							m_SelectedComponent->GetTransform().SetFromMatrix(transform);
					}
				}

				//ImGuizmo::ViewManipulate(glm::value_ptr(view), 1, ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowWidth() - 128, 30), ImVec2(128, 128), 0);
			}
		}
		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::Begin("Resources");
		{
			/*for (auto& directory : std::filesystem::directory_iterator("../"))
			{
				ImGui::BeginGroup();
				{
					if(directory.is_directory())
					{
						EUI::ImageButton(m_FolderTexture, 64);
						ImGui::Text("%s", directory.path().filename().string().c_str());
					}
					else
					{
						EUI::ImageButton(m_FileTexture, 64);
						ImGui::Text("%s", directory.path().filename().string().c_str());
					}
				}
				ImGui::EndGroup();

				if(ImGui::GetCursorPosX() < 500)
				{
					ImGui::SameLine();
				}
			}*/
		}
		ImGui::End();

		m_ConsoleWindow->Draw();

		ImGui::Begin("Properties");
		{
			SceneComponent* root = nullptr;

			if(m_SelectedActor)
			{
				root = m_SelectedActor->GetRootComponent();
				std::string name = m_SelectedActor->GetName();
				if(ImGui::InputTextLabel("Name", name))
				{
					m_SelectedActor->SetName(name);
				}
			}
			else
			{
				root = m_SelectedComponent;
			}

			if(root)
			{
				ImGui::Separator();
				if(ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::DrawVec3Control("Location", root->GetTransform().Location);
					ImGui::DrawVec3Control("Rotation", root->GetTransform().Rotation);
					ImGui::DrawVec3Control("Scale", root->GetTransform().Scale);
				}
			}


		}
		ImGui::End();

		// FIXME: This is just for debugging purposes
		if (m_MouseViewportGrabbed)
		{
			m_FlySpeed += ImGui::GetIO().MouseWheel;

			if(m_FlySpeed < 0) m_FlySpeed = 0;

			if(CameraComponent* camera = *AppContext::GetScene()->GetComponents<CameraComponent>().begin())
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

	void MainEditorPanel::BeginDockSpace()
	{
		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		if (!opt_padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace", NULL, window_flags);
		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
		else
		{

		}

		/*if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Options"))
			{
				// Disabling fullscreen would allow the window to be moved to the front of other windows,
				// which we can't undo at the moment without finer window depth/z control.
				ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
				ImGui::MenuItem("Padding", NULL, &opt_padding);
				ImGui::Separator();

				if (ImGui::MenuItem("Flag: NoSplit",                "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0))                 { dockspace_flags ^= ImGuiDockNodeFlags_NoSplit; }
				if (ImGui::MenuItem("Flag: NoResize",               "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0))                { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
				if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0))  { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode; }
				if (ImGui::MenuItem("Flag: AutoHideTabBar",         "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0))          { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
				if (ImGui::MenuItem("Flag: PassthruCentralNode",    "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}*/

		ImGui::End();
	}

	void MainEditorPanel::DrawMainMenu()
	{
		if(ImGui::BeginMainMenuBar())
		{

			if (ImGui::BeginMenu("File"))
			{
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
				if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
				ImGui::Separator();
				if (ImGui::MenuItem("Cut", "CTRL+X")) {}
				if (ImGui::MenuItem("Copy", "CTRL+C")) {}
				if (ImGui::MenuItem("Paste", "CTRL+V")) {}
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}
}