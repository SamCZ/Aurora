#pragma once

#include "WindowBase.hpp"
#include "Aurora/Core/Delegate.hpp"
#include "Aurora/Framework/Transform.hpp"
#include "Aurora/Framework/ActorComponent.hpp"
#include "Aurora/Tools/robin_hood.h"

namespace Aurora
{
	class MainEditorPanel;
	class MeshComponent;
	class LightComponent;
	class PointLightComponent;
	class DirectionalLightComponent;

	class PropertiesWindow : public EditorWindowBase
	{
	private:
		MainEditorPanel* m_MainPanel;
		Transform m_TransformToCopy;
		bool m_IsTransformBeingCopied;
		robin_hood::unordered_map<TTypeID, MethodDelegate<PropertiesWindow, void, ActorComponent*>> m_ComponentGUIMethods;
	public:
		explicit PropertiesWindow(MainEditorPanel* mainEditorPanel);

		void OnGui() override;

	private:
		template<class ComponentType>
		void AddComponentGuiMethod(typename MethodAction<PropertiesWindow, void, ActorComponent*>::Type method)
		{
			TTypeID typeId = ComponentType::TypeID();

			if (m_ComponentGUIMethods.contains(typeId))
			{
				AU_LOG_WARNING("Method for this component already exists !");
				return;
			}

			m_ComponentGUIMethods[typeId] = MethodDelegate<PropertiesWindow, void, ActorComponent*>(this, method);
		}

		template<class ComponentType>
		void InvokeComponentGui(ComponentType* component)
		{
			TTypeID typeId = component->GetTypeID();

			if (!m_ComponentGUIMethods.contains(typeId))
			{
				//AU_LOG_WARNING("Component draw for ", ComponentType::TypeName(), " is not registered!");
				return;
			}

			m_ComponentGUIMethods[typeId].Invoke(component);
		}

		void DrawMeshComponentGui(ActorComponent* baseComponent);
		void DrawLightComponentBaseGui(LightComponent* component);
		void DrawDirectionalLightComponentGui(ActorComponent* baseComponent);
		void DrawPointLightComponentGui(ActorComponent* baseComponent);
		void DrawSkyLightComponent(ActorComponent* baseComponent);
		void DrawRigidBodyComponent(ActorComponent* baseComponent);
	};
}
