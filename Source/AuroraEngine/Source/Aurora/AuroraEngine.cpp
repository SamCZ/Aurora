#include "AuroraEngine.hpp"

#include <queue>

#include <EngineFactoryVk.h>
#include <Timer.hpp>

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "App/Input/InputManager.hpp"

namespace Aurora
{
    bool AuroraEngine::IsInitialized = false;
    bool AuroraEngine::IsRunning = false;
    RefCntAutoPtr<IRenderDevice> AuroraEngine::RenderDevice(nullptr);
    RefCntAutoPtr<IDeviceContext> AuroraEngine::ImmediateContext(nullptr);
    SharedPtr<FAssetManager> AuroraEngine::AssetManager = nullptr;
    List<SharedPtr<FWindowGameContext>> AuroraEngine::GameContexts;

    static IEngineFactoryVk* EngineFactory = nullptr;

    void AuroraEngine::Init()
    {
#    if EXPLICITLY_LOAD_ENGINE_VK_DLL
        // Load the dll and import GetEngineFactoryVk() function
                auto GetEngineFactoryVk = LoadGraphicsEngineVk();
#    endif
        EngineVkCreateInfo EngineCI;
#    ifdef DILIGENT_DEBUG
        EngineCI.EnableValidation = true;
#    endif

        EngineCI.Features.ComputeShaders  = DEVICE_FEATURE_STATE_ENABLED;
        EngineCI.Features.DepthClamp      = DEVICE_FEATURE_STATE_OPTIONAL;
        EngineCI.Features.WireframeFill   = DEVICE_FEATURE_STATE_OPTIONAL;
        EngineCI.Features.GeometryShaders = DEVICE_FEATURE_STATE_OPTIONAL;

        EngineFactory = GetEngineFactoryVk();
        EngineFactory->CreateDeviceAndContextsVk(EngineCI, &RenderDevice, &ImmediateContext);

        AuroraEngine::AssetManager = New(FAssetManager);
        IsInitialized = true;
    }

    int AuroraEngine::Run()
    {
        if(IsRunning) {
            return 0;
        }

        IsRunning = true;

        bool anyWindowRunning;
        std::queue<SharedPtr<FWindowGameContext>> contextsToRemove;

        Timer timer;
        auto PrevTime = timer.GetElapsedTime();
        double filteredFrameTime = 0.0;

        do {
            auto CurrTime    = timer.GetElapsedTime();
            auto ElapsedTime = CurrTime - PrevTime;
            PrevTime         = CurrTime;

            anyWindowRunning = false;

            glfwPollEvents();

            for (auto& context : GameContexts) {
                const FWindowPtr& window = context->GetWindow();

                if(window->IsShouldClose()) {
                    window->Destroy();
                    contextsToRemove.push(context);
                    continue;
                } else {
                    anyWindowRunning = true;
                }

                window->GetInputManager()->Update();

                context->Update(ElapsedTime, CurrTime);
                context->Render();

                window->GetSwapChain()->Present(window->IsVsyncEnabled() ? 1 : 0);
            }

            while(!contextsToRemove.empty()) {
                List_Remove(GameContexts, contextsToRemove.front());
                contextsToRemove.pop();
            }

            double filterScale = 0.2;
            filteredFrameTime  = filteredFrameTime * (1.0 - filterScale) + filterScale * ElapsedTime;

            for (auto& context : GameContexts) {
                std::stringstream fpsCounterSS;
                fpsCounterSS << context->GetWindow()->GetOriginalTitle() << " - " << std::fixed << std::setprecision(1) << filteredFrameTime * 1000;
                fpsCounterSS << " ms (" << 1.0 / filteredFrameTime << " fps)";

                context->GetWindow()->SetTitle(fpsCounterSS.str());
            }
        } while(IsRunning && anyWindowRunning);

        glfwTerminate();
        return 0;
    }

    SharedPtr<FWindowGameContext>
    AuroraEngine::AddWindow(const SharedPtr<FWindowGameContext>& gameContext, const FWindowPtr& window,
                            const FWindowDefinition& windowDef, bool showImmediately)
    {
        window->Initialize(windowDef, nullptr);

        RefCntAutoPtr<ISwapChain> swapChain;

        if(!CreateSwapChain(window, {}, swapChain)) {
            window->Destroy();
            return nullptr;
        }

        window->SetSwapChain(swapChain);

        if(showImmediately) window->Show();

        GameContexts.push_back(gameContext);

        gameContext->Init();

        return gameContext;
    }

    bool AuroraEngine::CreateSwapChain(const FWindowPtr& window, const SwapChainDesc& desc, RefCntAutoPtr<ISwapChain>& swapChain)
    {
        HWND hWnd = glfwGetWin32Window(window->GetWindowHandle());

        if (!swapChain && hWnd != nullptr)
        {
            Win32NativeWindow Window{hWnd};
            EngineFactory->CreateSwapChainVk(RenderDevice, ImmediateContext, desc, Window, &swapChain);
            return true;
        }

        return false;
    }
}