#pragma once

#include "Aurora/App/IWindow.hpp"
#include "Engine.hpp"

namespace Aurora
{
	class IRenderDevice;
	class RenderManager;
	class ResourceManager;

	namespace Input
	{
		class IManager;
	}

	class AppContext
	{
	public:
		virtual ~AppContext() = default;

		virtual void Init() {}
		virtual void Update(double delta) {}
		virtual void Render() {}
	};

	class AuroraEngine
	{
	private:
		IWindow* m_Window;
		ISwapChain* m_SwapChain;
		IRenderDevice* m_RenderDevice;
		RenderManager* m_RenderManager;
		ResourceManager* m_ResourceManager;
		Input::IManager* m_InputManager;

		AppContext* m_AppContext;
	public:
		AuroraEngine();
		~AuroraEngine();

		void Init(AppContext* appContext, WindowDefinition& windowDefinition);
		void Run();
	};
}
