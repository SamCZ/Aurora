#include "AuroraEngine.hpp"

#include <queue>
#include <sstream>
#include <memory>
#include <thread>

#include <EngineFactoryVk.h>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#ifdef __unix__
#include <X11/Xlib-xcb.h>
    #define GLFW_EXPOSE_NATIVE_X11
#endif
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <imgui.h>
#include <ImGuiImplDiligent.hpp>
#include <ImGuiUtils.hpp>

#include "App/ImGuiImplGLFW.hpp"
#include "App/Input/GLFW/Manager.hpp"
#include "Graphics/GraphicUtilities.hpp"

#include "Profiler/Profiler.hpp"

namespace Aurora
{
	bool AuroraEngine::IsInitialized = false;
	bool AuroraEngine::IsRunning = false;

	RefCntAutoPtr<IRenderDevice> AuroraEngine::RenderDevice(nullptr);
	RefCntAutoPtr<IDeviceContext> AuroraEngine::ImmediateContext(nullptr);
	AssetManager_ptr AuroraEngine::AssetManager = nullptr;
#ifdef FMOD_SUPPORTED
	SoundSystem_ptr AuroraEngine::SoundSystem = nullptr;
#endif
	UIRenderer_ptr AuroraEngine::UI_Renderer = nullptr;

	std::unique_ptr<Diligent::ImGuiImplDiligent> AuroraEngine::ImGuiImpl(nullptr);

	std::vector<WindowGameContext_ptr> AuroraEngine::GameContexts;
	std::map<std::thread::id, WindowGameContext_ptr> AuroraEngine::GameContextsByThread;

	static IEngineFactoryVk* EngineFactory = nullptr;

	void AuroraEngine::joystick_callback(int jid, int event)
	{
		for (auto& context : AuroraEngine::GameContexts) {
			Input::Manager_ptr manager = std::static_pointer_cast<Input::Manager>(context->GetInputManager());
			manager->OnJoystickConnectChange(jid, event == GLFW_CONNECTED);
		}
	}

	void CustomDiligentLogger(enum DEBUG_MESSAGE_SEVERITY dSeverity,
							  const Char*                 Message,
							  const Char*                 Function,
							  const Char*                 File,
							  int                         Line)
	{
		if(std::strstr(Message, "VulkanMemoryManager")) return;

		Logger::Severity severity;
		switch (dSeverity) {
			case DEBUG_MESSAGE_SEVERITY_INFO:
				severity = Logger::Severity::Info;
				break;
			case DEBUG_MESSAGE_SEVERITY_WARNING:
				severity = Logger::Severity::Warning;
				break;
			case DEBUG_MESSAGE_SEVERITY_ERROR:
				severity = Logger::Severity::Error;
				break;
			case DEBUG_MESSAGE_SEVERITY_FATAL_ERROR:
				severity = Logger::Severity::FatalError;
				break;
		}

		Logger::Log(severity, Function, File, Line, Message);
	}

	void AuroraEngine::Init()
	{
		glfwInit();

		glfwSetJoystickCallback(joystick_callback);

#    if EXPLICITLY_LOAD_ENGINE_VK_DLL
		// Load the dll and import GetEngineFactoryVk() function
                auto GetEngineFactoryVk = LoadGraphicsEngineVk();
#    endif
		EngineVkCreateInfo EngineCI;
#    ifdef AU_VK_LAYERS
		EngineCI.EnableValidation = true;
#    endif

		//EngineCI.EnableValidation = true;

		EngineCI.Features.ComputeShaders  = DEVICE_FEATURE_STATE_ENABLED;
		EngineCI.Features.DepthClamp      = DEVICE_FEATURE_STATE_OPTIONAL;
		EngineCI.Features.WireframeFill   = DEVICE_FEATURE_STATE_OPTIONAL;
		EngineCI.Features.GeometryShaders = DEVICE_FEATURE_STATE_OPTIONAL;
		EngineCI.Features.SeparablePrograms = DEVICE_FEATURE_STATE_ENABLED;

		EngineCI.DynamicHeapSize = 1048576 * 512;

		EngineCI.DebugMessageCallback = CustomDiligentLogger;

		EngineFactory = GetEngineFactoryVk();
		EngineFactory->CreateDeviceAndContextsVk(EngineCI, &RenderDevice, &ImmediateContext);

		AuroraEngine::AssetManager = std::make_shared<Aurora::AssetManager>();
#ifdef FMOD_SUPPORTED
		AuroraEngine::SoundSystem = std::make_shared<Sound::SoundSystem>();
#endif

		IsInitialized = true;
	}

