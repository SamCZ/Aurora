#pragma once

namespace Aurora
{
	class ResourceManager;
	class RenderManager;
	class IRenderDevice;
	class IWindow;

	namespace Input
	{
		class IManager;
	}

	class AuroraContext
	{
		friend class AuroraEngine;
	private:
		ResourceManager* m_ResourceManager;
		RenderManager* m_RenderManager;
		IRenderDevice* m_RenderDevice;
		IWindow* m_Window;
		Input::IManager* m_InputManager;
	public:
		[[nodiscard]] inline ResourceManager* GetResourceManager() const { return m_ResourceManager; }
		[[nodiscard]] inline RenderManager* GetRenderManager() const { return m_RenderManager; }
		[[nodiscard]] inline IRenderDevice* GetRenderDevice() const { return m_RenderDevice; }
		[[nodiscard]] inline IWindow* GetWindow() const { return m_Window; }
		[[nodiscard]] inline Input::IManager* GetInputManager() const { return m_InputManager; }
	};

	extern AuroraContext* GetEngine();
}