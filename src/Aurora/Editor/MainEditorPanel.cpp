#include "MainEditorPanel.hpp"

#include "Style.hpp"

#include "Aurora/Engine.hpp"
#include "Aurora/Aurora.hpp"
#include "Aurora/Graphics/ViewPortManager.hpp"
#include "Aurora/Framework/Actor.hpp"

namespace Aurora
{
	MainEditorPanel::MainEditorPanel() : m_SelectedActor(nullptr)
	{
		m_ConsoleWindow = std::make_shared<ConsoleWindow>();
		Logger::AddSinkPtr(m_ConsoleWindow);

		m_RenderViewPort = GEngine->GetViewPortManager()->Create(0, GraphicsFormat::SRGBA8_UNORM);

		SetEditorStyle();
	}

	MainEditorPanel::~MainEditorPanel() = default;

	void MainEditorPanel::Update()
	{
		DrawMainMenu();
		BeginDockSpace();

		ImGui::Begin("Scene");
		{
			int i = 0;
			for (Actor* actor : AppContext::GetGameContext<GameContext>()->GetScene())
			{
				String name = String("[") + actor->GetTypeName() + "] " + actor->GetName();
				//ImGui::Text("%s", actor->GetTypeName());

				uint8 flags = actor == m_SelectedActor ? ImGuiTreeNodeFlags_Selected : 0;
				if(ImGui::TreeNodeEx((name + "##Node" + std::to_string(i)).c_str(), ImGuiTreeNodeFlags_OpenOnArrow | flags))
				{
					if (ImGui::IsItemActivated())
					{
						m_SelectedActor = actor;
					}

					ImGui::Text("yo");

					ImGui::TreePop();
				}
				i++;
			}
		}
		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_MenuBar);
		{
			if (ImGui::BeginMenuBar())
			{
				ImGui::Spacing();
				static bool isPlaying = false;
				if (ImGui::BeginMenu("Play", !isPlaying))
				{
					isPlaying = true;
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Stop", isPlaying))
				{
					isPlaying = false;
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			ImVec2 avail_size = ImGui::GetContentRegionAvail();
			ImVec2 dragDelta = ImGui::GetMouseDragDelta();
			Vector2i viewPortSize = {avail_size.x, avail_size.y};

			if((dragDelta.x == 0 && dragDelta.y == 0) || !m_RenderViewPort->Initialized())
			{
				m_RenderViewPort->Resize(viewPortSize);
			}

			ImGui::Image((ImTextureID)m_RenderViewPort->Target->GetRawHandle(), ImVec2(m_RenderViewPort->ViewPort.Width, m_RenderViewPort->ViewPort.Height));
		}
		ImGui::End();
		ImGui::PopStyleVar();



		ImGui::Begin("Resources");

		ImGui::End();

		m_ConsoleWindow->Draw();

		ImGui::Begin("Properties");
		if(m_SelectedActor)
		{
			std::string name = m_SelectedActor->GetName();
			if(ImGui::InputText("Name", name))
			{
				m_SelectedActor->SetName(name);
			}
		}
		ImGui::End();
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