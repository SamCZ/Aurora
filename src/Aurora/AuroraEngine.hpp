#pragma once

#include <memory>
#include <vector>
#include <iostream>
#include <thread>

#include "App/IWindow.hpp"
#include "App/WindowGameContext.hpp"

#include "Assets/AssetManager.hpp"
#include "Sound/SoundSystem.hpp"

#include "Graphics/Base/IRenderDevice.hpp"
#include "Graphics/UI/UIRenderer.hpp"

#if GLFW_ENABLED
#include "App/GLFWWindow.hpp"
#endif

namespace Aurora
{
#ifdef FMOD_SUPPORTED
	using namespace Sound;
#endif

	class AuroraEngine
	{
	private:
		static bool IsInitialized;
		static bool IsRunning;
		static std::vector<WindowGameContext_ptr> GameContexts;
		static std::map<std::thread::id, WindowGameContext_ptr> GameContextsByThread;
	public:
		static IRenderDevice* RenderDevice;
		static AssetManager_ptr AssetManager;
#ifdef FMOD_SUPPORTED
		static SoundSystem_ptr SoundSystem;
#endif
		static UIRenderer_ptr UI_Renderer;
	public:
		static void Init();
		static int Run();

		static WindowGameContext_ptr AddWindow(const WindowGameContext_ptr& gameContext, const IWindow_ptr & window, const WindowDefinition& windowDef, bool showImmediately = true);
		static const std::vector<WindowGameContext_ptr>& GetGameContexts();
		static WindowGameContext_ptr GetCurrentThreadContext();
	public:
		template<class GameContext>
		static std::shared_ptr<WindowGameContext> AddWindow(int width, int height, const String& title, bool showImmediately = true)
		{
			if(!IsInitialized) {
				std::cerr << "Cannot add window(" << title << ") ! Engine not initialized." << std::endl;
				return nullptr;
			}

			WindowDefinition windowDefinition = {};
			windowDefinition.Width = width;
			windowDefinition.Height = height;
			windowDefinition.HasOSWindowBorder = true;
			windowDefinition.Title = title;
			windowDefinition.Maximized = true;
#if GLFW_ENABLED
			IWindow_ptr window = std::make_shared<GLFWWindow>();
#else
			IWindow_ptr window = nullptr;
#endif
			return AddWindow(std::make_shared<GameContext>(window), window, windowDefinition, showImmediately);
		}
	private:
		static bool CreateSwapChain(const IWindow_ptr& window, const SwapChainDesc& desc, ISwapChain_ptr& swapChain);
		static void joystick_callback(int jid, int event);
	public:
		static void Play2DSound(const String& path, float volume = 1.0f, float pitch = 1.0f);
	};

#define ASM AuroraEngine::AssetManager
}
