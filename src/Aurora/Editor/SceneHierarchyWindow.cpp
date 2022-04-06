#include "SceneHierarchyWindow.hpp"

#include "Utils.hpp"

#include "Aurora/Engine.hpp"
#include "Aurora/Aurora.hpp"
#include "Aurora/Graphics/ViewPortManager.hpp"
#include "Aurora/Framework/Actor.hpp"
#include "Aurora/Framework/CameraComponent.hpp"
#include "Aurora/Framework/MeshComponent.hpp"
#include "Aurora/Framework/Lights.hpp"
#include "Aurora/Resource/ResourceManager.hpp"
#include "Aurora/Tools/IconsFontAwesome5.hpp"

#include "MainEditorPanel.hpp"

namespace Aurora
{
	namespace EUI
	{
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
	}

	SceneHierarchyWindow::SceneHierarchyWindow(MainEditorPanel* mainEditorPanel) : m_MainEditorPanel(mainEditorPanel)
	{

	}

	void SceneHierarchyWindow::LookAtObject(Actor* actor)
	{
		CameraComponent* editorCamera = m_MainEditorPanel->GetGameViewPortWindow()->GetEditorCamera();

		// Check if not clicking on same actor
		if(editorCamera->GetOwner() == actor)
			return;

		Vector3 actorWorldLocation = actor->GetRootComponent()->GetTransform().Location;

		float cameraDistance = 3.0f; // TODO:: Compute this by the actor mesh size (if present)

		Vector3 newCameraLocation = actorWorldLocation + cameraDistance;

		editorCamera->GetTransform().Location = newCameraLocation;
		editorCamera->GetTransform().Rotation = Vector3(-35, 45, 0);
	}

	void SceneHierarchyWindow::Update(double delta)
	{
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

			int flags = m_MainEditorPanel->GetSelectedComponent() == component ? ImGuiTreeNodeFlags_Selected : 0;

			if(childComponents.empty())
			{
				ImGui::AlignTextToFramePadding();
				ImGui::TreeNodeEx(component->GetName().c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth | flags);

				if(ImGui::IsItemClicked())
				{
					m_MainEditorPanel->SetSelectedComponent(component);
				}

				/*ImGui::TableNextColumn();
				ImGui::TextDisabled("%s", GetIconForComponent(component));*/
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("%s %s", EUI::GetIconForComponent(component), component->GetTypeName());
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
					m_MainEditorPanel->SetSelectedComponent(component);
				}

				/*ImGui::TableNextColumn();
				ImGui::TextDisabled("%s", GetIconForComponent(component));*/
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("%s %s", EUI::GetIconForComponent(component), component->GetTypeName());
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
				m_MainEditorPanel->ClearObjectSelection();
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
						"Spot light",
						"-",
						"Box",
						"Sphere"
					};

					for (int i = 0; i < std::size(types); ++i)
					{
						const char* name = types[i];

						if(strlen(name) == 1)
						{
							ImGui::Separator();
							continue;
						}

						if (ImGui::Selectable(name))
						{
							switch (i)
							{
								case 0:
									m_MainEditorPanel->SetSelectedActor(
										AppContext::GetScene()->SpawnActor<Actor>("EmptyActor", {0, 0, 0}) );
									break;
								case 1:
									m_MainEditorPanel->SetSelectedActor(
										AppContext::GetScene()->SpawnActor<DirectionalLight>("DirectionalLight", {0, 0, 0}) );
									break;
								case 2:
									m_MainEditorPanel->SetSelectedActor(
										AppContext::GetScene()->SpawnActor<PointLight>("PointLight", {0, 0, 0}) );
									break;
								case 3:
									m_MainEditorPanel->SetSelectedActor(
										AppContext::GetScene()->SpawnActor<SpotLight>("SpotLight", {0, 0, 0}) );
									break;
								default:
									break;
							}
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

					int flags = m_MainEditorPanel->GetSelectedActor() == actor ? ImGuiTreeNodeFlags_Selected : 0;

					ImGui::AlignTextToFramePadding();
					bool open = ImGui::TreeNodeEx(actor->GetName().c_str(), ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnArrow | flags);

					bool itemClicked = ImGui::IsItemClicked();

					if (itemClicked)
					{
						m_MainEditorPanel->SetSelectedActor(actor);
					}

					if (itemClicked && ImGui::IsMouseDoubleClicked(0) && !m_MainEditorPanel->IsPlayMode())
					{
						LookAtObject(actor);
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
	}
}