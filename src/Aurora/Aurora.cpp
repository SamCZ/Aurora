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

	AuroraEngine::~AuroraEngine()
	{
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

		// Init global context
		g_Context = new AuroraContext();
		g_Context->m_Window = m_Window;
		g_Context->m_RenderDevice = m_RenderDevice;
		g_Context->m_RenderManager = m_RenderManager;
		g_Context->m_ResourceManager = m_ResourceManager;

		// Init App context
		m_AppContext = appContext;
		m_AppContext->Init();
	}

	void AuroraEngine::Run()
	{
		double lastTime = glfwGetTime();
		int frameRate = 0;
		double frameRateAcc = 0;
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

					AU_LOG_INFO(ss.str());
					frameRateAcc = 0;
					frameRate = 0;
				}
			}

			// Update glfw events
			std::static_pointer_cast<Input::Manager>(m_Window->GetInputManager())->PrePollEvents();
			glfwPollEvents();
			std::static_pointer_cast<Input::Manager>(m_Window->GetInputManager())->Update(frameTime);

			{
				CPU_DEBUG_SCOPE("Game update")
				m_AppContext->Update(delta);
			}

			{
				CPU_DEBUG_SCOPE("Game render")
				m_AppContext->Render();
			}

			{
				m_RenderManager->EndFrame();
				CPU_DEBUG_SCOPE("Swap chain")
				m_SwapChain->Present(1);
			}

#ifdef AU_TRACY_ENABLED
			TracyGpuCollect
#endif

			lastTime = currentTime;
		}
	}
}