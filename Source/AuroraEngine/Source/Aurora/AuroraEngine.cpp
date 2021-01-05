#include "AuroraEngine.hpp"
#include <EngineFactoryVk.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <queue>

#include "App/Input/InputManager.hpp"

namespace Aurora
{
    bool AuroraEngine::IsInitialized = false;
    bool AuroraEngine::IsRunning = false;
    RefCntAutoPtr<IRenderDevice> AuroraEngine::RenderDevice(nullptr);
    RefCntAutoPtr<IDeviceContext> AuroraEngine::ImmediateContext(nullptr);
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

        EngineFactory = GetEngineFactoryVk();
        EngineFactory->CreateDeviceAndContextsVk(EngineCI, &RenderDevice, &ImmediateContext);

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

        do {
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

                context->Update(0, 0);
                context->Render();

                window->GetSwapChain()->Present(1);
            }

            while(!contextsToRemove.empty()) {
                List_Remove(GameContexts, contextsToRemove.front());
                contextsToRemove.pop();
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