#pragma once

#include "Core/Library.hpp"

#ifdef AU_FMOD_SOUND
	#include "Aurora/Sound/FMOD/FmodSoundSystem.hpp"
	#define AU_HAS_AUDIO 1
#else
	#define AU_HAS_AUDIO 0
#endif

namespace Aurora
{
	class ResourceManager;
	class RenderManager;
	class IRenderDevice;
	class ISystemWindow;
	class RmlUI;
	class VgRender;
	class ViewPortManager;
	class AppContext;

	namespace Input
	{
		class IManager;
	}

	class AuroraContext
	{
		friend class AuroraEngine;
	private:
		ResourceManager* m_ResourceManager = nullptr;
		RenderManager* m_RenderManager = nullptr;
		IRenderDevice* m_RenderDevice = nullptr;
		ISystemWindow* m_Window = nullptr;
		Input::IManager* m_InputManager = nullptr;
		RmlUI* m_RmlUI = nullptr;
		VgRender* m_VgRender = nullptr;
		ViewPortManager* m_ViewPortManager = nullptr;
		AppContext* m_AppContext = nullptr;

#if AU_HAS_AUDIO
		FMOD::SoundSystem* m_SoundSystem = nullptr;
#endif

		bool m_IsRunning = false;
	public:
		[[nodiscard]] inline ResourceManager* GetResourceManager() const { return m_ResourceManager; }
		[[nodiscard]] inline RenderManager* GetRenderManager() const { return m_RenderManager; }
		[[nodiscard]] inline IRenderDevice* GetRenderDevice() const { return m_RenderDevice; }
		[[nodiscard]] inline ISystemWindow* GetWindow() const { return m_Window; }
		[[nodiscard]] inline Input::IManager* GetInputManager() const { return m_InputManager; }
		[[nodiscard]] inline RmlUI* GetRmlUI() const { return m_RmlUI; }
		[[nodiscard]] inline VgRender* GetVgRender() const { return m_VgRender; }
		[[nodiscard]] inline ViewPortManager* GetViewPortManager() const { return m_ViewPortManager; }
		[[nodiscard]] inline AppContext* GetAppContext() const { return m_AppContext; }
#if AU_HAS_AUDIO
	[[nodiscard]] inline FMOD::SoundSystem* GetSoundSystem() const { return m_SoundSystem; }
#endif
		inline void Shutdown() { m_IsRunning = false; }
	};

	extern AU_API AuroraContext* GEngine;
}