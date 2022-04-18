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
#include "Graphics/ViewPortManager.hpp"
#include "Graphics/DShape.hpp"
#include "Resource/ResourceManager.hpp"
#include "Framework/CameraComponent.hpp"

#include "RmlUI/RmlUI.hpp"

#include "Render/VgRender.hpp"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include "Tools/IconsFontAwesome5.hpp"

#include <TracyOpenGL.hpp>

#include "Physics/PhysicsWorld.hpp"
#include "Editor/MainEditorPanel.hpp"
#include "Aurora/Core/Timer.hpp"

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
	GameModeBase* AppContext::m_GameModeToSwitch = nullptr;
	EventEmitter<Scene*> AppContext::m_SceneChangeEmitter;
	bool AppContext::m_EditorMode = false;

	ImFont* m_ImGuiDefaultFont = nullptr;

	AuroraEngine::AuroraEngine() :
		m_Window(),
		m_SwapChain(nullptr),
		m_RenderDevice(nullptr),
		m_RenderManager(nullptr),
		m_ResourceManager(nullptr),
		m_InputManager(nullptr),
		m_AppContext(nullptr),
		m_RmlUI(nullptr),
		m_ViewPortManager(nullptr),
		m_VgRender(nullptr),
		m_EditorPanel(nullptr),
		m_RenderViewPort(nullptr)
#ifdef NEWTON
        ,m_PhysicsWorld(nullptr)
