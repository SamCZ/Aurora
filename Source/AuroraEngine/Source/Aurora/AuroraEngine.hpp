#pragma once

#include <EngineFactory.h>
#include <RenderDevice.h>
#include <DeviceContext.h>
#include <SwapChain.h>
#include <RefCntAutoPtr.hpp>

#include <Aurora/Core/Container.hpp>

#include "App/WindowGameContext.hpp"
#include "App/Window.hpp"

using namespace Diligent;

using namespace Aurora::App;

namespace Aurora
{
    class AuroraEngine
    {
    private:
        static bool IsInitialized;
        static bool IsRunning;
    public:
        static RefCntAutoPtr<IRenderDevice> RenderDevice;
        static RefCntAutoPtr<IDeviceContext> ImmediateContext;
    private:
        static List<SharedPtr<FWindowGameContext>> GameContexts;
    public:
        template<class GameContext>
        static SharedPtr<FWindowGameContext> AddWindow(int width, int height, const String& title, bool showImmediately = true)
        {
            if(!IsInitialized) {
                std::cerr << "Cannot add window(" << title << ") ! Engine not initialized." << std::endl;
                return nullptr;
            }

            FWindowDefinition windowDefinition = {};
            windowDefinition.Width = width;
            windowDefinition.Height = height;
            windowDefinition.HasOSWindowBorder = true;
            windowDefinition.Title = title;

            FWindowPtr window = New(FWindow);
            return AddWindow(MakeShared<GameContext>(window), window, windowDefinition, showImmediately);
        }

        static void Init();
        static int Run();

        static SharedPtr<FWindowGameContext> AddWindow(const SharedPtr<FWindowGameContext>& gameContext, const FWindowPtr& window, const FWindowDefinition& windowDef, bool showImmediately = true);

    private:
        static bool CreateSwapChain(const FWindowPtr& window, const SwapChainDesc& desc, RefCntAutoPtr<ISwapChain>& swapChain);
    };
}