#include "PropertiesWindow.hpp"

#include "Aurora/Core/Profiler.hpp"
#include "Aurora/Framework/Actor.hpp"
#include "Aurora/Framework/CameraComponent.hpp"
#include "Aurora/Framework/MeshComponent.hpp"
#include "Aurora/Framework/Lights.hpp"
#include "Aurora/Resource/ResourceManager.hpp"

#include "MainEditorPanel.hpp"
#include "MaterialWindow.hpp"
#include "Utils.hpp"

namespace Aurora
{
	void PropertiesWindow::DrawComponentGui(MeshComponent* component)
	{
		if(!ImGui::CollapsingHeader("MeshComponent", ImGuiTreeNodeFlags_DefaultOpen))
			return;

		for (auto& [slotID, slot] : component->GetMaterialSet())
		{
			ImGui::PushID(slotID);
			String name = "#" + std::to_string(slotID) + ": " + slot.MaterialSlotName;

			if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				const matref& mat = slot.Material;

				bool clearClicked = false;

				if (mat)
				{
					if (EUI::Slot(slot.MaterialSlotName, mat->GetMaterialDef()->GetName().c_str(), &clearClicked))
					{
						m_MainPanel->GetMaterialWindow()->Open(mat);
					}

					if (clearClicked)
					{
						slot.Material = nullptr;
						ImGui::PopID();
						continue;
					}
				}
				else
				{
					EUI::Slot(slot.MaterialSlotName, ICON_FA_TIMES_CIRCLE);
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
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}

	void PropertiesWindow::DrawComponentGui(LightComponent* component)
	{
		if(!ImGui::CollapsingHeader("LightComponent", ImGuiTreeNodeFlags_DefaultOpen))
			return;

		ImGui::DragFloat("Intensity", &component->GetIntensity());
		ImGui::ColorEdit3("Color", reinterpret_cast<float*>(&component->GetColor()), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
	}

	void PropertiesWindow::DrawComponentGui(PointLightComponent* component)
	{
		if(!ImGui::CollapsingHeader("PointLightComponent", ImGuiTreeNodeFlags_DefaultOpen))
			return;

		ImGui::DragFloat("Radius", &component->GetRadius());
	}

	void PropertiesWindow::OnGui()
	{
		CPU_DEBUG_SCOPE("PropertiesWindow");

		SceneComponent* root;

		Actor* selectedActor = m_MainPanel->GetSelectedActor();
		if (selectedActor)
		{
			root = selectedActor->GetRootComponent();
			std::string name = selectedActor->GetName();
			if (ImGui::InputTextLabel("Name", name))
			{
				selectedActor->SetName(name);
			}
		}
		else
		{
			root = m_MainPanel->GetSelectedComponent();
		}

		if (root)
		{
			ImGui::Separator();
			bool transformOpened = ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen);
			if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
			{
				ImGui::OpenPopup("ComponentContextPopup");
			}

			if (ImGui::BeginPopup("ComponentContextPopup"))
			{
				if (ImGui::Button("Copy"))
				{
					m_TransformToCopy = root->GetTransform();
					m_IsTransformBeingCopied = true;
					ImGui::CloseCurrentPopup();
				}

				ImGui::BeginDisabled(!m_IsTransformBeingCopied);
				if (ImGui::Button("Paste"))
				{
					m_IsTransformBeingCopied = false;
					root->GetTransform() = m_TransformToCopy;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndDisabled();
				ImGui::EndPopup();
			}

			if (transformOpened)
			{
				ImGui::DrawVec3Control("Location", root->GetTransform().Location);
				ImGui::DrawVec3Control("Rotation", root->GetTransform().Rotation);
				ImGui::DrawVec3Control("Scale", root->GetTransform().Scale);
			}

			if (MeshComponent* meshComponent = MeshComponent::SafeCast(root))
			{
				DrawComponentGui(meshComponent);
			}

			if (LightComponent* component = LightComponent::SafeCast(root))
			{
				DrawComponentGui(component);
			}

			if (PointLightComponent* component = PointLightComponent::SafeCast(root))
			{
				DrawComponentGui(component);
			}
		}
	}
}