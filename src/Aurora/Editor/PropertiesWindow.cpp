#include "PropertiesWindow.hpp"

#include "Aurora/Core/Profiler.hpp"
#include "Aurora/Framework/Actor.hpp"
#include "Aurora/Framework/CameraComponent.hpp"
#include "Aurora/Framework/MeshComponent.hpp"
#include "Aurora/Framework/StaticMeshComponent.hpp"
#include "Aurora/Framework/SkeletalMeshComponent.hpp"
#include "Aurora/Framework/Lights.hpp"
#include "Aurora/Framework/SkyLight.hpp"
#include "Aurora/Framework/Physics/RigidBodyComponent.hpp"
#include "Aurora/Framework/Physics/ColliderComponent.hpp"
#include "Aurora/Resource/ResourceManager.hpp"

#include "MainEditorPanel.hpp"
#include "MaterialWindow.hpp"
#include "ResourceWindow.hpp"
#include "Utils.hpp"

#define ARSIZE(arr) (sizeof(arr)/sizeof((arr)[0]))

#define ENUM_COMBO(name, current, enum_name) ImGui::Combo(name, current, enum_name##_Strings, ARSIZE(enum_name##_Strings))

namespace Aurora
{
	PropertiesWindow::PropertiesWindow(MainEditorPanel *mainEditorPanel)
		: EditorWindowBase("Properties", true, true), m_MainPanel(mainEditorPanel), m_IsTransformBeingCopied(false)
	{
		AddComponentGuiMethod<StaticMeshComponent>(&PropertiesWindow::DrawMeshComponentGui);
		AddComponentGuiMethod<DirectionalLightComponent>(&PropertiesWindow::DrawDirectionalLightComponentGui);
		AddComponentGuiMethod<PointLightComponent>(&PropertiesWindow::DrawPointLightComponentGui);
		AddComponentGuiMethod<SkyLightComponent>(&PropertiesWindow::DrawSkyLightComponent);
		AddComponentGuiMethod<CameraComponent>(&PropertiesWindow::DrawCameraComponent);
		AddComponentGuiMethod<RigidBodyComponent>(&PropertiesWindow::DrawRigidBodyComponent);
		AddComponentGuiMethod<ColliderComponent>(&PropertiesWindow::DrawColliderComponent);
	}

