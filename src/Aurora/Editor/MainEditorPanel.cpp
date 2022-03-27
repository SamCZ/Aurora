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
		m_SelectedComponent(nullptr),
		m_IsPlayMode(false),
		m_SceneHierarchyWindow(this),
		m_ResourceWindow(this),
		m_MaterialWindow()
	{
		m_ConsoleWindow = std::make_shared<ConsoleWindow>();
		m_GameViewportWindow = std::make_shared<GameViewportWindow>(this);
		Logger::AddSinkPtr(m_ConsoleWindow);

		SetEditorStyle();
	}

	MainEditorPanel::~MainEditorPanel() = default;

	void MainEditorPanel::Update(double delta)
	{
		DrawMainMenu();
		BeginDockSpace();

		m_SceneHierarchyWindow.Update(delta);
		m_ResourceWindow.Update(delta);

		m_ConsoleWindow->Draw();
		m_GameViewportWindow->Update(delta);

		m_MaterialWindow.Update(delta);

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