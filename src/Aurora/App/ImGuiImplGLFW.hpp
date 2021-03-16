#pragma once

#include <memory>
#include <ImGuiImplDiligent.hpp>
#include <GLFW/glfw3.h>

using namespace Diligent;

namespace Aurora
{
	class ImGuiImplGLFW final : public ImGuiImplDiligent
	{
	public:
		ImGuiImplGLFW(GLFWwindow* glfWwindow,
		IRenderDevice* pDevice,
				TEXTURE_FORMAT BackBufferFmt,
		TEXTURE_FORMAT DepthBufferFmt,
				Uint32         InitialVertexBufferSize = ImGuiImplDiligent::DefaultInitialVBSize,
				Uint32         InitialIndexBufferSize  = ImGuiImplDiligent::DefaultInitialIBSize);
		~ImGuiImplGLFW();

		// clang-format off
		ImGuiImplGLFW             (const ImGuiImplGLFW&)  = delete;
		ImGuiImplGLFW             (      ImGuiImplGLFW&&) = delete;
		ImGuiImplGLFW& operator = (const ImGuiImplGLFW&)  = delete;
		ImGuiImplGLFW& operator = (      ImGuiImplGLFW&&) = delete;
		// clang-format on

		virtual void NewFrame(Uint32 RenderSurfaceWidth, Uint32 RenderSurfaceHeight, SURFACE_TRANSFORM SurfacePreTransform) override final;
	};
}