	void PropertiesWindow::DrawMeshComponentGui(ActorComponent* baseComponent)
	{
		if(!ImGui::CollapsingHeader("MeshComponent", ImGuiTreeNodeFlags_DefaultOpen))
			return;

		MeshComponent* component = MeshComponent::Cast(baseComponent);

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

	void PropertiesWindow::DrawLightComponentBaseGui(LightComponent* component)
	{
		ImGui::DragFloat("Intensity", &component->GetIntensity(), 0.1f);
		ImGui::ColorEdit3("Color", reinterpret_cast<float*>(&component->GetColor()), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
	}

	void PropertiesWindow::DrawDirectionalLightComponentGui(ActorComponent* baseComponent)
	{
		if(!ImGui::CollapsingHeader("DirectionalLightComponent", ImGuiTreeNodeFlags_DefaultOpen))
			return;

		DirectionalLightComponent* component = DirectionalLightComponent::Cast(baseComponent);

		DrawLightComponentBaseGui(component);
	}

	void PropertiesWindow::DrawPointLightComponentGui(ActorComponent* baseComponent)
	{
		if(!ImGui::CollapsingHeader("PointLightComponent", ImGuiTreeNodeFlags_DefaultOpen))
			return;

		PointLightComponent* component = PointLightComponent::Cast(baseComponent);

		DrawLightComponentBaseGui(component);

		ImGui::DragFloat("Radius", &component->GetRadius(), 0.1f);
	}

	void PropertiesWindow::DrawSkyLightComponent(ActorComponent* baseComponent)
	{
		if(!ImGui::CollapsingHeader("SkyLightComponent", ImGuiTreeNodeFlags_DefaultOpen))
			return;

		SkyLightComponent* component = SkyLightComponent::Cast(baseComponent);

		SkyLightMode mode = component->GetMode();
		int currentModeInt = (int) mode;

		if (ENUM_COMBO("Mode", &currentModeInt, SkyLightMode))
		{
			component->SetMode((SkyLightMode)currentModeInt);
		}

		Vector3 ambientColor = component->GetAmbientColor();
		if (ImGui::ColorEdit3("Ambient Color", glm::value_ptr(ambientColor), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR))
		{
			component->SetAmbientColor(ambientColor);
		}

		float ambientIntensity = component->GetAmbientIntensity();
		if (ImGui::DragFloat("Ambient Intensity", &ambientIntensity, 0.01f))
		{
			component->SetAmbientIntensity(ambientIntensity);
		}

		if (mode == SkyLightMode::Custom)
		{
			ImGui::Indent();
			DrawMeshComponentGui(component);
			return;
		}

		if (mode == SkyLightMode::Color)
		{

		}

		if (mode == SkyLightMode::SkyBox)
		{

		}
	}

	void PropertiesWindow::DrawCameraComponent(ActorComponent* baseComponent)
	{
		if(!ImGui::CollapsingHeader("CameraComponent", ImGuiTreeNodeFlags_DefaultOpen))
			return;

		CameraComponent* component = CameraComponent::Cast(baseComponent);
	}

	void PropertiesWindow::DrawRigidBodyComponent(ActorComponent* baseComponent)
	{
		if(!ImGui::CollapsingHeader("RigidBodyComponent", ImGuiTreeNodeFlags_DefaultOpen))
			return;

		RigidBodyComponent* component = RigidBodyComponent::Cast(baseComponent);


	}

	void PropertiesWindow::DrawColliderComponent(ActorComponent* baseComponent)
	{
		if(!ImGui::CollapsingHeader("ColliderComponent", ImGuiTreeNodeFlags_DefaultOpen))
			return;

		ColliderComponent* component = ColliderComponent::Cast(baseComponent);


	}

	void PropertiesWindow::OnGui()
	{
		CPU_DEBUG_SCOPE("PropertiesWindow");

		ActorComponent* root;

		Actor* selectedActor = m_MainPanel->GetSelectedActor();
		if (selectedActor)
		{
			root = selectedActor->GetRootComponent();
			std::string name = selectedActor->GetName();
			ImGui::Indent();
			if (ImGui::InputTextLabel("Name", name))
			{
				selectedActor->SetName(name);
			}
			ImGui::Unindent();
		}
		else
		{
			root = m_MainPanel->GetSelectedComponent();
		}

		if (root)
		{
			ImGui::Separator();

			if (SceneComponent* sceneComponent = SceneComponent::SafeCast(root))
			{
				bool transformOpened = ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen);
				if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
				{
					ImGui::OpenPopup("ComponentContextPopup");
				}

				if (ImGui::BeginPopup("ComponentContextPopup"))
				{
					if (ImGui::Button("Copy"))
					{
						m_TransformToCopy = sceneComponent->GetTransform();
						m_IsTransformBeingCopied = true;
						ImGui::CloseCurrentPopup();
					}

					ImGui::BeginDisabled(!m_IsTransformBeingCopied);
					if (ImGui::Button("Paste"))
					{
						m_IsTransformBeingCopied = false;
						sceneComponent->GetTransform() = m_TransformToCopy;
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndDisabled();
					ImGui::EndPopup();
				}

				if (transformOpened)
				{
					ImGui::Indent();

					Vector3 location = sceneComponent->GetTransform().GetLocation();
					if (ImGui::DrawVec3Control("Location", location))
					{
						sceneComponent->GetTransform().SetLocation(location);
					}

					Vector3 eulerAngles = sceneComponent->GetTransform().GetRotation();
					if (ImGui::DrawVec3Control("Rotation", eulerAngles))
					{
						sceneComponent->GetTransform().SetRotation(eulerAngles);
					}

					Vector3 scale = sceneComponent->GetTransform().GetScale();
					if (ImGui::DrawVec3Control("Scale", scale))
					{
						sceneComponent->GetTransform().SetScale(scale);
					}

					ImGui::Unindent();
				}
			}

			InvokeComponentGui(std::forward<ActorComponent*>(root));

			if (SceneComponent* sceneComponent = SceneComponent::SafeCast(root))
			{
				for (ActorComponent* childActor : sceneComponent->GetComponents())
				{
					InvokeComponentGui(std::forward<ActorComponent*>(childActor));
				}
			}
		}
	}
}