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
	ImFont* m_BigIconFont;

	MainEditorPanel::MainEditorPanel()
	: m_SelectedActor(nullptr),
		m_SelectedComponent(nullptr),
		m_IsPlayMode(false)
	{
		m_ConsoleWindow = std::make_shared<ConsoleWindow>();
		m_GameViewportWindow = std::make_shared<GameViewportPanel>(this);
		Logger::AddSinkPtr(m_ConsoleWindow);

		SetEditorStyle();

		static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 }; // Will not be copied by AddFont* so keep in scope.
		ImFontConfig config;
		config.MergeMode = false;
		//config.PixelSnapH = true;
		auto iconFontData = new std::vector<uint8>(GEngine->GetResourceManager()->LoadFile("Assets/Fonts/fa-solid-900.ttf"));
		m_BigIconFont = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(iconFontData->data(), iconFontData->size(), 56, &config, icons_ranges);
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
				ImGui::AlignTextToFramePadding();
				ImGui::TreeNodeEx(component->GetName().c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth | flags);

				if(ImGui::IsItemClicked())
				{
					m_SelectedActor = nullptr;
					m_SelectedComponent = component;
				}

				/*ImGui::TableNextColumn();
				ImGui::TextDisabled("%s", GetIconForComponent(component));*/
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("%s %s", GetIconForComponent(component), component->GetTypeName());
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				bool active = component->IsActive() && component->IsParentActive();
				if(ImGui::IconCheckbox(ICON_FA_EYE, &active))
				{
					component->SetActive(active);
				}
			}
			else
			{
				ImGui::AlignTextToFramePadding();
				bool open = ImGui::TreeNodeEx(component->GetName().c_str(), ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnArrow | flags);

				if(ImGui::IsItemClicked())
				{
					m_SelectedActor = nullptr;
					m_SelectedComponent = component;
				}

				/*ImGui::TableNextColumn();
				ImGui::TextDisabled("%s", GetIconForComponent(component));*/
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("%s %s", GetIconForComponent(component), component->GetTypeName());
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
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
		ImGui::Begin(ICON_FA_LIST " Hierarchy", nullptr, ImGuiWindowFlags_NoCollapse);
		{
			if(ImGui::IsWindowClicked())
			{
				m_SelectedActor = nullptr;
				m_SelectedComponent = nullptr;
			}

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 5));
			ImGui::BeginChildFrame(ImGui::GetID("search_frame"), ImVec2(0, 32));
			ImGui::PopStyleVar(1);
			{
				if (ImGui::IconButton(ICON_FA_PLUS "##AddActorToScene"))
				{
					ImGui::OpenPopup("add_new_to_scene");
				}

				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
				bool popupOpened = ImGui::BeginPopup("add_new_to_scene");

				if (popupOpened)
				{
					static const char* types[] = {
						"Empty Actor",
						"Directional light",
						"Point light",
						"-",
						"Box",
						"Sphere"
					};

					for (auto& type : types)
					{
						if(strlen(type) == 1)
						{
							ImGui::Separator();
							continue;
						}

						if (ImGui::Selectable(type))
						{

						}
					}

					ImGui::EndPopup();
				}
				ImGui::PopStyleVar(1);

				ImGui::SameLine();
				static String searchText;
				ImGui::InputTextLabel(ICON_FA_SEARCH, searchText, true);
			}
			ImGui::EndChildFrame();

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

					ImGui::AlignTextToFramePadding();
					bool open = ImGui::TreeNodeEx(actor->GetName().c_str(), ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnArrow | flags);

					if(ImGui::IsItemClicked())
					{
						m_SelectedActor = actor;
						m_SelectedComponent = nullptr;
					}

					ImGui::TableNextColumn();
					ImGui::AlignTextToFramePadding();
					ImGui::Text("%s %s", ICON_FA_USER, actor->GetTypeName());
					ImGui::TableNextColumn();
					ImGui::AlignTextToFramePadding();

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


		ImGui::Begin("Resources");
		{
			ImGui::PushFont(m_BigIconFont);
			const float BIG_ICON_BASE_WIDTH = ImGui::CalcTextSize(ICON_FA_FOLDER).x;
			ImGui::PopFont();

			static const Path basePath = AURORA_PROJECT_DIR "/Assets";
			static Path currentPath = AURORA_PROJECT_DIR "/Assets";
			static String searchText;

			{ // Top menu
				if(currentPath == basePath)
					ImGui::BeginDisabled();

				if(ImGui::IconButton(ICON_FA_ARROW_LEFT))
				{
					currentPath = currentPath.parent_path();
				}

				if(currentPath == basePath)
					ImGui::EndDisabled();

				ImGui::SameLine();
				ImGui::InputTextLabel(ICON_FA_SEARCH, searchText);
				ImGui::Separator();
			}

			float regionAvail = ImGui::GetContentRegionAvail().x;
			float columnSize = BIG_ICON_BASE_WIDTH + 15;
			int columnCount = std::max(1, (int)floor(regionAvail / columnSize));
			ImGui::Columns(columnCount, "resource-columns", false);

			auto drawFile = [](const std::filesystem::directory_entry& directoryIt) -> void
			{
				const Path& path = directoryIt.path();

				String fileName = path.filename().string();

				ImGui::PushID(fileName.c_str());

				ImGui::PushFont(m_BigIconFont);
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 3.0f);
				if(directoryIt.is_directory())
				{
					ImGui::Selectable(ICON_FA_FOLDER);
					if(ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(0))
					{
						currentPath = path;
					}
				}
				else
				{
					if(ImGui::Selectable(ICON_FA_FILE))
					{

					}
				}
				ImGui::PopStyleVar(2);
				ImGui::PopFont();

				ImGui::TextWrapped("%s", fileName.substr(0, std::min<int>(10, fileName.length())).c_str());

				ImGui::NextColumn();

				ImGui::PopID();
			};

			for (auto& directoryIt : std::filesystem::directory_iterator(currentPath))
			{
				if(directoryIt.is_directory())
					drawFile(directoryIt);
			}

			for (auto& directoryIt : std::filesystem::directory_iterator(currentPath))
			{
				if(!directoryIt.is_directory())
					drawFile(directoryIt);
			}

			ImGui::Columns(1);
		}
		ImGui::End();

		m_ConsoleWindow->Draw();
		m_GameViewportWindow->Update(delta);

		ImGui::Begin("Properties");
		{
			SceneComponent* root;

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

		static bool isDemoWindowRendering = false;

		if (ImGui::IsKeyPressed(ImGuiKey_F9, false))
		{
			isDemoWindowRendering = !isDemoWindowRendering;
		}

		if (isDemoWindowRendering)
			ImGui::ShowDemoWindow();

		// Handle input

		if (m_SelectedActor && ImGui::IsKeyPressed(ImGuiKey_Delete, false))
		{
			m_SelectedActor->Destroy();
			m_SelectedActor = nullptr;
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
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags | ImGuiDockNodeFlags_NoWindowMenuButton);
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

	bool MainEditorPanel::GetSelectedObjectTransform(Matrix4 &matrix) const
	{
		if(!IsAnySceneObjectSelected())
			return false;

		if(m_SelectedActor)
			matrix = m_SelectedActor->GetTransform().GetTransform();

		if(m_SelectedComponent)
			matrix = m_SelectedComponent->GetTransform().GetTransform();

		return true;
	}

	bool MainEditorPanel::SetSelectedObjectTransform(const Matrix4 &matrix)
	{
		if(!IsAnySceneObjectSelected())
			return false;

		if(m_SelectedActor)
			m_SelectedActor->GetTransform().SetFromMatrix(matrix);

		if(m_SelectedComponent)
			m_SelectedComponent->GetTransform().SetFromMatrix(matrix);

		return true;
	}
}