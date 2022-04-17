#pragma once

#include "ConsoleWindow.hpp"
#include "GameViewportWindow.hpp"
#include "SceneHierarchyWindow.hpp"
#include "ResourceWindow.hpp"
#include "MaterialWindow.hpp"
#include "PropertiesWindow.hpp"

#include "Aurora/Core/Vector.hpp"
#include "Aurora/Graphics/Base/Texture.hpp"

namespace Aurora
{
	class Actor;
	class SceneComponent;
	struct RenderViewPort;

	class MainEditorPanel
	{
	private:
		std::shared_ptr<ConsoleWindow> m_ConsoleWindow;
		std::shared_ptr<GameViewportWindow> m_GameViewportWindow;
		SceneHierarchyWindow m_SceneHierarchyWindow;
		ResourceWindow m_ResourceWindow;
		MaterialWindow m_MaterialWindow;
		PropertiesWindow m_PropertiesWindow;

		Actor* m_SelectedActor;
		SceneComponent* m_SelectedComponent;

		bool m_IsPlayMode;
		bool m_MaterialInstancesWindowOpened;
	public:
		MainEditorPanel();
		~MainEditorPanel();

		void Update(double delta);

		void BeginDockSpace();
		void DrawMainMenu();

		[[nodiscard]] bool IsPlayMode() const { return m_IsPlayMode; }
		void SetPlayMode(bool playMode) { m_IsPlayMode = playMode; }

		inline Actor* GetSelectedActor() { return m_SelectedActor; }

		inline void SetSelectedActor(Actor* actor)
		{
			m_SelectedActor = actor;
			m_SelectedComponent = nullptr;

			if (actor)
				m_PropertiesWindow.OpenWindow(true);
		}

		inline SceneComponent* GetSelectedComponent() { return m_SelectedComponent; }

		inline void SetSelectedComponent(SceneComponent* component)
		{
			m_SelectedComponent = component;
			m_SelectedActor = nullptr;

			if (component)
				m_PropertiesWindow.OpenWindow(true);
		}

		inline void ClearObjectSelection()
		{
			m_SelectedActor = nullptr;
			m_SelectedComponent = nullptr;
		}

		[[nodiscard]] inline bool IsAnySceneObjectSelected() const { return m_SelectedActor || m_SelectedComponent; }
		[[nodiscard]] bool GetSelectedObjectTransform(Matrix4& matrix) const;
		[[nodiscard]] bool SetSelectedObjectTransform(const Matrix4& matrix);

		inline GameViewportWindow* GetGameViewPortWindow() { return m_GameViewportWindow.get(); }
		inline MaterialWindow* GetMaterialWindow() { return &m_MaterialWindow; }
		inline ResourceWindow* GetResourceWindow() { return &m_ResourceWindow; }
	};
}
