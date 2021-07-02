#include "AuroraEngine.hpp"

#include <queue>
#include <sstream>
#include <memory>
#include <thread>

#include "Graphics/OpenGL/GLRenderDevice.hpp"
#include "Graphics/OpenGL/SwapChainGL4.hpp"

#if GLFW_ENABLED

	#ifdef _WIN32
	#define GLFW_EXPOSE_NATIVE_WIN32
	#endif
	#ifdef __unix__
	#include <X11/Xlib-xcb.h>
		#define GLFW_EXPOSE_NATIVE_X11
	#endif

	#include <GLFW/glfw3.h>
	#include <GLFW/glfw3native.h>

	#include "App/Input/GLFW/Manager.hpp"
#endif
#include "Graphics/GraphicUtilities.hpp"

#include "Profiler/Profiler.hpp"

#include "Graphics/OpenGL/GL.hpp"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

namespace Aurora
{
	/*class PipelineErrorHandler : public IErrorCallback
	{
		void SignalError(const char* function, const char* file, int line, const char* errorDesc) override
		{
			Logger::Log(Logger::Severity::Error, function, file, line, errorDesc);
		}
	};*/

	bool AuroraEngine::IsInitialized = false;
	bool AuroraEngine::IsRunning = false;

	IRenderDevice* AuroraEngine::RenderDevice = nullptr;
	AssetManager_ptr AuroraEngine::AssetManager = nullptr;
#ifdef FMOD_SUPPORTED
	SoundSystem_ptr AuroraEngine::SoundSystem = nullptr;
#endif
	UIRenderer_ptr AuroraEngine::UI_Renderer = nullptr;

	//std::unique_ptr<Diligent::ImGuiImplDiligent> AuroraEngine::ImGuiImpl(nullptr);

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

		bool show_demo_window = true;

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

				{
					if (show_demo_window)
						ImGui::ShowDemoWindow(&show_demo_window);
				}

				Profiler::Begin("WindowGameContext::Update");
				context->Update(ElapsedTime, CurrTime);
				Profiler::End("WindowGameContext::Update");

				if(!window->IsIconified()) {
					Profiler::Begin("Render");
					/*ITextureView* pRTV = swapChain->GetCurrentBackBufferRTV();
					ITextureView* pDSV = swapChain->GetDepthBufferDSV();
					ImmediateContext->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

					Profiler::Begin("Clear back buffer targets");
					AuroraEngine::ImmediateContext->ClearRenderTarget(pRTV, ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
					AuroraEngine::ImmediateContext->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
					Profiler::End("Clear back buffer targets");*/

					glViewport(0, 0, window->GetSize().x, window->GetSize().y);

					glClearColor(0, 0, 0, 1);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

					Profiler::Begin("WindowGameContext::Render");
					context->Render();
					Profiler::End("WindowGameContext::Render");

					/*ImmediateContext->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
					ImGuiImpl->Render(ImmediateContext);*/
					glDisable(GL_FRAMEBUFFER_SRGB);
					ImGui::Render();
					glViewport(0, 0, window->GetWidth(), window->GetHeight());
					ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
					glEnable(GL_FRAMEBUFFER_SRGB);

					Profiler::Begin("SwapChain()->Present");
					window->GetSwapChain()->Present(window->IsVsyncEnabled() ? 1 : 0);
					Profiler::End("SwapChain()->Present");
					Profiler::End("Render");
				} else {
					//ImGuiImpl->EndFrame();
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

		AuroraEngine::AssetManager.reset(); // This resolves that resources are destroyed before render device is deleted

		delete RenderDevice;

		Profiler::RestartProfiler();

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

		// TODO: Comple swap chain

#if GLFW_ENABLED
		swapChain = std::make_shared<SwapChainGL4>(((GLFWWindow*)window.get())->GetWindowHandle());
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
}