#include "AuroraEngine.hpp"

#include <queue>
#include <sstream>
#include <memory>
#include <thread>

#include "Graphics/OpenGL/GLRenderDevice.hpp"
#include "Graphics/OpenGL/GLSwapChain.hpp"

#if GLFW_ENABLED
	#include "App/Input/GLFW/Manager.hpp"
#endif

#include "Graphics/GraphicUtilities.hpp"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include "Tools/ImGuizmo.h"

#include <RmlUi/Core.h>

#define MAX_PHYSICS_FPS				60.0f
#define MAX_PHYSICS_SUB_STEPS		2
#define MAX_PHYSICS_THREAD_COUNT    1
#define PROJECTILE_INITIAL_SPEED	20.0f

#include <ndNewton.h>

#include <TracyOpenGL.hpp>

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

namespace Aurora
{
	bool AuroraEngine::IsInitialized = false;
	bool AuroraEngine::IsRunning = false;
	bool AuroraEngine::IsSRGBEnabled = true;

	IRenderDevice* AuroraEngine::RenderDevice = nullptr;
	AssetManager_ptr AuroraEngine::AssetManager = nullptr;
#ifdef FMOD_SUPPORTED
	SoundSystem_ptr AuroraEngine::SoundSystem = nullptr;
#endif
	UIRenderer_ptr AuroraEngine::UI_Renderer = nullptr;
	RmlUI_ptr AuroraEngine::RmlUserInterface = nullptr;
	PhysicsWorld_ptr AuroraEngine::Physics = nullptr;

	std::vector<WindowGameContext_ptr> AuroraEngine::GameContexts;
	std::map<std::thread::id, WindowGameContext_ptr> AuroraEngine::GameContextsByThread;

	//PipelineErrorHandler errorHandler;
#if GLFW_ENABLED
	void AuroraEngine::joystick_callback(int jid, int event)
	{
		for (auto& context : AuroraEngine::GameContexts) {
			Input::Manager_ptr manager = std::static_pointer_cast<Input::Manager>(context->GetInputManager());
			manager->OnJoystickConnectChange(jid, event == GLFW_CONNECTED);
		}
	}
#endif

