#include "Aurora.hpp"

#if _WIN32
#include <Windows.h>
#endif
#include <GLFW/glfw3.h>

#include "Logger/file_sink.hpp"
#include "Logger/std_sink.hpp"

#include "Core/assert.hpp"
#include "Core/Profiler.hpp"

#include "App/GLFWWindow.hpp"
#include "App/Input/GLFW/Manager.hpp"

#include "Graphics/OpenGL/GLSwapChain.hpp"
#include "Graphics/OpenGL/GLRenderDevice.hpp"
#include "Graphics/RenderManager.hpp"
#include "Resource/ResourceManager.hpp"

#include "RmlUI/RmlUI.hpp"

#include "Render/VgRender.hpp"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <TracyOpenGL.hpp>

#include "Physics/PhysicsWorld.hpp"

#undef DrawText

#ifdef _WIN32
#include <Windows.h>
extern "C"
{
/* http://developer.amd.com/community/blog/2015/10/02/amd-enduro-system-for-developers/ */
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
/* http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf */
__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
#endif

namespace Aurora
{
	AuroraContext* GEngine = nullptr;

	GameContext* AppContext::m_GameContext = nullptr;
	GameModeBase* AppContext::m_GameMode = nullptr;

	AuroraEngine::AuroraEngine() :
		m_Window(),
		m_SwapChain(nullptr),
		m_RenderDevice(nullptr),
		m_RenderManager(nullptr),
		m_ResourceManager(nullptr),
		m_InputManager(nullptr),
		m_AppContext(nullptr),
		m_RmlUI(nullptr),
		m_VgRender(nullptr)
#ifdef NEWTON
        ,m_PhysicsWorld(nullptr)
#endif
	{

	}

	AuroraEngine::~AuroraEngine()
	{
		delete m_AppContext;
#ifdef NEWTON
		delete m_PhysicsWorld;
#endif
		delete GEngine;
		delete m_VgRender;
		delete m_RmlUI;
		delete m_ResourceManager;
		delete m_RenderManager;
		delete m_RenderDevice;
		delete m_SwapChain;
		delete m_Window;
		glfwTerminate();
	}

	void AuroraEngine::Init(AppContext* appContext, WindowDefinition& windowDefinition, bool editor)
	{
		au_assert(appContext != nullptr);

		Logger::AddSink<std_sink>();
		Logger::AddSink<file_sink>("latest-log.txt");

		LocalProfileScope::Reset("GameInit");

		glfwInit();

		GEngine = new AuroraContext();
		GEngine->m_AppContext = appContext;

		// Init and create window
		m_Window = new GLFWWindow();
		m_Window->Initialize(windowDefinition, nullptr);
		m_Window->Show();

		// Instantiate OpenGL swap chain
		m_SwapChain = new GLSwapChain(((GLFWWindow*)m_Window)->GetHandle());

		// Init OpenGL device
#if AU_TRACY_ENABLED
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
#if AU_IN_PROJECT_ASSETS
		m_ResourceManager->AddFileSearchPath(AURORA_PROJECT_DIR);
#endif

		{ // Init imgui, TODO: add define for it
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			io.IniFilename = "../../imgui.ini";
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
			//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
			//io.ConfigViewportsNoAutoMerge = true;
			//io.ConfigViewportsNoTaskBarIcon = true;

			// Setup Dear ImGui style
			ImGui::StyleColorsDark();
			//ImGui::StyleColorsClassic();

			// Setup Platform/Renderer backends
			ImGui_ImplGlfw_InitForOpenGL(((GLFWWindow*)m_Window)->GetHandle(), true);
			ImGui_ImplOpenGL3_Init("#version 330 core");
		}

		// Init global context
		GEngine->m_Window = m_Window;
		GEngine->m_InputManager = m_Window->GetInputManager().get();
		GEngine->m_RenderDevice = m_RenderDevice;
		GEngine->m_RenderManager = m_RenderManager;
		GEngine->m_ResourceManager = m_ResourceManager;

		// Init RmlUI
		m_RmlUI = new RmlUI("RmlContext");
		GEngine->m_RmlUI = m_RmlUI;

		// Init NanoVG render
		m_VgRender = new VgRender();
		GEngine->m_VgRender = m_VgRender;
		m_VgRender->LoadFont("default", "Assets/Fonts/LatoLatin-Bold.ttf");

#ifdef NEWTON
		// Init Physics world
		m_PhysicsWorld = new PhysicsWorld();
		g_Context->m_PhysicsWorld = m_PhysicsWorld;
#endif

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
			LocalProfileScope::Reset("GameFrame");
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
				CPU_DEBUG_SCOPE("ImGui update");
				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();
			}

#ifdef NEWTON
			{
				CPU_DEBUG_SCOPE("Physics update");
				m_PhysicsWorld->Update(delta);
			}
#endif

			{
				CPU_DEBUG_SCOPE("Game update");
				m_AppContext->InternalUpdate(delta);
			}

			{
				CPU_DEBUG_SCOPE("RmlUI update");
				m_RmlUI->Update();
			}

			{
				CPU_DEBUG_SCOPE("Game render");
				GPU_DEBUG_SCOPE("Game render");
				m_AppContext->Render();
			}

			m_RenderDevice->SetViewPort(FViewPort(m_Window->GetSize()));

			{
				CPU_DEBUG_SCOPE("NanoVG");
				GPU_DEBUG_SCOPE("NanoVG");

				m_VgRender->Begin(m_Window->GetSize(), 1.0f); // TODO: Fix hdpi devices

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

				m_AppContext->RenderVg();

				{
					std::stringstream ss;
					ss << "FPS: " << FPS;
					m_VgRender->DrawString(ss.str(), {5, 55}, Color::black(), 12);
				}

				{


					const FrameRenderStatistics& renderStatistics = m_RenderDevice->GetFrameRenderStatistics();

					{
						std::stringstream ss;
						ss << "Draw calls: " << renderStatistics.DrawCalls;
						m_VgRender->DrawString(ss.str(), {5, 75}, Color::black(), 12);
					}

					{
						std::stringstream ss;
						ss << "Vertex Count: " << renderStatistics.VertexCount;
						m_VgRender->DrawString(ss.str(), {5, 85}, Color::black(), 12);
					}

					{
						std::stringstream ss;
						ss << "Buffer Writes: " << renderStatistics.BufferWrites;
						m_VgRender->DrawString(ss.str(), {5, 95}, Color::black(), 12);
					}

					{
						std::stringstream ss;
						ss << "Buffer Maps: " << renderStatistics.BufferMaps;
						m_VgRender->DrawString(ss.str(), {5, 105}, Color::black(), 12);
					}

					{
						std::stringstream ss;
						ss << "Memory usage: " << FormatBytes(renderStatistics.GPUMemoryUsage);
						m_VgRender->DrawString(ss.str(), {5, 115}, Color::black(), 12);
					}
				}

				m_VgRender->End();
			}

			{
				CPU_DEBUG_SCOPE("RmlUI render");
				GPU_DEBUG_SCOPE("RmlUI render");
				m_RmlUI->Render();
			}

			{
				CPU_DEBUG_SCOPE("ImGui render");
				GPU_DEBUG_SCOPE("ImGui render");
				ImGui::Render();
				ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

				// Update and Render additional Platform Windows
				// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
				//  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
				if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
				{
					GLFWwindow* backup_current_context = glfwGetCurrentContext();
					ImGui::UpdatePlatformWindows();
					ImGui::RenderPlatformWindowsDefault();
					glfwMakeContextCurrent(backup_current_context);
				}
			}

			GEngine->m_RenderDevice->InvalidateState();

			{
				m_RenderManager->EndFrame();
				m_RenderDevice->ResetFrameRenderStatistics();
				CPU_DEBUG_SCOPE("Swap chain");
				m_SwapChain->Present(0);
			}

#if AU_TRACY_ENABLED
			TracyGpuCollect
#endif

			lastTime = currentTime;
		}
	}
}