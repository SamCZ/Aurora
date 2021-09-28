#pragma once

#if GLFW_ENABLED

#include "../Base/SwapChain.hpp"

#include <GLFW/glfw3.h>

namespace Aurora
{
	AU_CLASS(GLSwapChain) : public ISwapChain
	{
	private:
		GLFWwindow* m_Window;
		int m_LastSyncInterval;
	public:
		inline explicit GLSwapChain(GLFWwindow* window) : m_Window(window), m_LastSyncInterval(-1)
		{

		}

		~GLSwapChain() override = default;

		void Present(int syncInterval) override
		{
			if(m_LastSyncInterval != syncInterval) {
				m_LastSyncInterval = syncInterval;
				glfwSwapInterval(syncInterval);
			}

			glfwSwapBuffers(m_Window);
		}

		void Resize(uint32_t width, uint32_t height) override
		{

		}

		const SwapChainDesc& GetDesc() const noexcept override
		{
			static SwapChainDesc desc;
			return desc;
		}
	};
}
#endif