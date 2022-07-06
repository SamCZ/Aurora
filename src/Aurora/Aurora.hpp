#pragma once

#include "App/ISystemWindow.hpp"
#include "App/AppContext.hpp"
#include "Engine.hpp"

namespace Aurora
{
	class IRenderDevice;
	class RenderManager;
	class ResourceManager;
	class ViewPortManager;
	struct RenderViewPort;
	class MainEditorPanel;

	namespace Input
	{
		class IManager;
	}

	class AU_API AuroraEngine
	{
	private:
		ISystemWindow* m_Window;
		ISwapChain* m_SwapChain;
		IRenderDevice* m_RenderDevice;
		RenderManager* m_RenderManager;
		ResourceManager* m_ResourceManager;
		Input::IManager* m_InputManager;
		RmlUI* m_RmlUI;
		VgRender* m_VgRender;
		ViewPortManager* m_ViewPortManager;

		AppContext* m_AppContext;

		RenderViewPort* m_RenderViewPort;
		MainEditorPanel* m_EditorPanel;

		bool m_HasGraphicsDebug = false;
	public:
		AuroraEngine();
		~AuroraEngine();

		void Init(AppContext* appContext, WindowDefinition& windowDefinition, bool editor);
		void Run();
	};
}
