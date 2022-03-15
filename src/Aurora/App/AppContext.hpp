#pragma once

#include <utility>
#include "Aurora/Logger/Logger.hpp"
#include "Aurora/Framework/GameModeBase.hpp"
#include "Aurora/Framework/Scene.hpp"

namespace Aurora
{
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

	class AppContext
	{
	private:
		static GameContext* m_GameContext;
		static GameModeBase* m_GameMode;
	public:
		friend class AuroraEngine;

		virtual ~AppContext()
		{
			delete m_GameMode;
			delete m_GameContext;
		}
	private:
		inline void InternalUpdate(double delta)
		{
			if(m_GameMode)
				m_GameMode->Tick(delta);

			Update(delta);
		}
	public:
		virtual void Init() {}
		virtual void Update(double delta) {}
		virtual void Render() {}
		virtual void RenderVg() {}

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
			if(m_GameMode)
			{
				m_GameMode->BeginDestroy();
				delete m_GameMode;
			}

			// TODO: Think if this should be the place that resets scene
			m_GameContext->m_Scene = new Scene();

			m_GameMode = new T(std::forward<Args>(args)...);
			m_GameMode->BeginPlay();

			return (T*)m_GameMode;
		}

		static Scene* GetScene() { return m_GameContext->GetScene(); }
	};
}