#endif
	{
		Logger::AddSink<std_sink>();
		Logger::AddSink<file_sink>("latest-log.txt");
	}

	AuroraEngine::~AuroraEngine()
	{
		DShapes::Destroy();
		delete Aurora::AppContext::m_GameMode; // Needs to be deleted here because of destroy order
		delete m_AppContext;
		delete m_EditorPanel;
#ifdef NEWTON
		delete m_PhysicsWorld;
#endif
		delete m_VgRender;
		delete m_RmlUI;
		delete m_ResourceManager;
		delete m_RenderManager;
		delete m_ViewPortManager;
		delete m_RenderDevice;
		delete m_SwapChain;
		delete m_Window;
		delete GEngine;
		glfwTerminate();
	}

    void GLFWErrorCallback(int errorCode, const char* description)
    {
        AU_LOG_FATAL("GLFW Error ", errorCode, ": ", description);
    }

	void AuroraEngine::Init(AppContext* appContext, WindowDefinition& windowDefinition, bool editor)
	{
		au_assert(appContext != nullptr);

		AppContext::m_EditorMode = editor;

		LocalProfileScope::Reset("GameInit");

		if(!glfwInit())
        {
            AU_LOG_FATAL("Could not initialize GLFW!");
        }

        glfwSetErrorCallback(GLFWErrorCallback);

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

			static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 }; // Will not be copied by AddFont* so keep in scope.

			auto fontData = new std::vector<uint8>(m_ResourceManager->LoadFile("Assets/Fonts/LatoLatin-Bold.ttf"));
			io.Fonts->AddFontFromMemoryTTF(fontData->data(), fontData->size(), 15);

			ImFontConfig config;
			config.MergeMode = true;
			//config.PixelSnapH = true;
			auto iconFontData = new std::vector<uint8>(m_ResourceManager->LoadFile("Assets/Fonts/fa-solid-900.ttf"));
			io.Fonts->AddFontFromMemoryTTF(iconFontData->data(), iconFontData->size(), 15, &config, icons_ranges);
			io.Fonts->Build();

			io.IniFilename = AURORA_PROJECT_DIR "/Assets/imgui.ini";
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
			//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

			ImGui::StyleColorsDark();

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

		m_ViewPortManager = new ViewPortManager();
		GEngine->m_ViewPortManager = m_ViewPortManager;

		m_RenderViewPort = GEngine->GetViewPortManager()->Create(0, GraphicsFormat::SRGBA8_UNORM);

		m_RenderViewPort->ResizeEmitter.Bind([](const glm::ivec2& coord) ->void
		{
			if(GEngine->GetRmlUI() == nullptr || GEngine->GetRmlUI()->GetRmlContext() == nullptr) return;
			GEngine->GetRmlUI()->GetRmlContext()->SetDimensions({coord.x, coord.y});
		});

		if(editor)
		{
			m_EditorPanel = new MainEditorPanel();
		}
		else
		{
			m_Window->AddResizeListener([this](int w, int h) -> void
			{
				m_RenderViewPort->Resize({w, h});
			});
			m_RenderViewPort->Resize(m_Window->GetSize());
		}

		// Init App context
		m_AppContext = appContext;
		m_AppContext->Init();

		DShapes::Init();
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
					ss << " - " << frameRate << " fps, " << (frameTime * 1000) << "ms, avg=" << avgFrameTimeMs;

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

			// IMPORTANT THIS WILL UPDATE SWITCHING GAME MODE
			if(AppContext::m_GameModeToSwitch != nullptr)
			{
				AppContext::SwitchGameModeRaw(AppContext::m_GameModeToSwitch);
				AppContext::m_GameModeToSwitch = nullptr;
			}

			// Update resource only when focused and once per second
			if (m_Window->IsFocused())
			{
				static TickTimer tickTimer(1);

				if (tickTimer)
				{
					CPU_DEBUG_SCOPE("ResourceManagerUpdate");
					m_ResourceManager->Update();
				}
			}

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
				bool updateScene = true;

				if(m_EditorPanel)
				{
					updateScene = m_EditorPanel->IsPlayMode();
				}

				CPU_DEBUG_SCOPE("Game update");
				Scene* currentScene = AppContext::GetScene();
				if(currentScene && updateScene)
				{
					currentScene->Update(delta);
				}
				m_AppContext->InternalUpdate(delta);
			}

			{
				CPU_DEBUG_SCOPE("RmlUI update");
				m_RmlUI->Update();
			}

			if (m_EditorPanel)
			{
				CPU_DEBUG_SCOPE("Editor update");
				m_EditorPanel->Update(delta);
			}

			{
				CPU_DEBUG_SCOPE("Game render");
				GPU_DEBUG_SCOPE("Game render");
				//m_RenderDevice->ClearTextureUInt(m_RenderViewPort->Target, 0);
				m_AppContext->Render();
			}



			// Disabled vg render for now
			if(true)
			{
				CPU_DEBUG_SCOPE("NanoVG");
				GPU_DEBUG_SCOPE("NanoVG");

				// This fixed fonts not rendering
				glBindSampler(0, 0);

				m_VgRender->Begin(m_Window->GetSize(), 1.0f); // TODO: Fix hdpi devices
				m_AppContext->RenderVg();

				for(auto* camera : AppContext::GetScene()->GetComponents<CameraComponent>())
				{
					if(camera->IsActive())
					{
						DShapes::RenderText(camera);
						break;
					}
				}

				/*{
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
				}*/

				m_VgRender->End();
			}

			{
				CPU_DEBUG_SCOPE("RmlUI render");
				GPU_DEBUG_SCOPE("RmlUI render");

				RenderViewPort* rwp = m_ViewPortManager->Get();

				DrawCallState drawCallState;
				drawCallState.BindTarget(0, rwp->Target);
				drawCallState.ViewPort = rwp->ViewPort;
				m_RenderDevice->BindRenderTargets(drawCallState);

				m_RmlUI->Render(drawCallState);
			}

			if(!m_EditorPanel)
			{
				glEnable(GL_FRAMEBUFFER_SRGB);
				m_RenderManager->Blit(m_RenderViewPort->Target);
				glDisable(GL_FRAMEBUFFER_SRGB);
			}

			m_RenderDevice->SetViewPort(FViewPort(m_Window->GetSize()));

			{
				CPU_DEBUG_SCOPE("ImGui render");
				GPU_DEBUG_SCOPE("ImGui render");

				DrawCallState drawCallState;
				drawCallState.ViewPort = m_Window->GetSize();
				m_RenderDevice->BindRenderTargets(drawCallState);

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
			DShapes::Reset();

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