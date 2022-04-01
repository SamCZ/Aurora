#include "PropertiesWindow.hpp"

#include "Aurora/Core/Profiler.hpp"
#include "Aurora/Framework/Actor.hpp"
#include "Aurora/Framework/CameraComponent.hpp"
#include "Aurora/Framework/MeshComponent.hpp"
#include "Aurora/Resource/ResourceManager.hpp"

#include "MainEditorPanel.hpp"
#include "MaterialWindow.hpp"

namespace Aurora
{
	void PropertiesWindow::DrawComponentGui(MeshComponent* component)
	{
		ImGui::Separator();

		for (auto& [slotID, slot] : component->GetMaterialSet())
		{
			String name = "#" + std::to_string(slotID) + ": " + slot.MaterialSlotName;

			if (ImGui::CollapsingHeader(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				const matref& mat = slot.Material;

				if (mat)
				{
					ImGui::Text("Material instance");
					if (ImGui::Button(mat->GetMaterialDef()->GetName().c_str()))
					{
						m_MainPanel->GetMaterialWindow()->Open(mat);
					}
				}
				else
				{
					ImGui::Text("Not material set");
				}

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RESOURCE_PATH"))
					{
						Path filePath = (char*)payload->Data;

						if (ResourceManager::IsFileType(filePath, FT_MATERIAL_DEF))
						{
							slot.Material = GEngine->GetResourceManager()->GetOrLoadMaterialDefinition(filePath);
						}
						else if (ResourceManager::IsFileType(filePath, FT_MATERIAL_INS))
						{
							slot.Material = GEngine->GetResourceManager()->LoadMaterial(filePath);
						}
					}

					ImGui::EndDragDropTarget();
				}
			}
		}
	}

	void PropertiesWindow::OnGui()
	{
		CPU_DEBUG_SCOPE("PropertiesWindow");

		SceneComponent* root;

		Actor* selectedActor = m_MainPanel->GetSelectedActor();
		if(selectedActor)
		{
			root = selectedActor->GetRootComponent();
			std::string name = selectedActor->GetName();
			if(ImGui::InputTextLabel("Name", name))
			{
				selectedActor->SetName(name);
			}
		}
		else
		{
			root = m_MainPanel->GetSelectedComponent();
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

			if (MeshComponent* meshComponent = MeshComponent::SafeCast(root))
			{
				DrawComponentGui(meshComponent);
			}
		}
	}
}