	int AuroraEngine::Run()
	{
		if(IsRunning) {
			return 0;
		}

		IsRunning = true;

		bool anyWindowRunning;
		std::queue<WindowGameContext_ptr> contextsToRemove;

		const float ClearColor[] = {0.0f, 0.0f, 0.0f, 1.0f};

		auto PrevTime = glfwGetTime();
		auto lastFpsTime = PrevTime;
		int frameCount = 0;

		do {
			Profiler::RestartProfiler();

			auto CurrTime    = glfwGetTime();
			auto ElapsedTime = CurrTime - PrevTime;
			PrevTime         = CurrTime;

			Profiler::Begin("FrameTimeCalculation");
			frameCount++;
			if(CurrTime - lastFpsTime >= 1.0) {
				for (auto& context : GameContexts) {
					std::stringstream fpsCounterSS;
					fpsCounterSS << context->GetWindow()->GetOriginalTitle() << " - " << (ElapsedTime * 1000.0);
					fpsCounterSS << " ms (" << frameCount << " fps)";

					context->GetWindow()->SetTitle(fpsCounterSS.str());
				}

				frameCount = 0;
				lastFpsTime += 1.0;
			}
			Profiler::End("FrameTimeCalculation");

			anyWindowRunning = false;

			//window->GetInputManager()

			for (auto& context : GameContexts) {
				const std::shared_ptr<Window>& window = context->GetWindow();

				if(window->IsShouldClose()) {
					continue;
				}

				std::dynamic_pointer_cast<Input::Manager>(window->GetInputManager())->PrePollEvents();
			}

			glfwPollEvents();
#ifdef FMOD_SUPPORTED
			SoundSystem->Update();
#endif
			for (auto& context : GameContexts) {
				const std::shared_ptr<Window>& window = context->GetWindow();

				if(window->IsShouldClose()) {
					window->Destroy();
					contextsToRemove.push(context);
					ImGuiImpl.reset();
					continue;
				} else {
					anyWindowRunning = true;
				}

				auto& swapChain = window->GetSwapChain();
				const SwapChainDesc& swapChainDesc = swapChain->GetDesc();

				//window->GetInputManager()->Update();
				std::dynamic_pointer_cast<Input::Manager>(window->GetInputManager())->Update(ElapsedTime);

				ImGuiImpl->NewFrame(swapChainDesc.Width, swapChainDesc.Height, swapChainDesc.PreTransform);

				Profiler::Begin("WindowGameContext::Update");
				context->Update(ElapsedTime, CurrTime);
				Profiler::End("WindowGameContext::Update");

				if(!window->IsIconified()) {
					Profiler::Begin("Render");
					ITextureView* pRTV = swapChain->GetCurrentBackBufferRTV();
					ITextureView* pDSV = swapChain->GetDepthBufferDSV();
					ImmediateContext->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

					Profiler::Begin("Clear back buffer targets");
					AuroraEngine::ImmediateContext->ClearRenderTarget(pRTV, ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
					AuroraEngine::ImmediateContext->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
					Profiler::End("Clear back buffer targets");

					Profiler::Begin("WindowGameContext::Render");
					context->Render();
					Profiler::End("WindowGameContext::Render");

					ImmediateContext->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
					ImGuiImpl->Render(ImmediateContext);

					Profiler::Begin("SwapChain()->Present");
					window->GetSwapChain()->Present(window->IsVsyncEnabled() ? 1 : 0);
					Profiler::End("SwapChain()->Present");
					Profiler::End("Render");
				} else {
					ImGuiImpl->EndFrame();
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
			}

			while(!contextsToRemove.empty()) {
				auto context = contextsToRemove.front();
				GameContextsByThread.erase(context->m_ThreadId);
				GameContexts.erase(std::find(GameContexts.begin(), GameContexts.end(), context));
				contextsToRemove.pop(); // This will call destructor on WindowGameContext
			}

			Profiler::Finalize();
		} while (IsRunning && anyWindowRunning);

		Profiler::RestartProfiler();

		GraphicUtilities::Destroy();

		glfwTerminate();

		return 0;
	}

	std::shared_ptr<WindowGameContext> AuroraEngine::AddWindow(const WindowGameContext_ptr& gameContext,
															   const Window_ptr& window,
															   const WindowDefinition &windowDef, bool showImmediately)
	{
		auto thisThreadId = std::this_thread::get_id();

		if(GameContextsByThread.contains(thisThreadId)) {
			AU_LOG_FATAL("Windows already exists in this thread !");
			return nullptr;
		}

		gameContext->m_ThreadId = thisThreadId;

		window->Initialize(windowDef, nullptr);

		RefCntAutoPtr<ISwapChain> swapChain;

		SwapChainDesc swapChainDesc = {};
		swapChainDesc.ColorBufferFormat = TEX_FORMAT_BGRA8_UNORM;

		if(!CreateSwapChain(window, swapChainDesc, swapChain)) {
			window->Destroy();
			return nullptr;
		}

		window->SetSwapChain(swapChain);

		if(showImmediately) window->Show();

		GameContexts.push_back(gameContext);

		GameContextsByThread[thisThreadId] = gameContext;

		if(ImGuiImpl == nullptr) {
			ImGuiImpl = std::make_unique<ImGuiImplGLFW>(window->GetWindowHandle(), RenderDevice, swapChainDesc.ColorBufferFormat, swapChainDesc.DepthBufferFormat);
		}

		gameContext->Init();

		return gameContext;
	}

	bool AuroraEngine::CreateSwapChain(const Window_ptr& window, const SwapChainDesc& desc, RefCntAutoPtr<ISwapChain>& swapChain)
	{
#ifdef _WIN32
		HWND hWnd = glfwGetWin32Window(window->GetWindowHandle());

		if (!swapChain && hWnd != nullptr)
		{
			Win32NativeWindow Window{hWnd};
			EngineFactory->CreateSwapChainVk(RenderDevice, ImmediateContext, desc, Window, &swapChain);
			return true;
		}
#endif
#ifdef __unix__
		if (!swapChain)
        {
            Uint32 x11window = glfwGetX11Window(window->GetWindowHandle());
            Display* x11display = glfwGetX11Display();
            xcb_connection_t* connection = XGetXCBConnection(x11display);

            LinuxNativeWindow Window{x11window, x11display, connection};
            EngineFactory->CreateSwapChainVk(RenderDevice, ImmediateContext, desc, Window, &swapChain);
            return true;
        }
#endif

		return false;
	}

	const std::vector<WindowGameContext_ptr> &AuroraEngine::GetGameContexts()
	{
		return GameContexts;
	}

	void AuroraEngine::Play2DSound(const String &path, float volume, float pitch)
	{
#ifdef FMOD_SUPPORTED
	SoundSystem->PlaySoundOneShot(path, volume, pitch);
#endif
	}

	WindowGameContext_ptr AuroraEngine::GetCurrentThreadContext()
	{
		auto thisThreadId = std::this_thread::get_id();

		auto it = GameContextsByThread.find(thisThreadId);

		if(it != GameContextsByThread.end()) {
			return it->second;
		}

		return nullptr;
	}
}