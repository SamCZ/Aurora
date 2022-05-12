#pragma once

#include <utility>
#include "Aurora/Core/Delegate.hpp"
#include "Aurora/Logger/Logger.hpp"
#include "Aurora/Framework/GameModeBase.hpp"
#include "Aurora/Framework/Scene.hpp"

namespace Aurora
{
	class SceneRenderer;
	class PhysicsWorld;

	class GameContext
	{
		friend class AppContext;
	protected:
		Scene* m_Scene = nullptr;
	public:
		virtual ~GameContext()
		{
			delete m_Scene;
		}

		Scene* GetScene() { return m_Scene; }
		[[nodiscard]] const Scene* GetScene() const { return m_Scene; }
	};

	class AU_API AppContext
	{
	private:
		static GameContext* m_GameContext;
		static GameModeBase* m_GameMode;
		static GameModeBase* m_GameModeToSwitch;
		static bool m_EditorMode;
		static EventEmitter<Scene*> m_SceneChangeEmitter;
	public:
		friend class AuroraEngine;

		virtual ~AppContext()
		{
			delete m_GameContext;
		}
	private:
		inline void InternalUpdate(double delta)
		{
			if(m_GameMode)
				m_GameMode->Tick(delta);

			Update(delta);

			if(m_GameMode)
				m_GameMode->TickUI(delta);
		}
	public:
		virtual void Init() {}
		virtual void Update(double delta) {}
		virtual void Render() {}
		virtual void RenderVg() {}

		virtual SceneRenderer* GetSceneRenderer() { return nullptr; }
		virtual PhysicsWorld* GetPhysicsWorld() { return nullptr; }

		static EventEmitter<Scene*>& GetSceneChangeEmitter() { return m_SceneChangeEmitter; }

		template<class T, typename... Args, typename std::enable_if<std::is_base_of<GameContext, T>::value>::type* = nullptr>
		static T* SetGameContext(Args... args)
		{
			if(m_GameContext)
			{
				AU_LOG_FATAL("Cannot change already initialized game context !");
			}

			return (m_GameContext = new T(std::forward<Args>(args)...));
		}

		template<class T>
		static T* GetGameContext()
		{
			return static_cast<T*>(m_GameContext);
		}

		template<class T, typename... Args, typename std::enable_if<std::is_base_of<GameModeBase, T>::value>::type* = nullptr>
		static T* SwitchGameMode(Args... args)
		{
			return static_cast<T*>(m_GameModeToSwitch = new T(std::forward<Args>(args)...));
		}

		static GameModeBase* SwitchGameModeRaw(GameModeBase* gameModeBase)
		{
			bool hadGameMode = m_GameMode;

			if(m_GameMode)
			{
				m_GameMode->BeginDestroy();
				delete m_GameMode;
			}

			// TODO: Think if this should be the place that resets scene
			Scene* newScene = new Scene();
			m_GameContext->m_Scene = newScene;
			m_SceneChangeEmitter.Invoke(std::forward<Scene*>(newScene));

			m_GameMode = gameModeBase;

			if(hadGameMode)
			{
				m_GameMode->BeginPlay();
			}

			return m_GameMode;
		}

		template<class T, typename... Args, typename std::enable_if<std::is_base_of<GameModeBase, T>::value>::type* = nullptr>
		static T* SwitchGameModeImmediately(Args... args)
		{

			return (T*)SwitchGameModeRaw(new T(std::forward<Args>(args)...));
		}

		static GameModeBase* GetGameModeBase() { return m_GameMode; }

		template<class T, typename... Args, typename std::enable_if<std::is_base_of<GameModeBase, T>::value>::type* = nullptr>
		static T* GetGameMode()
		{
			return static_cast<T*>(m_GameMode);
		}

		static Scene* GetScene() { return m_GameContext->GetScene(); }

		static bool IsEditorMode() { return m_EditorMode; }
	};
}