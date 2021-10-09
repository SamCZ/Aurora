#include "Aurora.hpp"

#include <GLFW/glfw3.h>

#include "Aurora/Logger/file_sink.hpp"
#include "Aurora/Logger/std_sink.hpp"

#include "Aurora/Core/assert.hpp"
#include "Aurora/Core/Profiler.hpp"

#include "Aurora/App/GLFWWindow.hpp"
#include "Aurora/App/Input/GLFW/Manager.hpp"

#include "Aurora/Graphics/OpenGL/GLSwapChain.hpp"
#include "Aurora/Graphics/OpenGL/GLRenderDevice.hpp"
#include "Aurora/Graphics/RenderManager.hpp"
#include "Aurora/Resource/ResourceManager.hpp"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "Aurora/Graphics/NanoVG/nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "Aurora/Graphics/NanoVG/nanovg_gl.h"

namespace Aurora
{
	static AuroraContext* g_Context = nullptr;

	AuroraContext* GetEngine()
	{
		return g_Context;
	}

	AuroraEngine::AuroraEngine() :
		m_Window(),
		m_SwapChain(nullptr),
		m_RenderDevice(nullptr),
		m_RenderManager(nullptr),
		m_ResourceManager(nullptr),
		m_InputManager(nullptr),
		m_AppContext(nullptr)
	{

	}

	struct NVGcontext* vg;

	AuroraEngine::~AuroraEngine()
	{
		nvgDeleteGL3(vg);

		delete m_AppContext;
		delete g_Context;
		delete m_ResourceManager;
		delete m_RenderManager;
		delete m_RenderDevice;
		delete m_SwapChain;
		delete m_Window;
		glfwTerminate();
	}

	void AuroraEngine::Init(AppContext* appContext, WindowDefinition& windowDefinition)
	{
		au_assert(appContext != nullptr);

		Logger::AddSink<std_sink>();
		Logger::AddSink<file_sink>("latest-log.txt");

		glfwInit();

		// Init and create window
		m_Window = new GLFWWindow();
		m_Window->Initialize(windowDefinition, nullptr);
		m_Window->Show();

		// Instantiate OpenGL swap chain
		m_SwapChain = new GLSwapChain(((GLFWWindow*)m_Window)->GetHandle());

		// Init OpenGL device
#ifdef AU_TRACY_ENABLED
		TracyGpuContext
#endif
		m_RenderDevice = new GLRenderDevice();
		m_RenderDevice->Init();

		// Clear backbuffer (Linux will preset random data to screen if not cleared)
		{
			DrawCallState drawCallState;
			drawCallState.ClearColorTarget = true;
			drawCallState.ClearDepthTarget = true;
			drawCallState.ClearColor = Color::black();
			m_RenderDevice->ClearRenderTargets(drawCallState);
			m_SwapChain->Present(1);
		}

		// Init render manager
		m_RenderManager = new RenderManager(m_RenderDevice);

		// Init resource manager (for loading assets)
		m_ResourceManager = new ResourceManager(m_RenderDevice);
		m_ResourceManager->AddFileSearchPath(AURORA_PROJECT_DIR);

		{ // Init imgui, TODO: add define for it
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
			//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

			// Setup Dear ImGui style
			ImGui::StyleColorsDark();
			//ImGui::StyleColorsClassic();

			// Setup Platform/Renderer backends
			ImGui_ImplGlfw_InitForOpenGL(((GLFWWindow*)m_Window)->GetHandle(), true);
			ImGui_ImplOpenGL3_Init("#version 330 core");
		}

		// Init global context
		g_Context = new AuroraContext();
		g_Context->m_Window = m_Window;
		g_Context->m_InputManager = m_Window->GetInputManager().get();
		g_Context->m_RenderDevice = m_RenderDevice;
		g_Context->m_RenderManager = m_RenderManager;
		g_Context->m_ResourceManager = m_ResourceManager;

		vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES); //NVG_DEBUG
		nvgCreateFont(vg, "default", "../Assets/Fonts/GROBOLD.ttf");

		// Init App context
		m_AppContext = appContext;
		m_AppContext->Init();
	}

	void AuroraEngine::Run()
	{
		double lastTime = glfwGetTime();
		int frameRate = 0;
		double frameRateAcc = 0;

		int FPS = 0;

		while(!m_Window->IsShouldClose())
		{
			double currentTime = glfwGetTime();
			double frameTime = currentTime - lastTime;
			double delta = frameTime;

			{ // Calculate FPS
				frameRateAcc += frameTime;
				frameRate++;

				if(frameRateAcc >= 1.0)
				{
					double avgFrameTime = frameRateAcc / (double)(frameRate);
					double avgFrameTimeMs = avgFrameTime * 1000;

					std::stringstream ss;

					ss << m_Window->GetOriginalTitle();
					ss << " - " << frameRate << " fps, " << avgFrameTimeMs << "ms";

					m_Window->SetTitle(ss.str());

					FPS = frameRate;

					AU_LOG_INFO(ss.str());
					frameRateAcc = 0;
					frameRate = 0;
				}
			}

			// Update glfw events
			std::static_pointer_cast<Input::Manager>(m_Window->GetInputManager())->PrePollEvents();
			glfwPollEvents();
			std::static_pointer_cast<Input::Manager>(m_Window->GetInputManager())->Update(frameTime);

			{ // ImGui update
				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();
			}

			{
				CPU_DEBUG_SCOPE("Game update");
				m_AppContext->Update(delta);
			}

			{
				CPU_DEBUG_SCOPE("Game render");
				GPU_DEBUG_SCOPE("Game render");
				m_AppContext->Render();
			}

			{
				nvgBeginFrame(vg, m_Window->GetWidth(), m_Window->GetHeight(), 1.0f); // TODO: Fix hdpi devices

				/*nvgBeginPath(vg);
				nvgRect(vg, 100,100, 120,30);
				nvgFillColor(vg, nvgRGBA(255,192,0,255));
				nvgFill(vg);

				{
					nvgSave(vg);
					nvgBeginPath(vg);
					nvgTranslate(vg, 100 - 15, 100 - 16);
					nvgMoveTo(vg, 10, 17);
					nvgLineTo(vg, 13, 20);
					nvgLineTo(vg, 20, 13);
					nvgStrokeWidth(vg, 1.0f);
					nvgStrokeColor(vg, {1, 0, 1, 1});
					nvgStroke(vg);
					nvgRestore(vg);
				}*/

				{
					std::stringstream ss;
					ss << "FPS: " << FPS << std::endl;

					nvgFontSize(vg, 16);
					nvgFontFace(vg, "default");

					nvgFontBlur(vg, 0);
					nvgGlobalAlpha(vg, 1.0f);
					nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
					nvgFillColor(vg, nvgRGBAf(1, 1, 1, 1));
					nvgText(vg, 10, 10, ss.str().c_str(), nullptr);
				}

				nvgEndFrame(vg);
			}

			{
				CPU_DEBUG_SCOPE("ImGui render");
				GPU_DEBUG_SCOPE("ImGui render");
				ImGui::Render();
				ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			}

			g_Context->m_RenderDevice->InvalidateState();

			{
				m_RenderManager->EndFrame();
				CPU_DEBUG_SCOPE("Swap chain")
				m_SwapChain->Present(0);
			}

#ifdef AU_TRACY_ENABLED
			TracyGpuCollect
#endif

			lastTime = currentTime;
		}
	}
}