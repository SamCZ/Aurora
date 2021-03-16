#pragma once

#include <memory>
#include <vector>
#include <iostream>

#include <EngineFactory.h>
#include <RenderDevice.h>
#include <DeviceContext.h>
#include <SwapChain.h>
#include <RefCntAutoPtr.hpp>

#include "App/Window.hpp"
#include "App/WindowGameContext.hpp"

#include "Assets/AssetManager.hpp"

namespace Diligent
{
	class ImGuiImplDiligent;
}

namespace Aurora
{
	class AuroraEngine
	{
	private:
		static bool IsInitialized;
		static bool IsRunning;
		static std::vector<WindowGameContext_ptr> GameContexts;

		static std::unique_ptr<Diligent::ImGuiImplDiligent> ImGuiImpl;
	public:
		static RefCntAutoPtr<IRenderDevice> RenderDevice;
		static RefCntAutoPtr<IDeviceContext> ImmediateContext;
		static AssetManager_ptr AssetManager;
	public:
		static void Init();
		static int Run();

		static WindowGameContext_ptr AddWindow(const WindowGameContext_ptr& gameContext, const Window_ptr & window, const WindowDefinition& windowDef, bool showImmediately = true);
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

			Window_ptr window = std::make_shared<Window>();
			return AddWindow(std::make_shared<GameContext>(window), window, windowDefinition, showImmediately);
		}
	private:
		static bool CreateSwapChain(const Window_ptr& window, const SwapChainDesc& desc, RefCntAutoPtr<ISwapChain>& swapChain);
		static void joystick_callback(int jid, int event);
	};
}
