#pragma once

namespace Aurora
{
	class ResourceManager;
	class RenderManager;
	class IRenderDevice;
	class IWindow;
	class RmlUI;
	class VgRender;
	class PhysicsWorld;

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
		RmlUI* m_RmlUI;
		VgRender* m_VgRender;
		PhysicsWorld* m_PhysicsWorld;
	public:
		[[nodiscard]] inline ResourceManager* GetResourceManager() const { return m_ResourceManager; }
		[[nodiscard]] inline RenderManager* GetRenderManager() const { return m_RenderManager; }
		[[nodiscard]] inline IRenderDevice* GetRenderDevice() const { return m_RenderDevice; }
		[[nodiscard]] inline IWindow* GetWindow() const { return m_Window; }
		[[nodiscard]] inline Input::IManager* GetInputManager() const { return m_InputManager; }
		[[nodiscard]] inline RmlUI* GetRmlUI() const { return m_RmlUI; }
		[[nodiscard]] inline VgRender* GetVgRender() const { return m_VgRender; }
		[[nodiscard]] inline PhysicsWorld* GetPhysicsWorld() const { return m_PhysicsWorld; }
	};

	extern AuroraContext* GetEngine();
}