	void AuroraEngine::Init()
	{
#if GLFW_ENABLED
		glfwInit();

		glfwSetJoystickCallback(joystick_callback);
#endif
		AuroraEngine::AssetManager = std::make_shared<Aurora::AssetManager>();
#ifdef FMOD_SUPPORTED
		AuroraEngine::SoundSystem = std::make_shared<Sound::SoundSystem>();
#endif

		Physics = std::make_shared<PhysicsWorld>();

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

		auto PrevTime = glfwGetTime();
		auto lastFpsTime = PrevTime;
		int frameCount = 0;

		double targetFrameRate = 80.0;
		double wait_time = 1.0 / targetFrameRate;

		bool show_demo_window = true;

		do {
			ZoneNamedN(gameLoopZone, "GameLoop", true)

			auto CurrTime    = glfwGetTime();
			auto ElapsedTime = CurrTime - PrevTime;
			PrevTime         = CurrTime;

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

			anyWindowRunning = false;

			//window->GetInputManager()

			for (auto& context : GameContexts) {
				const std::shared_ptr<IWindow>& window = context->GetWindow();

				if(window->IsShouldClose()) {
					continue;
				}
#if GLFW_ENABLED
				std::dynamic_pointer_cast<Input::Manager>(window->GetInputManager())->PrePollEvents();
#endif
			}
#if GLFW_ENABLED
			glfwPollEvents();
#endif
#ifdef FMOD_SUPPORTED
			SoundSystem->Update();
#endif
			for (auto& context : GameContexts) {
				const std::shared_ptr<IWindow>& window = context->GetWindow();

				if(window->IsShouldClose()) {
					window->Destroy();
					contextsToRemove.push(context);
					//ImGuiImpl.reset();
					continue;
				} else {
					anyWindowRunning = true;
				}
#if GLFW_ENABLED
				glfwMakeContextCurrent(((GLFWWindow*)window.get())->GetWindowHandle());
#endif
				auto& swapChain = window->GetSwapChain();
				const SwapChainDesc& swapChainDesc = swapChain->GetDesc();

				//window->GetInputManager()->Update();
#if GLFW_ENABLED
				std::dynamic_pointer_cast<Input::Manager>(window->GetInputManager())->Update(ElapsedTime);
#endif
				//ImGuiImpl->NewFrame(swapChainDesc.Width, swapChainDesc.Height, swapChainDesc.PreTransform);
				// Start the Dear ImGui frame
				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();

				ImGuizmo::BeginFrame();

				{
					if (show_demo_window)
						ImGui::ShowDemoWindow(&show_demo_window);
				}

				{
					ZoneNamedN(contextUpdateZone, "ContextUpdate", true)
					context->Update(ElapsedTime, CurrTime);
				}

				{
					ZoneNamedN(rmlUpdateZone, "RmlUpdate", true)
					RmlUserInterface->Update();
				}

				if(!window->IsIconified()) {
					glViewport(0, 0, window->GetSize().x, window->GetSize().y);

					glClearColor(0, 0, 0, 1);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

					{
						ZoneNamedN(contextRenderZone, "ContextRender", true)
						context->Render();
					}

					glDisable(GL_FRAMEBUFFER_SRGB);
					glViewport(0, 0, window->GetWidth(), window->GetHeight());

					{
						ZoneNamedN(rmlRenderZone, "RmlRender", true)
						RmlUserInterface->Render();
					}

					{
						ZoneNamedN(physicsUpdateZone, "PhysicsUpdate", true)
						Physics->Update(ElapsedTime);
					}

					// This is for syncing threads
					/*if(false)
					{
						Physics->Sync();
					}*/

					{
						ZoneNamedN(imGuiZone, "ImGui", true)
						GPU_DEBUG_SCOPE("ImGui")

						ImGui::Render();
						ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
					}
					glEnable(GL_FRAMEBUFFER_SRGB);

					{
						ZoneNamedN(swapChainZone, "SwapChain", true)
						window->GetSwapChain()->Present(window->IsVsyncEnabled() ? 1 : 0);
					}
				} else {
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
			}

			TracyGpuCollect

			while(!contextsToRemove.empty()) {
				auto context = contextsToRemove.front();
				GameContextsByThread.erase(context->m_ThreadId);
				GameContexts.erase(std::find(GameContexts.begin(), GameContexts.end(), context));
				contextsToRemove.pop(); // This will call destructor on WindowGameContext
			}

			/*double dur = 1000.0 * (wait_time - (glfwGetTime() - CurrTime)) + 0.5;
			if(dur > 0.0) {
				std::this_thread::sleep_for(std::chrono::milliseconds((int64_t)dur));
			}*/
		} while (IsRunning && anyWindowRunning);

		GameUI.reset();

		Physics.reset();
		AuroraEngine::AssetManager.reset(); // This resolves that resources are destroyed before render device is deleted

		delete RenderDevice;
        RenderDevice = nullptr;

		GraphicUtilities::Destroy();
#if GLFW_ENABLED
		glfwTerminate();
#endif
		return 0;
	}

	std::shared_ptr<WindowGameContext> AuroraEngine::AddWindow(const WindowGameContext_ptr& gameContext,
															   const IWindow_ptr& window,
															   const WindowDefinition &windowDef, bool showImmediately)
	{
		auto thisThreadId = std::this_thread::get_id();

		if(GameContextsByThread.contains(thisThreadId)) {
			AU_LOG_FATAL("Windows already exists in this thread !");
			return nullptr;
		}

		gameContext->m_ThreadId = thisThreadId;

		window->Initialize(windowDef, nullptr);

		if(RenderDevice == nullptr) {
			auto oglRender = new GLRenderDevice();
			RenderDevice = oglRender;
			oglRender->Init();
		}

		ISwapChain_ptr swapChain;
		SwapChainDesc swapChainDesc = {};
		//swapChainDesc.ColorBufferFormat = TEX_FORMAT_BGRA8_UNORM;

		if(!CreateSwapChain(window, swapChainDesc, swapChain)) {
			window->Destroy();
			return nullptr;
		}

		window->SetSwapChain(swapChain);

		if(showImmediately) window->Show();

		GameContexts.push_back(gameContext);

		GameContextsByThread[thisThreadId] = gameContext;

		if(RmlUserInterface == nullptr)
		{
			RmlUserInterface = std::make_shared<Aurora::RmlUI>("master");
		}

		/*if(ImGuiImpl == nullptr) {
			ImGuiImpl = std::make_unique<ImGuiImplGLFW>(window->GetWindowHandle(), RenderDevice, swapChainDesc.ColorBufferFormat, swapChainDesc.DepthBufferFormat);
		}*/

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWWindow*>(window.get())->GetWindowHandle(), true);
		ImGui_ImplOpenGL3_Init("#version 330 core");

		gameContext->Init();

		return gameContext;
	}

	bool AuroraEngine::CreateSwapChain(const IWindow_ptr& window, const SwapChainDesc& desc, ISwapChain_ptr& swapChain)
	{
/*#ifdef _WIN32
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
#endif*/

#if GLFW_ENABLED
		swapChain = std::make_shared<GLSwapChain>(((GLFWWindow*)window.get())->GetWindowHandle());
#endif
		return true;
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

	void AuroraEngine::Shutdown()
	{
		IsRunning = false;
	}
}